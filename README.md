# Višedretveni HTTP Proxy

Ovaj projekt implementira **višedretveni** HTTP proxy u vezi pomoću C++ standardne biblioteke i POSIX socket API-ja. Ključne značajke uključuju:

* **Višenitnost**: koristi se `ThreadPool` za učinkovito upravljanje nitima i razdvajanje obrade klijenata.
* **LRU Cache**: implementacija Least-Recently-Used cachea (`CacheLRU`) za keširanje GET zahtjeva.
* **Filtriranje URL-ova**: lista zabranjenih URI segmenata (`listaZabrana`) nakon kojih se vraća HTTP 403 Forbidden.
* **HTTPS tuneliranje**: podrška za HTTP `CONNECT` metodu koja uspostavlja tunel između klijenta i poslužitelja.
* **Sinkronizacija**: `std::shared_mutex` omogućuje višestruke čitatelje (readers) i ekskluzivnog pisca (writer) pri pristupu cacheu.

## Struktura projekta

```
ProjektMrezeRacunala/
├── src/
│   ├── main.cpp
│   ├── ProxyServer.h
│   ├── ProxyServer.cpp
│   ├── ThreadPool.h
│   ├── ThreadPool.cpp
│   ├── CacheLRU.h
│   └── CacheLRU.cpp
└── README.md
```

* **src/**: direktorij sa svim izvornim (.cpp/.h) datotekama.
* **README.md**: ovaj dokument.

## Datoteke

### main.cpp

* Inicijalizira `ProxyServer` na portu 8080.
* Kreira `ThreadPool` s brojem radnih niti jednakim broju hardware koncurrency.
* U beskonačnoj petlji prima veze i raspoređuje obrade klijent zahtjeva na pool.

### ProxyServer.h / ProxyServer.cpp

* **ProxyServer(int port)**: otvara server-socket, `bind()`, `listen()`.
* **primiVezu()**: `accept()` nove klijente.
* **obradiKlijenta(int klijentSock)**:

  1. Parsira HTTP request liniju.
  2. Provjera liste zabranjenih URI segmenata.
  3. Ako je `CONNECT`, uspostavlja tunel uz `select()` za dvosmjerni prijenos.
  4. Za ostale metode, podržava GET keširanje (preuzimanje iz `CacheLRU` ili spremanje) i prosljeđivanje zahtjeva prema poslužitelju.

### ThreadPool.h / ThreadPool.cpp

* **Konstruktor**: pokreće N radničkih niti koje čekaju zadatke.
* **enqueue(...)**: sprema lambda zadatke u queue i budi čekajuću nit.
* **Destruktor**: zatvara pool, budi sve niti, čeka njihov završetak.

### CacheLRU.h / CacheLRU.cpp

* Interni LRU cache: `std::list` + `std::unordered_map`.
* `dohvati(key)`: uzima `shared_lock`, vraća `std::optional<string>` i pomiče stavku na početak liste.
* `spremi(key,value)`: uzima `unique_lock`, dodaje novi unos, briše najstariji ako je prekoračen kapacitet.

## Korištenje

1. **Kompilacija**
   U korijenskom direktoriju:

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
2. **Pokretanje**

   ```bash
   ./ProxyServer
   ```
3. **Testiranje**

   * Konfigurirajte browser ili `curl` da koristi `http://localhost:8080` kao proxy.

   ```bash
   curl -x http://localhost:8080 http://example.com
   ```

## Konfiguracija

* **Port**: zadano `8080` (može se promijeniti u `main.cpp`).
* **Kapacitet cachea**: zadano 100 unosa (promjena u `ProxyServer.cpp`).
* **Lista zabranjenih URI**: definira se u `ProxyServer.cpp` u konstruktoru.

## Ograničenja i budući rad

* **Sigurnost**: nema validacije zaglavlja i ograničenja veličine.
* **HTTP verzija**: podržava samo HTTP/1.1 protokol.
* **HTTPS**: tuneliranje samo pasivno, nema keširanja kraijenog promet.

Moguće nadogradnje:

* Dinamičko prilagođavanje veličine poola.
* Više kriterija filtriranja i HTTPS keširanje.

---

© 2025 Matej Čiček
