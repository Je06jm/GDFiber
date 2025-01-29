#ifndef GDFIBER_PROJECT_SETTINGS_HPP
#define GDFIBER_PROJECT_SETTINGS_HPP

namespace godot {

class GDFiberProjectSettings {
public:
    static void register_settings();

    static bool is_enabled();
    static size_t thread_count();
    static size_t thread_stack_size();
};

}

#endif