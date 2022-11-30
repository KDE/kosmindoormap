/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssrule_p.h"
#include "mapcssresult_p.h"
#include "mapcssstate_p.h"

#include <QDebug>
#include <QIODevice>

#include <cmath>

using namespace KOSMIndoorMap;

MapCSSRule::MapCSSRule() = default;
MapCSSRule::~MapCSSRule() = default;

void MapCSSRule::compile(const OSM::DataSet &dataSet)
{
    m_selector->compile(dataSet);
    for (const auto &decl : m_declarations) {
        decl->compile(dataSet);
    }
}

void MapCSSRule::evaluate(const MapCSSState &state, MapCSSResult &result) const
{
    // TODO how do we deal with chained selectors here??
    m_selector->matches(state, result, [this](auto &result, auto layer) {
        for (const auto &decl : m_declarations) {
            switch (decl->type()) {
                case MapCSSDeclaration::PropertyDeclaration:
                    result[layer].addDeclaration(decl.get());
                    break;
                case MapCSSDeclaration::ClassDeclaration:
                    result[layer].addClass(decl->classSelectorKey());
                    break;
                case MapCSSDeclaration::TagDeclaration:
                    if (!std::isnan(decl->doubleValue())) {
                        result[layer].setTag(OSM::Tag{decl->tagKey(), QByteArray::number(decl->doubleValue())});
                    } else {
                        result[layer].setTag(OSM::Tag{decl->tagKey(), decl->stringValue().toUtf8()});
                    }
                    break;
            }
        }
    });
}

void MapCSSRule::evaluateCanvas(const MapCSSState &state, MapCSSResult &result) const
{
    if (!m_selector->matchesCanvas(state)) {
        return;
    }

    for (const auto &decl : m_declarations) {
        if (decl->type() == MapCSSDeclaration::PropertyDeclaration) {
            result[{}].addDeclaration(decl.get());
        }
    }
}

void MapCSSRule::write(QIODevice *out) const
{
    m_selector->write(out);
    out->write("\n{\n");
    for (const auto &decl : m_declarations) {
        decl->write(out);
    }
    out->write("}\n\n");
}

void MapCSSRule::setSelector(MapCSSSelector *selector)
{
    m_selector.reset(selector);
}

void MapCSSRule::addDeclaration(MapCSSDeclaration *decl)
{
    std::unique_ptr<MapCSSDeclaration> declPtr(decl);
    if (declPtr->isValid()) {
        m_declarations.push_back(std::move(declPtr));
    }
}
