/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_ELEMENT_H
#define OSM_ELEMENT_H

#include "kosm_export.h"

#include "datatypes.h"
#include "internal.h"

#include <cstdint>

namespace OSM {
class Languages;

/** A reference to any of OSM::Node/OSM::Way/OSM::Relation.
 *  Lifetime of the referenced object needs to extend beyond the lifetime of this.
 */
class KOSM_EXPORT Element
{
public:
    inline constexpr Element() : m_elem(nullptr, static_cast<uint8_t>(Type::Null)) {}
    inline Element(const Node *node) : m_elem(node, static_cast<uint8_t>(Type::Node)) {}
    inline Element(const Way *way) : m_elem(way, static_cast<uint8_t>(Type::Way)) {}
    inline Element(const Relation *relation) : m_elem(relation, static_cast<uint8_t>(Type::Relation)) {}

    [[nodiscard]] inline bool operator==(Element other) const { return m_elem == other.m_elem; }
    [[nodiscard]] inline bool operator!=(Element other) const { return m_elem != other.m_elem; }
    [[nodiscard]] inline bool operator<(Element other) const { return m_elem < other.m_elem; }
    [[nodiscard]] explicit inline operator bool() const { return type() != OSM::Type::Null; }

    [[nodiscard]] inline Type type() const { return static_cast<Type>(m_elem.tag()); }
    [[nodiscard]] inline const Node* node() const { return static_cast<const Node*>(m_elem.get()); }
    [[nodiscard]] inline const Way* way() const { return static_cast<const Way*>(m_elem.get()); }
    [[nodiscard]] inline const Relation* relation() const { return static_cast<const Relation*>(m_elem.get()); }
    [[nodiscard]] Id id() const;

    [[nodiscard]] Coordinate center() const;
    [[nodiscard]] BoundingBox boundingBox() const;
    [[nodiscard]] QByteArray tagValue(TagKey key) const;
    [[nodiscard]] QByteArray tagValue(const char *keyName) const;
    [[nodiscard]] QByteArray tagValue(const OSM::Languages &languages, const char *keyName) const;
    /** Returns the value of the first non-empty tag.
     *  Both OSM::TagKey (fast) and const char* (slow) keys are accepted.
     */
    template <typename K, typename ...Args> [[nodiscard]] QByteArray tagValue(K key, Args... args) const;
    template <typename K, typename ...Args> [[nodiscard]] QByteArray tagValue(const OSM::Languages &languages, K key, Args... args) const;
    /** Returns whether or not this element has any tags set. */
    [[nodiscard]] inline bool hasTags() const { return std::distance(tagsBegin(), tagsEnd()) > 0; }
    /** Returns @c true if this element has a tag with key @p key. */
    [[nodiscard]] bool hasTag(TagKey key) const;

    [[nodiscard]] std::vector<Tag>::const_iterator tagsBegin() const;
    [[nodiscard]] std::vector<Tag>::const_iterator tagsEnd() const;
    [[nodiscard]] QString url() const;

    /** Returns all nodes belonging to the outer path of this element.
     *  In the simplest case that's a single closed polygon, but it can also be a sequence of multiple
     *  closed loop polygons, or a polyline.
     */
    [[nodiscard]] std::vector<const Node*> outerPath(const DataSet &dataSet) const;

    /** Recompute the bounding box of this element.
     *  We usually assume those to be provided by Overpass/osmconvert, but there seem to be cases where those
     *  aren't reliable.
     */
    void recomputeBoundingBox(const DataSet &dataSet);

private:
    Internal::TaggedPointer<const void> m_elem;
};

template <typename K, typename ...Args>
QByteArray Element::tagValue(K k, Args... args) const
{
    const auto v = tagValue(k);
    if (!v.isEmpty()) {
        return v;
    }
    return tagValue(args...);
}

template <typename K, typename ...Args>
QByteArray Element::tagValue(const OSM::Languages &languages, K key, Args... args) const
{
    const auto v = tagValue(languages, key);
    if (!v.isEmpty()) {
        return v;
    }
    return tagValue(languages, args...);
}


/** A std::unique_ptr-like object for OSM element types. */
class KOSM_EXPORT UniqueElement
{
public:
    explicit inline UniqueElement() = default;
    explicit inline UniqueElement(Node *node) : m_element(node) {}
    explicit inline UniqueElement(Way *way) : m_element(way) {}
    explicit inline UniqueElement(Relation *rel) : m_element(rel) {}

    UniqueElement(const UniqueElement&) = delete;
    inline UniqueElement(UniqueElement &&other) noexcept {
        std::swap(m_element, other.m_element);
    }

    ~UniqueElement();

    UniqueElement& operator=(const UniqueElement&) = delete;
    UniqueElement& operator=(UniqueElement &&other) noexcept {
        std::swap(m_element, other.m_element);
        return *this;
    }

    [[nodiscard]] explicit inline operator bool() const { return m_element.type() != OSM::Type::Null; }

    [[nodiscard]] constexpr inline Element element() const { return m_element; }
    [[nodiscard]] constexpr inline operator Element() const { return m_element; }

    [[nodiscard]] inline Node* node() const { return const_cast<Node*>(m_element.node()); }
    [[nodiscard]] inline Way* way() const { return const_cast<Way*>(m_element.way()); }
    [[nodiscard]] inline Relation* relation() const { return const_cast<Relation*>(m_element.relation()); }

    void setId(Id id);
    void setTagValue(TagKey key, QByteArray &&value);
    void removeTag(TagKey key);

private:
    Element m_element;
};

/** Creates a copy of @p element. */
KOSM_EXPORT UniqueElement copy_element(Element e);

/** Utility function similar to SQL COALESCE for OSM::Element, ie. this returns the first non-null element passed as argument. */
[[nodiscard]] constexpr Element coalesce(Element e) { return e; }
template <typename ...Args>
[[nodiscard]] constexpr Element coalesce(Element e, Args... args) { return e ? e : coalesce(args...); }

/** Lookup element of given @p type and @p id in @p dataSet. */
inline Element lookupElement(const DataSet &dataSet, OSM::Type type, OSM::Id id)
{
    switch (type) {
        case Type::Null:
            break;
        case Type::Node:
            if (auto node = dataSet.node(id)) {
                return {node};
            }
            break;
        case Type::Way:
            if (auto way = dataSet.way(id)) {
                return {way};
            }
            break;
        case Type::Relation:
            if (auto rel = dataSet.relation(id)) {
                return {rel};
            }
            break;
    }
    return {};
}

enum ForeachFlag : uint8_t {
    IncludeRelations = 1,
    IncludeWays = 2,
    IncludeNodes = 4,
    IterateAll = IncludeRelations | IncludeWays | IncludeNodes,
};

template <typename Func>
inline void for_each(const DataSet &dataSet, Func func, uint8_t flags = IterateAll)
{
    if (flags & IncludeRelations) {
        for (const auto &rel : dataSet.relations) {
            func(Element(&rel));
        }
    }
    if (flags & IncludeWays) {
        for (const auto &way : dataSet.ways) {
            func(Element(&way));
        }
    }
    if (flags & IncludeNodes) {
        for (const auto &node : dataSet.nodes) {
            func(Element(&node));
        }
    }
}

template <typename Func>
inline void for_each_node(const DataSet &dataSet, const Way &way, Func func)
{
    for (auto nodeId : way.nodes) {
        if (auto node = dataSet.node(nodeId)) {
            func(*node);
        }
    }
}

template <typename Func>
inline void for_each_member(const DataSet &dataSet, const Relation &rel, Func func)
{
    for (const auto &mem : rel.members) {
        if (auto elem = lookupElement(dataSet, mem.type(), mem.id); elem.type() != OSM::Type::Null) {
            func(elem);
        }
    }
}

}

template<>
struct std::hash<OSM::Element>
{
    std::size_t operator()(OSM::Element e) const noexcept
    {
        std::size_t h1 = std::hash<OSM::Id>{}(e.id());
        std::size_t h2 = std::hash<int>{}(qToUnderlying(e.type()));
        return h1 ^ (h2 << 1);
    }
};

Q_DECLARE_METATYPE(OSM::Element)

#endif // OSM_ELEMENT_H
