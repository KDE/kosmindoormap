/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSM_STRINGPOOL_H
#define KOSM_STRINGPOOL_H

#include "kosm_export.h"

#include <cstring>
#include <vector>

namespace OSM {

enum class StringMemory { Persistent, Transient };

/** @internal */
class KOSM_EXPORT StringKeyRegistryBase
{
protected:
    explicit StringKeyRegistryBase();
    StringKeyRegistryBase(StringKeyRegistryBase&&) noexcept;
    StringKeyRegistryBase& operator=(StringKeyRegistryBase&&) noexcept;
    ~StringKeyRegistryBase();

    [[nodiscard]] const char* makeKeyInternal(const char *name, std::size_t len, StringMemory memOpt);
    [[nodiscard]] const char* keyInternal(const char *name) const;

    std::vector<char*> m_pool;
    std::vector<const char*> m_registry;
};

/** Registry of unique string keys.
 *  @tparam T Sub-classes of StringKey, to have a compile-time check against comparing keys from different pools.
 */
template <typename T>
class StringKeyRegistry : protected StringKeyRegistryBase
{
public:
    explicit StringKeyRegistry() = default;
    StringKeyRegistry(const StringKeyRegistry&) = delete;
    StringKeyRegistry(StringKeyRegistry&&) = default;
    ~StringKeyRegistry() = default;
    StringKeyRegistry& operator=(const StringKeyRegistry&) = delete;
    StringKeyRegistry& operator=(StringKeyRegistry&&) = default;

    /** Add a new string to the registry if needed, or returns an existing one if already present. */
    inline T makeKey(const char *name, StringMemory memOpt)
    {
        return makeKey(name, std::strlen(name), memOpt);
    }
    inline T makeKey(const char *name, std::size_t len, StringMemory memOpt)
    {
        T key;
        key.key = makeKeyInternal(name, len, memOpt);
        return key;
    }

    /** Looks up an existing key, if that doesn't exist an null key is returned. */
    inline T key(const char *name) const
    {
        T key;
        key.key = keyInternal(name);
        return key;
    }
};

/** Base class for unique string keys. */
class StringKey
{
public:
    constexpr inline StringKey() = default;
    constexpr inline const char* name() const { return key; }
    constexpr inline bool isNull() const { return !key; }

    // yes, pointer compare is enough here
    inline constexpr bool operator<(StringKey other) const { return key < other.key; }
    inline constexpr bool operator==(StringKey other) const { return key == other.key; }
    inline constexpr bool operator!=(StringKey other) const { return key != other.key; }

protected:
    explicit constexpr inline StringKey(const char *keyData) : key(keyData) {}

private:
    template <typename T> friend class StringKeyRegistry;
    const char* key = nullptr;
};

}

#endif // KOSM_STRINGPOOL_H
