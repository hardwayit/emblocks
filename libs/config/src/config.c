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

static bool get_bank(size sz, bank *b)
{
    if(sz == 0) return false;
    else if(sz <= 4) { if(b) *b = BANK_DW; }
    else if(sz <= 28) { if(b) *b = BANK_SHORT_BUF; }
    else if(sz <= 60) { if(b) *b = BANK_LONG_BUF; }
    else return false;

    return true;
}

static errval e_find(bank b, const char* name, word* index)
{
    address offset = b_offset(b);
    dword count = b_count(b);

    for(dword i = 0; i < count; i++)
    {
        if(e_name_equals(b, i, name))
        {
            if(index) *index = offset + i;

            return ENO;
        }
    }

    return ENOTEXIST;
}

static errval e_find_free(bank b, word* index)
{
    address offset = b_offset(b);
    dword count = b_count(b);

    for(dword i = 0; i < count; i++)
    {
        if(e_is_free(b, i))
        {
            if(index) *index = offset + i;

            return ENO;
        }
    }

    return ENOMEM;
}

static errval e_set(bank b, word index, const char* name, const void* data, size sz)
{
    e_set_name(b, index, name);
    e_set_data(b, index, data, sz);

    return ENO;
}

static errval e_set_data(bank b, word index, const void* data, size sz)
{
    return kmemcpy(module.banks[b][index - b_offset(b)].data, data, sz);
}

static errval e_set_name(bank b, word index, const char* name)
{
    if(err = kmemcpy(module.banks[b][index - b_offset(b)].data, data, sz))
    {
        return err;
    }
}

static errval e_get_data(bank b, word index, void* data, size sz)
{
    return kmemcpy(data, module.banks[b][index - b_offset(b)].data, sz);
}

static errval set (const char* name, const void* data, size sz)
{
    bank b;
    word index;
    errval err;

    MODULE_CHECK_INITIALIZED

    if(!get_bank(sz, &b)) return EARG;

    if(err = find(b, name, &index)) {
        if(err = find_free(b, &index)) {
            return err;
        }
    }

    return e_set(b, index, name, data, sz);
}

static errval get (const char* name, void* data, size sz)
{
    bank b;
    word index;
    errval err;

    MODULE_CHECK_INITIALIZED

    if(!get_bank(sz, &b)) return EARG;

    if(err = find(b, name, &index)) return ENOTEXIST;

    return e_get(b, index, data, sz);
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

