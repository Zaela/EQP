
#include "login.hpp"
#include "clock.hpp"
#include <exception>

int main(int argc, const char** argv)
{
    // argv[1] => path to ipc sharemem buffer
    // argv[2] => server display name
    
    if (argc < 3)
    {
        printf("LOGIN expected 3 args, got %i. Aborting.\n", argc);
        return 1;
    }
    
    Login login;
    
    try
    {
#ifdef EQP_WINDOWS
        {
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2, 2), &wsa))
                throw Exception("WSAStartup failed");
        }
#endif
        
        login.init(argv[1], argv[2]);
        login.mainLoop();
    }
    catch (std::exception& e)
    {
        printf("exception: %s\n", e.what());
    }
    
#ifdef EQP_WINDOWS
    WSACleanup();
#endif
    
    return 0;
}