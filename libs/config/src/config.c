#include <config.h>


static errval __config_init (void);
static bool __config_initialized (void);

static errval __config_push (void);
static errval __config_pop (void);

static errval __config_set (word index, const void* data, size sz);
static errval __config_get (word index, void* data, size sz);

static errval __config_set_nvm_driver (struct IFaceNVM* drv);

struct TypeDesc TypeDescConfig = { "config" };

struct ModuleConfig config = {
    .type           = &TypeDescConfig,
    .iface          = NULL,
    .init           = &__config_init,
    .initialized    = &__config_initialized,
    .push           = &__config_push,
    .pop            = &__config_pop,
    .set            = &__config_set,
    .get            = &__config_get,
    .set_nvm_driver = &__config_set_nvm_driver
};

static struct {
    bool initialized;

    struct IFaceNVM* nvm_drv;
} module;


static errval __config_init (void)
{
    module.initialized = false;

    module.nvm_drv = NULL;

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

    if(module.nvm_drv == NULL) {
        return ESTATE;
    }

    // using module.nvm_drv->write_sectors

    return ENO;
}

static errval __config_pop (void)
{
    MODULE_CHECK_INITIALIZED

    if(module.nvm_drv == NULL) {
        return ESTATE;
    }

    // using module.nvm_drv->read_sectors

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

static errval __config_set_nvm_driver (struct IFaceNVM* drv)
{
    if(drv->iface != &IFaceDescNVM) {
        return ETYPE;
    }

    if(!drv->initialized) {
        return EARG;
    }

    module.nvm_drv = drv;

    return ENO;
}

