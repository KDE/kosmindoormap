/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scenegraphdumper.h"

#include <KOSMIndoorMap/SceneGraph>
#include <KOSMIndoorMap/View>

#include <QDebug>
#include <QFile>
#include <QIODevice>

#include <cmath>

using namespace KOSMIndoorMap;

SceneGraphDumper::SceneGraphDumper() = default;
SceneGraphDumper::~SceneGraphDumper() = default;

void SceneGraphDumper::render(const SceneGraph &sg, View *view)
{
    QFile f(QStringLiteral("scenegraph.txt")); // TODO pass in externally
    f.open(QFile::WriteOnly);
    m_io = &f;

    m_view = view;
    for (const auto &layerOffsets : sg.layerOffsets()) {
        const auto layerBegin = sg.itemsBegin(layerOffsets);
        const auto layerEnd = sg.itemsEnd(layerOffsets);
        m_io->write("BEGIN LAYER ");
        m_io->write(QByteArray::number((*layerBegin).layer));
        m_io->write(" LEVEL ");
        m_io->write(QByteArray::number((*layerBegin).level));
        m_io->write("\n");

        // select elements currently in view
        m_renderBatch.clear();
        m_renderBatch.reserve(layerOffsets.second - layerOffsets.first);
        const QRectF screenRect(QPointF(0, 0), QSizeF(m_view->screenWidth(), m_view->screenHeight()));
        for (auto it = layerBegin; it != layerEnd; ++it) {
            if ((*it).payload->inSceneSpace() && m_view->viewport().intersects((*it).payload->boundingRect(view))) {
                m_renderBatch.push_back(&(*it));
            }
            if ((*it).payload->inHUDSpace()) {
                auto bbox = (*it).payload->boundingRect(view);
                bbox.moveCenter(m_view->mapSceneToScreen(bbox.center()));
                if (screenRect.intersects(bbox)) {
                    m_renderBatch.push_back(&(*it));
                }
            }
        }

        for (auto phase : {SceneGraphItemPayload::FillPhase, SceneGraphItemPayload::CasingPhase, SceneGraphItemPayload::StrokePhase, SceneGraphItemPayload::IconPhase, SceneGraphItemPayload::LabelPhase}) {
            beginPhase(phase);
            for (const auto sgItem : m_renderBatch) {
                const auto item = sgItem->payload.get();
                if ((item->renderPhases() & phase) == 0) {
                    continue;
                }

                m_io->write("  z");
                m_io->write(QByteArray::number(item->z));
                m_io->write(" ");
                if (dynamic_cast<PolygonItem*>(item)) {
                    m_io->write("polygon ");
                } else if (dynamic_cast<MultiPolygonItem*>(item)) {
                    m_io->write("multi-polygon ");
                } else if (dynamic_cast<PolylineItem*>(item)) {
                    m_io->write("line ");
                } else if (dynamic_cast<LabelItem*>(item)) {
                    m_io->write("label ");
                } else {
                    qCritical() << "Unsupported scene graph item!";
                }
                m_io->write(sgItem->element.url().toUtf8());
                m_io->write(" ");
                m_io->write(QByteArray::number((quint64)sgItem->element.boundingBox().width() * (quint64)sgItem->element.boundingBox().height()));
                m_io->write("\n");
            }
        }
        m_io->write("END LAYER\n");
    }

    m_view = nullptr;
}

void SceneGraphDumper::beginPhase(SceneGraphItemPayload::RenderPhase phase)
{
    switch (phase) {
        case SceneGraphItemPayload::NoPhase:
            Q_UNREACHABLE();
        case SceneGraphItemPayload::FillPhase:
            m_io->write("BEGIN FILL PHASE\n");
            break;
        case SceneGraphItemPayload::CasingPhase:
            m_io->write("BEGIN CASING PHASE\n");
            break;
        case SceneGraphItemPayload::StrokePhase:
            m_io->write("BEGIN STROKE PHASE\n");
            break;
        case SceneGraphItemPayload::IconPhase:
            m_io->write("BEGIN ICON PHASE\n");
            break;
        case SceneGraphItemPayload::LabelPhase:
            m_io->write("BEGIN LABEL PHASE\n");
            break;
    }
}
