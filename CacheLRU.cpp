#include "CacheLRU.h"
#include <bits/stdc++.h>

CacheLRU::CacheLRU(size_t c)
    : kapacitet(c)
{}

std::optional<std::string> CacheLRU::dohvati(const std::string &k) {
    std::shared_lock<std::shared_mutex> sl(smutex);
    auto it = mapLRU.find(k);
    if (it == mapLRU.end()) return std::nullopt;

    lista.splice(lista.begin(), lista, it->second);
    return it->second->second;
}

void CacheLRU::spremi(const std::string &k, const std::string &v) {
    std::unique_lock<std::shared_mutex> ul(smutex);
    auto it = mapLRU.find(k);
    if (it != mapLRU.end()) {
        lista.erase(it->second);
    }

    lista.emplace_front(k, v);
    mapLRU[k] = lista.begin();

    if (lista.size() > kapacitet) {
        auto zad = lista.back();
        mapLRU.erase(zad.first);
        lista.pop_back();
    }
}
