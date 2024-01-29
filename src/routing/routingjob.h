/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_ROUTINGJOB_H
#define KOSMINDOORROUTING_ROUTINGJOB_H

#include "kosmindoorrouting_export.h"
#include "navmeshtransform.h"

#include <QObject>

#include <memory>

namespace KOSMIndoorRouting {

class NavMesh;
class Route;
class RoutingJobPrivate;
class RoutingProfile;

/** Job for running a routing query on a compiled NavMesh instance. */
class KOSMINDOORROUTING_EXPORT RoutingJob : public QObject
{
    Q_OBJECT
public:
    explicit RoutingJob(QObject *parent = nullptr);
    ~RoutingJob();

    void setNavMesh(const NavMesh &navMesh);

    /// start/end position in nav coordinates
    void setStart(rcVec3 startPos);
    void setEnd(rcVec3 endPost);

    /// routing profile
    void setRoutingProfile(const RoutingProfile &profile);

    void start();

    [[nodiscard]] Route route() const;

Q_SIGNALS:
    void finished();

private:
    std::unique_ptr<RoutingJobPrivate> d;
};
}

#endif
