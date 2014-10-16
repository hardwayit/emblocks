#ifndef EMB_CONFIG_H
#define EMB_CONFIG_H

#include <nvm.h>

typedef errval (*config_push_func) (void);
typedef errval (*config_pop_func) (void);

typedef errval (*config_set_func) (word index, const void* data, size sz);
typedef errval (*config_get_func) (word index, void* data, size sz);

typedef errval (*config_set_nvm_driver_func) (struct IFaceNVM* drv);

extern struct TypeDesc TypeDescConfig;

extern struct ModuleConfig {
    struct Type* type;
    struct IFace* iface;

    module_init_func init;
    module_initialized_func initialized;

    config_push_func push;
    config_pop_func pop;

    config_set_func set;
    config_get_func get;

    config_set_nvm_driver_func set_nvm_driver;
} config;

#endif

