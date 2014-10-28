#ifndef EMB_STRING_H
#define EMB_STRING_H


extern struct TypeDesc TypeDescString;

extern struct ModuleString {
    struct Type* type;
    struct IFace* iface;

    errval (*init) (void);
    errval (*initialized) (void);

    bool (*cmp)(const char* str1, const char* str2);
} string;

#endif

