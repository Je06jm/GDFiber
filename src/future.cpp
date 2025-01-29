#include "future.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace godot {

void Future::_bind_methods() {
    ClassDB::bind_method(D_METHOD("has_owning_promise"), &Future::has_owning_promise);
    ClassDB::bind_method(D_METHOD("is_completed"), &Future::is_completed);
    ClassDB::bind_method(D_METHOD("value"), &Future::value);
}

Future::Future() {
    m_owning_promise = false;
}

Future::~Future() {

}

bool Future::has_owning_promise() const {
    return m_owning_promise;
}

bool Future::is_completed() const {
    return bool(m_value);
}

Variant Future::value() const {
    return m_value;
}

}