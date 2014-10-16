#ifndef EMB_NVM_H
#define EMB_NVM_H

typedef errval (*nvm_read_sectors_t) (dword start, dword num, void* buf);
typedef errval (*nvm_write_sectors_t) (dword start, dword num, const void* buf);
typedef size (*nvm_sector_size_t) (void);

extern struct IFaceDesc IFaceDescNVM;

struct IFaceNVM {
    struct TypeDesc* type;
    struct IFaceDesc* iface;

    module_init_t init;
    module_initialized_t initialized;

    nvm_read_sectors_t read_sectors;
    nvm_write_sectors_t write_sectors;

    nvm_sector_size_t sector_size;
};


#endif

