#include <config.h>


static errval __config_init (void);
static bool __config_initialized (void);

static errval __config_push (void);
static errval __config_pop (void);

static errval __config_set (word index, const void* data, size sz);
static errval __config_get (word index, void* data, size sz);


struct ModuleConfig config = {
    .init        = &__config_init,
    .initialized = &__config_initialized,
    .push        = &__config_push,
    .pop         = &__config_pop,
    .set         = &__config_set,
    .get         = &__config_get
};

static struct {
    bool initialized;
} module;


static errval __config_init (void)
{
    module.initialized = false;

    module.initialized = true;

    return ENO;
}

static bool __config_initialized (void)
{
    return module.initialized;
}

static errval __config_push (void)
{
    MODULE_CHECK_INITIALIZED

    return ENO;
}

static errval __config_pop (void)
{
    MODULE_CHECK_INITIALIZED

    return ENO;
}

static errval __config_set (word index, const void* data, size sz)
{
    MODULE_CHECK_INITIALIZED

    return ENO;
}

static errval __config_get (word index, void* data, size sz)
{
    MODULE_CHECK_INITIALIZED

    return ENO;
}

