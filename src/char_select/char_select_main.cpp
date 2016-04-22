
#include "char_select.hpp"
#include "clock.hpp"
#include <exception>

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;
    
    CharSelect charSelect;
    
    try
    {
        charSelect.init();
    }
    catch (std::exception& e)
    {
        
    }
    
    return 0;
}
