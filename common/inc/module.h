typedef errval (*module_init_t) (void);
typedef bool (*module_initialized_t) (void);

#define MODULE_CHECK_INITIALIZED { if(!module.initialized) return EUNINIT; }
