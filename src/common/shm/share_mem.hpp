
#ifndef _EQP_SHARE_MEM_HPP_
#define _EQP_SHARE_MEM_HPP_

#include "define_share_mem.hpp"
#include "exception.hpp"

class ShareMemViewer
{
protected:
    uint32_t    m_regionSize;
    void*       m_memory;

#ifdef EQP_WINDOWS
    HANDLE  m_handle;
#endif

protected:
    void setRegionSize(uint32_t size) { m_regionSize = size; }

public:
    ShareMemViewer();
    virtual ~ShareMemViewer();

    void open(const char* name, uint32_t size);
    void close();

    uint32_t    getMemorySize() const { return m_regionSize; }
    void*       getMemory() { return m_memory; }
};

class ShareMemCreator : public ShareMemViewer
{
private:
    const char* m_name;
    
public:
    ShareMemCreator();
    virtual ~ShareMemCreator();

    void create(const char* name, uint32_t size);
    void destroy();
};

#endif//_EQP_SHARE_MEM_HPP_
