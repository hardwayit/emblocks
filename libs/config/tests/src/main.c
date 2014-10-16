#include <config.h>

#include <test.h>

static char module_name[] = "config";

int main(int argc, char* argv[])
{
    TEST("", "Initialization module.", config.init() == ENO);

    return 0;
}

