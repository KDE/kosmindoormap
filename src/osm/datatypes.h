/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_DATATYPES_H
#define OSM_DATATYPES_H

#include "kosm_export.h"
#include "internal.h"
#include "stringpool.h"

#include <QByteArray>
#include <QDebug>
#include <QLocale>
#include <QString>

#include <cstdint>
#include <cstring>
#include <vector>

/** Low-level types and functions to work with raw OSM data as efficiently as possible. */
namespace OSM {

class DataSet;
class Member;

/** OSM element identifier. */
typedef int64_t Id;

/** Coordinate, stored as 1e7 * degree to avoid floating point precision issues,
 *  and offset to unsigned values to make the z-order curve work.
 *  Can be in an invalid state with coordinates out of range, see isValid().
 *  @see https://en.wikipedia.org/wiki/Z-order_curve for the z-order curve stuff
 */
class Coordinate {
public:
    Coordinate() = default;
    explicit constexpr Coordinate(double lat, double lon)
        : latitude((lat + 90.0) * 10'000'000)
        , longitude((lon + 180.0) * 10'000'000)
    {}
    explicit constexpr Coordinate(uint32_t lat, uint32_t lon)
        : latitude(lat)
        , longitude(lon)
    {}

    /** Create a coordinate from a z-order curve index. */
    explicit constexpr Coordinate(uint64_t z)
        : latitude(0)
        , longitude(0)
    {
        for (int i = 0; i < 32; ++i) {
            latitude += (z & (1ull << (i * 2))) >> i;
            longitude += (z & (1ull << (1 + i * 2))) >> (i + 1);
        }
    }

    constexpr inline bool isValid() const
    {
        return latitude != std::numeric_limits<uint32_t>::max() && longitude != std::numeric_limits<uint32_t>::max();
    }
    constexpr inline bool operator==(Coordinate other) const
    {
        return latitude == other.latitude && longitude == other.longitude;
    }

    /** Z-order curve value for this coordinate. */
    constexpr inline uint64_t z() const
    {
        uint64_t z = 0;
        for (int i = 0; i < 32; ++i) {
            z += ((uint64_t)latitude & (1 << i)) << i;
            z += ((uint64_t)longitude & (1 << i)) << (i + 1);
        }
        return z;
    }

    constexpr inline double latF() const
    {
        return (latitude / 10'000'000.0) - 90.0;
    }
    constexpr inline double lonF() const
    {
        return (longitude / 10'000'000.0) - 180.0;
    }

    uint32_t latitude = std::numeric_limits<uint32_t>::max();
    uint32_t longitude = std::numeric_limits<uint32_t>::max();
};


/** Bounding box, ie. a pair of coordinates. */
class BoundingBox {
public:
    constexpr BoundingBox() = default;
    constexpr inline BoundingBox(Coordinate c1, Coordinate c2)
        : min(c1)
        , max(c2)
    {}
    constexpr inline bool isValid() const
    {
        return min.isValid() && max.isValid();
    }
    constexpr inline bool operator==(BoundingBox other) const
    {
        return min == other.min && max == other.max;
    }

    constexpr inline uint32_t width() const
    {
        return max.longitude - min.longitude;
    }
    constexpr inline uint32_t height() const
    {
        return max.latitude - min.latitude;
    }

    constexpr inline double widthF() const
    {
        return width() / 10'000'000.0;
    }
    constexpr inline double heightF() const
    {
        return height() / 10'000'000.0;
    }

    constexpr inline Coordinate center() const
    {
        return Coordinate(min.latitude + height() / 2, min.longitude + width() / 2);
    }

    Coordinate min;
    Coordinate max;
};

constexpr inline BoundingBox unite(BoundingBox bbox1, BoundingBox bbox2)
{
    if (!bbox1.isValid()) {
        return bbox2;
    }
    if (!bbox2.isValid()) {
        return bbox1;
    }
    BoundingBox ret;
    ret.min.latitude = std::min(bbox1.min.latitude, bbox2.min.latitude);
    ret.min.longitude = std::min(bbox1.min.longitude, bbox2.min.longitude);
    ret.max.latitude = std::max(bbox1.max.latitude, bbox2.max.latitude);
    ret.max.longitude = std::max(bbox1.max.longitude, bbox2.max.longitude);
    return ret;
}

constexpr inline bool intersects(BoundingBox bbox1, BoundingBox bbox2)
{
    return !(bbox2.min.latitude > bbox1.max.latitude || bbox2.max.latitude < bbox1.min.latitude
        || bbox2.min.longitude > bbox1.max.longitude || bbox2.max.longitude < bbox1.min.longitude);
}

constexpr inline bool contains(BoundingBox bbox, Coordinate coord)
{
    return bbox.min.latitude <= coord.latitude && bbox.max.latitude >= coord.latitude
        && bbox.min.longitude <= coord.longitude && bbox.max.longitude >= coord.longitude;
}

constexpr inline uint32_t latitudeDistance(BoundingBox bbox1, BoundingBox bbox2)
{
    return bbox1.max.latitude < bbox2.min.latitude ? bbox2.min.latitude - bbox1.max.latitude : bbox1.min.latitude - bbox2.max.latitude;
}

constexpr inline uint32_t longitudeDifference(BoundingBox bbox1, BoundingBox bbox2)
{
    return bbox1.max.longitude < bbox2.min.longitude ? bbox2.min.longitude - bbox1.max.longitude : bbox1.min.longitude - bbox2.max.longitude;
}

/** A key of an OSM tag.
 *  See DataSet::tagKey().
 */
class TagKey : public StringKey {};

/** An OSM element tag. */
class Tag {
public:
    inline constexpr bool operator<(const Tag &other) const { return key < other.key; }

    TagKey key;
    QByteArray value;
};

inline constexpr bool operator<(const Tag &lhs, TagKey rhs) { return lhs.key < rhs; }
inline constexpr bool operator<(TagKey lhs, const Tag &rhs) { return lhs < rhs.key; }

/** An OSM node. */
class KOSM_EXPORT Node {
public:
    explicit Node() = default;
    Node(const Node&) = default;
    Node(Node &&other)
    {
        *this = std::move(other);
    }
    Node& operator=(const Node &other) = default;
    Node& operator=(Node &&other)
    {
        id = std::move(other.id);
        coordinate = std::move(other.coordinate);
        std::swap(tags, other.tags);
        return *this;
    }

    constexpr inline bool operator<(const Node &other) const { return id < other.id; }

    QString url() const;

    Id id;
    Coordinate coordinate;
    std::vector<Tag> tags;
};

/** An OSM way. */
class KOSM_EXPORT Way {
public:
    explicit Way() = default;
    Way(const Way&) = default;
    Way(Way &&other)
    {
        *this = std::move(other);
    }
    Way& operator=(const Way &other) = default;
    Way& operator=(Way &&other)
    {
        id = std::move(other.id);
        bbox = std::move(other.bbox);
        std::swap(nodes, other.nodes);
        std::swap(tags, other.tags);
        return *this;
    }

    constexpr inline bool operator<(const Way &other) const { return id < other.id; }

    bool isClosed() const;

    QString url() const;

    Id id;
    mutable BoundingBox bbox;
    std::vector<Id> nodes;
    std::vector<Tag> tags;
};

/** Element type. */
enum class Type : uint8_t {
    Null,
    Node,
    Way,
    Relation
};

/** A relation role name key.
 *  See DataSet::role().
 */
class Role : public StringKey
{
public:
    constexpr inline Role() = default;
private:
    friend class Member;
    explicit constexpr inline Role(const char *keyData) : StringKey(keyData) {}
};

/** A member in a relation. */
class Member {
public:
    inline bool operator==(const Member &other) const { return id == other.id && m_roleAndType == other.m_roleAndType; }

    Id id;

    constexpr inline Role role() const
    {
        return Role(m_roleAndType.get());
    }
    constexpr inline void setRole(Role role)
    {
        m_roleAndType.set(role.name());
    }

    constexpr inline Type type() const
    {
        return static_cast<Type>(m_roleAndType.tag());
    }
    constexpr inline void setType(Type type)
    {
        m_roleAndType.setTag(static_cast<uint8_t>(type));
    }

private:
    Internal::TaggedPointer<const char> m_roleAndType;
};

/** An OSM relation. */
class KOSM_EXPORT Relation {
public:
    explicit Relation() = default;
    Relation(const Relation&) = default;
    Relation(Relation &&other)
    {
        *this = std::move(other);
    }
    Relation& operator=(const Relation &other) = default;
    Relation& operator=(Relation &&other)
    {
        id = std::move(other.id);
        bbox = std::move(other.bbox);
        std::swap(members, other.members);
        std::swap(tags, other.tags);
        return *this;
    }

    constexpr inline bool operator<(const Relation &other) const { return id < other.id; }

    QString url() const;

    Id id;
    mutable BoundingBox bbox;
    std::vector<Member> members;
    std::vector<Tag> tags;
};

/** A set of nodes, ways and relations. */
class KOSM_EXPORT DataSet {
public:
    explicit DataSet();
    DataSet(const DataSet&) = delete;
    DataSet(DataSet &&other);
    ~DataSet();

    DataSet& operator=(const DataSet&) = delete;
    DataSet& operator=(DataSet &&);

    /** Find a node by its id.
     *  @returns @c nullptr if the node doesn't exist.
     */
    const Node* node(Id id) const;

    /** Find a way by its id.
     *  @returns @c nullptr if the way doesn't exist.
     */
    const Way* way(Id id) const;
    Way* way(Id id);

    /** Find a relation by its id.
     *  @returns @c nullptr if the relation doesn't exist.
     */
    const Relation* relation(Id id) const;

    void addNode(Node &&node);
    void addWay(Way &&way);
    void addRelation(Relation &&rel);

    /** Look up a tag key for the given tag name, if it exists.
     *  If no key exists, an empty/invalid/null key is returned.
     *  Use this for tag lookup, not for creating/adding tags.
     */
    TagKey tagKey(const char *keyName) const;

    /** Create a tag key for the given tag name. If none exist yet a new one is created.
     *  Use this for creating tags, not for lookup, prefer tagKey() for that.
     *  @param keyMemOpt specifies whether @p keyName is persisent for the lifetime of this
     *  instance and thus can be used without requiring a copy. If the memory is transient
     *  the string is copied if needed, and released in the DataSet destructor.
     */
    TagKey makeTagKey(const char *keyName, StringMemory keyMemOpt = StringMemory::Transient);

    /** Looks up a role name key.
     *  @see tagKey()
     */
    Role role(const char *roleName) const;
    /** Creates a role name key.
     *  @see makeTagKey()
     */
    Role makeRole(const char *roleName, StringMemory memOpt = StringMemory::Transient);

    /** Create a unique id for internal use (ie. one that will not clash with official OSM ids). */
    Id nextInternalId() const;

    std::vector<Node> nodes;
    std::vector<Way> ways;
    std::vector<Relation> relations;

private:
    template <typename T> T stringKey(const char *name, const std::vector<T> &registry) const;
    template <typename T> T makeStringKey(const char *name, StringMemory memOpt, std::vector<T> &registry);

    StringKeyRegistry<TagKey> m_tagKeyRegistry;
    StringKeyRegistry<Role> m_roleRegistry;
};

/** Returns the tag value for @p key of @p elem. */
template <typename Elem>
inline QByteArray tagValue(const Elem& elem, TagKey key)
{
    const auto it = std::lower_bound(elem.tags.begin(), elem.tags.end(), key);
    if (it != elem.tags.end() && (*it).key == key) {
        return (*it).value;
    }
    return {};
}

/** Returns the tag value for key name @p keyName of @p elem.
 *  @warning This is slow due to doing a linear search and string comparissons.
 *  Where possible avoid this in favor of tagValue().
 */
template <typename Elem>
inline QByteArray tagValue(const Elem& elem, const char *keyName)
{
    const auto it = std::find_if(elem.tags.begin(), elem.tags.end(), [keyName](const auto &tag) { return std::strcmp(tag.key.name(), keyName) == 0; });
    if (it != elem.tags.end()) {
        return (*it).value;
    }
    return {};
}

/** Returns the localized version of the tag value for key name @p keyName of @p elem.
 *  @warning This is slow due to doing a linear search and string comparissons.
 */
template <typename Elem>
inline QByteArray tagValue(const Elem& elem, const char *keyName, const QLocale &locale)
{
    QByteArray key(keyName);
    key.push_back(':');
    const auto baseLen = key.size();
    for (const auto &lang : locale.uiLanguages()) {
        key.resize(baseLen);
        key.append(lang.toUtf8());
        const auto it = std::find_if(elem.tags.begin(), elem.tags.end(), [key](const auto &tag) { return std::strcmp(tag.key.name(), key.constData()) == 0; });
        if (it != elem.tags.end()) {
            return (*it).value;
        }

        const auto idx = lang.indexOf(QLatin1Char('-'));
        if (idx > 0) {
            key.resize(baseLen);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            key.append(lang.leftRef(idx).toUtf8());
#else
            key.append(QStringView(lang).left(idx).toUtf8());
#endif
            const auto it = std::find_if(elem.tags.begin(), elem.tags.end(), [key](const auto &tag) { return std::strcmp(tag.key.name(), key.constData()) == 0; });
            if (it != elem.tags.end()) {
                return (*it).value;
            }
        }
    }

    // fall back to generic value, if present
    const auto v = tagValue(elem, keyName);
    if (!v.isEmpty()) {
        return v;
    }

    // check if there is at least one in any language we can use
    key.resize(baseLen);
    const auto it = std::find_if(elem.tags.begin(), elem.tags.end(), [key, baseLen](const auto &tag) {
        return std::strncmp(tag.key.name(), key.constData(), baseLen) == 0
            && std::strlen(tag.key.name()) == key.size() + 2; // primitive check whether this is a plausible language rather than some other qualifier
    });
    if (it != elem.tags.end()) {
        return (*it).value;
    }
    return {};
}

/** Inserts a new tag, or replaces an existing one with the same key. */
template <typename Elem>
inline void setTag(Elem &elem, Tag &&tag)
{
    const auto it = std::lower_bound(elem.tags.begin(), elem.tags.end(), tag);
    if (it == elem.tags.end() || (*it).key != tag.key) {
        elem.tags.insert(it, std::move(tag));
    } else {
        (*it) = std::move(tag);
    }
}

/** Inserts a new tag, or updates an existing one. */
template <typename Elem>
inline void setTagValue(Elem &elem, TagKey key, const QByteArray &value)
{
    Tag tag{ key, value };
    setTag(elem, std::move(tag));
}

/** Removes a tag from the given element. */
template <typename Elem>
inline void removeTag(Elem &elem, TagKey key)
{
    const auto it = std::lower_bound(elem.tags.begin(), elem.tags.end(), key);
    if (it != elem.tags.end() && (*it).key == key) {
        elem.tags.erase(it);
    }
}

template <typename Elem>
inline bool operator<(const Elem &elem, Id id)
{
    return elem.id < id;
}

}

KOSM_EXPORT QDebug operator<<(QDebug debug, OSM::Coordinate coord);
KOSM_EXPORT QDebug operator<<(QDebug debug, OSM::BoundingBox bbox);

#endif // OSM_DATATYPES_H
