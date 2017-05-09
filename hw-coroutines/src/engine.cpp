#include <coroutine/engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Coroutine {

void Engine::Store(context& ctx) {
    char stackStart;

    ctx.Low = ctx.Hight = this->StackBottom;
    if (&stackStart > ctx.Low) {
        ctx.Hight = &stackStart;
    } else {
        ctx.Low = &stackStart;
    }

    int size = ctx.Hight - ctx.Low;
    if (std::get<1>(ctx.Stack) < size) {
        delete std::get<0>(ctx.Stack);
        std::get<0>(ctx.Stack) = new char[size];
        std::get<1>(ctx.Stack) = size;
    }

    memcpy(std::get<0>(ctx.Stack), ctx.Low, size);
}

void Engine::Restore(context& ctx) {
    char stackStart;
    char* stackAddr = &stackStart;

    if (ctx.Low <= stackAddr && stackAddr <= ctx.Hight) {
        Restore(ctx);
    }

    memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    sched(nullptr);
}

void Engine::sched(void* routine_) {
    context* routine = (context*)routine_;

    if (cur_routine != nullptr) {
        if (setjmp(cur_routine->Environment) != 0) {
            return;
        }
        Store(*cur_routine);
    }

    // pass control to the another coroutine
    // if no another coroutine specified, the same semantics as for yield()

    // these lines are required to exit last run() correctly
    if (routine == nullptr && cur_routine == nullptr) {
        return;
    }

    if (routine == nullptr && cur_routine != nullptr) {
        // invoke the caller of current coroutine
        if (cur_routine->caller != nullptr) {
            routine = cur_routine->caller;
        }
        // invoke ANY coroutine another than cur_routine
        // if no coroutines remain, pass control back to cur_routine
        else {
            for (context* p = alive; p != nullptr; p = p->next) {
                if (p != cur_routine) { // find any routine != cur_routine
                    routine = p;
                    break;
                }
            }
            // if only cur_routine remains -> pass back to cur_routine
            if (routine == nullptr) {
                routine = cur_routine;
            }
        }
    }

    if (routine->callee != nullptr && routine->callee == cur_routine) {
        routine->callee = routine->callee->caller = nullptr;
    }

    while (routine->callee != nullptr) {
        routine = routine->callee;
    }

    // set new caller
    // recall: caller is a coroutine which invoked "routine" via yield() or sched()
    routine->caller = cur_routine;

    cur_routine = routine;
    Restore(*routine);
}

} // namespace Coroutine
