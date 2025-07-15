#include "ProxyServer.h"
#include "ThreadPool.h"
#include "CacheLRU.h"
#include <vector>
#include <string>
#include <thread>

int main() {

    int port = 8080;

    ProxyServer posluzitelj(port);

    ThreadPool pool(std::thread::hardware_concurrency());

    while (true) {
        int klijentSock = posluzitelj.primiVezu();

        pool.enqueue([klijentSock, &posluzitelj]() {
            posluzitelj.obradiKlijenta(klijentSock);
        });
    }

    return 0;
}
