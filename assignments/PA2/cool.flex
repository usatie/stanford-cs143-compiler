/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */
static int fill_string_buf(const char *text, size_t len);

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
ASSIGN          <-
WS              [ \n\f\r\t\v]
BLANK           {WS}
LETTER          [a-zA-Z]
DIGIT           [0-9]
INTEGER         {DIGIT}+
IDENTIFIER      ({LETTER}|_)({LETTER}|_|{DIGIT})*
IDCHAR          ({LETTER}|{DIGIT}|_)
TYPEID          [A-Z]{IDCHAR}*
OBJECTID        [a-z]{IDCHAR}*
SPECIAL_CHAR    [{}():;+\-/*~<=,.]
SINGLE_COMMENT  --.*
BEGIN_COMMENT   "(*"
END_COMMENT     "*)"
DOUBLE_QUOTE    \"

%START COMMENT STRING STRING_ERROR


%option noyywrap
%%
{SINGLE_COMMENT}        { /* ignore comment after two dashes */ }

 /*
  *  Nested comments
  */
{BEGIN_COMMENT}           { BEGIN COMMENT; }
<COMMENT>{
    {END_COMMENT}           { BEGIN INITIAL; }
    [^\*\)]*                { /* anything is ignored */ }
    \*                      { /* '*' is ignored */ }
    \)                      { /* ')' is ignored */ }
    <<EOF>>                 {
        BEGIN INITIAL;
        cool_yylval.error_msg = "EOF in comment";
        return (ERROR); 
    }
}

{END_COMMENT}             {
    cool_yylval.error_msg = "Unmatched *)";
    return (ERROR);
}


 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{ASSIGN}		{ return (ASSIGN); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
(?i:class)              { return (CLASS); }
(?i:else)               { return (ELSE); }
(?i:fi)                 { return (FI); }
(?i:if)                 { return (IF); }
(?i:in)                 { return (IN); }
(?i:inherits)           { return (INHERITS); }
(?i:isvoid)             { return (ISVOID); }
(?i:let)                { return (LET); }
(?i:loop)               { return (LOOP); }
(?i:pool)               { return (POOL); }
(?i:then)               { return (THEN); }
(?i:while)              { return (WHILE); }
(?i:case)               { return (CASE); }
(?i:esac)               { return (ESAC); }
(?i:new)                { return (NEW); }
(?i:of)                 { return (OF); }
(?i:not)                { return (NOT); }

 /* 
  * Keywords but case sensitive boolean constants
  */
false                   {
    yylval.boolean = 0;
    return (BOOL_CONST);
}
true                    {
    yylval.boolean = 1;
    return (BOOL_CONST);
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
<STRING_ERROR>{
    \\(.|\n)            { /* skip until the end of the string */ }
    [^"\\\n]+           { /* skip until the end of the string */ }
    \n                  { BEGIN INITIAL; }
    <<EOF>>             { BEGIN INITIAL; }
    .                   { /* skip until the end of the string */ }
}
<STRING>{
    \\b                 { if (fill_string_buf("\b", 1) < 0) return (ERROR); }
    \\t                 { if (fill_string_buf("\t", 1) < 0) return (ERROR); }
    \\n                 { if (fill_string_buf("\n", 1) < 0) return (ERROR); }
    \\f                 { if (fill_string_buf("\f", 1) < 0) return (ERROR); }
    \\(.|\n)            { if (fill_string_buf(yytext+1, 1) < 0) return (ERROR); }
    [^"\\\n]+           { if (fill_string_buf(yytext, yyleng) < 0) return (ERROR); }
    \n                  {
        BEGIN INITIAL;
        cool_yylval.error_msg = "Unterminated string constant";
        return (ERROR);
    }
    <<EOF>>             {
        BEGIN INITIAL;
        cool_yylval.error_msg = "EOF in string";
        return (ERROR);
    }
    {DOUBLE_QUOTE}      {
        BEGIN INITIAL;
	if (fill_string_buf("\0", 1) < 0) return (ERROR);
        cool_yylval.symbol = stringtable.add_string(string_buf);
        return (STR_CONST);
    }
    .                   {
        BEGIN STRING_ERROR;
        cool_yylval.error_msg = "String contains null character";
        return (ERROR);
    }
}
{DOUBLE_QUOTE}          {
    BEGIN STRING;
    string_buf_ptr = string_buf;
}

{TYPEID}                {
    cool_yylval.symbol = idtable.add_string(yytext, yyleng);
    return (TYPEID);
}
{OBJECTID}              {
    cool_yylval.symbol = idtable.add_string(yytext, yyleng);
    return (OBJECTID);
}
{INTEGER}               {
    cool_yylval.symbol = inttable.add_string(yytext, yyleng);
    return (INT_CONST);
}
 /*
  * Special Characters
  */
{SPECIAL_CHAR}          { return ((int)*yytext); }

{BLANK}                 { /* ignore white space */ }
.                       {
    char buf[2];
    buf[0] = *yytext;
    buf[1] = '\0';
    cool_yylval.error_msg = strdup(buf);
    return (ERROR);
}

%%

static int fill_string_buf(const char *text, size_t len) {
    if (string_buf_ptr + len <= string_buf + MAX_STR_CONST) {
        memcpy(string_buf_ptr, text, len);
        string_buf_ptr += len;
        return 0;
    } else {
        BEGIN STRING_ERROR;
        yylval.error_msg = "String constant too long";
        return -1;
    }
}
