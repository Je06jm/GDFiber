#ifndef GDFIBER_REGISTER_TYPES_HPP
#define GDFIBER_REGISTER_TYPES_HPP

#include <godot_cpp/core/class_db.hpp>

void initialize_gdthread_module(godot::ModuleInitializationLevel p_level);
void uninitialize_gdthread_module(godot::ModuleInitializationLevel p_level);

#endif