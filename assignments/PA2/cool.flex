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
static void fill_string_buf(const char *text, int len);

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
ASSIGN          <-
WS              [ \n\f\r\t\v]
BLANK           {WS}+
LETTER          [a-zA-Z]
DIGIT           [0-9]
INTEGER         {DIGIT}+
IDENTIFIER      ({LETTER}|_)({LETTER}|_|{DIGIT})*
IDCHAR          ({LETTER}|{DIGIT}|_)
TYPEID          [A-Z]{IDCHAR}*
OBJECTID        [a-z]{IDCHAR}*
SPECIAL_CHAR    [{}():;+\-/*~<=]
DQ              \"
ESCAPED_DQ      \\\"
STR_CONST       {DQ}([^"]|{ESCAPED_DQ})*{DQ}
SINGLE_COMMENT  --.*
OPEN_COMMENT    \(\*
CLOSE_COMMENT   \*\)

%START NESTED_COMMENT


%option noyywrap
%%
{BLANK}                 { /* ignore white space */ }
{SINGLE_COMMENT}        { printf("single comment\n"); /* ignore comment after two dashes */ }

 /*
  *  Nested comments
  */
\(\*                    { printf("nested comment start\n"); BEGIN NESTED_COMMENT; }
<NESTED_COMMENT>\*\)           { printf("nexted comment end\n"); BEGIN 0; }
<NESTED_COMMENT>([^*/])*       { /* ignore */ }
<NESTED_COMMENT>\*             { /* ignore */ }
<NESTED_COMMENT>\)             { /* ignore */ }


 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{ASSIGN}		{ return (ASSIGN); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
class                   { return (CLASS); }
else                    { return (ELSE); }
false                   { return (BOOL_CONST); }
fi                      { return (FI); }
if                      { return (IF); }
in                      { return (IN); }
inherits                { return (INHERITS); }
isvoid                  { return (ISVOID); }
let                     { return (LET); }
loop                    { return (LOOP); }
pool                    { return (POOL); }
then                    { return (THEN); }
while                   { return (WHILE); }
case                    { return (CASE); }
esac                    { return (ESAC); }
new                     { return (NEW); }
of                      { return (OF); }
not                     { return (NOT); }
true                    { return (BOOL_CONST); }
{SPECIAL_CHAR}          { return ((int)*yytext); }


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
{STR_CONST}             {
    fill_string_buf(yytext, yyleng);
    cool_yylval.symbol = stringtable.add_string(string_buf);
    return (STR_CONST);
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
.                       {
    char buf[2];
    buf[0] = *yytext;
    buf[1] = '\0';
    cool_yylval.error_msg = strdup(buf);
    return (ERROR);
}

%%

static int starts_with(const char *text, const char *prefix) {
  while (*prefix) {
    if (*text++ != *prefix++) { return 0; }
  }
  return 1;
}

static void fill_string_buf(const char *text, int len) {
  const char *p = text + 1;
  string_buf_ptr = string_buf;
  int i = 1;
  while (p < text + len - 1) {
    if (starts_with(p, "\\b")) {
      *string_buf_ptr++ = '\b';
      p += 2;
    } else if (starts_with(p, "\\t")) {
      *string_buf_ptr++ = '\t';
      p += 2;
    } else if (starts_with(p, "\\n")) {
      *string_buf_ptr++ = '\n';
      p += 2;
    } else if (starts_with(p, "\\f")) {
      *string_buf_ptr++ = '\f';
      p += 2;
    } else if (text[i] == '\\') {
      *string_buf_ptr++ = *++p;
      p++;
    } else {
      *string_buf_ptr++ = *p++;
    }
  }
  *string_buf_ptr = '\0';
}
