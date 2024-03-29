/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <map/scene/view.h>

#include <QTest>

using namespace KOSMIndoorMap;

class MapViewTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProjection()
    {
        View v;
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{0.0, 0.0}), QPointF(128.0, 128.0));
        QCOMPARE(v.mapSceneToGeo(QPointF(128.0, 128.0)), OSM::Coordinate(0.0, 0.0));
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{0.0, -180.0}), QPointF(0.0, 128.0));
        QCOMPARE(v.mapSceneToGeo(QPointF(0.0, 128.0)), OSM::Coordinate(0.0, -180.0));
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{0.0, 180.0}), QPointF(256.0, 128.0));
        QCOMPARE(v.mapSceneToGeo(QPointF(256.0, 128.0)), OSM::Coordinate(0.0, 180.0));

        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{90.0, 0.0}).toPoint().y(), 0);
        QCOMPARE(v.mapSceneToGeo(QPointF(0.0, 128.0)).latF(), 0.0);
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{85.0512, 0.0}).toPoint().y(), 0);
        QCOMPARE(v.mapSceneToGeo(QPointF(0.0, 0.0)).latF(), 85.0511287);
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{60.0, 0.0}).toPoint().y(), 74);
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{-60.0, 0.0}).toPoint().y(), 182);
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{-85.0512, 0.0}).toPoint().y(), 256.0);
        QCOMPARE(v.mapSceneToGeo(QPointF(0.0, 256.0)).latF(), -85.0511288);
        QCOMPARE(v.mapGeoToScene(OSM::Coordinate{-90.0, 0.0}).toPoint().y(), 256.0);

        QCOMPARE(v.mapGeoToScene(OSM::BoundingBox{OSM::Coordinate{-90.0, -180.0}, OSM::Coordinate{90.0, 180.0}}).toRect(), QRect(0, 0, 256, 256));
        QCOMPARE(v.mapGeoToScene(OSM::BoundingBox{OSM::Coordinate{0.0, 0.0}, OSM::Coordinate{90.0, 90.0}}).toRect(), QRect(128, 0, 64, 128));
    }

    void testViewport()
    {
        {
            View v;
            v.setScreenSize({100, 200});
            v.setSceneBoundingBox(QRectF{QPointF{13.0, 52.0}, QPointF{14.0, 54.0}});
            QCOMPARE(v.viewport(), QRectF(QPointF{13.0, 52.0}, QPointF{14.0, 54.0}));
        } {
            View v;
            v.setScreenSize({200, 100});
            v.setSceneBoundingBox(QRectF(QPointF{13.0, 52.0}, QPointF{14.0, 54.0}));
            QCOMPARE(v.viewport(), QRectF(QPointF{13.0, 52.0}, QPointF{14.0, 52.5}));
        } {
            View v;
            v.setScreenSize({100, 100});
            v.setSceneBoundingBox(QRectF(QPointF{13.0, 52.0}, QPointF{14.0, 54.0}));
            QCOMPARE(v.viewport(), QRectF(QPointF{13.0, 52.0}, QPointF{14.0, 53.0}));
        }
    }

    void testTransform()
    {
        {
            View v;
            v.setScreenSize({100, 100});
            v.setSceneBoundingBox(QRectF(QPointF{13.0, 53.0}, QPointF{14.0, 54.0}));
            QCOMPARE(v.sceneToScreenTransform().m11(), 100.0);
            QCOMPARE(v.sceneToScreenTransform().m22(), 100.0);
            QCOMPARE(v.sceneToScreenTransform().m31(), -1300.0);
            QCOMPARE(v.sceneToScreenTransform().m32(), -5300.0);
        } {
            View v;
            v.setScreenSize({100, 100});
            v.setSceneBoundingBox(QRectF(QPointF{13.0, 52.0}, QPointF{15.0, 53.0}));
            QCOMPARE(v.sceneToScreenTransform().m11(), 100.0);
            QCOMPARE(v.sceneToScreenTransform().m22(), 100.0);
            QCOMPARE(v.sceneToScreenTransform().m31(), -1300.0);
            QCOMPARE(v.sceneToScreenTransform().m32(), -5200.0);
        } {
            View v;
            v.setScreenSize({100, 200});
            v.setSceneBoundingBox(QRectF(QPointF{13.0, 52.0}, QPointF{13.1, 52.5}));
            QCOMPARE(v.sceneToScreenTransform().m11(), 1000.0);
            QCOMPARE(v.sceneToScreenTransform().m22(), 1000.0);
            QCOMPARE(v.sceneToScreenTransform().m31(), -13000.0);
            QCOMPARE(v.sceneToScreenTransform().m32(), -52000.0);
        }
    }

    void testZoomLevel()
    {
        View v;
        v.setScreenSize({512, 256});
        v.setSceneBoundingBox(QRectF(QPointF{-180.0, -90.0}, QPointF{180.0, 90.0}));
        QCOMPARE(v.zoomLevel(), 1.0);
        v.setScreenSize({1024, 512});
        v.setSceneBoundingBox(QRectF(QPointF{-180.0, -90.0}, QPointF{180.0, 90.0}));
        QCOMPARE(v.zoomLevel(), 2.0);
        v.zoomIn({512, 256});
        QCOMPARE(v.zoomLevel(), 3.0);
        v.zoomOut({512, 256});
        QCOMPARE(v.zoomLevel(), 2.0);
        v.zoomOut({512, 256});
        v.zoomOut({512, 256});
        v.zoomOut({512, 256});
        QCOMPARE(v.zoomLevel(), 2.0);
    }

    void testPan()
    {
        View v;
        v.setScreenSize({512, 256});
        v.setSceneBoundingBox(QRectF(QPointF{-180.0, -90.0}, QPointF{180.0, 90.0}));
        QCOMPARE(v.viewport(), QRectF(QPointF{-180.0, -90.0}, QPointF{180.0, 90.0}));
        v.zoomIn({256, 128});
        v.panScreenSpace(QPoint(-10000, -10000));
        QCOMPARE(v.viewport().left(), -180.0);
        QCOMPARE(v.viewport().top(), -90.0);
        v.panScreenSpace(QPoint(10000, 10000));
        QCOMPARE(v.viewport().left(), 0.0);
        QCOMPARE(v.viewport().top(), 0.0);
    }
};

QTEST_GUILESS_MAIN(MapViewTest)

#include "mapviewtest.moc"
