
#pragma once

#include "sqlite/Token.h"
#include "sqlite/yyStackEntry.h"
  typedef union YYMINORTYPE YYMINORTYPE;

  struct yyParser;
  struct yyParser {
    yyStackEntry *yytos;

    Parse *pParse;
    yyStackEntry *yystackEnd;
    yyStackEntry *yystack;
    yyStackEntry yystk0[50];
  };

  int yyGrowStack(yyParser * p);
  void yy_destructor(yyParser * yypParser, unsigned short int yymajor, YYMINORTYPE *yypminor);
  void yy_pop_parser_stack(yyParser * pParser);
  void yyStackOverflow(yyParser * yypParser);
  void yy_shift(yyParser * yypParser, unsigned short int yyNewState, unsigned short int yyMajor, Token yyMinor);
  void yy_accept(yyParser *);
  unsigned short int yy_reduce(yyParser * yypParser, unsigned int yyruleno, int yyLookahead, Token yyLookaheadToken,
                               Parse *pParse);
  void yy_syntax_error(yyParser * yypParser, int yymajor, Token yyminor);

  extern const signed char yyRuleInfoNRhs[412];


