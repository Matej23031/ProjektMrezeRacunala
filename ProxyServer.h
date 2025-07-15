#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <string>
#include <vector>
#include "CacheLRU.h"

class ProxyServer {

    int soketPosluzitelja;
    CacheLRU predmemorija;
    std::vector<std::string> listaZabrana;

public:

    ProxyServer(int port);
    int primiVezu();
    void obradiKlijenta(int klijentSock);
};

#endif