/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSDECLARATION_P_H
#define KOSMINDOORMAP_MAPCSSDECLARATION_P_H

#include "kosmindoormap_export.h"
#include "mapcssproperty.h"
#include "mapcsstypes.h"

#include <osm/datatypes.h>

#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QPen>

#include <cmath>

namespace OSM {
class DataSet;
}

class QIODevice;
class PenWidthUtilTest;

namespace KOSMIndoorMap { class MapCSSParserPrivate; }
int yyparse(KOSMIndoorMap::MapCSSParserPrivate*, void*);

namespace KOSMIndoorMap {

/** Property/value declaration of a MapCSS rule.
 *  @see https://wiki.openstreetmap.org/wiki/MapCSS/0.2#Vocabulary
 *  @internal only exported for unit tests
 */
class KOSMINDOORMAP_EXPORT MapCSSDeclaration
{
public:
    /** Type of declaration. */
    enum Type {
        PropertyDeclaration, ///< sets a style propery
        TagDeclaration, ///< sets a tag value
        ClassDeclaration, ///< sets a class type
    };

    explicit MapCSSDeclaration(Type type);
    ~MapCSSDeclaration();

    /** Checks if this is a meaningful declaration. */
    bool isValid() const;

    Type type() const;
    [[nodiscard]] MapCSSProperty property() const;

    /** The type of property. Helps to determine which kind of geometry we need to emit for a rule. */
    enum PropertyFlag {
        NoFlag = 0,
        AreaProperty = 1,
        LineProperty = 2,
        LabelProperty = 4,
        CanvasProperty = 8,
        ExtrudeProperty = 16,
    };
    int propertyFlags() const;

    /** Numeric value for this property. */
    int intValue() const;
    double doubleValue() const;
    bool boolValue() const;
    /** Quoted string value. */
    QString stringValue() const;
    /** Color value for this property. */
    QColor colorValue() const;
    /** Tag key name value. */
    QByteArray keyValue() const;
    /** Line dashes. */
    QVector<double> dashesValue() const;

    /** Tag key of the tag to change in a tag setting declaration. */
    OSM::TagKey tagKey() const;

    Qt::PenCapStyle capStyle() const;
    Qt::PenJoinStyle joinStyle() const;
    QFont::Capitalization capitalizationStyle() const;
    bool isBoldStyle() const;
    bool isItalicStyle() const;
    bool isUnderlineStyle() const;

    /** Text position property. */
    enum class Position {
        NoPostion,
        Line,
        Center,
    };
    Position textPosition() const;

    /** Unit type for numeric value. */
    enum Unit {
        NoUnit,
        Pixels,
        Point,
        Meters,
    };
    Unit unit() const;

    ClassSelectorKey classSelectorKey() const;

    void compile(const OSM::DataSet &dataSet);
    void write(QIODevice *out) const;

private:
    friend int ::yyparse(KOSMIndoorMap::MapCSSParserPrivate*, void*);
    friend class ::PenWidthUtilTest;

    /** @internal, for use by the parser. */
    void setPropertyName(const char *name, std::size_t len);
    void setIdentifierValue(const char *val, int len);
    void setDoubleValue(double val);
    void setBoolValue(bool val);
    void setStringValue(char *str);
    void setColorRgba(uint32_t argb);
    void setDashesValue(const QVector<double> &dashes);
    void setUnit(const char *val, int len);
    void setClassSelectorKey(ClassSelectorKey key);

    MapCSSProperty m_property = MapCSSProperty::Unknown;
    int m_flags = NoFlag;
    // ### merge all of this into a QVariant?
    QByteArray m_identValue;
    QColor m_colorValue;
    double m_doubleValue = NAN;
    QVector<double> m_dashValue;
    QString m_stringValue;
    OSM::TagKey m_tagKey;
    ClassSelectorKey m_class;
    Unit m_unit = NoUnit;
    Type m_type;
    bool m_boolValue = false;
};

}

#endif // KOSMINDOORMAP_MAPCSSDECLARATION_P_H
