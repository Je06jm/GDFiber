#ifndef GDFIBER_GDFIBER_SERVER_HPP
#define GDFIBER_GDFIBER_SERVER_HPP

#include <godot_cpp/core/object.hpp>

#include <mutex>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace godot {

class GDFiberServer : public Object {
    GDCLASS(GDFiberServer, Object)

public:
    using FID = uint64_t;

private:
    static GDFiberServer *singleton;

    struct Scheduler;

    struct Fiber {
        Fiber();
        ~Fiber();

        uintptr_t rsp;

        uint8_t* stack;

        uint8_t* sse_storage;
        uint16_t sse_address;

        void* ctx;

        FID fid;

        Scheduler* owning_scheduler;

        bool exited;
        bool awake;

        std::mutex lock;
        std::unordered_set<FID> dependents;
    };

    template <typename T>
    struct Queue {
    private:
        struct Node {
            T data;
            std::shared_ptr<Node> next;
        };

        mutable std::mutex lock;
        std::shared_ptr<Node> head, tail;
    
    public:
        bool has_item() const {
            if (!lock.try_lock()) {
                return false;
            }
            bool item_present = head != nullptr;
            lock.unlock();
            return item_present;
        }

        T pop_head() {
            std::lock_guard guard(lock);
            auto item = head;
            head = head->next;
            if (!head) {
                tail = nullptr;
            }
            return item->data;
        }

        void push_tail(const T& data) {
            std::lock_guard guard(lock);
            auto node = std::make_shared<Node>(data, nullptr);

            if (tail) {
                tail->next = node;
            }

            tail = node;

            if (!head) {
                head = node;
            }
        }
    };

    struct Scheduler {
    private:
        static std::mutex global_lock;
        static std::unordered_map<FID, std::shared_ptr<Fiber>> global_fibers;
        
        std::unordered_map<FID, std::shared_ptr<Fiber>> owned_fibers;

        std::vector<FID> active_fibers;

        std::shared_ptr<Fiber> current_fiber;

        static void swap_to_fiber(uintptr_t new_rsp, uintptr_t *old_rsp, uintptr_t new_sse_storage);
        static void swap_to_scheduler();

        uintptr_t rsp;
        static bool running;

        void run();

    public:
        Scheduler();
        ~Scheduler();

        Queue<FID> fibers_to_awaken;
        Queue<std::shared_ptr<Fiber>> fibers_to_add;

        std::atomic_size_t fiber_count;

        void yield();
        void wait_for(FID fid);
        void exit();

        static FID create_fiber(void (*function)(), void* ctx);
    };

    static thread_local Scheduler *scheduler;

protected:
    static void _bind_methods();

public:
    static void init();

    static GDFiberServer *get_singleton();

    GDFiberServer();
    ~GDFiberServer();
};

}

#endif