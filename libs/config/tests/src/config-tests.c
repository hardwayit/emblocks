#include <string.h>
#include <config.h>

#include <test.h>

static char module_name[] = "config";

int main(int argc, char* argv[])
{
    TEST("", "Initialization module.", config.init() == ENO);

    dword dw;
    char in_str[32], out_str[32];
    
    dw = 3824;

    TEST("", "Setting dword value", config.set("hahaval", &dw, 4) == ENO);

    TEST("", "Getting value", config.get("hahaval", &dw, 4) == ENO);

    TEST("", "Getting exact that writed", string.cmp(in_str, out_str));

    return 0;
}

