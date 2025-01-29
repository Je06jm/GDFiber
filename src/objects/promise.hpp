#ifndef GDFIBER_PROMISE_HPP
#define GDFIBER_PROMISE_HPP

#include <godot_cpp/classes/ref_counted.hpp>

#include "future.hpp"

namespace godot {

class Promise : public RefCounted {
    GDCLASS(Promise, RefCounted);

private:
    Ref<Future> m_future;

protected:
    static void _bind_methods();

public:
    Promise();
    ~Promise();

    Ref<Future> future() const;

    void fulfill(Variant p_value);
};

}

#endif