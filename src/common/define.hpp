
#ifndef _EQP_DEFINE_HPP_
#define _EQP_DEFINE_HPP_

#ifdef _WIN32
# define EQP_WINDOWS
#else
# define EQP_LINUX
#endif

#include "netcode.hpp"

#ifdef EQP_WINDOWS
# include <windows.h>
#else
# include <cerrno>
# include <cinttypes>
#endif

#include <stdint.h>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <cctype>
#include <cmath>
#include <new>
#include <limits>
#include <algorithm>

typedef uint8_t byte;

#define KILOBYTES(n) (1024 * (n))
#define MEGABYTES(n) (1024 * KILOBYTES(n))

#endif//_EQP_DEFINE_HPP_
