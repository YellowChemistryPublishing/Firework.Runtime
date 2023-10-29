#pragma once

#include <exception>
#include <iostream>

inline void __internal_fatal(const char* error, const char* file, int line)
{
    std::cerr << error << " (at line " << line << " of " << file << ".)\n";
    std::terminate();
}

#define __fatal(err) __internal_fatal(err, __FILE__, __LINE__)
