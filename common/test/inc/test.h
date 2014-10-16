#ifndef EMB_COMMON_TEST_H
#define EMB_COMMON_TEST_H

#define TEST(req, desc, action) \
    { \
        if(!(action)) { \
            tprintf(module_name, "Requirement " req " failed.\n"); \
            tprintf(module_name, "Action: " #action "\n"); \
            tprintf(module_name, "Description: " desc "\n"); \
        } \
    }

void tprintf(const char* module, const char* msg, ...);

#endif

