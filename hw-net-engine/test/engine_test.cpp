#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>

#include <coroutine/channel.h>
#include <coroutine/engine.h>
#include <coroutine/net-engine.h>

void _calculator_add(int& result, int left, int right) {
    result = left + right;
}

TEST(CoroutineTest, SimpleStart) {
    Coroutine::Engine engine;

    int result;
    engine.start(_calculator_add, result, 1, 2);

    ASSERT_EQ(3, result);
}

void printa(Coroutine::Engine& pe, std::stringstream& out, void*& other) {
    out << "A1 ";
    std::cout << "A1 " << std::endl;
    pe.sched(other);

    out << "A2 ";
    std::cout << "A2 " << std::endl;
    pe.sched(other);

    out << "A3 ";
    std::cout << "A3 " << std::endl;
    pe.sched(other);
}

void printb(Coroutine::Engine& pe, std::stringstream& out, void*& other) {
    out << "B1 ";
    std::cout << "B1 " << std::endl;
    pe.sched(other);

    out << "B2 ";
    std::cout << "B2 " << std::endl;
    pe.sched(other);

    out << "B3 ";
    std::cout << "B3 " << std::endl;
}

std::stringstream out;
void *pa = nullptr, *pb = nullptr;
void _printer(Coroutine::Engine& pe, std::string& result) {
    // Create routines, note it doens't get control yet
    pa = pe.run(printa, pe, out, pb);
    pb = pe.run(printb, pe, out, pa);

    // Pass control to first routine, it will ping pong
    // between printa/printb greedely then we will get
    // control back
    pe.sched(pa);

    out << "END";
    std::cout << "END" << std::endl;

    // done
    result = out.str();
}

TEST(CoroutineTest, Printer) {
    Coroutine::Engine engine;

    std::string result;
    engine.start(_printer, engine, result);
    ASSERT_STREQ("A1 B1 A2 B2 A3 B3 END", result.c_str());
}

TEST(CoroutineTest, Channel) {
    Coroutine::Channel c = Coroutine::Channel(10);
    char buf[] = "HelloWorldSphere!";
    char out[20];
    size_t nbytes = strlen(buf);
    size_t p1 = 0, p2 = 0;

    while (p2 < strlen(buf)) {
        int nw = c.write(buf+p1, nbytes);
        p1 += nw;
        nbytes -= nw;

        int nr = c.read(out+p2, 5);
        p2 += nr;
    }
    out[p2] = '\0';

    ASSERT_STREQ(out, "HelloWorldSphere!");
    ASSERT_STREQ(buf, "HelloWorldSphere!");
}


TEST(CoroutineTest, CreateChannel) {
    Coroutine::NetEngine net_engine;
    std::ostringstream out;

    for (int i = 0; i < 5; i++) {
        int chd = net_engine.create_channel();
        out << chd << " ";
    }

    net_engine.close_channel(2);

    out << net_engine.create_channel();

    // test for the smallest possible unique desciptor
    ASSERT_STREQ("0 1 2 3 4 2", out.str().c_str());
}

/*************************** SimpleMessages test ******************************/

void print_msg(char* bytes, size_t len)
{
    for (size_t i = 0; i < len; i++)
        std::cout << bytes[i];
    std::cout << std::endl;
}

void do_send(Coroutine::NetEngine& ne, void*& other) {
    char send_buf[] = "Hello, do_recv coroutine!";  // 25 bytes
    size_t len = strlen(send_buf);

    int chd = ne.create_channel(1);
    ne.send_channel(chd, &len, 1);   // send future message length first
    ne.send_channel(chd, send_buf, len);

    ne.sched(other);

    char recv_buf[100];
    ne.recv_channel(chd, &len, 1);
    ne.recv_channel(chd, recv_buf, len);
    print_msg(recv_buf, len);
}

void do_recv(Coroutine::NetEngine& ne, void*& other) {
    char recv_buf[100];
    size_t len = 0;

    int chd = ne.create_channel(1);

    ne.recv_channel(chd, &len, 1);
    ne.recv_channel(chd, recv_buf, len);

    print_msg(recv_buf, len);

    char answer[] = "Answer: Hi, do_send coroutine!";
    len = strlen(answer);
    ne.send_channel(chd, &len, 1);
    ne.send_channel(chd, answer, len);
}

void _msgtest_main(Coroutine::NetEngine& ne) {
    void *s = nullptr, *r = nullptr;
    s = ne.run(do_send, ne, r);
    r = ne.run(do_recv, ne, s);

    // Pass control to recv routine (to observe blocking)
    ne.sched(r);
}

TEST(CoroutineTest, SimpleMessages) {
    Coroutine::NetEngine engine;

    engine.start(_msgtest_main, engine);
}

/**************************** Ping Pang Pong Test *****************************/
void _p_x_ng(Coroutine::NetEngine& ne, int& in_channel, int& out_channel,
             std::string& name, int& max_count) {
    int count;

    while (count < max_count) {
        ne.recv_channel(in_channel, &count, sizeof(int));
        std::cout << name << std::endl;
        count++;
        ne.send_channel(out_channel, &count, sizeof(int));
    }
}

void _ppp_main(Coroutine::NetEngine& ne) {
    void *p1 = nullptr, *p2 = nullptr, *p3 = nullptr;
    int count = 0, max_count = 9;

    int c1 = ne.create_channel();
    int c2 = ne.create_channel();
    int c3 = ne.create_channel();

    ne.send_channel(c1, &count, sizeof(int));

    std::string names[] = {"ping", "pang", "pong"};

    p1 = ne.run(_p_x_ng, ne, c1, c2, names[0], max_count);
    p2 = ne.run(_p_x_ng, ne, c2, c3, names[1], max_count);
    p3 = ne.run(_p_x_ng, ne, c3, c1, names[2], max_count);

    ne.sched(p1);
}

TEST(CoroutineTest, PingPangPong) {
    Coroutine::NetEngine engine;

    engine.start(_ppp_main, engine);
}

/******************************* Gradient descent *****************************/
/*
// z = x^2 + y^2
double f(double x, double y) {
    return x*x + y*y;
}

void df(double x, double y, double* dfdx, double* dfdy) {
    *dfdx = 2*x;
    *dfdy = 2*y;
}

void _grad_step(Coroutine::NetEngine& ne, int& channel, int& answer_channel) {
    double point[2];
    double grad[2];
    double tol = 1e-5;

    while (true) {
        ne.recv_channel(channel, point, sizeof(point));
        std::cout << "Point (x,y):" << point[0] << " " << point[1] << std::endl;
        double x = point[0], y = point[1];
        df(x, y, &grad[0], &grad[1]);
        point[0] = x - 0.2*grad[0];
        point[1] = y - 0.2*grad[1];

        double diff = fabs(fabs(f(x, y)) - fabs(f(point[0], point[1])));
        if (diff >= tol) {
            ne.send_channel(channel, point, sizeof(point));
        }
        else {
            ne.send_channel(answer_channel, point, sizeof(point));
        }
    }
}


void _optimize(Coroutine::NetEngine& ne) {
    void* p1 = nullptr;

    double guess[] = {2.0, 2.0};

    int chan = ne.create_channel();
    int ans_chan = ne.create_channel();

    ne.send_channel(chan, guess, sizeof(guess));

    p1 = ne.run(_grad_step, ne, chan, ans_chan);

    ne.sched(p1);

    std::cout << "start wait" << std::endl;
    //ne.recv_channel(ans_chan, guess, sizeof(guess));
    //std::cout << "Optimal point (x,y) = (" << guess[0] << ", " << guess[1] << ")" << std::endl;
}

TEST(CoroutineTest, GradientDescent) {
    Coroutine::NetEngine engine;

    engine.start(_optimize, engine);
}*/
