#ifndef EMB_CONFIG_H
#define EMB_CONFIG_H

#include <nvm.h>


extern struct TypeDesc TypeDescConfig;

extern struct ModuleConfig {
    struct Type* type;
    struct IFace* iface;

    errval (*init) (void);
    errval (*initialized) (void);

    errval (*push) (void);
    errval (*pop) (void);

    errval (*set) (const char* name, const void* data, size sz);
    errval (*get) (const char* name, void* data, size sz);

    errval (*set_nvm_driver) (struct IFaceNVM* drv);
} config;

#endif

