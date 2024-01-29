/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routingprofile.h"
#include "routingarea.h"

#include <array>

namespace KOSMIndoorRouting {
class RoutingProfilePrivate : public QSharedData
{
public:
    AreaFlags flags = ~AreaFlags{};
    std::array<float, AREA_TYPE_COUNT> costs;
};
}

using namespace KOSMIndoorRouting;

RoutingProfile::RoutingProfile()
    : d(new RoutingProfilePrivate)
{
    // TODO support for multiple pre-defined profiles
    d->costs = {
        1.0f,
        5.0f, // stairs
        10.0f, // elevators
        2.5f, // escalator
        1.0f, // moving walkways
        1.5f, // tactile paving
        10.0f, // street crossing
        4.0f, // ramp
        1.5f, // walkable area
    };
}

RoutingProfile::RoutingProfile(const RoutingProfile&) = default;
RoutingProfile::RoutingProfile(RoutingProfile&&) noexcept = default;
RoutingProfile::~RoutingProfile() = default;
RoutingProfile& RoutingProfile::operator=(const RoutingProfile&) = default;
RoutingProfile& RoutingProfile::operator=(RoutingProfile&&) noexcept = default;

bool RoutingProfile::operator==(const RoutingProfile &other) const
{
    return d->flags == other.d->flags && std::equal(d->costs.begin(), d->costs.end(), other.d->costs.begin());
}

AreaFlags RoutingProfile::flags() const
{
    return d->flags;
}

void RoutingProfile::setFlags(AreaFlags flags)
{
    d.detach();
    d->flags = flags;
}

float RoutingProfile::cost(KOSMIndoorRouting::AreaType area) const
{
    return area == AreaType::Walkable ? d->costs[d->costs.size() - 1] : d->costs[qToUnderlying(area)];
}

void RoutingProfile::setCost(KOSMIndoorRouting::AreaType area, float cost)
{
    d.detach();
    if (area == AreaType::Walkable) {
        d->costs[d->costs.size() - 1] = std::max(1.0f, cost);
    } else {
        d->costs[qToUnderlying(area)] = std::max(1.0f, cost);
    }
}

#include "moc_routingprofile.cpp"
