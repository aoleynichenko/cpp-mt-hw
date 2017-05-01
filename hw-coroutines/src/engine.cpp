#include <coroutine/engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Coroutine {

void Engine::Store(context& ctx) {
    char stackStart;

    ctx.Low = ctx.Hight = this->StackBottom;
    // ctx.Hight = &stackStart;
    // uint32_t size = ctx.Low - ctx.Hight;
    if (stackStart > ctx.Low) {
      ctx.Hight = stackStart;
    } else {
      ctx.Low = stackStart;
    }

  /*  printf("Store:\n");
    printf("Low = %p\nHight = %p\n", ctx.Low, ctx.Hight);
    printf("Low %s Hight\n", ctx.Low < ctx.Hight ? "<" : ">");
    printf("Size = %u\n", size);*/

    int size = ctx.Hight - ctx.Low;
    if (std::get<1>(ctx.Stack) < size) {
        delete std::get<0>(ctx.Stack);
        std::get<0>(ctx.Stack) = new char[size];
        std::get<1>(ctx.Stack) = size;
  }


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

    if (ctx.Low <= stackAddr && stackAddr <= ctx.Hight) {
            Restore(ctx);
    }

    memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    // TODO: implements
    // setjmp, longjmp...
}

void Engine::sched(void* routine) {
    // TODO: implements
    // setjmp, longjmp...
    if (cur_routine != nullptr) {
      if (setjmp(cur_routine->Env) != 0) {
        return;
      }
      Store(cur_routine)
    }

    if (routine->callee != nullptr && routine->callee == cur_routine) {
      routine->callee = routine->callee->caller = nullptr;
    }

    while (routine->callee != nullptr) {
      routine = routine->callee;
    }

    cur_routine = routine;
    Restore(routine);

    // context *ctx = (context *) routine;
    // if (i ++ > 10)
    //   exit(0);
    //
    // printf("sched(): cur_routine = %p\n", cur_routine);
    // printf("sched(): ctx = %p\n", ctx);
    //
    // if (!ctx->was_invoked) {
    //     cur_routine = ctx;
    //     Restore(*ctx);
    // }
    // else {
    //     // point for longjmp
    //     int s = setjmp(cur_routine->Environment);
    //     if (s == 0) {
    //           Store(*cur_routine);
    //           printf("do longjmp\n");
    //           cur_routine = ctx;
    //           Restore(*ctx);   // longjmp here
    //     }
    //     // longjumped here
    //     printf("jumped to continue place\n");
    //
    //     /*else {
    //         // continue execution
    //         printf("jumped to continue place\n");
    //
    //         Store(*cur_routine);
    //         cur_routine = ctx;
    //         Restore(*ctx);   // longjmp here
    //     }*/
    // }
    //
    // cur_routine = ctx;
    //
    // continue coroutine execution after sched()
}

} // namespace Coroutine
