%{
/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssparser_impl.h"
#include "mapcssscanner.h"

#include "style/mapcssexpression_p.h"
#include "style/mapcssparsercontext_p.h"
#include "style/mapcssrule_p.h"
#include "style/mapcssselector_p.h"
#include "style/mapcssstyle.h"
#include "style/mapcssterm_p.h"

void yyerror(YYLTYPE *loc, KOSMIndoorMap::MapCSSParserContext *context, yyscan_t scanner, char const* msg)
{
    (void)scanner;
    qWarning() << "PARSER ERROR:" << msg << "in" << context->m_currentUrl.toString() << "line:" << loc->first_line << "column:" << loc->first_column;
    context->setError(QString::fromUtf8(msg), loc->first_line, loc->first_column);
}

using namespace KOSMIndoorMap;

%}

%code requires {

#include "style/mapcsscondition_p.h"
#include "style/mapcssselector_p.h"

namespace KOSMIndoorMap {
class MapCSSDeclaration;
class MapCSSParserContext;
class MapCSSRule;
class MapCSSStyle;
class MapCSSTerm;

struct StringRef {
    const char *str;
    int len;
};

struct ZoomRange {
    int low;
    int high;
};

}

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

using namespace KOSMIndoorMap;

}

%define api.pure
%define parse.error verbose

%locations
%lex-param { yyscan_t scanner }
%parse-param { KOSMIndoorMap::MapCSSParserContext *context }
%parse-param { yyscan_t scanner }

%union {
    uint32_t uintVal;
    double doubleVal;
    bool boolVal;
    ZoomRange zoomRange;

    char *str;
    StringRef strRef;

    MapCSSRule *rule;
    MapCSSSelector *selector;
    MapCSSBasicSelector *basicSelector;
    MapCSSCondition *condition;
    MapCSSConditionHolder *conditionHolder;
    MapCSSCondition::Operator binaryOp;
    MapCSSDeclaration *declaration;
    MapCSSTerm *term;
}

%token T_MAPCSS_START
%token T_EVAL_EXPRESSION_START

%token T_LBRACKET
%token T_RBRACKET
%token T_LBRACE
%token T_RBRACE
%token T_LPAREN
%token T_RPAREN
%token T_DOUBLE_COLON
%token T_COLON
%token T_SEMICOLON
%token T_COMMA
%token T_DASH
%token T_PLUS
%token T_STAR
%token T_ZOOM
%token T_EXCLAMATION_MARK
%token T_EQUALS
%token T_DOT
%token T_SLASH
%token T_COMPARE_EQUAL
%token T_COMPARE_NOT_EQUAL
%token T_COMPARE_LT
%token T_COMPARE_GT
%token T_COMPARE_LE
%token T_COMPARE_GE
%token T_LOGICAL_AND
%token T_LOGICAL_OR
%token T_KEYWORD_IMPORT
%token T_KEYWORD_URL
%token T_KEYWORD_RGBA
%token T_KEYWORD_RGB
%token T_KEYWORD_SET
%token T_KEYWORD_EVAL
%token <strRef> T_IDENT
%token <uintVal> T_HEX_COLOR
%token <str> T_STRING
%token <doubleVal> T_DOUBLE
%token <boolVal> T_BOOLEAN_LITERAL

%type <rule> Rule
%type <selector> Selectors
%type <selector> Selector
%type <basicSelector> BasicSelector
%type <strRef> ClassSelector
%type <strRef> PseudoClassSelector
%type <conditionHolder> Tests
%type <condition> Test
%type <zoomRange> ZoomRange
%type <condition> Condition
%type <binaryOp> BinaryOp
%type <strRef> LayerSelector
%type <strRef> Key
%type <rule> Declarations
%type <declaration> Declaration
%type <strRef> PropertyName
%type <declaration> PropertyValue
%type <doubleVal> DoubleValue
%type <term> EvalExpression
%type <term> EvalFunction
%type <term> EvalArguments

%destructor { free($$); } <str>
%destructor { delete $$; } <rule>
%destructor { delete $$; } <selector>
%destructor { delete $$; } <conditionHolder>
%destructor { delete $$; } <condition>
%destructor { delete $$; } <declaration>
%destructor { delete $$; } <term>

// precendence order from low to high
%left T_DOT
%nonassoc T_COMPARE_EQUAL T_COMPARE_NOT_EQUAL T_COMPARE_LT T_COMPARE_GT T_COMPARE_LE T_COMPARE_GE
%left T_LOGICAL_OR
%left T_LOGICAL_AND
%left T_EXCLAMATION_MARK
%left T_PLUS T_DASH
%left T_STAR T_SLASH

%verbose

%%
// see https://wiki.openstreetmap.org/wiki/MapCSS/0.2/BNF
// Dummy start rule to allow multiple entry points
// https://www.gnu.org/software/bison/manual/html_node/Multiple-start_002dsymbols.html
Start:
  T_MAPCSS_START Ruleset
| T_EVAL_EXPRESSION_START EvalExpression[E] {
    context->m_term = $E;
  }
;

// see https://wiki.openstreetmap.org/wiki/MapCSS/0.2/BNF
Ruleset:
  Rule
| Ruleset Rule { Q_UNUSED($2); }
;

Rule:
  Selectors T_LBRACE Declarations T_RBRACE {
    $3->setSelector($1);
    context->addRule($3);
    $$ = nullptr;
  }
| Import { $$ = nullptr; }
;

Import:
  T_KEYWORD_IMPORT T_KEYWORD_URL T_LPAREN T_STRING[I] T_RPAREN T_SEMICOLON {
    if (!context->addImport($I, {})) {
        YYABORT;
    }
  }
| T_KEYWORD_IMPORT T_KEYWORD_URL T_LPAREN T_STRING[I] T_RPAREN T_IDENT[C] T_SEMICOLON {
    if (!context->addImport($I, context->makeClassSelector($C.str, $C.len))) {
        YYABORT;
    }
  }
;

Selectors:
  Selector { $$ = $1; }
| Selectors T_COMMA Selector { if (auto u = dynamic_cast<MapCSSUnionSelector*>($1)) {
    u->addSelector(std::unique_ptr<MapCSSSelector>($3));
    $$ = $1;
  } else {
    auto s = new MapCSSUnionSelector;
    s->addSelector(std::unique_ptr<MapCSSSelector>($1));
    s->addSelector(std::unique_ptr<MapCSSSelector>($3));
    $$ = s;
  }}
;

Selector:
  BasicSelector { $$ = $1; }
| Selector BasicSelector { if (auto chain = dynamic_cast<MapCSSChainedSelector*>($1)) {
    chain->selectors.push_back(std::unique_ptr<MapCSSBasicSelector>($2));
    $$ = $1;
  } else {
    auto s = new MapCSSChainedSelector;
    s->selectors.push_back(std::unique_ptr<MapCSSBasicSelector>(static_cast<MapCSSBasicSelector*>($1)));
    s->selectors.push_back(std::unique_ptr<MapCSSBasicSelector>($2));
    $$ = s;
  }}
;

BasicSelector:
  T_IDENT[I] ClassSelector[C] ZoomRange[Z] Tests[T] PseudoClassSelector[P] LayerSelector[L] {
    $$ = new MapCSSBasicSelector;
    if ($C.str) {
        $$->setClass(context->makeClassSelector($C.str, $C.len));
    }
    $$->setObjectType($I.str, $I.len);
    $$->setZoomRange($Z.low, $Z.high);
    $$->setConditions($T);
    if ($P.str) {
        $$->setPseudoClass($P.str, $P.len);
    }
    $$->setLayer(context->makeLayerSelector($L.str, $L.len));
  }
| T_STAR ClassSelector[C] ZoomRange[Z] Tests[T] PseudoClassSelector[P] LayerSelector[L] {
    $$ = new MapCSSBasicSelector;
    if ($C.str) {
        $$->setClass(context->makeClassSelector($C.str, $C.len));
    }
    $$->setZoomRange($Z.low, $Z.high);
    $$->setConditions($T);
    if ($P.str) {
        $$->setPseudoClass($P.str, $P.len);
    }
    $$->setLayer(context->makeLayerSelector($L.str, $L.len));
  }
;

ClassSelector:
  %empty { $$.str = nullptr; $$.len = 0; }
| T_DOT T_IDENT[I] { $$.str = $I.str; $$.len = $I.len; }
;

PseudoClassSelector:
  %empty { $$.str = nullptr; $$.len = 0; }
| T_COLON T_IDENT[I] { $$.str = $I.str; $$.len = $I.len; }
;

ZoomRange:
  %empty { $$.low = 0; $$.high = 0; }
| T_ZOOM T_DOUBLE[Low] T_DASH T_DOUBLE[High] { $$.low = $Low; $$.high = $High; }
| T_ZOOM T_DOUBLE[Low] T_DASH { $$.low = $Low; $$.high = 0; }
| T_ZOOM T_DOUBLE { $$.low = $2; $$.high = $2; }
| T_ZOOM T_DASH T_DOUBLE[High] { $$.low = 0; $$.high = $High; }

Tests:
  %empty { $$ = nullptr; }
| Tests Test { if ($1) { $1->addCondition($2); $$ = $1; } else { auto holder = new MapCSSConditionHolder; holder->addCondition($2); $$ = holder; }}
;

Test: T_LBRACKET Condition T_RBRACKET { $$ = $2; };

// TODO incomplete: quoted names, regexps
Condition:
  Key BinaryOp T_IDENT {
    $$ = new MapCSSCondition; $$->setKey($1.str, $1.len);
    $$->setOperation($2);
    $$->setValue($3.str, $3.len);
  }
| Key BinaryOp T_STRING[S] {
    $$ = new MapCSSCondition;
    $$->setKey($1.str, $1.len);
    $$->setOperation($2);
    $$->setValue($3, std::strlen($S));
    free($S);
  }
| Key BinaryOp DoubleValue {
    $$ = new MapCSSCondition;
    $$->setKey($1.str, $1.len);
    $$->setOperation($2);
    $$->setValue($3);
  }
| T_EXCLAMATION_MARK Key {
    $$ = new MapCSSCondition;
    $$->setOperation(MapCSSCondition::KeyNotSet);
    $$->setKey($2.str, $2.len);
  }
| Key {
    $$ = new MapCSSCondition;
    $$->setOperation(MapCSSCondition::KeySet);
    $$->setKey($1.str, $1.len);
  }
;

BinaryOp:
  T_COMPARE_NOT_EQUAL { $$ = MapCSSCondition::NotEqual; }
| T_COMPARE_LT { $$ = MapCSSCondition::LessThan; }
| T_COMPARE_GT { $$ = MapCSSCondition::GreaterThan; }
| T_COMPARE_LE { $$ = MapCSSCondition::LessOrEqual; }
| T_COMPARE_GE { $$ = MapCSSCondition::GreaterOrEqual; }
| T_EQUALS    { $$ = MapCSSCondition::Equal; }

LayerSelector:
  %empty { $$.str = nullptr; $$.len = 0; }
| T_DOUBLE_COLON T_IDENT[L] { $$ = $L; }
;

Key:
  T_IDENT { $$ = $1; }
| Key T_COLON T_IDENT { $$.str = $1.str; $$.len = $3.str - $1.str + $3.len; }
;

Declarations:
  %empty { $$ = new MapCSSRule; }
| Declarations Declaration {
    $$ = $1;
    if ($2) {
        $$->addDeclaration($2);
    }
  }
;

Declaration:
  PropertyName T_COLON PropertyValue T_SEMICOLON {
    $$ = $3;
    if ($$) {
        $$->setPropertyName($1.str, $1.len);
    }
  }
| T_KEYWORD_SET Key[K] T_EQUALS T_STRING[V] T_SEMICOLON {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::TagDeclaration);
    $$->setIdentifierValue($K.str, $K.len);
    $$->setStringValue($V);
  }
| T_KEYWORD_SET Key[K] T_EQUALS T_DOUBLE[V] T_SEMICOLON {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::TagDeclaration);
    $$->setIdentifierValue($K.str, $K.len);
    $$->setDoubleValue($V);
  }
| T_KEYWORD_SET Key[K] T_EQUALS T_DASH T_DOUBLE[V] T_SEMICOLON {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::TagDeclaration);
    $$->setIdentifierValue($K.str, $K.len);
    $$->setDoubleValue(-$V);
  }
| T_KEYWORD_SET Key[K] T_SEMICOLON {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::TagDeclaration);
    $$->setIdentifierValue($K.str, $K.len);
  }
| T_KEYWORD_SET T_DOT T_IDENT[C] T_SEMICOLON {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::ClassDeclaration);
    $$->setClassSelectorKey(context->makeClassSelector($C.str, $C.len));
  }
| T_KEYWORD_SET Key[K] T_EQUALS T_KEYWORD_EVAL T_LPAREN T_STRING[E] T_RPAREN T_SEMICOLON {
    auto exp = MapCSSExpression::fromString($E);
    free($E);
    if (!exp.isValid()) {
        qWarning() << "invalid string-wrapped eval expression"; // TODO error details propgation
        YYABORT;
    }
    $$ = new MapCSSDeclaration(MapCSSDeclaration::TagDeclaration);
    $$->setIdentifierValue($K.str, $K.len);
    $$->m_evalExpression = std::move(exp);
  }
| T_KEYWORD_SET Key[K] T_EQUALS T_KEYWORD_EVAL T_LPAREN EvalFunction[E] T_RPAREN T_SEMICOLON {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::TagDeclaration);
    $$->setIdentifierValue($K.str, $K.len);
    $$->m_evalExpression = MapCSSExpression($E);
  }
;

PropertyName:
  T_IDENT { $$ = $1; }
| T_IDENT T_DASH PropertyName { $$.str = $1.str; $$.len = $3.str - $1.str + $3.len; }
;

// TODO incomplete: missing size
// TODO url does not preserve type, and argument quoting differs from spec
PropertyValue:
  Key { $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration); $$->setIdentifierValue($1.str, $1.len); }
| T_HEX_COLOR { $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration); $$->setColorRgba($1); }
| DoubleValue T_IDENT { $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration); $$->setDoubleValue($1); $$->setUnit($2.str, $2.len); }
| DoubleValue { $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration); $$->setDoubleValue($1); }
| T_DOUBLE T_COMMA T_DOUBLE { $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration); $$->setDashesValue({$1, $3}); } // generalize to n dash distances
| T_STRING { $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration); $$->setStringValue($1); }
| T_KEYWORD_RGBA T_LPAREN T_DOUBLE[R] T_COMMA T_DOUBLE[G] T_COMMA T_DOUBLE[B] T_COMMA T_DOUBLE[A] T_RPAREN {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration);
    uint32_t c = 0;
    c |= (uint32_t)($A * 255.0) << 24;
    c |= (uint32_t)($R * 255.0) << 16;
    c |= (uint32_t)($G * 255.0) << 8;
    c |= (uint32_t)($B * 255.0) << 0;
    $$->setColorRgba(c);
  }
| T_KEYWORD_RGB T_LPAREN T_DOUBLE[R] T_COMMA T_DOUBLE[G] T_COMMA T_DOUBLE[B] T_RPAREN {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration);
    uint32_t c = 0;
    c |= 0xff << 24;
    c |= (uint32_t)($R * 255.0) << 16;
    c |= (uint32_t)($G * 255.0) << 8;
    c |= (uint32_t)($B * 255.0) << 0;
    $$->setColorRgba(c);
  }
| T_KEYWORD_URL T_LPAREN T_STRING[S] T_RPAREN {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration);
    $$->setStringValue($S);
  }
| T_BOOLEAN_LITERAL[B] {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration);
    $$->setBoolValue($B);
  }
| T_KEYWORD_EVAL T_LPAREN T_STRING[E] T_RPAREN {
    auto exp = MapCSSExpression::fromString($E);
    free($E);
    if (!exp.isValid()) {
        qWarning() << "invalid string-wrapped eval expression"; // TODO error details propgation
        YYABORT;
    }
    $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration);
    $$->m_evalExpression = std::move(exp);
  }
| T_KEYWORD_EVAL T_LPAREN EvalFunction[E] T_RPAREN {
    $$ = new MapCSSDeclaration(MapCSSDeclaration::PropertyDeclaration);
    $$->m_evalExpression = MapCSSExpression($E);
  }
;

DoubleValue:
  T_DOUBLE { $$ = $1; }
| T_DASH T_DOUBLE { $$ = -$2; }
| T_PLUS T_DOUBLE { $$ = $2; }
;

EvalExpression:
  T_STRING[S] {
    $$ = new MapCSSTerm(MapCSSTerm::Literal);
    $$->m_literal = MapCSSValue(QByteArray($S));
    free($S);
  }
| T_DOUBLE[N] {
    $$ = new MapCSSTerm(MapCSSTerm::Literal);
    $$->m_literal = MapCSSValue($N);
  }
| T_BOOLEAN_LITERAL[B] {
    $$ = new MapCSSTerm(MapCSSTerm::Literal);
    $$->m_literal = MapCSSValue($B);
  }
| EvalFunction {
    $$ = $1;
  }
| T_LPAREN EvalExpression[E] T_RPAREN {
    $$ = $E;
  }
;

EvalFunction:
  T_IDENT[F] T_LPAREN EvalArguments[A] T_RPAREN {
    $$ = $A;
    $$->m_op = MapCSSTerm::parseOperation($F.str, $F.len);
    if ($$->m_op == MapCSSTerm::Unknown) {
        qWarning() << "eval expression with unknown function:" << QByteArrayView($F.str, $F.len);
        YYABORT;
    }
    if (!$$->validChildCount()) {
        qWarning() << "wrong number of arguments for function:" << QByteArrayView($F.str, $F.len);
        YYABORT;
    }
  }
| EvalExpression[OP1] T_PLUS EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::Addition, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_DASH EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::Subtraction, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_STAR EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::Multiplication, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_SLASH EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::Division, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_DOT EvalExpression[OP2] {
    if ($OP1->m_op == MapCSSTerm::Concatenate) { // collapse chained dot operators
        $$ = $OP1;
        $$->addChildTerm($OP2);
    } else {
        $$ = new MapCSSTerm(MapCSSTerm::Concatenate, {$OP1, $OP2});
    }
  }
| T_EXCLAMATION_MARK EvalExpression[OP] {
    $$ = new MapCSSTerm(MapCSSTerm::LogicalNot, {$OP});
  }
| EvalExpression[OP1] T_LOGICAL_AND EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::LogicalAnd, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_LOGICAL_OR EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::LogicalOr, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_COMPARE_EQUAL EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::CompareEqual, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_COMPARE_NOT_EQUAL EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::CompareNotEqual, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_COMPARE_LT EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::CompareLess, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_COMPARE_GT EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::CompareGreater, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_COMPARE_LE EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::CompareLessOrEqual, {$OP1, $OP2});
  }
| EvalExpression[OP1] T_COMPARE_GE EvalExpression[OP2] {
    $$ = new MapCSSTerm(MapCSSTerm::CompareGreaterOrEqual, {$OP1, $OP2});
  }
;

EvalArguments:
  EvalExpression[E] {
    $$ = new MapCSSTerm;
    $$->addChildTerm($E);
  }
| EvalArguments[A] T_COMMA EvalExpression[E] {
    $$ = $A;
    $$->addChildTerm($E);
  }
;
%%
