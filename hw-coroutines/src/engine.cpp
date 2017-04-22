#include <coroutine/engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Coroutine {

void Engine::Store(context& ctx) {
    char stackStart;

    ctx.Low = this->StackBottom;
    ctx.Hight = &stackStart;
    uint32_t size = ctx.Low - ctx.Hight;

  /*  printf("Store:\n");
    printf("Low = %p\nHight = %p\n", ctx.Low, ctx.Hight);
    printf("Low %s Hight\n", ctx.Low < ctx.Hight ? "<" : ">");
    printf("Size = %u\n", size);*/

    if (std::get<0>(ctx.Stack) != nullptr) {
        delete std::get<0>(ctx.Stack);
    }

    std::get<0>(ctx.Stack) = new char[size];
    std::get<1>(ctx.Stack) = size;

    memcpy(std::get<0>(ctx.Stack), ctx.Hight, size);
    //printf("Stack saved\n");

    /*if (cur_routine == nullptr) {
        cur_routine = &ctx;
    }*/
}

void Engine::Restore(context& ctx) {
    // TODO: implements
    char stackStart;
    char *stackAddr = &stackStart;

    //printf("from Restore()\n");
    //printf("Low = %p   Hight = %p    Addr = %p\n", ctx.Low, ctx.Hight, stackAddr);

    if (ctx.Low <= stackAddr && stackAddr <= ctx.Hight ||
        ctx.Hight <= stackAddr && stackAddr <= ctx.Low) {
            Restore(ctx);
    }

    memcpy(ctx.Hight, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));

    //printf("LONGJMP!\n");

    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    // TODO: implements
    // setjmp, longjmp...
}
int i =0;
void Engine::sched(void* routine) {
    // TODO: implements
    // setjmp, longjmp...
    context *ctx = (context *) routine;
    if (i ++ > 5)
      exit(0);

    printf("sched(): cur_routine = %p\n", cur_routine);
    printf("sched(): ctx = %p\n", ctx);

    if (!ctx->was_invoked) {
        Restore(*ctx);
    }
    else {
        // point for longjmp
        int s = setjmp(cur_routine->Environment);
        if (s == 0) {
              Store(*cur_routine);
              printf("do longjmp\n");
              Restore(*ctx);   // longjmp here
        }
    }

    cur_routine = ctx;

    // continue coroutine execution after sched()
}

} // namespace Coroutine
