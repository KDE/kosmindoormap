/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSPROPERTY_H
#define KOSMINDOORMAP_MAPCSSPROPERTY_H

namespace KOSMIndoorMap {

/** Known properties in MapCSS declarations. */
enum class MapCSSProperty {
    Unknown,

    // general properties
    ZIndex, /// z-order

    // line properties
    Width, /// line width
    Color, /// line color
    Opacity, /// line opacity
    Dashes, /// line dash pattern
    Image, /// fill image for the line
    LineCap, /// line end cap style: none (default), round, square
    LineJoin, /// line join style: round (default), miter, bevel

    // line casing properties
    CasingWidth, /// line casing width
    CasingColor, /// line casing color
    CasingOpacity, /// line casing opacity
    CasingDashes, /// line casing dash pattern
    CasingLineCap, /// line casing end cap
    CasingLineJoin, /// line casing join style

    // missing here: extrude properties

    // polygon (and canvas) properties
    FillColor, /// area fill color
    FillOpacity, /// area fill opacity
    FillImage, /// image to fill the area with

    // icon properties
    IconImage, /// URL to the icon image
    IconWidth, /// icon width
    IconHeight, /// icon height
    IconOpacity, /// icon opacity
    IconColor, /// for colorized SVGs, non-standard extension
    IconAllowTextOverlap, /// the equivalent to CartoCSS's allow-overlap, non-standard extension
    IconAllowIconOverlap, /// the equivalent to CartoCSS's ignore-placement, non-standard extension

    // label properties
    FontFamily, /// font name
    FontSize, /// font size
    FontWeight, /// font weight: bold or normal (default)
    FontStyle, /// italic or normal (default)
    FontVariant, /// small-caps or normal (default)
    TextDecoration, /// none (default) or underline
    TextTransform, /// none (default), uppercase, lowercase or capitalize
    TextColor, /// text color used for the label
    TextOpacity, /// text opacity
    TextPosition, /// @p line or @p center
    TextOffset, /// vertical offset from the center of the way or point
    MaxWidth, /// maximum width before wrapping
    Text, /// label content
    TextHaloColor, /// text halo color
    TextHaloRadius, /// text halo radius

    // shield properties (casing > frame > shield > text)
    ShieldColor, /// shield color
    ShieldOpacity, /// shield opacity
    ShieldFrameColor, /// shield frame color
    ShieldFrameWidth, /// shield frame width (0 to disable)
    ShieldCasingColor, /// shield casing color
    ShieldCasingWidth, /// shield  casing width
    ShieldText, /// text to render on the shield
    ShieldImage, /// background image of the shield
    ShieldShape, /// @p rounded or @p rectangular

    // 3D extrusion
    Extrude,
};

}

#endif
