# KOSMIndoorMap

A library and QML component for rendering multi-level OSM indoor maps of for example
a (large) train station.

## Features

User facing:
* Floor-level separation of OSM data and inter-floor navigation using stairs, escalators or elevators.
* Information model for showing details about a selected amenity.
* Support for identifying railway platforms or airport gates in the map data.
* Integration with KPublicTransport line meta-data to show line icons for railway platforms.
* Integration with KPublicTransport rental vehicle data to show availability of rental bikes.

Technical:
* QPainter and QML integration interface.
* Declarative styling using MapCSS.
* Picking support for implementing interaction with map elements.
* Support for externally provided overlay elements.
* Based on OSM raw data tiles from maps.kde.org.
* Pre-loading and caching API for offline support in applications.
