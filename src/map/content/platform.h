/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_PLATFORM_H
#define KOSMINDOORMAP_PLATFORM_H

#include "kosmindoormap_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>
#include <QStringList>

#include <vector>

namespace OSM {
class Coordinate;
class DataSet;
class Element;
}

namespace KOSMIndoorMap {

class PlatformSectionPrivate;

/** A railway platform section. */
class KOSMINDOORMAP_EXPORT PlatformSection
{
public:
    explicit PlatformSection();
    PlatformSection(const PlatformSection&);
    PlatformSection(PlatformSection&&);
    ~PlatformSection();
    PlatformSection& operator=(const PlatformSection&);
    PlatformSection& operator=(PlatformSection&&);

    /** Platform section has enough data to work with. */
    bool isValid() const;

    /** Platform section name. */
    QString name() const;
    void setName(const QString &name);
    /** Platform section position. */
    OSM::Element position() const;
    void setPosition(const OSM::Element &position);

private:
    QExplicitlySharedDataPointer<PlatformSectionPrivate> d;
};

class PlatformPrivate;

/** A railway platform/track. */
class KOSMINDOORMAP_EXPORT Platform {
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    Q_PROPERTY(QString ifopt READ ifopt WRITE setIfopt)
public:
    explicit Platform();
    Platform(const Platform&);
    Platform(Platform&&);
    ~Platform();
    Platform& operator=(const Platform&);
    Platform& operator=(Platform&&);

    /** Platform has enough data to work with. */
    bool isValid() const;

    /** User-visible name of the platform. */
    QString name() const;
    void setName(const QString &name);

    /** Floor level. */
    int level() const;
    bool hasLevel() const;
    void setLevel(int level);

    /** A singular position for this platform (typically the stop position).
     *  This can be useful for positining views or labels.
     */
    OSM::Coordinate position() const;
    OSM::Element stopPoint() const;
    void setStopPoint(OSM::Element stop);

    /** The platform edge path. */
    OSM::Element edge() const;
    void setEdge(OSM::Element edge);

    /** The platform area.
     *  This is often shared between multiple tracks.
     */
    OSM::Element area() const;
    void setArea(OSM::Element area);

    /** The (railway) track this platform is serving. */
    const std::vector<OSM::Element>& track() const;
    void setTrack(std::vector<OSM::Element> &&track);
    std::vector<OSM::Element>&& takeTrack();

    /** Platform sections. */
    const std::vector<PlatformSection>& sections() const;
    void setSections(std::vector<PlatformSection> &&sections);
    std::vector<PlatformSection>&& takeSections();

    /** Transportation mode served by a platform. */
    enum Mode {
        Unknown,
        Rail,
        LightRail,
        Subway,
        Tram,
        Monorail,
        Bus,
    };
    Q_ENUM(Mode)
    Mode mode() const;
    void setMode(Mode mode);

    /** IFOPT identifier */
    QString ifopt() const;
    void setIfopt(const QString &ifopt);

    /** Names of public transport lines stopping at this platform. */
    QStringList lines() const;
    void setLines(QStringList &&lines);
    QStringList&& takeLines();

    /** Checks if two instances refer to the same platform. */
    static bool isSame(const Platform &lhs, const Platform &rhs, const OSM::DataSet &dataSet);
    /** Merge two platform objects. */
    static Platform merge(const Platform &lhs, const Platform &rhs, const OSM::DataSet &dataSet);

    /** Checks if @p name is a plausible name for a platform. */
    static bool isPlausibleName(const QString &name);
    /** Returns the preferred platform name given two possible alternatives. */
    static QString preferredName(const QString &lhs, const QString &rhs);

private:
    friend class PlatformPrivate;
    QExplicitlySharedDataPointer<PlatformPrivate> d;
};

}

Q_DECLARE_METATYPE(KOSMIndoorMap::Platform)

#endif // KOSMINDOORMAP_PLATFORM_H
