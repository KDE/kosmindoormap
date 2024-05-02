/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <map/style/mapcssexpression_p.h>
#include <map/style/mapcssexpressioncontext_p.h>
#include <map/style/mapcssresult.h>
#include <map/style/mapcssstate_p.h>
#include <map/style/mapcssvalue_p.h>

#include <QTest>

using namespace KOSMIndoorMap;

class MapCSSExpressionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testString_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("result");

        QTest::newRow("empty") << "\"\"" << "";
        QTest::newRow("string") << "\"kde\"" << "kde";

        QTest::newRow("concat2") << "concat(\"hello\", \"kde\")" << "hellokde";
        QTest::newRow("concat3") << "concat(\"hello\", ' ', \"KDE\")" << "hello KDE";

        QTest::newRow("dot-concat2") << "\"hello\" . \"kde\"" << "hellokde";
        QTest::newRow("dot-concat3") << "\"hello\" . ' ' . \"KDE\"" << "hello KDE";

        QTest::newRow("implicit-cast") << "42" << "42";
        QTest::newRow("explicit-cast") << "str(42)" << "42";

        QTest::newRow("canonical-boolean-true") << "boolean(\"yes\")" << "true";
        QTest::newRow("canonical-boolean-false") << "boolean(0)" << "false";

        QTest::newRow("any2") << "any(\"\", \"KDE\")" << "KDE";
        QTest::newRow("any2of3") << "any(\"\", \"KDE\", \"Hello\")" << "KDE";
        QTest::newRow("any3") << "any(\"\", \"\", \"KDE\")" << "KDE";

        QTest::newRow("tag") << "tag(\"name\")" << "M2 Building";
    }

    void testString()
    {
        QFETCH(QString, input);
        QFETCH(QString, result);

        auto exp = MapCSSExpression::fromString(input.toUtf8().constData());
        QVERIFY(exp.isValid());

        OSM::DataSet dataSet;
        OSM::Node node;
        OSM::setTagValue(node, dataSet.makeTagKey("name"), "M2 Building");

        exp.compile(dataSet);

        MapCSSState state;
        state.element = OSM::Element(&node);
        MapCSSResultLayer cssResult;
        QCOMPARE(exp.evaluate({state, cssResult}).asString(), result.toUtf8());
    }

    void testNumeric_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("result");

        QTest::newRow("null") << "0" << 0;
        QTest::newRow("number") << "42" << 42;

        QTest::newRow("addition") << "2+2" << 4;
        QTest::newRow("subtraction") << "4-2" << 2;
        QTest::newRow("multiplication") << "2*3" << 6;
        QTest::newRow("division") << "6/3" << 2;

        QTest::newRow("precedence-right") << "2+3*3" << 11;
        QTest::newRow("precedence-left") << "3*3+2" << 11;

        QTest::newRow("parenthesis") << "(2+3)*3" << 15;

        QTest::newRow("implicit-cast1") << "'42'" << 42;
        QTest::newRow("implicit-cast2") << "\"42\"" << 42;

        QTest::newRow("explicit-cast") << "num(\"42\")" << 42;

        QTest::newRow("sqrt") << "sqrt(144.0)" << 12;

        QTest::newRow("int1") << "int(3.1415)" << 3;
        QTest::newRow("int2") << "int(2.7182)" << 2;

        QTest::newRow("string-math1") << "\"2\" + 4" << 6;
        QTest::newRow("string-math2") << "2 + \"\"" << 2;

        QTest::newRow("empty-concat") << "2 . \"\"" << 2;

        QTest::newRow("min2") << "min(23, 42)" << 23;
        QTest::newRow("min3") << "min(23, 42, 4)" << 4;
        QTest::newRow("max2") << "max(23, 42)" << 42;
        QTest::newRow("max3") << "max(23, 42, 4)" << 42;

        QTest::newRow("condition-true") << "cond(\"true\", 1, 2)" << 1;
        QTest::newRow("condition-false") << "cond(\"false\", 1, 2)" << 2;
    }

    void testNumeric()
    {
        QFETCH(QString, input);
        QFETCH(int, result);

        const auto exp = MapCSSExpression::fromString(input.toUtf8().constData());
        QVERIFY(exp.isValid());

        MapCSSState state;
        MapCSSResultLayer cssResult;
        QCOMPARE(exp.evaluate({state, cssResult}).asNumber(), result);
    }

    void testBoolean_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<bool>("result");

        QTest::newRow("literal-true") << "true" << true;
        QTest::newRow("literal-false") << "false" << false;

        QTest::newRow("empty-string") << "\"\"" << false;
        QTest::newRow("null") << "0" << false;
        QTest::newRow("string") << "\"kde\"" << true;
        QTest::newRow("number") << "42" << true;

        QTest::newRow("string-false") << "\"false\"" << false;
        QTest::newRow("no") << "\"no\"" << false;
        QTest::newRow("string-true") << "\"true\"" << true;

        QTest::newRow("explicit-cast") << "boolean(\"yes\")" << true;

        QTest::newRow("not1") << "!'false'" << true;
        QTest::newRow("not2") << "!(\"a\" == \"a\")" << false;

        QTest::newRow("and-2-true") << "'true' && 'true'" << true;
        QTest::newRow("and-3-true") << "'true' && 'true' && 'true'" << true;
        QTest::newRow("and-2-false") << "0 && 'true'" << false;
        QTest::newRow("and-3-false") << "'true' && 0 && 'true'" << false;

        QTest::newRow("or-2-true") << "'true' || 'false'" << true;
        QTest::newRow("or-3-true") << "'true' || 'true' || 0" << true;
        QTest::newRow("or-2-false") << "0 || 'false'" << false;
        QTest::newRow("or-3-false") << "'false' || 0 || ''" << false;

        QTest::newRow("precedence-left") << "0 || 'true' && 'true'" << true;
        QTest::newRow("precedence-right") << "'true' && 'true' || 0" << true;

        QTest::newRow("less-than") << "2 < 3" << true;
        QTest::newRow("greater-than") << "'3' > '2'" << true;
        QTest::newRow("less-eq") << "2 <= 3" << true;
        QTest::newRow("greater-eq") << "2 >= 3" << false;

        QTest::newRow("equal-strings-true") << "\"2\" == \"2\"" << true;
        QTest::newRow("not-equal-strings-false") << "\"2\" != \"2\"" << false;
        QTest::newRow("equal-strings-false") << "\"3\" == \"2\"" << false;
        QTest::newRow("not-equal-strings-true") << "\"3\" != \"2\"" << true;
        // QTest::newRow("numeric-equal-true") << "2 == \"02\"" << true; TODO in the spec but not supported at this point
        QTest::newRow("numeric-not-equal-true") << "2 != \"02\"" << true;
    }

    void testBoolean()
    {
        QFETCH(QString, input);
        QFETCH(bool, result);

        const auto exp = MapCSSExpression::fromString(input.toUtf8().constData());
        QVERIFY(exp.isValid());

        MapCSSState state;
        MapCSSResultLayer cssResult;
        QCOMPARE(exp.evaluate({state, cssResult}).asBoolean(), result);
    }

    void testNone_data()
    {
        QTest::addColumn<QString>("input");

        QTest::newRow("empty") << "\"\"";

        QTest::newRow("div-by-zero") << "2/0";
        QTest::newRow("div-by-none") << "2/\"\"";

        QTest::newRow("tag") << "tag(\"name\")";

        QTest::newRow("invalid-string-math") << "2 + \"a\"";
    }

    void testNone()
    {
        QFETCH(QString, input);

        const auto exp = MapCSSExpression::fromString(input.toUtf8().constData());
        QVERIFY(exp.isValid());

        MapCSSState state;
        MapCSSResultLayer cssResult;
        QVERIFY(exp.evaluate({state, cssResult}).isNone());
    }

    void testInvalid_data()
    {
        QTest::addColumn<QString>("input");

        QTest::newRow("empty") << "";
        QTest::newRow("zero-arg-concat") << "concat()";
        QTest::newRow("one-arg-concat") << "concat(\"kde\")";

        QTest::newRow("two-arg-cond") << "cond(\"kde\", 32)";
        QTest::newRow("four-arg-cond") << "cond(\"true\", 32, 31, 20)";

        QTest::newRow("chained comparators") << "23 < 42 < 100";
    }

    void testInvalid()
    {
        QFETCH(QString, input);
        const auto exp = MapCSSExpression::fromString(input.toUtf8().constData());
        QVERIFY(!exp.isValid());
    }
};

QTEST_GUILESS_MAIN(MapCSSExpressionTest)

#include "mapcssexpressiontest.moc"
