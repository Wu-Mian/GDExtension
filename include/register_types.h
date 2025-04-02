#ifndef REGISTER_TYPES_H
#define REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

void initialize_diff_detector_module(godot::ModuleInitializationLevel p_level);
void uninitialize_diff_detector_module(godot::ModuleInitializationLevel p_level);

// GDExtension入口点
extern "C" {
    GDExtensionBool GDE_EXPORT diff_detector_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization);
}

#endif // REGISTER_TYPES_H 