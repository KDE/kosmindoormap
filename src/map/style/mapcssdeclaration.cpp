/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssdeclaration_p.h"
#include "logging.h"
#include "mapcssproperty.h"

#include <QDebug>
#include <QIODevice>

#include <cstring>

using namespace KOSMIndoorMap;

// keep this sorted by property name!
struct {
    const char* name;
    MapCSSProperty property;
    int flags;
} static constexpr const property_types[] = {
    // only those properties have their corresonding flag set that actually trigger emission of a scene graph item
    // e.g. for a label we either need a text or an icon, the visual properties for those on their own would be a no-op
    { "casing-color", MapCSSProperty::CasingColor, MapCSSDeclaration::NoFlag },
    { "casing-dashes", MapCSSProperty::CasingDashes, MapCSSDeclaration::NoFlag },
    { "casing-linecap", MapCSSProperty::CasingLineCap, MapCSSDeclaration::NoFlag },
    { "casing-linejoin", MapCSSProperty::CasingLineJoin, MapCSSDeclaration::NoFlag },
    { "casing-opacity", MapCSSProperty::CasingOpacity, MapCSSDeclaration::NoFlag },
    { "casing-width", MapCSSProperty::CasingWidth, MapCSSDeclaration::NoFlag },
    { "color", MapCSSProperty::Color, MapCSSDeclaration::LineProperty },
    { "dashes", MapCSSProperty::Dashes, MapCSSDeclaration::NoFlag },
    { "extrude", MapCSSProperty::Extrude, MapCSSDeclaration::ExtrudeProperty },
    { "fill-color", MapCSSProperty::FillColor, MapCSSDeclaration::AreaProperty | MapCSSDeclaration::CanvasProperty }, // TODO this also applies to lines
    { "fill-image", MapCSSProperty::FillImage, MapCSSDeclaration::AreaProperty | MapCSSDeclaration::CanvasProperty },
    { "fill-opacity", MapCSSProperty::FillOpacity, MapCSSDeclaration::AreaProperty },
    { "font-family", MapCSSProperty::FontFamily, MapCSSDeclaration::NoFlag },
    { "font-size", MapCSSProperty::FontSize, MapCSSDeclaration::NoFlag },
    { "font-style", MapCSSProperty::FontStyle, MapCSSDeclaration::NoFlag },
    { "font-variant", MapCSSProperty::FontVariant, MapCSSDeclaration::NoFlag },
    { "font-weight", MapCSSProperty::FontWeight, MapCSSDeclaration::NoFlag },
    { "icon-allow-icon-overlap", MapCSSProperty::IconAllowIconOverlap, MapCSSDeclaration::NoFlag },
    { "icon-allow-text-overlap", MapCSSProperty::IconAllowTextOverlap, MapCSSDeclaration::NoFlag },
    { "icon-color", MapCSSProperty::IconColor, MapCSSDeclaration::NoFlag },
    { "icon-height", MapCSSProperty::IconHeight, MapCSSDeclaration::NoFlag },
    { "icon-image", MapCSSProperty::IconImage, MapCSSDeclaration::LabelProperty },
    { "icon-opacity", MapCSSProperty::IconOpacity, MapCSSDeclaration::NoFlag },
    { "icon-width", MapCSSProperty::IconWidth, MapCSSDeclaration::NoFlag },
    { "image", MapCSSProperty::Image, MapCSSDeclaration::LineProperty },
    { "linecap", MapCSSProperty::LineCap, MapCSSDeclaration::NoFlag },
    { "linejoin", MapCSSProperty::LineJoin, MapCSSDeclaration::NoFlag },
    { "max-width", MapCSSProperty::MaxWidth, MapCSSDeclaration::NoFlag },
    { "opacity", MapCSSProperty::Opacity, MapCSSDeclaration::NoFlag },
    { "shield-casing-color", MapCSSProperty::ShieldCasingColor, MapCSSDeclaration::LabelProperty },
    { "shield-casing-width", MapCSSProperty::ShieldCasingWidth, MapCSSDeclaration::NoFlag },
    { "shield-color", MapCSSProperty::ShieldColor, MapCSSDeclaration::LabelProperty },
    { "shield-frame-color", MapCSSProperty::ShieldFrameColor, MapCSSDeclaration::LabelProperty },
    { "shield-frame-width", MapCSSProperty::ShieldFrameWidth, MapCSSDeclaration::NoFlag },
    { "shield-image", MapCSSProperty::ShieldImage, MapCSSDeclaration::LabelProperty },
    { "shield-opacity", MapCSSProperty::ShieldOpacity, MapCSSDeclaration::NoFlag },
    { "shield-shape", MapCSSProperty::ShieldShape, MapCSSDeclaration::NoFlag },
    { "shield-text", MapCSSProperty::ShieldText, MapCSSDeclaration::LabelProperty },
    { "text", MapCSSProperty::Text, MapCSSDeclaration::LabelProperty },
    { "text-color", MapCSSProperty::TextColor, MapCSSDeclaration::CanvasProperty },
    { "text-decoration", MapCSSProperty::TextDecoration, MapCSSDeclaration::NoFlag },
    { "text-halo-color", MapCSSProperty::TextHaloColor, MapCSSDeclaration::NoFlag },
    { "text-halo-radius", MapCSSProperty::TextHaloRadius, MapCSSDeclaration::NoFlag },
    { "text-offset", MapCSSProperty::TextOffset, MapCSSDeclaration::NoFlag },
    { "text-opacity", MapCSSProperty::TextOpacity, MapCSSDeclaration::NoFlag },
    { "text-position", MapCSSProperty::TextPosition, MapCSSDeclaration::NoFlag },
    { "text-transform", MapCSSProperty::TextTransform, MapCSSDeclaration::NoFlag },
    { "width", MapCSSProperty::Width, MapCSSDeclaration::LineProperty },
    { "z-index", MapCSSProperty::ZIndex, MapCSSDeclaration::NoFlag },
};

struct {
    const char *name;
    Qt::PenCapStyle capStyle;
} static constexpr const capstyle_map[] = {
    { "none", Qt::FlatCap },
    { "round", Qt::RoundCap },
    { "square", Qt::SquareCap },
};

struct {
    const char *name;
    Qt::PenJoinStyle joinStyle;
} static constexpr const joinstyle_map[] = {
    { "bevel", Qt::BevelJoin },
    { "miter", Qt::MiterJoin },
    { "round", Qt::RoundJoin },
};

struct {
    const char *name;
    QFont::Capitalization capitalizationStyle;
} static constexpr const capitalizationstyle_map[] = {
    { "capitalize", QFont::Capitalize },
    { "lowercase", QFont::AllLowercase },
    { "none", QFont::MixedCase },
    { "normal", QFont::MixedCase },
    { "small-caps", QFont::SmallCaps },
    { "uppercase", QFont::AllUppercase },
};

struct {
    const char *name;
    MapCSSDeclaration::Unit unit;
} static constexpr const unit_map[] = {
    { "m", MapCSSDeclaration::Meters },
    { "pt", MapCSSDeclaration::Point },
    { "px", MapCSSDeclaration::Pixels },
};

struct {
    const char *name;
    MapCSSDeclaration::Position position;
} static constexpr const position_map[] = {
    { "center", MapCSSDeclaration::Position::Center },
    { "line", MapCSSDeclaration::Position::Line },
};

MapCSSDeclaration::MapCSSDeclaration(Type type)
    : m_type(type)
{
}

MapCSSDeclaration::~MapCSSDeclaration() = default;

bool MapCSSDeclaration::isValid() const
{
    switch (m_type) {
        case PropertyDeclaration:
            return property() != MapCSSProperty::Unknown;
        case TagDeclaration:
            return !m_identValue.isEmpty();
        case ClassDeclaration:
            return !m_class.isNull();
    }

    Q_UNREACHABLE();
    return false;
}

MapCSSDeclaration::Type MapCSSDeclaration::type() const
{
    return m_type;
}

MapCSSProperty MapCSSDeclaration::property() const
{
    return m_property;
}

int MapCSSDeclaration::propertyFlags() const
{
    return m_flags;
}

int MapCSSDeclaration::intValue() const
{
    return m_doubleValue;
}

double MapCSSDeclaration::doubleValue() const
{
    return m_doubleValue;
}

bool MapCSSDeclaration::boolValue() const
{
    return m_boolValue;
}

QString MapCSSDeclaration::stringValue() const
{
    return m_stringValue;
}

QColor MapCSSDeclaration::colorValue() const
{
    if (!m_colorValue.isValid() && !m_stringValue.isEmpty()) {
        return QColor(m_stringValue);
    }
    return m_colorValue;
}

QByteArray MapCSSDeclaration::keyValue() const
{
    return m_identValue;
}

QVector<double> MapCSSDeclaration::dashesValue() const
{
    return m_dashValue;
}

OSM::TagKey MapCSSDeclaration::tagKey() const
{
    return m_tagKey;
}

void MapCSSDeclaration::setDoubleValue(double val)
{
    m_doubleValue = val;
}

void MapCSSDeclaration::setBoolValue(bool val)
{
    m_boolValue = val;
}

bool MapCSSDeclaration::hasExpression() const
{
    return m_evalExpression.isValid();
}

MapCSSProperty MapCSSDeclaration::propertyFromName(const char *name, std::size_t len)
{
    const auto it = std::lower_bound(std::begin(property_types), std::end(property_types), name, [len](const auto &lhs, const char *rhs) {
        const auto lhsLen = std::strlen(lhs.name);
        const auto cmp = std::strncmp(lhs.name, rhs, std::min(lhsLen, len));
        return cmp < 0 || (cmp == 0 && lhsLen < len);
    });
    if (it == std::end(property_types) || std::strncmp((*it).name, name, std::max(len, std::strlen((*it).name))) != 0) {
        return MapCSSProperty::Unknown;
    }
    return (*it).property;
}

void MapCSSDeclaration::setPropertyName(const char *name, std::size_t len)
{
    const auto it = std::lower_bound(std::begin(property_types), std::end(property_types), name, [len](const auto &lhs, const char *rhs) {
        const auto lhsLen = std::strlen(lhs.name);
        const auto cmp = std::strncmp(lhs.name, rhs, std::min(lhsLen, len));
        return cmp < 0 || (cmp == 0 && lhsLen < len);
    });
    if (it == std::end(property_types) || std::strncmp((*it).name, name, std::max(len, std::strlen((*it).name))) != 0) {
        qCWarning(Log) << "Unknown property declaration:" << QByteArray::fromRawData(name, len);
        m_property = MapCSSProperty::Unknown;
        return;
    }
    m_property = (*it).property;
    m_flags = (*it).flags;
}

void MapCSSDeclaration::setIdentifierValue(const char *val, int len)
{
    m_identValue = QByteArray(val, len);
}

void MapCSSDeclaration::setStringValue(char *str)
{
    m_stringValue = QString::fromUtf8(str);
    free(str);
}

void MapCSSDeclaration::setColorRgba(uint32_t argb)
{
    m_colorValue = QColor::fromRgba(argb);
    //qDebug() << m_colorValue << argb;
}

void MapCSSDeclaration::setDashesValue(const QVector<double> &dashes)
{
    m_dashValue = dashes;
}

Qt::PenCapStyle MapCSSDeclaration::capStyle() const
{
    for (const auto &c : capstyle_map) {
        if (std::strcmp(c.name, m_identValue.constData()) == 0) {
            return c.capStyle;
        }
    }
    qDebug() << "unknown line cap style:" << m_identValue;
    return Qt::FlatCap;
}

Qt::PenJoinStyle MapCSSDeclaration::joinStyle() const
{
    for (const auto &j : joinstyle_map) {
        if (std::strcmp(j.name, m_identValue.constData()) == 0) {
            return j.joinStyle;
        }
    }
    return Qt::RoundJoin;
}

QFont::Capitalization MapCSSDeclaration::capitalizationStyle() const
{
    for (const auto &c : capitalizationstyle_map) {
        if (std::strcmp(c.name, m_identValue.constData()) == 0) {
            return c.capitalizationStyle;
        }
    }
    return QFont::MixedCase;
}

bool MapCSSDeclaration::isBoldStyle() const
{
    return m_identValue == "bold";
}

bool MapCSSDeclaration::isItalicStyle() const
{
    return m_identValue == "italic";
}

bool MapCSSDeclaration::isUnderlineStyle() const
{
    return m_identValue == "underline";
}

MapCSSDeclaration::Position MapCSSDeclaration::textPosition() const
{
    for (const auto &p : position_map) {
        if (std::strcmp(p.name, m_identValue.constData()) == 0) {
            return p.position;
        }
    }
    return Position::NoPostion;
}

MapCSSDeclaration::Unit MapCSSDeclaration::unit() const
{
    return m_unit;
}

void MapCSSDeclaration::setUnit(const char *val, int len)
{
    for (const auto &u : unit_map) {
        if (std::strncmp(u.name, val, std::max<std::size_t>(std::strlen(u.name), len)) == 0) {
            m_unit = u.unit;
            return;
        }
    }
    qCWarning(Log) << "unknown unit:" << QByteArray(val, len);
    m_unit = NoUnit;
}

ClassSelectorKey MapCSSDeclaration::classSelectorKey() const
{
    return m_class;
}

void MapCSSDeclaration::setClassSelectorKey(ClassSelectorKey key)
{
    m_class = key;
}

void MapCSSDeclaration::compile(const OSM::DataSet &dataSet)
{
    // TODO resolve tag key if m_identValue is one
    if (m_type == TagDeclaration) {
        // TODO handle the case that the tag isn't actually available in dataSet
        m_tagKey = dataSet.tagKey(m_identValue.constData());
    }

    if (m_evalExpression.isValid()) {
        m_evalExpression.compile(dataSet);
    }
}

static void writeQuotedString(QIODevice *out, QByteArrayView str)
{
    out->write("\"");
    for (const auto c : str) {
        switch (c) {
            case '"':
                out->write("\\\"");
                break;
            case '\n':
                out->write("\\n");
                break;
            case '\t':
                out->write("\\t");
                break;
            default:
                out->write(&c, 1);
        }
    }
    out->write("\"");
}

void MapCSSDeclaration::write(QIODevice *out) const
{
    out->write("    ");

    switch (m_type) {
        case PropertyDeclaration:
            for (const auto &p : property_types) {
                if (p.property == m_property) {
                    out->write(p.name);
                    break;
                }
            }

            out->write(": ");
            if (!std::isnan(m_doubleValue)) {
                out->write(QByteArray::number(m_doubleValue));
            } else if (m_colorValue.isValid()) {
                out->write(m_colorValue.name(QColor::HexArgb).toUtf8());
            } else if (!m_dashValue.isEmpty()) {
                for (const auto &d : m_dashValue) {
                    out->write(QByteArray::number(d));
                    out->write(", ");
                }
            } else if (!m_stringValue.isNull()) {
                writeQuotedString(out, m_stringValue.toUtf8());
            } else if (!m_identValue.isEmpty()) {
                out->write(m_identValue);
            } else if (m_evalExpression.isValid()) {
                out->write("eval(");
                m_evalExpression.write(out);
                out->write(")");
            } else {
                out->write(m_boolValue ? "true" : "false");
            }

            for (const auto &u : unit_map) {
                if (u.unit == m_unit) {
                    out->write(u.name);
                    break;
                }
            }
            break;
        case TagDeclaration:
            out->write("set ");
            out->write(m_identValue);
            if (!std::isnan(m_doubleValue)) {
                out->write(" = ");
                out->write(QByteArray::number(m_doubleValue));
            } else if (!m_stringValue.isEmpty()) {
                out->write(" = ");
                writeQuotedString(out, m_stringValue.toUtf8());
            } else if (m_evalExpression.isValid()) {
                out->write(" = eval(");
                m_evalExpression.write(out);
                out->write(")");
            }
            break;
        case ClassDeclaration:
            out->write("set .");
            out->write(m_class.name());
            break;
    }

    out->write(";\n");
}
