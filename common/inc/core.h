#define true (1)
#define false (0)

#warning "TODO: make *WORD_MAX portable"
#define DWORD_MAX (0xFFFFFFFF)
#define WORD_MAX (0xFFFF)

typedef _Bool bool;

typedef unsigned char  byte;
typedef signed   short sword;
typedef unsigned short word;
typedef signed   int   sdword;
typedef unsigned int   dword;
typedef unsigned int   size;

#define __use_result __attribute__((warn_unused_result))

typedef dword errval;

#define ENO        (0)
#define EUNDEFINED (1)
#define EIO        (2)
#define EUNINIT    (3)
#define ETYPE      (4)
#define EARG       (5)
#define ESTATE     (6)

struct TypeDesc
{
    char name[16];
};

struct IFaceDesc
{
    char name[16];
};

