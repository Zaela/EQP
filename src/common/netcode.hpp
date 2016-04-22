
#ifndef _EQP_NETCODE_H_
#define _EQP_NETCODE_H_

#ifdef EQP_WINDOWS
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
#else
# include <errno.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netdb.h>
# include <signal.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netinet/tcp.h>
# include <net/if.h>
#endif

#define toNetworkShort htons
#define toNetworkLong htonl
#define toHostShort ntohs
#define toHostLong ntohl

#ifndef EQP_WINDOWS
# define closesocket close
# define INVALID_SOCKET -1
#else
typedef int socklen_t;
#endif

typedef struct sockaddr_in IpAddress;

#endif//_EQP_NETCODE_H_
