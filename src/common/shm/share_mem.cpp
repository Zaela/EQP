
#include "share_mem.hpp"

ShareMemViewer::ShareMemViewer()
: m_regionSize(0),
  m_memory(nullptr)
#ifdef EQP_WINDOWS
, m_handle(INVALID_HANDLE_VALUE)
#endif
{
    
}

ShareMemViewer::~ShareMemViewer()
{
    close();
}

void ShareMemViewer::open(const char* name, uint32_t len)
{
#ifdef EQP_WINDOWS
    m_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name);
    
    if (m_handle == NULL)
    {
        m_handle = INVALID_HANDLE_VALUE;
        throw 1; //fixme
    }
    
    m_memory = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, len);
#else
    int fd = ::open(name, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    
    if (fd == -1)
        throw Exception("[ShareMemViewer::open] Could not open '%s', errno: %i", name, errno);
    
    m_memory = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    ::close(fd);
#endif
    
    if (!m_memory)
        throw Exception("[ShareMemViewer::open] Memory mapping of '%s' failed, errno: %i", name, errno);
    
    m_regionSize = len;
}

void ShareMemViewer::close()
{
    if (m_memory)
    {
#ifdef EQP_WINDOWS
        UnmapViewOfFile(m_memory);
#else
        munmap(m_memory, m_regionSize);
#endif
        m_memory = nullptr;
    }
    
#ifdef EQP_WINDOWS
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
#endif
}

ShareMemCreator::ShareMemCreator()
: m_name(nullptr)
{
    
}

ShareMemCreator::~ShareMemCreator()
{
    destroy();
}

void ShareMemCreator::create(const char* name, uint32_t len)
{
#ifdef EQP_WINDOWS
    m_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, len, name);
    
    if (m_handle == NULL)
    {
        m_handle = INVALID_HANDLE_VALUE;
        throw 1; //fixme
    }
    
    m_memory = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, len);
#else
    int fd = ::open(name, O_CREAT | O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    
    if (fd == -1)
        throw Exception("[ShareMemCreator::create] Could not create or open '%s', errno: %i", name, errno);
    
    m_name = name;
    
    if (ftruncate(fd, len))
    {
        ::close(fd);
        throw Exception("[ShareMemCreator::create] ftruncate() failed for '%s', errno: %i", name, errno);
    }
    
    m_memory = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    ::close(fd);
#endif
    
    if (!m_memory)
        throw Exception("[ShareMemCreator::create] Memory mapping failed for '%s', errno: %i", name, errno);
    
    setRegionSize(len);
}

void ShareMemCreator::destroy()
{
#ifndef EQP_WINDOWS
    if (m_name)
    {
        ::unlink(m_name);
        m_name = nullptr;
    }
#endif
}
