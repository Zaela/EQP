
#include "login_client.hpp"

LoginClient::LoginClient(uint32_t ip, uint16_t port)
: m_ipAddress(ip),
  m_port(port),
  m_isTrilogy(false),
  m_recvAck(0),
  m_sendAck(0),
  m_progress(Progress::None),
  m_playSequence(0),
  m_playAck(0),
  m_accountId(0)
{
    
}
