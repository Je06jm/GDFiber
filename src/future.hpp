#ifndef GDFIBER_FUTURE_HPP
#define GDFIBER_FUTURE_HPP

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class Promise;

class Future : public RefCounted {
    GDCLASS(Future, RefCounted)

    friend Promise;

private:
    Variant m_value;

    bool m_owning_promise;

protected:
    static void _bind_methods();

public:
    Future();
    ~Future();

    bool has_owning_promise() const;

    bool is_completed() const;

    Variant value() const;
};

}

#endif