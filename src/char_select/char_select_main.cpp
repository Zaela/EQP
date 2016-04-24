
#include "char_select.hpp"
#include "clock.hpp"
#include <exception>

int main(int argc, const char** argv)
{
    // argv[1] => path to ipc sharemem buffer
    
    if (argc < 2)
    {
        printf("CHAR SELECT expected 2 args, got %i. Aborting.\n", argc);
        return 1;
    }
    
#ifdef EQP_LINUX
    signal(SIGINT, SIG_IGN);
#endif
    
    // Scope for the CharSelect object
    {
        CharSelect charSelect;
        
        try
        {
#ifdef EQP_WINDOWS
            {
                WSADATA wsa;
                if (WSAStartup(MAKEWORD(2, 2), &wsa))
                    throw Exception("WSAStartup failed");
            }
#endif
            
            charSelect.init(argv[1]);
            charSelect.mainLoop();
        }
        catch (std::exception& e)
        {
            printf("CHAR SELECT exception: %s\n", e.what());
        }
    }
    
#ifdef EQP_WINDOWS
    WSACleanup();
#endif
    
    return 0;
}
