/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "route.h"

namespace KOSMIndoorRouting {
class RoutePrivate : public QSharedData {
public:
    std::vector<RouteStep> m_steps;
};
}

using namespace KOSMIndoorRouting;

Route::Route()
    : d(new RoutePrivate)
{
}

Route::Route(const Route &) = default;
Route::Route(Route &&) noexcept = default;
Route::~Route() = default;
Route& Route::operator=(const Route &) = default;
Route& Route::operator=(Route &&) noexcept = default;

const std::vector<RouteStep>& Route::steps() const
{
    return d->m_steps;
}

void Route::setSteps(std::vector<RouteStep> &&steps)
{
    d.detach();
    d->m_steps = std::move(steps);
}
