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

    printf("Store:\n");
    printf("Low = %p\nHight = %p\n", ctx.Low, ctx.Hight);
    printf("Low %s Hight\n", ctx.Low < ctx.Hight ? "<" : ">");
    printf("Size = %u\n", size);

    if (std::get<0>(ctx.Stack) != nullptr) {
        delete std::get<0>(ctx.Stack);
    }

    std::get<0>(ctx.Stack) = new char[size];
    std::get<1>(ctx.Stack) = size;

    memcpy(std::get<0>(ctx.Stack), ctx.Hight, size);
    printf("Stack saved\n");

    /*if (cur_routine == nullptr) {
        cur_routine = &ctx;
    }*/
}

void Engine::Restore(context& ctx) {
    // TODO: implements
    char stackStart;
    char *stackAddr = &stackStart;

    printf("from Restore()\n");
    printf("Low = %p   Hight = %p    Addr = %p\n", ctx.Low, ctx.Hight, stackAddr);

    if (ctx.Low <= stackAddr && stackAddr <= ctx.Hight ||
        ctx.Hight <= stackAddr && stackAddr <= ctx.Low) {
            Restore(ctx);
    }

    memcpy(ctx.Hight, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));

    printf("LONGJUMP!\n");

    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    // TODO: implements
    // setjmp, longjmp...
}

void Engine::sched(void* routine) {
    // TODO: implements
    // setjmp, longjmp...
    context *ctx = (context *) routine;

    if (cur_routine == nullptr) {
        cur_routine = ctx;
    }

    printf("sched(): cur_routine = %p\n", cur_routine);
    printf("sched(): ctx = %p\n", ctx);

    //Store(*cur_routine);

    printf("Stack stored\nNow Restore()\n");

    Restore(*ctx);

}

} // namespace Coroutine
