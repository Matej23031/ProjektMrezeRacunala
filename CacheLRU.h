#ifndef CACHELRU_H
#define CACHELRU_H

#include <string>
#include <list>
#include <unordered_map>
#include <shared_mutex>
#include <optional>

class CacheLRU {

    size_t kapacitet;
    std::list<std::pair<std::string, std::string>> lista;
    std::unordered_map<std::string, decltype(lista.begin())> mapLRU;
    mutable std::shared_mutex smutex;

public:

    CacheLRU(size_t c);
    std::optional<std::string> dohvati(const std::string &k);
    void spremi(const std::string &k, const std::string &v);
};

#endif