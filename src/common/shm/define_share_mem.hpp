
#ifndef _EQP_DEFINE_SHAREMEM_HPP_
#define _EQP_DEFINE_SHAREMEM_HPP_

#include "define.hpp"

#ifdef EQP_LINUX
# include <sys/mman.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
# include <unistd.h>
#endif

#endif//_EQP_DEFINE_SHAREMEM_HPP_
