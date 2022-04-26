/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmaddress.h"

#include <KCountry>
#include <KCountrySubdivision>

using namespace KOSMIndoorMap;

OSMAddress::OSMAddress() = default;
OSMAddress::OSMAddress(OSM::Element elem)
    : m_element(elem)
{
}

OSMAddress::~OSMAddress() = default;

QString OSMAddress::street() const
{
    return QString::fromUtf8(m_element.tagValue("addr:street", "contact:street", "addr:housename"));
}

QString OSMAddress::houseNumber() const
{
    return QString::fromUtf8(m_element.tagValue("addr:housenumber", "contact:housenumber"));
}

QString OSMAddress::postalCode() const
{
    return QString::fromUtf8(m_element.tagValue("addr:postcode", "contact:postcode"));
}

QString OSMAddress::city() const
{
    return QString::fromUtf8(m_element.tagValue("addr:city", "contact:city"));
}

QString OSMAddress::state() const
{
    const auto state = QString::fromUtf8(m_element.tagValue("addr:state"));
    if (!state.isEmpty()) {
        return state;
    }

    const auto s = KCountrySubdivision::fromLocation(m_element.center().latF(), m_element.center().lonF());
    return s.isValid() ? s.code().mid(3) : QString();
}

QString OSMAddress::country() const
{
    const auto country = QString::fromUtf8(m_element.tagValue("addr:country", "contact:country"));
    if (!country.isEmpty()) {
        return country;
    }

    const auto c = KCountry::fromLocation(m_element.center().latF(), m_element.center().lonF());
    return c.alpha2();
}
