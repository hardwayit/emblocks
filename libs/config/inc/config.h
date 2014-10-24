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

    errval (*module_init_func init);
    errval (*module_initialized_func initialized;

    errval (*config_push_func) (void);
    errval (*config_pop_func) (void);

    errval (*config_set_func) (word index, const void* data, size sz);
    errval (*config_get_func) (word index, void* data, size sz);

    errval (*config_set_nvm_driver_func) (struct IFaceNVM* drv);
} config;

#endif

