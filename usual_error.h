#ifndef _USUAL_ERROR_H
#define _USUAL_ERROR_H

#include <errno.h>
// void raise_error(const int errCode, const char* message, const char* funcName);
#define malloc_error() error(1, ENOSPC, "函数“%s”中，调用malloc函数分配内存失败\n", __func__)
#define fopen_error() error(1, ENOENT, "函数“%s”中，文件打开失败\n", __func__)

#endif
