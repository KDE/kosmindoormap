/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stringpool.h"

#include <algorithm>

OSM::StringKeyRegistryBase::StringKeyRegistryBase() = default;
OSM::StringKeyRegistryBase::StringKeyRegistryBase(OSM::StringKeyRegistryBase&&) noexcept = default;
OSM::StringKeyRegistryBase& OSM::StringKeyRegistryBase::operator=(OSM::StringKeyRegistryBase&&) noexcept = default;

OSM::StringKeyRegistryBase::~StringKeyRegistryBase()
{
    std::for_each(m_pool.begin(), m_pool.end(), free);
}

const char* OSM::StringKeyRegistryBase::makeKeyInternal(const char *name, std::size_t len, OSM::StringMemory memOpt)
{
    const auto it = std::lower_bound(m_registry.begin(), m_registry.end(), name, [len](const char *lhs, const char *rhs) {
        return std::strncmp(lhs, rhs, len) < 0;
    });
    if (it == m_registry.end() || std::strncmp((*it), name, len) != 0 || std::strlen(*it) != len) {
        if (memOpt == OSM::StringMemory::Transient) {
#ifndef _MSC_VER
            auto s = strndup(name, len);
#else
            auto s = static_cast<char*>(malloc(len + 1));
            std::strncpy(s, name, len);
            s[len] = '\0';
#endif
            m_pool.push_back(s);
            name = s;
        }
        m_registry.insert(it, name);
        return name;
    }
    return (*it);
}

const char* OSM::StringKeyRegistryBase::keyInternal(const char *name) const
{
    const auto it = std::lower_bound(m_registry.begin(), m_registry.end(), name, [](const char *lhs, const char *rhs) {
        return std::strcmp(lhs, rhs) < 0;
    });
    if (it == m_registry.end() || std::strcmp((*it), name) != 0) {
        return {};
    }
    return (*it);
}
