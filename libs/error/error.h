#ifndef EMB_ERROR_H
#define EMB_ERROR_H


#define EINVALARG 1


#define error_msg(msg, ...) error_throw(false, __FILE__, __LINE__, __func__, msg, __VA_ARGS__)
#define error_critical(msg, ...) error_throw(true, __FILE__, __LINE__, __func__, msg, __VA_ARGS__)


void error_throw(bool critical, const char* filename, unsigned int line, const char* funcname, const char* msg, ...);

void error_msg_set(const char* msg);

extern int error_code;

#endif

