/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_ROUTINGPROFILE_H
#define KOSMINDOORROUTING_ROUTINGPROFILE_H

#include "kosmindoorrouting_export.h"
#include "routingarea.h"

#include <QExplicitlySharedDataPointer>
#include <qobjectdefs.h>

namespace KOSMIndoorRouting {

class RoutingProfilePrivate;

/** Routing profile. That is, cost factors and area types to use/avoid
 *  for a RoutingJob run.
 */
class KOSMINDOORROUTING_EXPORT RoutingProfile
{
    Q_GADGET
    Q_PROPERTY(KOSMIndoorRouting::AreaFlags flags READ flags WRITE setFlags)
public:
    explicit RoutingProfile();
    ~RoutingProfile();
    RoutingProfile(const RoutingProfile&);
    RoutingProfile(RoutingProfile&&) noexcept;
    RoutingProfile& operator=(const RoutingProfile&);
    RoutingProfile& operator=(RoutingProfile&&) noexcept;

    [[nodiscard]] bool operator==(const RoutingProfile &other) const;

    /** Area types that should be included in the search. */
    [[nodiscard]] AreaFlags flags() const;
    void setFlags(AreaFlags flags);

    /** Cost factors (>= 1.0) for each area type that is included in the search. */
    Q_INVOKABLE [[nodiscard]] float cost(KOSMIndoorRouting::AreaType area) const;
    Q_INVOKABLE void setCost(KOSMIndoorRouting::AreaType area, float cost);

private:
    QExplicitlySharedDataPointer<RoutingProfilePrivate> d;
};
}

#endif
