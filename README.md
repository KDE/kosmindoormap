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
* Integration with KPublicTransport realtime equipment (elevators/escalators/etc) status information.

Technical:
* QPainter and QML integration interface.
* Declarative styling using MapCSS.
* Picking support for implementing interaction with map elements.
* Support for externally provided overlay elements.
* Based on OSM raw data tiles from maps.kde.org.
* Pre-loading and caching API for offline support in applications.

## Development/Testing

There's two test applications included:
- A fairly minimal widget-based one that can be launched via `$buildir/bin/indoormap -c <lat>,<lon>`
  (use e.g. `-c 52.52512,13.36966` for a good example).
- A QML-based one that can be launched via `qmlscene src/app/indoormap.qml`. This one demos a number
  of additional features, such as element picking and the KPublicTransport integration.

The latter is also available as nightly build:
* Flatpak:
```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak remote-add --if-not-exists kdeapps --from https://distribute.kde.org/kdeapps.flatpakrepo
flatpak install kdeapps org.kde.kosmindoormap
```
* [KDE nightly F-Droid repository](https://community.kde.org/Android/FDroid)
* [Raw APKs from binary factory](https://binary-factory.kde.org/view/Android/job/KOSMIndoorMap_android/)

### Dynamic MapCSS

By default the compiled-in MapCSS files are used. If you put files with the same name into
`$PREFIX/share/org.kde.kosmindoormap/assets/css` or `~/.local/share/org.kde.kosmindoormap/assets/css`
those will be preferred. Symlinking the `src/map/assets` directory to any of those locations is therefore
useful for style development. Styles are reevaluated when switching between them, so this allows testing
style changes without restarting or recompiling the applications.
