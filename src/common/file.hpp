
#ifndef _EQP_FILE_HPP_
#define _EQP_FILE_HPP_

#include "define.hpp"
#include <string>

class File
{
private:
    FILE* m_fp;
    
public:
    File(const std::string& path, const std::string& mode = "rb");
    ~File();

    bool isOpen() const { return m_fp != nullptr; }
    bool exists() const { return isOpen(); }
    
    std::string readAll();
    void        readAll(std::string& str);
    uint64_t    calcSize();
};

#endif//_EQP_FILE_HPP_
