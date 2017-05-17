#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <string.h>

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
