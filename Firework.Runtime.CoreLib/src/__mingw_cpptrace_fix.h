#pragma once

#ifdef __cplusplus
#include <cerrno>
#include <cstdio>
#else
#include <errno.h>
#include <stdio.h>
#endif

inline int __compat_fopen_s(FILE** pFile, const char* filename, const char* mode)
{
    if (!pFile || !filename || !mode)
        return EINVAL;

    FILE* f = fopen(filename, mode);
    if (!f)
    {
        if (errno != 0)
            return errno;
        else return ENOENT;
    }
    *pFile = f;

    return 0;
}
#define fopen_s __compat_fopen_s
