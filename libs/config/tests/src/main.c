#include <config.h>

int main(int argc, char* argv[])
{
    errval res;

    if(res = config.init()) {
        ;
    }

    return 0;
}

