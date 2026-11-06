%option noyywrap
%option nounistd
%option never-interactive

%{
#include <cstdlib>
#include <cstring>
#include <io.h>

#define isatty _isatty

#include "parser.tab.h"
%}

%%

"int"       { return INT; }
"main"      { return MAIN; }
"return"    { return RETURN; }
"print"     { return PRINT; }

"if"        { return IF; }
"else"      { return ELSE; }

"=="        { return EQ; }
"!="        { return NE; }
"<="        { return LE; }
">="        { return GE; }

[0-9]+ {
    yylval.number = atoi(yytext);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    size_t length = strlen(yytext) + 1;
    yylval.text = (char*)malloc(length);

    if (!yylval.text)
        YY_FATAL_ERROR("Out of memory");

    memcpy(yylval.text, yytext, length);
    return IDENTIFIER;
}

[ \t\r\n]+  { /* Ignore whitespace */ }

.           { return yytext[0]; }

%%
