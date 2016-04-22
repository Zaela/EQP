
#include "file.hpp"

File::File(const std::string& path, const std::string& mode)
{
    m_fp = ::fopen(path.c_str(), mode.c_str());
}

File::~File()
{
    if (m_fp)
        ::fclose(m_fp);
}

std::string File::readAll()
{
    std::string str;
    uint64_t len = calcSize();
    str.resize(len);
    ::fread(&str[0], 1, len, m_fp);
    return str;
}

void File::readAll(std::string& str)
{
    uint64_t size   = calcSize();
    uint64_t len    = str.length();
    str.reserve(len + size);
    ::fread(&str[len], 1, size, m_fp);
}

uint64_t File::calcSize()
{
    ::fseek(m_fp, 0, SEEK_END);
    uint64_t size = ::ftell(m_fp);
    ::rewind(m_fp);
    return size;
}
