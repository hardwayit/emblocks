#ifndef EMB_CONFIG_H
#define EMB_CONFIG_H

typedef errval (*config_push_t) (void);
typedef errval (*config_pop_t) (void);

typedef errval (*config_set_t) (word index, const void* data, size sz);
typedef errval (*config_get_t) (word index, void* data, size sz);

extern struct ModuleConfig {
    module_init_t init;
    module_initialized_t initialized;

    config_push_t push;
    config_pop_t pop;

    config_set_t set;
    config_get_t get;
} config;

#endif

