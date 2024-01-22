/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_ROUTE_H
#define KOSMINDOORROUTING_ROUTE_H

#include "kosmindoorrouting_export.h"

#include <KOSM/Datatypes>

#include <QExplicitlySharedDataPointer>

#include <vector>

namespace KOSMIndoorRouting {

class RouteStep {
public:
    OSM::Coordinate coordinate;
    int floorLevel = 0;
    // TODO instructiosn/area types
};

class RoutePrivate;

class KOSMINDOORROUTING_EXPORT Route
{
public:
    explicit Route();
    Route(const Route &);
    Route(Route &&) noexcept;
    ~Route();
    Route& operator=(const Route &);
    Route& operator=(Route &&) noexcept;

    [[nodiscard]] const std::vector<RouteStep>& steps() const;
    void setSteps(std::vector<RouteStep> &&steps);

private:
    QExplicitlySharedDataPointer<RoutePrivate> d;
};

}

#endif
