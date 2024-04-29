%{
/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssparser_impl.h"
#include "style/mapcssparsercontext_p.h"
#include "style/mapcssselector_p.h"

// ### ugly workaround for locale-ignoring string to float conversion
// std::from_chars offers that with C++17, but isn't actually implemented yet for floats/doubles...
#include <private/qlocale_tools_p.h>

[[nodiscard]] extern char* unquoteSingleQuotedString(const char *str);
[[nodiscard]] extern char* unquoteDoubleQuotedString(const char *str);

using namespace KOSMIndoorMap;

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for (int i = 0; i < yyleng; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } else { \
            yylloc->last_column++; \
        } \
    }

%}

%option warn nodefault

%option reentrant
%option noyywrap
%option nounput
%option never-interactive
%option bison-bridge
%option bison-locations
%option yylineno
%option extra-type="MapCSSParserContext*"

INITIAL     [[:alpha:]]|"_"
SUBSEQUENT  {INITIAL}|[0-9]
IDENTIFIER  {INITIAL}{SUBSEQUENT}*

SPACE       [ \t\r\n]+

HEX         [a-fA-F0-9]
HEXCOLOR8   "#"{HEX}{8}
HEXCOLOR6   "#"{HEX}{6}
HEXCOLOR3   "#"{HEX}{3}

INTEGER     [0-9]+
DOUBLE      {INTEGER}|([0-9]+[.][0-9]*|[0-9]*[.][0-9]+)([eE][-+]?[0-9]+)?
STRING1_ELEMENT  [^']|"\\\'"|"\\\\"|"\\n"|"\\t"
STRING1      [']{STRING1_ELEMENT}*[']
STRING2_ELEMENT  [^\\"]|"\\\""|"\\\\"|"\\n"|"\\t"
STRING2      ["]{STRING2_ELEMENT}*["]

%x comment

%%

%{
    // select entry point for the parser by starting with a dummy token
    // https://www.gnu.org/software/bison/manual/html_node/Multiple-start_002dsymbols.html
    if (!yyextra->m_init) {
        yyextra->m_init = true;
        return yyextra->m_mode == MapCSSParserContext::ParseMapCSS ? T_MAPCSS_START : T_EVAL_EXPRESSION_START;
    }
%}

{SPACE} {}

"[" { return T_LBRACKET; }
"]" { return T_RBRACKET; }
"{" { return T_LBRACE; }
"}" { return T_RBRACE; }
"(" { return T_LPAREN; }
")" { return T_RPAREN; }
"::" { return T_DOUBLE_COLON; }
":" { return T_COLON; }
";" { return T_SEMICOLON; }
"," { return T_COMMA; }
"-" { return T_DASH; }
"+" { return T_PLUS; }
"|z" { return T_ZOOM; }
"*" { return T_STAR; }
"!" { return T_EXCLAMATION_MARK; }
"=" { return T_EQUALS; }
"." { return T_DOT; }
"/" { return T_SLASH; }

"==" { return T_COMPARE_EQUAL; }
"!=" { return T_COMPARE_NOT_EQUAL; }
"<"  { return T_COMPARE_LT; }
">"  { return T_COMPARE_GT; }
"<=" { return T_COMPARE_LE; }
">=" { return T_COMPARE_GE; }

"&&" { return T_LOGICAL_AND; }
"||" { return T_LOGICAL_OR; }

"@import" { return T_KEYWORD_IMPORT; }
"url"     { return T_KEYWORD_URL; }
"rgba"    { return T_KEYWORD_RGBA; }
"rgb"     { return T_KEYWORD_RGB; }
"set"     { return T_KEYWORD_SET; }
"eval"    { return T_KEYWORD_EVAL; }

"true" {
    yylval->boolVal = true;
    return T_BOOLEAN_LITERAL;
}
"false" {
    yylval->boolVal = false;
    return T_BOOLEAN_LITERAL;
}

{IDENTIFIER} {
    yylval->strRef.str = yytext;
    yylval->strRef.len = yyleng;
    return T_IDENT;
}

{HEXCOLOR8}  {
    // the 32bit signed version of strtol isn't good enough for 32bit unsigned
    static_assert(sizeof(long) == 4 || sizeof(long) == 8);
    if constexpr (sizeof(long) == 8) {
        yylval->uintVal = strtol(yytext + 1, nullptr, 16);
    } else if constexpr (sizeof(long) == 4) {
        yylval->uintVal = strtoll(yytext + 1, nullptr, 16);
    }
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

{STRING1} {
    yylval->str = unquoteSingleQuotedString(yytext);
    return T_STRING;
}
{STRING2} {
    yylval->str = unquoteDoubleQuotedString(yytext);
    return T_STRING;
}

{DOUBLE} {
    yylval->doubleVal = qstrtod(yytext, nullptr, nullptr); // ### waiting for std::from_chars
    return T_DOUBLE;
}

"//"[^\n]*\n {}

"/*"             BEGIN(comment);
<comment>[^*\n]*
<comment>"*"+[^*/\n]*
<comment>\n      {}
<comment>"*"+"/" BEGIN(INITIAL);

. {
    printf("unexpected character: %s at %d:%d\n", yytext, yylloc->first_line, yylloc->first_column);
}

%%
