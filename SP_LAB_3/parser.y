%code requires {
    #include "ast.h"
}

%{
#include <cstdio>
#include <memory>
#include <string>

#include "ast.h"

int yylex(void);
void yyerror(const char* message);

std::unique_ptr<Program> root;
%}

%union {
    int number;
    char* text;
    Expression* expression;
    Statement* statement;
    Program* program;
}

%token INT MAIN RETURN PRINT
%token <number> NUMBER
%token <text> IDENTIFIER

%token IF ELSE
%token EQ NE LE GE

%type <expression> expression
%type <statement> statement
%type <program> statements
%type <statement> block

%destructor { free($$); } <text>
%destructor { delete $$; } <expression> <statement> <program>

%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE

%nonassoc EQ NE '<' '>' LE GE
%left '+' '-'
%left '*' '/'

%start program

%%

program:
    INT MAIN '(' ')' '{' statements '}'
    {
        root.reset($6);
    }
    ;

statements:
    /* empty */
    {
        $$ = new Program();
    }
    | statements statement
    {
        $1->addStatement($2);
        $$ = $1;
    }
    ;

statement:
    INT IDENTIFIER '=' expression ';'
    {
        $$ = new DeclarationStatement($2, $4);
        free($2);
    }
    | IDENTIFIER '=' expression ';'
    {
        $$ = new AssignmentStatement($1, $3);
        free($1);
    }
    | PRINT '(' expression ')' ';'
    {
        $$ = new PrintStatement($3);
    }
    | RETURN expression ';'
    {
        $$ = new ReturnStatement($2);
    }
    | IF '(' expression ')' block %prec IF_WITHOUT_ELSE
    {
        $$ = new IfStatement($3, $5, nullptr);
    }
    | IF '(' expression ')' block ELSE block
    {
        $$ = new IfStatement($3, $5, $7);
    }

    ;
    
block:
    '{' statements '}'
    {
        auto* result = new BlockStatement();

        // Transfer statements from Program into a block
        // Implemented through the Program method below
        for (auto& statement : $2->releaseStatements()) {
            result->addStatement(statement.release());
        }

        delete $2;
        $$ = result;
    }
    ;

expression:
    NUMBER
    {
        $$ = new NumberExpression($1);
    }
    | IDENTIFIER
    {
        $$ = new VariableExpression($1);
        free($1);
    }
    | expression '+' expression
    {
        $$ = new BinaryExpression('+', $1, $3);
    }
    | expression '-' expression
    {
        $$ = new BinaryExpression('-', $1, $3);
    }
    | expression '*' expression
    {
        $$ = new BinaryExpression('*', $1, $3);
    }
    | expression '/' expression
    {
        $$ = new BinaryExpression('/', $1, $3);
    }
    | '(' expression ')'
    {
        $$ = $2;
    }
    
    | expression EQ expression
    {
        $$ = new ComparisonExpression("==", $1, $3);
    }
    | expression NE expression
    {
        $$ = new ComparisonExpression("!=", $1, $3);
    }
    | expression '<' expression
    {
        $$ = new ComparisonExpression("<", $1, $3);
    }
    | expression '>' expression
    {
        $$ = new ComparisonExpression(">", $1, $3);
    }
    | expression LE expression
    {
        $$ = new ComparisonExpression("<=", $1, $3);
    }
    | expression GE expression
    {
        $$ = new ComparisonExpression(">=", $1, $3);
    }

    ;

%%

void yyerror(const char* message)
{
    fprintf(stderr, "Syntax error: %s\n", message);
}
