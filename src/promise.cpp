#include "promise.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace godot {

void Promise::_bind_methods() {
    ClassDB::bind_method(D_METHOD("fulfill", "p_value"), &Promise::fulfill);
}

Promise::Promise() {
    m_future.m_owning_promise = true;
}

Promise::~Promise() {

}

Future Promise::future() const {
    return m_future;
}

void Promise::fulfill(Variant p_value) {
    m_future.m_value = p_value;
}

}