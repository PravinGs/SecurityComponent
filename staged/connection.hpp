#ifndef CONNECTION_HPP
#define CONNNECTION_HPP

#include "agentUtils.hpp"

class TlsConnection
{
private:
    string _port;
    string _caKey;
    string _serverCert;
    string _serverKey;

private:
    SSL_CTX *_getServerContext(const string caKey, const string serverCert, const string serverKey);
    int _get_socket(int port_num);

public:
    TlsConnection(string port, string caKey, string serverCert, string serverKey);
    int start();
    ~TlsConnection();
};

#endif