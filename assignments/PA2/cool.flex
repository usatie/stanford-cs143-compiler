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
static int num_nested_comment = 0;
static void fill_string_buf(const char *text, size_t len);

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
ASSIGN          <-
LE              <=
WS              [ \n\f\r\t\v]
BLANK           {WS}
LETTER          [a-zA-Z]
DIGIT           [0-9]
INTEGER         {DIGIT}+
IDENTIFIER      ({LETTER}|_)({LETTER}|_|{DIGIT})*
IDCHAR          ({LETTER}|{DIGIT}|_)
TYPEID          [A-Z]{IDCHAR}*
OBJECTID        [a-z]{IDCHAR}*
SPECIAL_CHAR    [{}():;+\-/*~<=,.@]
SINGLE_COMMENT  --.*
BEGIN_COMMENT   "(*"
END_COMMENT     "*)"
DOUBLE_QUOTE    \"

%START COMMENT STRING STRING_ERROR


%option noyywrap
%%

 /*
  *  Nested comments
  */
<INITIAL>{
    {SINGLE_COMMENT}        { /* ignore comment after two dashes */ }
    {BEGIN_COMMENT}         {
        BEGIN COMMENT;
        num_nested_comment++;
    }
    {END_COMMENT}           {
        cool_yylval.error_msg = "Unmatched *)";
        return (ERROR);
    }
}
<COMMENT>{
	{BEGIN_COMMENT}         {
        num_nested_comment++;
	}
    {END_COMMENT}           {
        num_nested_comment--;
        if (num_nested_comment == 0) {
            BEGIN INITIAL;
        }
	}
    [^\*\(\)\n]*            { /* anything is ignored */ }
    \*                      { /* '*' is ignored */ }
    \)                      { /* ')' is ignored */ }
    \(                      { /* '(' is ignored */ }
    \n                      { curr_lineno++; }
    <<EOF>>                 {
        BEGIN INITIAL;
        cool_yylval.error_msg = "EOF in comment";
        return (ERROR); 
    }
}

 /*
 *  The multiple-character operators.
 */
<INITIAL>{
    {DARROW}                { return (DARROW); }
    {ASSIGN}                { return (ASSIGN); }
    {LE}                    { return (LE); }
}
    
 /*
 *  Keywords
 */
<INITIAL>{
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
    f(?i:alse)              {
        yylval.boolean = 0;
        return (BOOL_CONST);
    }
    t(?i:rue)               {
        yylval.boolean = 1;
        return (BOOL_CONST);
    }
}
    
 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
<STRING_ERROR>{
    \\\n                { /* skip until the end of the string */ curr_lineno++; }
    \\.                 { /* skip until the end of the string */ }
    [^"\\\n]+           { /* skip until the end of the string */ }
    \n                  { BEGIN INITIAL; curr_lineno++; return (ERROR); }
    <<EOF>>             { BEGIN INITIAL; return (ERROR); }
    {DOUBLE_QUOTE}      { BEGIN INITIAL; return (ERROR); }
    .                   { /* skip until the end of the string */ }
}
<STRING>{
    \\b                 { fill_string_buf("\b", 1); }
    \\t                 { fill_string_buf("\t", 1); }
    \\n                 { fill_string_buf("\n", 1); }
    \\f                 { fill_string_buf("\f", 1); }
    \\\0                {
        BEGIN STRING_ERROR;
        cool_yylval.error_msg = "String contains escaped null character";
    }
    \\\n                { fill_string_buf(yytext+1, 1); curr_lineno++; }
    \\.                 { fill_string_buf(yytext+1, 1); }
    [^"\\\n\0]+         { fill_string_buf(yytext, yyleng);  }
    \n                  {
        BEGIN INITIAL;
        cool_yylval.error_msg = "Unterminated string constant";
		curr_lineno++;
        return (ERROR);
    }
    <<EOF>>             {
        BEGIN INITIAL;
        cool_yylval.error_msg = "EOF in string";
        return (ERROR);
    }
    {DOUBLE_QUOTE}      {
        BEGIN INITIAL;
		*string_buf_ptr = '\0';
        cool_yylval.symbol = stringtable.add_string(string_buf);
        return (STR_CONST);
    }
    .                   {
        BEGIN STRING_ERROR;
        cool_yylval.error_msg = "String contains null character";
    }
}
<INITIAL>{
    {DOUBLE_QUOTE}          {
        BEGIN STRING;
        string_buf_ptr = string_buf;
    }
}

 /*
  * Integer Constant
  */
<INITIAL>{
    {INTEGER}               {
        cool_yylval.symbol = inttable.add_string(yytext, yyleng);
        return (INT_CONST);
    }
}

 /*
  * Identifiers
  */
<INITIAL>{
    {TYPEID}                {
        cool_yylval.symbol = idtable.add_string(yytext, yyleng);
        return (TYPEID);
    }
    {OBJECTID}              {
        cool_yylval.symbol = idtable.add_string(yytext, yyleng);
        return (OBJECTID);
    }
}

 /*
  * Special Characters
  */
<INITIAL>{
    {SPECIAL_CHAR}          { return ((int)*yytext); }
}
    
 /*
  * Blanks
  */
<INITIAL>{
    \n                      { curr_lineno++; }
    {BLANK}                 { /* ignore white space */ }
}

 /*
  * Invalid Characters
  */
<INITIAL>{
    .                       {
        char buf[2];
        buf[0] = *yytext;
        buf[1] = '\0';
        cool_yylval.error_msg = strdup(buf);
        return (ERROR);
    }
}

%%

static void fill_string_buf(const char *text, size_t len) {
    if (string_buf_ptr + len < string_buf + MAX_STR_CONST) {
        memcpy(string_buf_ptr, text, len);
        string_buf_ptr += len;
    } else {
        BEGIN STRING_ERROR;
        yylval.error_msg = "String constant too long";
    }
}
