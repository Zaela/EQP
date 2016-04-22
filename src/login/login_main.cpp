
#include "login.hpp"
#include "clock.hpp"
#include <exception>

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;
    
    // argv[1] => path to ipc sharemem buffer
    // argv[2] => server display name ?
    
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
        
        login.init(EQP_IPC_LOGIN);
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