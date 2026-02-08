#pragma once

#include <functional>

template<typename>
struct Handle {
    uint32_t id = ++m_lastID;

    bool operator==(const Handle& other) const {
        return id == other.id;
    }

private:
    inline static uint32_t m_lastID = 0;
};

struct HandleHasher {
    template<typename T>
    size_t operator()(const Handle<T>& k) const {
        return std::hash<uint32_t>()(k.id);
    }
};