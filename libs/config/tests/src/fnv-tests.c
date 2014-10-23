#include <fnv.h>

const char* strarr[] = {
    "Hello, world!",
    "Tratata",
    "config",
    "nvm",
    "data",
    "src",
    "inc",
    "emmc"
};

const int strarr_count = sizeof(strarr)/sizeof(strarr[0]);

int main(int argc, char* argv[])
{
    for(int i = 0; i < strarr_count; i++) {
        printf("%s: %08d\n", strarr[i], fnv(strarr[i]));
    }

    return 0;
}

