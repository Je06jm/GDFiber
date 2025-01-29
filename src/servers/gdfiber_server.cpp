#include "gdfiber_server.hpp"

#include "gdfiber_project_settings.hpp"

#include <random>
#include <cstdlib>

namespace godot {

void GDFiberServer::Scheduler::swap_to_fiber(uintptr_t new_rsp, uintptr_t *old_rsp, uintptr_t new_sse_storage) {
#ifdef _MSC_VER
    __asm(
        push rbx
        push rdi
        push rsi
        push r12
        push r13
        push r14
        push r15
        mov r12, new_sse_storage
        mov [old_rsp], rsp
        mov rsp, new_rsp
        fxrstor r12
        pop r15
        pop r14
        pop r13
        pop r12
        pop rsi
        pop rdi
        pop rbx
    );
#else
    asm(
        "pushq %%rbx\n"
        "pushq %%r12\n"
        "pushq %%r13\n"
        "pushq %%r14\n"
        "pushq %%r15\n"
        "movq %%rsp, (%0)\n"
        "movq %1, %%rsp\n"
        "fxrstor %2\n"
        "popq %%r15\n"
        "popq %%r14\n"
        "popq %%r13\n"
        "popq %%r12\n"
        "popq %%rbx\n"
        : :
        "r"(old_rsp), "r"(new_rsp), "r"(new_sse_storage)
    );
#endif
}

void GDFiberServer::Scheduler::swap_to_scheduler() {
    auto current_fiber = scheduler->current_fiber;
    uintptr_t old_rsp = scheduler->rsp;
    uintptr_t new_rsp = current_fiber->rsp;
    uintptr_t old_sse_storage = current_fiber->sse_address;

#ifdef _MSC_VER
    __asm(
        push rbx
        push rdi
        push rsi
        push r12
        push r13
        push r14
        push r15
        mov r12, old_sse_storage
        fxsave r12
        mov [old_rsp], rsp
        mov rsp, new_rsp
        pop r15
        pop r14
        pop r13
        pop r12
        pop rsi
        pop rdi
        pop rbx
    );
#else
    asm(
        "pushq %%rbx\n"
        "pushq %%r12\n"
        "pushq %%r13\n"
        "pushq %%r14\n"
        "pushq %%r15\n"
        "fxsave %0\n"
        "movq %%rsp, (%1)\n"
        "movq %2, %%rsp\n"
        "popq %%r15\n"
        "popq %%r14\n"
        "popq %%r13\n"
        "popq %%r12\n"
        "popq %%rbx\n"
        : :
        "r"(old_sse_storage), "r"(old_rsp), "r"(new_rsp)
    );
#endif
}

void GDFiberServer::Scheduler::run() {
    while (running) {
        while (fibers_to_awaken.has_item()) {
            auto fid = fibers_to_awaken.pop_head();

            if (owned_fibers.find(fid) == owned_fibers.end()) {
                ERR_FAIL_MSG("GDFiber Scheduler tried to awaken a fiber it does not own");
            }

            auto fiber = owned_fibers[fid];

            if (!fiber->awake) {
                fiber->awake = true;
                active_fibers.push_back(fid);
            }
        }

        while (fibers_to_add.has_item()) {
            auto fiber = fibers_to_add.pop_head();

            if (owned_fibers.find(fiber->fid) != owned_fibers.end()) {
                ERR_FAIL_MSG("GDFiber Scheduler tried to add a fiber it already owns");
            }

            owned_fibers.insert({fiber->fid, fiber});
            active_fibers.push_back(fiber->fid);
        }

        static std::random_device rd;
        static std::mt19937 rng(rd());

        std::uniform_int_distribution<std::mt19937::result_type> rand_range(0, active_fibers.size() - 1);

        auto fid = active_fibers[rand_range(rng)];

        auto fiber = owned_fibers[fid];

        current_fiber = fiber;

        swap_to_fiber(fiber->rsp, &rsp, fiber->sse_address);

        current_fiber = nullptr;

        if (fiber->exited) {
            auto it = std::find(active_fibers.begin(), active_fibers.end(), current_fiber->fid);
            active_fibers.erase(it);

            owned_fibers.erase(fiber->fid);

            fiber_count--;
        }
    }
}

GDFiberServer::Scheduler::Scheduler() {
    fiber_count = 0;
}

GDFiberServer::Scheduler::~Scheduler() {
    
}

void GDFiberServer::Scheduler::yield() {
    swap_to_scheduler();
}

void GDFiberServer::Scheduler::wait_for(FID fid) {
    {
        std::lock_guard this_guard(global_lock);

        auto fiber = global_fibers.find(fid);
        if (fiber == global_fibers.end()) {
            ERR_FAIL_MSG("GDFiber scheduler was asked to wait for a fiber that does not exist");
        }

        std::lock_guard other_guard(fiber->second->lock);

        fiber->second->dependents.insert(current_fiber->fid);
    }

    auto it = std::find(active_fibers.begin(), active_fibers.end(), current_fiber->fid);
    active_fibers.erase(it);

    yield();
}

void GDFiberServer::Scheduler::exit() {
    current_fiber->exited = true;
    yield();
}

GDFiberServer::FID GDFiberServer::Scheduler::create_fiber(void (*function)(), void* ctx) {
    static FID next_fid = 0;
    
    auto fiber = std::make_shared<Fiber>();

    auto stack_size = GDFiberProjectSettings::thread_stack_size();
    
    fiber->stack = new uint8_t[stack_size];
    fiber->rsp = reinterpret_cast<uintptr_t>(fiber->stack);

    fiber->rsp += stack_size;
    fiber->rsp &= ~15;
    fiber->rsp -= 32;
}

}