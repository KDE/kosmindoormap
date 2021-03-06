%{
/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssparser_p.h"
#include "style/mapcssselector_p.h"

// ### ugly workaround for locale-ignoring string to float conversion
// std::from_chars offers that with C++17, but isn't actually implemented yet for floats/doubles...
#include <private/qlocale_tools_p.h>

extern char* unquoteString(const char *str);

using namespace KOSMIndoorMap;

int yy_column = 1;
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno + 1; \
    yylloc->first_column = yy_column; \
    yylloc->last_column = yy_column + yyleng - 1; \
    yy_column += yyleng;

%}

%option warn nodefault

%option reentrant
%option noyywrap
%option nounput
%option never-interactive
%option bison-bridge
%option bison-locations
%option yylineno

INITIAL     [[:alpha:]]|"_"
SUBSEQUENT  {INITIAL}|[0-9]
IDENTIFIER  {INITIAL}{SUBSEQUENT}*

SPACE       [ \t\r]+

HEX         [a-fA-F0-9]
HEXCOLOR8   "#"{HEX}{8}
HEXCOLOR6   "#"{HEX}{6}
HEXCOLOR3   "#"{HEX}{3}

INTEGER     [0-9]+
DOUBLE      {INTEGER}|([0-9]+[.][0-9]*|[0-9]*[.][0-9]+)([eE][-+]?[0-9]+)?
STRING_ELEMENT  [^\\"]|"\\\""|"\\\\"|"\\n"|"\\t"
STRING      ["]{STRING_ELEMENT}*["]

%x comment

%%

"\n"    { yy_column = 1; }
{SPACE} {}

"[" { return T_LBRACKET; }
"]" { return T_RBRACKET; }
"{" { return T_LBRACE; }
"}" { return T_RBRACE; }
"(" { return T_LPAREN; }
")" { return T_RPAREN; }
":" { return T_COLON; }
";" { return T_SEMICOLON; }
"," { return T_COMMA; }
" " { return T_SPACE; }
"-" { return T_DASH; }
"+" { return T_PLUS; }
"|z" { return T_ZOOM; }
"*" { return T_STAR; }
"!" { return T_EXCLAMATION_MARK; }
"=" { return T_EQUALS; }
"." { return T_DOT; }

"!=" { yylval->binaryOp = MapCSSCondition::NotEqual; return T_BINARY_OP; }
"<"  { yylval->binaryOp = MapCSSCondition::LessThan; return T_BINARY_OP; }
">"  { yylval->binaryOp = MapCSSCondition::GreaterThan; return T_BINARY_OP; }
"<=" { yylval->binaryOp = MapCSSCondition::LessOrEqual; return T_BINARY_OP; }
">=" { yylval->binaryOp = MapCSSCondition::GreaterOrEqual; return T_BINARY_OP; }

"@import" { return T_KEYWORD_IMPORT; }
"url"     { return T_KEYWORD_URL; }
"rgba"    { return T_KEYWORD_RGBA; }
"rgb"     { return T_KEYWORD_RGB; }
"set"     { return T_KEYWORD_SET; }

{IDENTIFIER} {
    yylval->strRef.str = yytext;
    yylval->strRef.len = yyleng;
    return T_IDENT;
}

{HEXCOLOR8}  {
    // the 32bit signed version of strtol isn't good enough for 32bit unsigned
#if Q_PROCESSOR_WORDSIZE == 8
    yylval->uintVal = strtol(yytext + 1, nullptr, 16);
#elif Q_PROCESSOR_WORDSIZE == 4
    yylval->uintVal = strtoll(yytext + 1, nullptr, 16);
#else
#error unknown wordsize!?
#endif
    return T_HEX_COLOR;
}
{HEXCOLOR6}  {
    yylval->uintVal = 0xff000000 | strtol(yytext + 1, nullptr, 16);
    return T_HEX_COLOR;
}
{HEXCOLOR3}  {
    const int n = strtol(yytext + 1, nullptr, 16);
    yylval->uintVal = 0xff000000 | (n & 0xf00) << 12 | (n & 0xf00) << 8 | (n & 0x0f0) << 8 | (n & 0x0f0) << 4 | (n & 0x00f) << 4 | (n & 0x00f) << 0;
    return T_HEX_COLOR;
}

{STRING} {
    yylval->str = unquoteString(yytext);
    return T_STRING;
}

{DOUBLE} {
    yylval->doubleVal = qstrtod(yytext, nullptr, nullptr); // ### waiting for std::from_chars
    return T_DOUBLE;
}

"//"[^\n]*\n { yy_column = 1; }

"/*"             BEGIN(comment);
<comment>[^*\n]*
<comment>"*"+[^*/\n]*
<comment>\n      { yy_column = 1; }
<comment>"*"+"/" BEGIN(INITIAL);

. {
    printf("unexpected character: %s at %d:%d\n", yytext, yylloc->first_line, yylloc->first_column);
}

%%
