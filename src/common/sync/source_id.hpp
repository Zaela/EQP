
#ifndef _EQP_SOURCE_ID_HPP_
#define _EQP_SOURCE_ID_HPP_

#include "define.hpp"

class SourceId
{
public:
    static const int ZoneClusterOffset  = 10000;
    static const int ProcessOffset      = 20000;
    static const int Master             = ProcessOffset + 1;
    static const int Login              = ProcessOffset + 2;
    static const int CharSelect         = ProcessOffset + 3;
};

#endif//_EQP_SOURCE_ID_HPP_
