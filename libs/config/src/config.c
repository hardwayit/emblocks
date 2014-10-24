#include <config.h>


static errval init (void);
static bool initialized (void);

static errval push (void);
static errval pop (void);

static errval set (word index, const void* data, size sz);
static errval get (word index, void* data, size sz);

static errval set_nvm_driver (struct IFaceNVM* drv);

struct TypeDesc TypeDescConfig = { "config" };

struct ModuleConfig config = {
    .type           = &TypeDescConfig,
    .iface          = NULL,
    .init           = &init,
    .initialized    = &initialized,
    .push           = &push,
    .pop            = &pop,
    .set            = &set,
    .get            = &get,
    .set_nvm_driver = &set_nvm_driver
};

static struct {
    bool initialized;

    struct IFaceNVM* nvm_drv;
} module;


static errval init (void)
{
    module.initialized = false;

    module.nvm_drv = NULL;

    module.initialized = true;

    return ENO;
}

static bool initialized (void)
{
    return module.initialized;
}

static errval push (void)
{
    MODULE_CHECK_INITIALIZED

    if(module.nvm_drv == NULL) {
        return ESTATE;
    }

    // using module.nvm_drv->write_sectors

    return ENO;
}

static errval pop (void)
{
    MODULE_CHECK_INITIALIZED

    if(module.nvm_drv == NULL) {
        return ESTATE;
    }

    // using module.nvm_drv->read_sectors

    return ENO;
}

static errval set (word index, const void* data, size sz)
{
    bank b;
    word rindex;

    MODULE_CHECK_INITIALIZED

    if(!get_bank(sz, &b)) return EARG;

    if(!find_free(b, &rindex)) return ENOMEM;

    return rset(b, rindex, data, sz);
}

static errval get (word index, void* data, size sz)
{
    MODULE_CHECK_INITIALIZED

    return ENO;
}

static errval set_nvm_driver (struct IFaceNVM* drv)
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

