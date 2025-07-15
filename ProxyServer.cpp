#include "ProxyServer.h"
#include <shared_mutex>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <algorithm>

ProxyServer::ProxyServer(int port)
    : predmemorija(100)
{
    listaZabrana = {"/zabrana1", "/zabrana2"};

    soketPosluzitelja = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in adr{};
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = INADDR_ANY;
    adr.sin_port = htons(port);

    bind(soketPosluzitelja, (sockaddr*)&adr, sizeof(adr));
    listen(soketPosluzitelja, SOMAXCONN);
}

int ProxyServer::primiVezu() {
    return accept(soketPosluzitelja, nullptr, nullptr);
}

void ProxyServer::obradiKlijenta(int klijentSock) {

    char buf[4096];
    int len = recv(klijentSock, buf, sizeof(buf), 0);

    if (len <= 0) {
        close(klijentSock);
        return;
    }

    std::istringstream tok(std::string(buf, len));
    std::string metoda, uri, verzija;
    tok >> metoda >> uri >> verzija;

    if (std::any_of(listaZabrana.begin(), listaZabrana.end(),
                    [&](auto &z){ return uri.find(z) != std::string::npos; }))
    {
        std::string zabranjeno = "HTTP/1.1 403 Forbidden\r\n\r\n";
        send(klijentSock, zabranjeno.c_str(), zabranjeno.size(), 0);
        close(klijentSock);
        return;
    }

    if (metoda == "CONNECT") {

        size_t dvotochka = uri.find(":");
        std::string host = uri.substr(0, dvotochka);
        int port = stoi(uri.substr(dvotochka + 1));

        int srvSock = socket(AF_INET, SOCK_STREAM, 0);
        hostent* he = gethostbyname(host.c_str());

        sockaddr_in srvAddr{};
        srvAddr.sin_family = AF_INET;
        srvAddr.sin_port = htons(port);
        srvAddr.sin_addr = *((in_addr*)he->h_addr);

        connect(srvSock, (sockaddr*)&srvAddr, sizeof(srvAddr));

        std::string ok = "HTTP/1.1 200 Connection Established\r\n\r\n";
        send(klijentSock, ok.c_str(), ok.size(), 0);

        fd_set fds;

        while (true) {
            FD_ZERO(&fds);
            FD_SET(klijentSock, &fds);
            FD_SET(srvSock, &fds);

            int mx = std::max(klijentSock, srvSock) + 1;
            select(mx, &fds, nullptr, nullptr, nullptr);

            if (FD_ISSET(klijentSock, &fds)) {
                int r = recv(klijentSock, buf, sizeof(buf), 0);
                if (r <= 0) break;
                send(srvSock, buf, r, 0);
            }

            if (FD_ISSET(srvSock, &fds)) {
                int r = recv(srvSock, buf, sizeof(buf), 0);
                if (r <= 0) break;
                send(klijentSock, buf, r, 0);
            }
        }

        close(srvSock);
        close(klijentSock);

    } else {

        if (metoda == "GET") {
            auto zapis = predmemorija.dohvati(uri);
            if (zapis) {
                send(klijentSock, zapis->data(), zapis->size(), 0);
                close(klijentSock);
                return;
            }
        }

        std::string zaglavlja(buf, len);
        std::string host;
        int port = 80;

        if (auto pos = zaglavlja.find("Host: "); pos != std::string::npos) {
            size_t kraj = zaglavlja.find("\r\n", pos);
            host = zaglavlja.substr(pos + 6, kraj - pos - 6);
        }

        int srvSock = socket(AF_INET, SOCK_STREAM, 0);
        hostent* he = gethostbyname(host.c_str());

        sockaddr_in srvAdr{};
        srvAdr.sin_family = AF_INET;
        srvAdr.sin_port = htons(port);
        srvAdr.sin_addr = *((in_addr*)he->h_addr);

        connect(srvSock, (sockaddr*)&srvAdr, sizeof(srvAdr));

        send(srvSock, buf, len, 0);

        int r;
        std::string odgovor;

        while ((r = recv(srvSock, buf, sizeof(buf), 0)) > 0) {
            odgovor.append(buf, r);
        }

        if (metoda == "GET") predmemorija.spremi(uri, odgovor);

        send(klijentSock, odgovor.data(), odgovor.size(), 0);

        close(srvSock);
        close(klijentSock);
    }
}
