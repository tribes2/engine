//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

/* yacc -P console\yyparse.c -p CMD -D console\gram.h -o console\gram.cc console\gram.y */
#ifdef YYTRACE
#define YYDEBUG 1
#else
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#endif
/*
 * Portable way of defining ANSI C prototypes
 */
#ifndef YY_ARGS
#ifdef __STDC__
#define YY_ARGS(x)	x
#else
#define YY_ARGS(x)	()
#endif
#endif

#ifdef YACC_WINDOWS

#include <windows.h>

/*
 * the following is the handle to the current
 * instance of a windows program. The user
 * program calling CMDparse must supply this!
 */

#ifdef STRICT
extern HINSTANCE hInst;	
#else
extern HANDLE hInst;	
#endif

#endif	/* YACC_WINDOWS */

#if YYDEBUG
typedef struct yyNamedType_tag {	/* Tokens */
	char	* name;		/* printable name */
	short	token;		/* token # */
	short	type;		/* token type */
} yyNamedType;
typedef struct yyTypedRules_tag {	/* Typed rule table */
	char	* name;		/* compressed rule string */
	short	type;		/* rule result type */
} yyTypedRules;

#endif

#line 1 "console/gram.y"

#include "console/console.h"
#include "console/ast.h"
#include <stdlib.h>
#include "stdio.h"
#include "console/consoleInternal.h"

#ifndef YYDEBUG
#define YYDEBUG
#endif

#define YYSSIZE 350

int outtext(char *fmt, ...);
extern int serrors;
#define nil 0
#undef YY_ARGS
#define YY_ARGS(x)	x
#line 20 "console/gram.y"

        /* Reserved Word Definitions */
#define rwDEFINE	257
#define rwENDDEF	258
#define rwDECLARE	259
#define rwBREAK	260
#define rwELSE	261
#define rwCONTINUE	262
#define rwGLOBAL	263
#define rwIF	264
#define rwNIL	265
#define rwRETURN	266
#define rwWHILE	267
#define rwENDIF	268
#define rwENDWHILE	269
#define rwENDFOR	270
#define rwDEFAULT	271
#define rwFOR	272
#define rwDATABLOCK	273
#define rwSWITCH	274
#define rwCASE	275
#define rwSWITCHSTR	276
#define rwCASEOR	277
#define rwPACKAGE	278
#define ILLEGAL_TOKEN	279
#line 30 "console/gram.y"

        /* Constants and Identifier Definitions */
#define CHRCONST	280
#define INTCONST	281
#define TTAG	282
#define VAR	283
#define IDENT	284
#define STRATOM	285
#define TAGATOM	286
#define FLTCONST	287
#line 42 "console/gram.y"

        /* Operator Definitions */
#define opMINUSMINUS	288
#define opPLUSPLUS	289
#define STMT_SEP	290
#define opSHL	291
#define opSHR	292
#define opPLASN	293
#define opMIASN	294
#define opMLASN	295
#define opDVASN	296
#define opMODASN	297
#define opANDASN	298
#define opXORASN	299
#define opORASN	300
#define opSLASN	301
#define opSRASN	302
#define opCAT	303
#define opEQ	304
#define opNE	305
#define opGE	306
#define opLE	307
#define opAND	308
#define opOR	309
#define opSTREQ	310
#define opCOLONCOLON	311
typedef union {
	char c;
	int i;
	const char *s;
   char *str;
	double f;
	StmtNode *stmt;
	ExprNode *expr;
   SlotAssignNode *slist;
   VarNode *var;
   SlotDecl slot;
   ObjectBlockDecl odcl;
   ObjectDeclNode *od;
   AssignDecl asn;
   IfStmtNode *ifnode;
} YYSTYPE;
#define opMDASN	312
#define opNDASN	313
#define opNTASN	314
#define opSTRNE	315
#define UNARY	316
extern int CMDchar, yyerrflag;
extern YYSTYPE CMDlval;
#if YYDEBUG
enum YY_Types { YY_t_NoneDefined, YY_t_i, YY_t_c, YY_t_s, YY_t_str, YY_t_f, YY_t_ifnode, YY_t_stmt, YY_t_expr, YY_t_od, YY_t_odcl, YY_t_slist, YY_t_slot, YY_t_var, YY_t_asn
};
#endif
#if YYDEBUG
yyTypedRules yyRules[] = {
	{ "&00: %35 &00",  0},
	{ "%35: %04",  0},
	{ "%04:",  7},
	{ "%04: %04 %03",  7},
	{ "%03: %09",  7},
	{ "%03: %06",  7},
	{ "%03: %05",  7},
	{ "%05: &23 &29 &49 %07 &50 &48",  7},
	{ "%07: %06",  7},
	{ "%07: %07 %06",  7},
	{ "%08:",  7},
	{ "%08: %08 %09",  7},
	{ "%09: %19",  7},
	{ "%09: %20",  7},
	{ "%09: %21",  7},
	{ "%09: %23",  7},
	{ "%09: %02",  7},
	{ "%09: &05 &48",  7},
	{ "%09: &07 &48",  7},
	{ "%09: &11 &48",  7},
	{ "%09: &11 %27 &48",  7},
	{ "%09: %31 &48",  7},
	{ "%09: &27 &39 %27 &48",  7},
	{ "%09: &27 &39 %27 &46 %27 &48",  7},
	{ "%06: &02 &29 &44 %33 &45 &49 %08 &50",  7},
	{ "%06: &02 &29 &78 &29 &44 %33 &45 &49 %08 &50",  7},
	{ "%33:",  13},
	{ "%33: %32",  13},
	{ "%32: &28",  13},
	{ "%32: %32 &46 &28",  13},
	{ "%23: &18 &29 &44 &29 &45 &49 %28 &50 &48",  7},
	{ "%23: &18 &29 &44 &29 &45 &47 &29 &49 %28 &50 &48",  7},
	{ "%24: &04 %18 &44 %14 %15 &45 &49 %26 &50",  9},
	{ "%24: &04 %18 &44 %14 %15 &45",  9},
	{ "%14:",  8},
	{ "%14: %27",  8},
	{ "%15:",  8},
	{ "%15: &46 %10",  8},
	{ "%26:",  10},
	{ "%26: %28",  10},
	{ "%26: %25",  10},
	{ "%26: %28 %25",  10},
	{ "%25: %24 &48",  9},
	{ "%25: %25 %24 &48",  9},
	{ "%22: &49 %08 &50",  7},
	{ "%22: %09",  7},
	{ "%02: &19 &44 %27 &45 &49 %01 &50",  7},
	{ "%02: &21 &44 %27 &45 &49 %01 &50",  7},
	{ "%01: &20 %17 &47 %08",  6},
	{ "%01: &20 %17 &47 %08 &16 &47 %08",  6},
	{ "%01: &20 %17 &47 %08 %01",  6},
	{ "%17: %27",  8},
	{ "%17: %17 &22 %27",  8},
	{ "%19: &09 &44 %27 &45 %22",  7},
	{ "%19: &09 &44 %27 &45 %22 &06 %22",  7},
	{ "%20: &12 &44 %27 &45 %22",  7},
	{ "%21: &17 &44 %27 &48 %27 &48 %27 &45 %22",  7},
	{ "%31: %16",  7},
	{ "%27: %16",  8},
	{ "%27: &44 %27 &45",  8},
	{ "%27: %27 &51 %27",  8},
	{ "%27: %27 &43 %27",  8},
	{ "%27: %27 &42 %27",  8},
	{ "%27: %27 &41 %27",  8},
	{ "%27: %27 &33 %27",  8},
	{ "%27: %27 &34 %27",  8},
	{ "%27: %27 &35 %27",  8},
	{ "%27: %27 &36 %27",  8},
	{ "%27: &34 %27",  8},
	{ "%27: &35 %27",  8},
	{ "%27: &27",  8},
	{ "%27: %27 &83 %27 &47 %27",  8},
	{ "%27: %27 &37 %27",  8},
	{ "%27: %27 &38 %27",  8},
	{ "%27: %27 &73 %27",  8},
	{ "%27: %27 &74 %27",  8},
	{ "%27: %27 &71 %27",  8},
	{ "%27: %27 &72 %27",  8},
	{ "%27: %27 &76 %27",  8},
	{ "%27: %27 &58 %27",  8},
	{ "%27: %27 &59 %27",  8},
	{ "%27: %27 &75 %27",  8},
	{ "%27: %27 &77 %27",  8},
	{ "%27: %27 &84 %27",  8},
	{ "%27: %27 &54 %27",  8},
	{ "%27: &53 %27",  8},
	{ "%27: &52 %27",  8},
	{ "%27: &31",  8},
	{ "%27: &32",  8},
	{ "%27: &26",  8},
	{ "%27: &05",  8},
	{ "%27: %30",  8},
	{ "%27: &29",  8},
	{ "%27: &30",  8},
	{ "%27: &28",  8},
	{ "%27: &28 &79 %12 &86",  8},
	{ "%30: %27 &40 &29",  12},
	{ "%30: %27 &40 &29 &79 %12 &86",  12},
	{ "%18: &29",  8},
	{ "%18: &44 %27 &45",  8},
	{ "%34: &56",  14},
	{ "%34: &55",  14},
	{ "%34: &60 %27",  14},
	{ "%34: &61 %27",  14},
	{ "%34: &62 %27",  14},
	{ "%34: &63 %27",  14},
	{ "%34: &64 %27",  14},
	{ "%34: &65 %27",  14},
	{ "%34: &66 %27",  14},
	{ "%34: &67 %27",  14},
	{ "%34: &68 %27",  14},
	{ "%34: &69 %27",  14},
	{ "%16: %13",  8},
	{ "%16: %24",  8},
	{ "%16: &28 &39 %27",  8},
	{ "%16: &28 &79 %12 &86 &39 %27",  8},
	{ "%16: &28 %34",  8},
	{ "%16: &28 &79 %12 &86 %34",  8},
	{ "%16: %30 %34",  8},
	{ "%16: %30 &39 %27",  8},
	{ "%16: %30 &39 &49 %10 &50",  8},
	{ "%13: &29 &44 %11 &45",  8},
	{ "%13: &29 &78 &29 &44 %11 &45",  8},
	{ "%13: %27 &40 &29 &44 %11 &45",  8},
	{ "%11:",  8},
	{ "%11: %10",  8},
	{ "%10: %27",  8},
	{ "%10: %10 &46 %27",  8},
	{ "%28: %29",  11},
	{ "%28: %28 %29",  11},
	{ "%29: &29 &39 %27 &48",  11},
	{ "%29: &18 &39 %27 &48",  11},
	{ "%29: &29 &79 %12 &86 &39 %27 &48",  11},
	{ "%12: %27",  8},
	{ "%12: %12 &46 %27",  8},
{ "$accept",  0},{ "error",  0}
};
yyNamedType yyTokenTypes[] = {
	{ "$end",  0,  0},
	{ "error",  256,  0},
	{ "rwDEFINE",  257,  1},
	{ "rwENDDEF",  258,  1},
	{ "rwDECLARE",  259,  1},
	{ "rwBREAK",  260,  1},
	{ "rwELSE",  261,  1},
	{ "rwCONTINUE",  262,  1},
	{ "rwGLOBAL",  263,  1},
	{ "rwIF",  264,  1},
	{ "rwNIL",  265,  1},
	{ "rwRETURN",  266,  1},
	{ "rwWHILE",  267,  1},
	{ "rwENDIF",  268,  1},
	{ "rwENDWHILE",  269,  1},
	{ "rwENDFOR",  270,  1},
	{ "rwDEFAULT",  271,  1},
	{ "rwFOR",  272,  1},
	{ "rwDATABLOCK",  273,  1},
	{ "rwSWITCH",  274,  1},
	{ "rwCASE",  275,  1},
	{ "rwSWITCHSTR",  276,  1},
	{ "rwCASEOR",  277,  1},
	{ "rwPACKAGE",  278,  1},
	{ "ILLEGAL_TOKEN",  279,  0},
	{ "CHRCONST",  280,  2},
	{ "INTCONST",  281,  1},
	{ "TTAG",  282,  3},
	{ "VAR",  283,  3},
	{ "IDENT",  284,  3},
	{ "STRATOM",  285,  4},
	{ "TAGATOM",  286,  4},
	{ "FLTCONST",  287,  5},
	{ "'+'",  43,  1},
	{ "'-'",  45,  1},
	{ "'*'",  42,  1},
	{ "'/'",  47,  1},
	{ "'<'",  60,  1},
	{ "'>'",  62,  1},
	{ "'='",  61,  1},
	{ "'.'",  46,  1},
	{ "'|'",  124,  1},
	{ "'&'",  38,  1},
	{ "'%'",  37,  1},
	{ "'('",  40,  1},
	{ "')'",  41,  1},
	{ "','",  44,  1},
	{ "':'",  58,  1},
	{ "';'",  59,  1},
	{ "'{'",  123,  1},
	{ "'}'",  125,  1},
	{ "'^'",  94,  1},
	{ "'~'",  126,  1},
	{ "'!'",  33,  1},
	{ "'@'",  64,  1},
	{ "opMINUSMINUS",  288,  1},
	{ "opPLUSPLUS",  289,  1},
	{ "STMT_SEP",  290,  1},
	{ "opSHL",  291,  1},
	{ "opSHR",  292,  1},
	{ "opPLASN",  293,  1},
	{ "opMIASN",  294,  1},
	{ "opMLASN",  295,  1},
	{ "opDVASN",  296,  1},
	{ "opMODASN",  297,  1},
	{ "opANDASN",  298,  1},
	{ "opXORASN",  299,  1},
	{ "opORASN",  300,  1},
	{ "opSLASN",  301,  1},
	{ "opSRASN",  302,  1},
	{ "opCAT",  303,  1},
	{ "opEQ",  304,  1},
	{ "opNE",  305,  1},
	{ "opGE",  306,  1},
	{ "opLE",  307,  1},
	{ "opAND",  308,  1},
	{ "opOR",  309,  1},
	{ "opSTREQ",  310,  1},
	{ "opCOLONCOLON",  311,  1},
	{ "'['",  91,  0},
	{ "opMDASN",  312,  0},
	{ "opNDASN",  313,  0},
	{ "opNTASN",  314,  0},
	{ "'?'",  63,  0},
	{ "opSTRNE",  315,  0},
	{ "UNARY",  316,  0},
	{ "']'",  93,  0}

};
#endif
static short yydef[] = {

	   3, 65535,   49,   48,   47, 65531,   95,   91, 65527,   46, 
	  45,   30,   29,   62,   61,   60,   59,   58,   57,   56, 
	  55,   54,   53,   52,   69,   66,   67,   64,   51,   44, 
	  43,   42,   41,   40,   39,   38,   37,   36,   35,   34, 
	  33,   32,   28,   27,   26,   25,   24,   23,   22,   21, 
	65523, 65517,   50, 65513, 65509,    8, 65505,    5,   70,   68, 
	  31,   19, 65501,   63,   10,    6,  123,   17, 65497,  123, 
	  13,   12,  123,   18,   15,   14,  123,   16
};
static short yyex[] = {

	   0,    0, 65535,    1,   59,   20, 65535,   97,   41,   65, 
	65535,    1,   41,    7,   44,    7, 65535,    1,   41,    4, 
	65535,    1,   41,   65, 65535,    1,   41,   65, 65535,    1, 
	  41,    9, 65535,    1,   41,    4, 65535,    1,  125,   11, 
	65535,    1
};
static short yyact[] = {

	65310, 65314, 65311, 65312, 65309, 65322, 65320, 65528, 65325, 65317, 
	65324, 65316, 65315, 65321, 65319, 65318, 65326, 65287, 65529, 65533, 
	65532, 65289, 65285, 65286,  287,  286,  285,  284,  283,  282, 
	 281,  278,  276,  274,  273,  272,  267,  266,  264,  262, 
	 260,  259,  257,  126,   45,   42,   40,   33, 65327, 65338, 
	65294, 65293, 65337, 65336, 65335, 65334, 65333, 65332, 65331, 65330, 
	65329, 65328,  302,  301,  300,  299,  298,  297,  296,  295, 
	 294,  293,  289,  288,   91,   61, 65527, 65339,  311,   40, 
	65340, 65294, 65293, 65337, 65336, 65335, 65334, 65333, 65332, 65331, 
	65330, 65329, 65328,  302,  301,  300,  299,  298,  297,  296, 
	 295,  294,  293,  289,  288,   61, 65310, 65314, 65311, 65312, 
	65309, 65320, 65288, 65287, 65284, 65533, 65532, 65289, 65285, 65286, 
	 287,  286,  285,  284,  283,  282,  281,  260,  259,  126, 
	  45,   42,   40,   33, 65362, 65361, 65357, 65359, 65358, 65341, 
	65356, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 65350, 
	65349, 65352, 65351, 65345, 65348, 65344, 65343,  315,  310,  309, 
	 308,  307,  306,  305,  304,  292,  291,  124,   94,   64, 
	  63,   62,   60,   47,   46,   45,   43,   42,   38,   37, 
	65365,   40, 65366,   40, 65367,   40, 65368,   40, 65369,   40, 
	65370, 65291,  284,   40, 65372,  284, 65373,  284, 65374,   61, 
	65262,   59, 65310, 65314, 65311, 65312, 65260, 65309, 65320, 65288, 
	65287, 65284, 65533, 65532, 65289, 65285, 65286,  287,  286,  285, 
	 284,  283,  282,  281,  260,  259,  126,   59,   45,   42, 
	  40,   33, 65259,   59, 65258,   59, 65376,  284, 65378,  284, 
	65310, 65314, 65311, 65312, 65380, 65309, 65320, 65288, 65287, 65284, 
	65533, 65532, 65289, 65285, 65286,  287,  286,  285,  284,  283, 
	 282,  281,  260,  259,  126,  123,   45,   42,   40,   33, 
	65341,   46, 65507,  284, 65362, 65361, 65283, 65357, 65359, 65358, 
	65341, 65356, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 
	65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343,  315,  310, 
	 309,  308,  307,  306,  305,  304,  292,  291,  124,   94, 
	  64,   63,   62,   60,   47,   46,   45,   43,   42,   41, 
	  38,   37, 65485,   40, 65388,   40, 65484, 65389,  311,   40, 
	65362, 65361, 65357, 65359, 65358, 65341, 65356, 65261, 65354, 65353, 
	65355, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 
	65345, 65348, 65344, 65343,  315,  310,  309,  308,  307,  306, 
	 305,  304,  292,  291,  124,   94,   64,   63,   62,   60, 
	  59,   47,   46,   45,   43,   42,   38,   37, 65391,  123, 
	65392, 65483,   93,   44, 65482,   40, 65393,   44, 65301,   41, 
	65481, 65395,   91,   40, 65362, 65357, 65359, 65358, 65341, 65356, 
	65347, 65346,  292,  291,   47,   46,   45,   43,   42,   37, 
	65362, 65361, 65357, 65359, 65358, 65341, 65356, 65354, 65353, 65342, 
	65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 65344, 65343, 
	 315,  310,  307,  306,  305,  304,  292,  291,  124,   94, 
	  64,   62,   60,   47,   46,   45,   43,   42,   38,   37, 
	65362, 65357, 65359, 65358, 65341, 65356,   47,   46,   45,   43, 
	  42,   37, 65362, 65361, 65357, 65359, 65358, 65341, 65356, 65354, 
	65353, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 
	65345, 65344, 65343,  315,  310,  308,  307,  306,  305,  304, 
	 292,  291,  124,   94,   64,   62,   60,   47,   46,   45, 
	  43,   42,   38,   37, 65362, 65357, 65359, 65358, 65341, 65356, 
	65354, 65353, 65342, 65347, 65346, 65352, 65351, 65344, 65343,  315, 
	 310,  307,  306,  292,  291,   64,   62,   60,   47,   46, 
	  45,   43,   42,   37, 65362, 65357, 65359, 65358, 65341, 65356, 
	65342, 65347, 65346, 65344, 65343,  315,  310,  292,  291,   64, 
	  47,   46,   45,   43,   42,   37, 65362, 65361, 65357, 65359, 
	65358, 65341, 65356, 65396, 65354, 65353, 65355, 65342, 65363, 65360, 
	65347, 65346, 65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343, 
	 315,  310,  309,  308,  307,  306,  305,  304,  292,  291, 
	 124,   94,   64,   63,   62,   60,   58,   47,   46,   45, 
	  43,   42,   38,   37, 65362, 65357, 65341, 65356,   47,   46, 
	  42,   37, 65362, 65361, 65357, 65359, 65358, 65341, 65356, 65354, 
	65353, 65342, 65363, 65347, 65346, 65350, 65349, 65352, 65351, 65344, 
	65343,  315,  310,  307,  306,  305,  304,  292,  291,   94, 
	  64,   62,   60,   47,   46,   45,   43,   42,   38,   37, 
	65362, 65357, 65359, 65358, 65341, 65356, 65354, 65353, 65342, 65347, 
	65346, 65350, 65349, 65352, 65351, 65344, 65343,  315,  310,  307, 
	 306,  305,  304,  292,  291,   64,   62,   60,   47,   46, 
	  45,   43,   42,   37, 65362, 65361, 65357, 65359, 65358, 65341, 
	65356, 65354, 65353, 65342, 65347, 65346, 65350, 65349, 65352, 65351, 
	65344, 65343,  315,  310,  307,  306,  305,  304,  292,  291, 
	  64,   62,   60,   47,   46,   45,   43,   42,   38,   37, 
	65362, 65361, 65357, 65359, 65358, 65341, 65356, 65397, 65354, 65353, 
	65355, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 
	65345, 65348, 65344, 65343,  315,  310,  309,  308,  307,  306, 
	 305,  304,  292,  291,  124,   94,   64,   63,   62,   60, 
	  59,   47,   46,   45,   43,   42,   38,   37, 65362, 65361, 
	65398, 65357, 65359, 65358, 65341, 65356, 65354, 65353, 65355, 65342, 
	65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 65345, 65348, 
	65344, 65343,  315,  310,  309,  308,  307,  306,  305,  304, 
	 292,  291,  124,   94,   64,   63,   62,   60,   47,   46, 
	  45,   43,   42,   41,   38,   37, 65362, 65361, 65399, 65357, 
	65359, 65358, 65341, 65356, 65354, 65353, 65355, 65342, 65363, 65360, 
	65347, 65346, 65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343, 
	 315,  310,  309,  308,  307,  306,  305,  304,  292,  291, 
	 124,   94,   64,   63,   62,   60,   47,   46,   45,   43, 
	  42,   41,   38,   37, 65362, 65361, 65400, 65357, 65359, 65358, 
	65341, 65356, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 
	65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343,  315,  310, 
	 309,  308,  307,  306,  305,  304,  292,  291,  124,   94, 
	  64,   63,   62,   60,   47,   46,   45,   43,   42,   41, 
	  38,   37, 65362, 65361, 65401, 65357, 65359, 65358, 65341, 65356, 
	65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 
	65352, 65351, 65345, 65348, 65344, 65343,  315,  310,  309,  308, 
	 307,  306,  305,  304,  292,  291,  124,   94,   64,   63, 
	  62,   60,   47,   46,   45,   43,   42,   41,   38,   37, 
	65362, 65361, 65292, 65357, 65359, 65358, 65341, 65356, 65354, 65353, 
	65355, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 
	65345, 65348, 65344, 65343,  315,  310,  309,  308,  307,  306, 
	 305,  304,  292,  291,  124,   94,   64,   63,   62,   60, 
	  47,   46,   45,   43,   42,   41,   38,   37, 65402,  284, 
	65403,  284, 65267,  283, 65362, 65361, 65357, 65359, 65405, 65358, 
	65341, 65356, 65263, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 
	65346, 65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343,  315, 
	 310,  309,  308,  307,  306,  305,  304,  292,  291,  124, 
	  94,   64,   63,   62,   60,   59,   47,   46,   45,   44, 
	  43,   42,   38,   37, 65322,  257, 65407, 65294, 65293, 65337, 
	65336, 65335, 65334, 65333, 65332, 65331, 65330, 65329, 65328,  302, 
	 301,  300,  299,  298,  297,  296,  295,  294,  293,  289, 
	 288,   61, 65393, 65300,  125,   44, 65310, 65314, 65311, 65312, 
	65256, 65309, 65320, 65528, 65325, 65317, 65324, 65316, 65315, 65321, 
	65319, 65318, 65287, 65529, 65533, 65532, 65289, 65285, 65286,  287, 
	 286,  285,  284,  283,  282,  281,  276,  274,  273,  272, 
	 267,  266,  264,  262,  260,  259,  126,  123,   45,   42, 
	  40,   33, 65412,  123, 65413,  123, 65414,   44, 65416,   41, 
	65473,   40, 65417,   44, 65418,   41, 65420, 65322,  257,  125, 
	65302,   41, 65303,   41, 65392, 65290,   93,   44, 65362, 65361, 
	65357, 65359, 65358, 65341, 65356, 65354, 65353, 65342, 65363, 65360, 
	65347, 65346, 65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343, 
	 315,  310,  309,  308,  307,  306,  305,  304,  292,  291, 
	 124,   94,   64,   62,   60,   47,   46,   45,   43,   42, 
	  38,   37, 65362, 65361, 65357, 65359, 65358, 65341, 65356, 65421, 
	65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 
	65352, 65351, 65345, 65348, 65344, 65343,  315,  310,  309,  308, 
	 307,  306,  305,  304,  292,  291,  124,   94,   64,   63, 
	  62,   60,   59,   47,   46,   45,   43,   42,   38,   37, 
	65423,  261, 65424,  275, 65470,   41, 65427, 65428,  123,   58, 
	65268,  283, 65469,  123, 65362, 65361, 65357, 65359, 65358, 65341, 
	65356, 65264, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 
	65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343,  315,  310, 
	 309,  308,  307,  306,  305,  304,  292,  291,  124,   94, 
	  64,   63,   62,   60,   59,   47,   46,   45,   43,   42, 
	  38,   37, 65253,   59, 65310, 65314, 65311, 65312, 65274, 65309, 
	65320, 65528, 65325, 65317, 65324, 65316, 65315, 65321, 65319, 65318, 
	65287, 65529, 65533, 65532, 65289, 65285, 65286,  287,  286,  285, 
	 284,  283,  282,  281,  276,  274,  273,  272,  267,  266, 
	 264,  262,  260,  259,  126,  125,   45,   42,   40,   33, 
	65277,  125, 65276,  125, 65467,  123, 65432,  284, 65433, 65434, 
	 284,  273, 65436,   41, 65362, 65361, 65438, 65357, 65359, 65358, 
	65341, 65356, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 
	65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343,  315,  310, 
	 309,  308,  307,  306,  305,  304,  292,  291,  124,   94, 
	  64,   63,   62,   60,   47,   46,   45,   43,   42,   41, 
	  38,   37, 65466, 65439,  277,   58, 65320, 65433, 65434,  284, 
	 273,  259, 65442,  123, 65443,   61, 65445, 65444,   91,   61, 
	65446, 65433, 65434,  284,  273,  125, 65463,  123, 65310, 65314, 
	65311, 65312, 65265, 65309, 65320, 65528, 65325, 65317, 65324, 65316, 
	65315, 65321, 65319, 65318, 65287, 65529, 65533, 65532, 65289, 65285, 
	65286,  287,  286,  285,  284,  283,  282,  281,  276,  274, 
	 273,  272,  267,  266,  264,  262,  260,  259,  126,  125, 
	  45,   42,   40,   33, 65272,   59, 65320,  259, 65271,  125, 
	65269,   59, 65310, 65314, 65311, 65312, 65309, 65320, 65528, 65325, 
	65317, 65324, 65316, 65453, 65315, 65321, 65319, 65424, 65318, 65287, 
	65529, 65533, 65532, 65289, 65285, 65286,  287,  286,  285,  284, 
	 283,  282,  281,  276,  275,  274,  273,  272,  271,  267, 
	 266,  264,  262,  260,  259,  126,   45,   42,   40,   33, 
	65273,   59, 65454, 65433, 65434,  284,  273,  125, 65362, 65361, 
	65357, 65359, 65358, 65341, 65356, 65307, 65354, 65353, 65355, 65342, 
	65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 65345, 65348, 
	65344, 65343,  315,  310,  309,  308,  307,  306,  305,  304, 
	 292,  291,  124,   94,   64,   63,   62,   60,   59,   47, 
	  46,   45,   43,   42,   38,   37, 65392, 65455,   93,   44, 
	65362, 65361, 65357, 65359, 65358, 65341, 65356, 65306, 65354, 65353, 
	65355, 65342, 65363, 65360, 65347, 65346, 65350, 65349, 65352, 65351, 
	65345, 65348, 65344, 65343,  315,  310,  309,  308,  307,  306, 
	 305,  304,  292,  291,  124,   94,   64,   63,   62,   60, 
	  59,   47,   46,   45,   43,   42,   38,   37, 65310, 65314, 
	65311, 65312, 65266, 65309, 65320, 65528, 65325, 65317, 65324, 65316, 
	65315, 65321, 65319, 65318, 65287, 65529, 65533, 65532, 65289, 65285, 
	65286,  287,  286,  285,  284,  283,  282,  281,  276,  274, 
	 273,  272,  267,  266,  264,  262,  260,  259,  126,  125, 
	  45,   42,   40,   33, 65459,   58, 65270,   59, 65456,   61, 
	65310, 65314, 65311, 65312, 65309, 65320, 65528, 65325, 65317, 65324, 
	65316, 65315, 65321, 65319, 65318, 65287, 65529, 65533, 65532, 65289, 
	65285, 65286,  287,  286,  285,  284,  283,  282,  281,  276, 
	 274,  273,  272,  267,  266,  264,  262,  260,  259,  126, 
	  45,   42,   40,   33, 65362, 65361, 65357, 65359, 65358, 65341, 
	65356, 65308, 65354, 65353, 65355, 65342, 65363, 65360, 65347, 65346, 
	65350, 65349, 65352, 65351, 65345, 65348, 65344, 65343,  315,  310, 
	 309,  308,  307,  306,  305,  304,  292,  291,  124,   94, 
	  64,   63,   62,   60,   59,   47,   46,   45,   43,   42, 
	  38,   37,   -1
};
static short yypact[] = {

	  24,   48,   62,   78,   93,  180,  199,  235,  120,  271, 
	 271,  271,  271,  157,  157,  157,  157,  157,  157,  157, 
	 157,  157,  157,  157,  157,  387,  157,  157,  392,  402, 
	 402,  402,  430,  456,  456,  483,  519,  519,  545,  545, 
	 545,  545,  271,  271,  608,  608,  631,  667,  271,  702, 
	 120, 1013, 1079,  120,  120,  157, 1147, 1153,  157,  157, 
	1190, 1261, 1013,  157,  387, 1375, 1384,  157, 1439, 1504, 
	1507, 1439, 1512,  157, 1536, 1507, 1720, 1742, 1788,  120, 
	1719, 1717, 1715, 1691, 1644, 1618, 1592, 1565, 1561, 1511, 
	 120,  120,  120, 1380, 1509, 1505,  120, 1119, 1481, 1457, 
	1453, 1448, 1445, 1443, 1434, 1408, 1383, 1380, 1377, 1373, 
	1371,  120, 1119, 1347,  120, 1323, 1298, 1273, 1271, 1268, 
	1265,  120, 1263, 1263, 1236, 1166, 1163, 1161,  120, 1158, 
	 120, 1155, 1151, 1149, 1145, 1143, 1119, 1119,  120,  120, 
	 120, 1094,  120,  120, 1065, 1039, 1011, 1009,  984,  936, 
	 888,  840,  792,  744,  580,  120,  389,  385,  382,  379, 
	 354,  120,  328,  325,  323,  120,  120,  120,  120,  120, 
	 120,  298,  120,  120,  120,  120,  120,  120,  120,  120, 
	 120,  120,  120,  120,  120,  120,  120,  120,  120,  120, 
	 120,  120,  120,  120,  273,  255,  239,  120,  120,  120, 
	 120,  120,  120,  120,  120,  120,  120,  120,  120,  237, 
	 233,  217,  201,  197,  195,  192,  189,  187,  185,  183, 
	 181,  120,  157,  120,  120,  120,  120
};
static short yygo[] = {

	65278, 65426, 65425,  122,   74, 65243, 65249, 65535, 65252, 65255, 
	65254, 65251,  144,  129, 65406, 65437, 65461, 65452, 65458, 65422, 
	  76,   72,   69,   66, 65250, 65275, 65275, 65275, 65275, 65257, 
	 137,  136,  112,   97,    0, 65471, 65394, 65510,  155,  121, 
	65408, 65409, 65379,   54,   53, 65450, 65410, 65377,  140,   91, 
	65295, 65479, 65415, 65530, 65530, 65530, 65530, 65530, 65530, 65530, 
	65530, 65530, 65530, 65282,  137,  136,  113,  112,   98,   97, 
	  83,   77,   74,    0, 65431, 65371, 65247, 65246, 65245, 65281, 
	65279, 65474, 65280,  136,  112,   97, 65244, 65440, 65447, 65440, 
	65447, 65296,   75,   71,   70,   68, 65460, 65465,   71, 65441, 
	65509, 65480, 65509, 65509, 65457, 65451, 65511, 65449, 65462, 65468, 
	65430, 65509, 65472, 65419, 65411, 65475, 65511, 65476, 65477, 65509, 
	65390, 65387, 65386, 65385, 65384, 65383, 65382, 65486, 65487, 65488, 
	65489, 65490, 65491, 65492, 65493, 65381, 65494, 65495, 65496, 65497, 
	65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65508, 
	65511, 65512, 65513, 65514, 65515, 65516, 65517, 65518, 65519, 65520, 
	65521, 65522, 65375, 65364, 65523, 65524, 65525, 65526, 65313,  226, 
	 225,  224,  223,  221,  211,  208,  207,  206,  205,  204, 
	 203,  202,  201,  200,  199,  198,  197,  195,  193,  192, 
	 191,  190,  189,  188,  187,  186,  185,  184,  183,  182, 
	 181,  180,  179,  178,  177,  176,  175,  174,  173,  172, 
	 170,  169,  168,  167,  166,  165,  161,  155,  143,  142, 
	 140,  139,  138,  130,  128,  121,  114,  111,   96,   92, 
	  91,   90,   79,   54,   53,   50,    8, 65464, 65448, 65435, 
	  93,   68, 65305, 65305, 65305, 65304,  100,   87,   71, 65531, 
	65323, 65478, 65429, 65404,   62, 65299, 65298, 65297,   52,    4, 
	65534,   -1
};
static short yypgo[] = {

	   0,    0,    0,  260,  253,  253,   91,   51,   51,   52, 
	  52,   99,   99,   99,   99,    2,    2,   74,   74,   76, 
	 250,  168,  168,  168,  168,  168,  168,  168,  168,  168, 
	 168,  168,  168,  168,  168,  168,  168,  168,  168,  168, 
	 168,  168,  168,  168,  168,  168,  168,  168,  168,  168, 
	 168,  249,  257,  257,  257,  257,  257,  257,  257,  257, 
	 257,  257,   63,   63,   63,   42,   42,   37,   37,   47, 
	  47,  245,  245,  245,  239,  239,   50,   50,   50,   63, 
	  63,   63,   63,   63,   63,  257,  257,   75,   75,  249, 
	 168,  168,  168,  168,  168,  168,  168,  168,   78,   77, 
	  76,    2,    5,    5,   82,   82,   97,   97,   91,   86, 
	  86,  251,  251,   11,   11,   29,   29,   29,   29,   29, 
	  29,   29,   19,   19,   14,   14,    8,    6,    6,    6, 
	   7,    7,   29,   29,   29,   29,   29,    0
};
static short yyrlen[] = {

	   0,    0,    0,    1,    0,    1,    6,    0,    1,    0, 
	   2,    0,    1,    1,    2,    4,    7,    1,    3,    5, 
	   1,    3,    3,    3,    3,    3,    3,    3,    3,    2, 
	   2,    5,    3,    3,    3,    3,    3,    3,    3,    3, 
	   3,    3,    3,    3,    3,    2,    2,    1,    1,    1, 
	   4,    3,    2,    2,    2,    2,    2,    2,    2,    2, 
	   2,    2,    3,    6,    3,    0,    1,    1,    3,    1, 
	   3,    7,    4,    4,    2,    1,    6,    6,    4,    5, 
	   2,    5,    2,    1,    1,    1,    1,    3,    1,    6, 
	   1,    1,    1,    1,    1,    1,    3,    1,    9,    5, 
	   7,    5,    7,    7,    1,    3,    3,    2,    9,   11, 
	   9,    3,    1,   10,    8,    6,    4,    2,    3,    2, 
	   2,    2,    2,    0,    2,    1,    6,    1,    1,    1, 
	   2,    0,    1,    1,    1,    1,    1,    2
};
#define YYS0	287
#define YYDELTA	156
#define YYNPACT	227
#define YYNDEF	78

#define YYr135	0
#define YYr136	1
#define YYr137	2
#define YYr1	3
#define YYr26	4
#define YYr27	5
#define YYr33	6
#define YYr34	7
#define YYr35	8
#define YYr36	9
#define YYr37	10
#define YYr38	11
#define YYr39	12
#define YYr40	13
#define YYr41	14
#define YYr48	15
#define YYr49	16
#define YYr51	17
#define YYr52	18
#define YYr53	19
#define YYr57	20
#define YYr60	21
#define YYr61	22
#define YYr62	23
#define YYr63	24
#define YYr64	25
#define YYr65	26
#define YYr66	27
#define YYr67	28
#define YYr68	29
#define YYr69	30
#define YYr71	31
#define YYr72	32
#define YYr73	33
#define YYr74	34
#define YYr75	35
#define YYr76	36
#define YYr77	37
#define YYr78	38
#define YYr79	39
#define YYr80	40
#define YYr81	41
#define YYr82	42
#define YYr83	43
#define YYr84	44
#define YYr85	45
#define YYr86	46
#define YYr91	47
#define YYr92	48
#define YYr94	49
#define YYr95	50
#define YYr96	51
#define YYr102	52
#define YYr103	53
#define YYr104	54
#define YYr105	55
#define YYr106	56
#define YYr107	57
#define YYr108	58
#define YYr109	59
#define YYr110	60
#define YYr111	61
#define YYr114	62
#define YYr115	63
#define YYr119	64
#define YYr124	65
#define YYr125	66
#define YYr126	67
#define YYr127	68
#define YYr133	69
#define YYr134	70
#define YYr132	71
#define YYr131	72
#define YYr130	73
#define YYr129	74
#define YYr128	75
#define YYr123	76
#define YYr122	77
#define YYr121	78
#define YYr120	79
#define YYr118	80
#define YYr117	81
#define YYr116	82
#define YYr113	83
#define YYr112	84
#define YYr101	85
#define YYr100	86
#define YYr99	87
#define YYr98	88
#define YYr97	89
#define YYr93	90
#define YYr90	91
#define YYr89	92
#define YYr88	93
#define YYr87	94
#define YYr70	95
#define YYr59	96
#define YYr58	97
#define YYr56	98
#define YYr55	99
#define YYr54	100
#define YYr50	101
#define YYr47	102
#define YYr46	103
#define YYr45	104
#define YYr44	105
#define YYr43	106
#define YYr42	107
#define YYr32	108
#define YYr31	109
#define YYr30	110
#define YYr29	111
#define YYr28	112
#define YYr25	113
#define YYr24	114
#define YYr23	115
#define YYr22	116
#define YYr21	117
#define YYr20	118
#define YYr19	119
#define YYr18	120
#define YYr17	121
#define YYr11	122
#define YYr10	123
#define YYr9	124
#define YYr8	125
#define YYr7	126
#define YYr6	127
#define YYr5	128
#define YYr4	129
#define YYr3	130
#define YYr2	131
#define YYrACCEPT	YYr135
#define YYrERROR	YYr136
#define YYrLR2	YYr137
#if YYDEBUG
char * yysvar[] = {
	"$accept",
	"case_block",
	"switch_stmt",
	"decl",
	"decl_list",
	"package_decl",
	"fn_decl_stmt",
	"fn_decl_list",
	"statement_list",
	"stmt",
	"expr_list",
	"expr_list_decl",
	"aidx_expr",
	"funcall_expr",
	"object_name",
	"object_args",
	"stmt_expr",
	"case_expr",
	"class_name_expr",
	"if_stmt",
	"while_stmt",
	"for_stmt",
	"stmt_block",
	"datablock_decl",
	"object_decl",
	"object_decl_list",
	"object_declare_block",
	"expr",
	"slot_assign_list",
	"slot_assign",
	"slot_acc",
	"expression_stmt",
	"var_list",
	"var_list_decl",
	"assign_op_struct",
	"start",
	0
};
short yyrmap[] = {

	 135,  136,  137,    1,   26,   27,   33,   34,   35,   36, 
	  37,   38,   39,   40,   41,   48,   49,   51,   52,   53, 
	  57,   60,   61,   62,   63,   64,   65,   66,   67,   68, 
	  69,   71,   72,   73,   74,   75,   76,   77,   78,   79, 
	  80,   81,   82,   83,   84,   85,   86,   91,   92,   94, 
	  95,   96,  102,  103,  104,  105,  106,  107,  108,  109, 
	 110,  111,  114,  115,  119,  124,  125,  126,  127,  133, 
	 134,  132,  131,  130,  129,  128,  123,  122,  121,  120, 
	 118,  117,  116,  113,  112,  101,  100,   99,   98,   97, 
	  93,   90,   89,   88,   87,   70,   59,   58,   56,   55, 
	  54,   50,   47,   46,   45,   44,   43,   42,   32,   31, 
	  30,   29,   28,   25,   24,   23,   22,   21,   20,   19, 
	  18,   17,   11,   10,    9,    8,    7,    6,    5,    4, 
	   3,    2,   12,   13,   14,   15,   16,    0
};
short yysmap[] = {

	   1,    2,    5,    7,    8,   18,   27,   31,   58,   63, 
	  65,   66,   67,  109,  110,  111,  112,  113,  114,  115, 
	 116,  117,  118,  119,  121,  123,  125,  127,  128,  129, 
	 130,  131,  132,  133,  134,  135,  136,  137,  138,  139, 
	 140,  141,  143,  144,  145,  146,  147,  148,  149,  150, 
	 158,  161,  166,  167,  171,  180,  181,  185,  191,  195, 
	 199,  204,  210,  216,  226,  227,  232,  239,  243,  253, 
	 255,  256,  264,  267,  268,  271,  287,  290,  291,  289, 
	 284,  282,  280,  278,  276,  275,  274,  273,  270,  263, 
	 261,  260,  259,  258,  257,  254,  252,  251,  250,  249, 
	 248,  246,  245,  244,  240,  235,  230,  229,  228,  225, 
	 224,  223,  222,  221,  220,  215,  213,  212,  211,  209, 
	 208,  207,  206,  205,  200,  198,  197,  194,  193,  190, 
	 187,  186,  183,  182,  178,  177,  176,  175,  174,  173, 
	 172,  170,  168,  165,  164,  162,  160,  159,  157,  156, 
	 155,  154,  153,  152,  142,  126,  124,  122,  120,  108, 
	 104,  102,  101,  100,   99,   97,   96,   95,   94,   93, 
	  92,   91,   90,   89,   88,   87,   86,   85,   84,   83, 
	  82,   81,   80,   79,   78,   77,   76,   75,   74,   73, 
	  72,   71,   70,   69,   68,   59,   57,   56,   53,   52, 
	  51,   50,   49,   48,   47,   46,   45,   44,   43,   37, 
	  30,   29,   28,   26,   25,   24,   23,   22,   21,   20, 
	  19,   17,   16,   15,   14,   13,   12,  292,  283,  285, 
	 262,  247,  218,  217,  169,  196,   60,  192,   42,    3, 
	   4,   54,   55,  179,   98,  219,    6,   61,    9,   10, 
	  11,   62,  151,   64,  266,  201,  238,  279,  241,  242, 
	 202,  236,  281,  269,  272,  288,  277,  231,  184,  286, 
	 265,  233,  188,  103,  163,  105,  106,  107,  237,  203, 
	 214,  189,  234,   38,   39,   40,   41,    0,   36,   35, 
	  34,   33,   32
};
int yyntoken = 87;
int yynvar = 36;
int yynstate = 293;
int yynrule = 138;
#endif

#if YYDEBUG
/*
 * Package up YACC context for tracing
 */
typedef struct yyTraceItems_tag {
	int	state, lookahead, errflag, done;
	int	rule, npop;
	short	* states;
	int	nstates;
	YYSTYPE * values;
	int	nvalues;
	short	* types;
} yyTraceItems;
#endif

#line 2 "console/yyparse.c"

/*
 * Copyright 1985, 1990 by Mortice Kern Systems Inc.  All rights reserved.
 * 
 * Automaton to interpret LALR(1) tables.
 *
 * Macros:
 *	yyclearin - clear the lookahead token.
 *	yyerrok - forgive a pending error
 *	YYERROR - simulate an error
 *	YYACCEPT - halt and return 0
 *	YYABORT - halt and return 1
 *	YYRETURN(value) - halt and return value.  You should use this
 *		instead of return(value).
 *	YYREAD - ensure CMDchar contains a lookahead token by reading
 *		one if it does not.  See also YYSYNC.
 *	YYRECOVERING - 1 if syntax error detected and not recovered
 *		yet; otherwise, 0.
 *
 * Preprocessor flags:
 *	YYDEBUG - includes debug code if 1.  The parser will print
 *		 a travelogue of the parse if this is defined as 1
 *		 and CMDdebug is non-zero.
 *		yacc -t sets YYDEBUG to 1, but not CMDdebug.
 *	YYTRACE - turn on YYDEBUG, and undefine default trace functions
 *		so that the interactive functions in 'ytrack.c' will
 *		be used.
 *	YYSSIZE - size of state and value stacks (default 150).
 *	YYSTATIC - By default, the state stack is an automatic array.
 *		If this is defined, the stack will be static.
 *		In either case, the value stack is static.
 *	YYALLOC - Dynamically allocate both the state and value stacks
 *		by calling malloc() and free().
 *	YYDYNAMIC - Dynamically allocate (and reallocate, if necessary)
 *		both the state and value stacks by calling malloc(),
 *		realloc(), and free().
 *	YYSYNC - if defined, yacc guarantees to fetch a lookahead token
 *		before any action, even if it doesnt need it for a decision.
 *		If YYSYNC is defined, YYREAD will never be necessary unless
 *		the user explicitly sets CMDchar = -1
 *
 * Copyright (c) 1983, by the University of Waterloo
 */
/*
 * Prototypes
 */

extern int CMDlex YY_ARGS((void));
extern void CMDerror YY_ARGS((char *, ...));

#if YYDEBUG

#include <stdlib.h>		/* common prototypes */
#include <string.h>

extern char *	yyValue YY_ARGS((YYSTYPE, int));	/* print CMDlval */
extern void yyShowState YY_ARGS((yyTraceItems *));
extern void yyShowReduce YY_ARGS((yyTraceItems *));
extern void yyShowGoto YY_ARGS((yyTraceItems *));
extern void yyShowShift YY_ARGS((yyTraceItems *));
extern void yyShowErrRecovery YY_ARGS((yyTraceItems *));
extern void yyShowErrDiscard YY_ARGS((yyTraceItems *));

extern void yyShowRead YY_ARGS((int));
#endif

/*
 * If YYDEBUG defined and CMDdebug set,
 * tracing functions will be called at appropriate times in CMDparse()
 * Pass state of YACC parse, as filled into yyTraceItems yyx
 * If yyx.done is set by the tracing function, CMDparse() will terminate
 * with a return value of -1
 */
#define YY_TRACE(fn) { \
	yyx.state = yystate; yyx.lookahead = CMDchar; yyx.errflag =yyerrflag; \
	yyx.states = yys+1; yyx.nstates = yyps-yys; \
	yyx.values = yyv+1; yyx.nvalues = yypv-yyv; \
	yyx.types = yytypev+1; yyx.done = 0; \
	yyx.rule = yyi; yyx.npop = yyj; \
	fn(&yyx); \
	if (yyx.done) YYRETURN(-1); }

#ifndef I18N
#define m_textmsg(id, str, cls)	(str)
#else /*I18N*/
#include <m_nls.h>
#endif/*I18N*/

#ifndef YYSSIZE
# define YYSSIZE	150
#endif

#ifdef YYDYNAMIC
#define YYALLOC
char *getenv();
int atoi();
int yysinc = -1; /* stack size increment, <0 = double, 0 = none, >0 = fixed */
#endif

#ifdef YYALLOC
int yyssize = YYSSIZE;
#endif

#define YYERROR		goto yyerrlabel
#define yyerrok		yyerrflag = 0
#if YYDEBUG
#define yyclearin	{ if (CMDdebug) yyShowRead(-1); CMDchar = -1; }
#else
#define yyclearin	CMDchar = -1
#endif
#define YYACCEPT	YYRETURN(0)
#define YYABORT		YYRETURN(1)
#define YYRECOVERING()	(yyerrflag != 0)
#ifdef YYALLOC
#define YYRETURN(val)	{ retval = (val); goto yyReturn; }
#else
#define YYRETURN(val)	return(val);
#endif
#if YYDEBUG
/* The if..else makes this macro behave exactly like a statement */
# define YYREAD	if (CMDchar < 0) {					\
			if ((CMDchar = CMDlex()) < 0)	{		\
				if (CMDchar == -2) YYABORT; \
				CMDchar = 0;				\
			}	/* endif */			\
			if (CMDdebug)					\
				yyShowRead(CMDchar);			\
		} else
#else
# define YYREAD	if (CMDchar < 0) {					\
			if ((CMDchar = CMDlex()) < 0) {			\
				if (CMDchar == -2) YYABORT; \
				CMDchar = 0;				\
			}	/* endif */			\
		} else
#endif

#define YYERRCODE	1591738624		/* value of `error' */
#define YYTOKEN_BASE	256
#define	YYQYYP	yyq[yyq-yyp]

/*
 * Simulate bitwise negation as if was done on a two's complement machine.
 * This makes the generated code portable to machines with different
 * representations of integers (ie. signed magnitude).
 */
#define	yyneg(s)	(-((s)+1))

YYSTYPE	yyval;				/* $ */
YYSTYPE	*yypvt;				/* $n */
YYSTYPE	CMDlval;				/* CMDlex() sets this */

int	CMDchar,				/* current token */
	yyerrflag,			/* error flag */
	yynerrs;			/* error count */

#if YYDEBUG
int CMDdebug = 0;		/* debug if this flag is set */
extern char	*yysvar[];	/* table of non-terminals (aka 'variables') */
extern yyNamedType yyTokenTypes[];	/* table of terminals & their types */
extern short	yyrmap[], yysmap[];	/* map internal rule/states */
extern int	yynstate, yynvar, yyntoken, yynrule;

extern int	yyGetType YY_ARGS((int));	/* token type */
extern char	*yyptok YY_ARGS((int));	/* printable token string */
extern int	yyExpandName YY_ARGS((int, int, char *, int));
				  /* expand yyRules[] or yyStates[] */
static char *	yygetState YY_ARGS((int));

#define yyassert(condition, msg, arg) \
	if (!(condition)) { \
		printf(m_textmsg(2824, "\nyacc bug: ", "E")); \
		printf(msg, arg); \
		YYABORT; }
#else /* !YYDEBUG */
#define yyassert(condition, msg, arg)
#endif



#ifdef YACC_WINDOWS

/*
 * the following is the CMDparse() function that will be
 * callable by a windows type program. It in turn will
 * load all needed resources, obtain pointers to these
 * resources, and call a statically defined function
 * win_yyparse(), which is the original CMDparse() fn
 * When win_yyparse() is complete, it will return a
 * value to the new CMDparse(), where it will be stored
 * away temporarily, all resources will be freed, and
 * that return value will be given back to the caller
 * CMDparse(), as expected.
 */

static int win_yyparse();			/* prototype */

int CMDparse() 
{
	int wReturnValue;
	HANDLE hRes_table;		/* handle of resource after loading */
	short far *old_yydef;		/* the following are used for saving */
	short far *old_yyex;		/* the current pointers */
	short far *old_yyact;
	short far *old_yypact;
	short far *old_yygo;
	short far *old_yypgo;
	short far *old_yyrlen;

	/*
	 * the following code will load the required
	 * resources for a Windows based parser.
	 */

	hRes_table = LoadResource (hInst, 
		FindResource (hInst, "UD_RES_yyYACC", "yyYACCTBL"));
	
	/*
	 * return an error code if any
	 * of the resources did not load
	 */

	if (hRes_table == NULL)
		return (1);
	
	/*
	 * the following code will lock the resources
	 * into fixed memory locations for the parser
	 * (also, save the current pointer values first)
	 */

	old_yydef = yydef;
	old_yyex = yyex;
	old_yyact = yyact;
	old_yypact = yypact;
	old_yygo = yygo;
	old_yypgo = yypgo;
	old_yyrlen = yyrlen;

	yydef = (short far *)LockResource (hRes_table);
	yyex = (short far *)(yydef + Sizeof_yydef);
	yyact = (short far *)(yyex + Sizeof_yyex);
	yypact = (short far *)(yyact + Sizeof_yyact);
	yygo = (short far *)(yypact + Sizeof_yypact);
	yypgo = (short far *)(yygo + Sizeof_yygo);
	yyrlen = (short far *)(yypgo + Sizeof_yypgo);

	/*
	 * call the official CMDparse() function
	 */

	wReturnValue = win_yyparse();

	/*
	 * unlock the resources
	 */

	UnlockResource (hRes_table);

	/*
	 * and now free the resource
	 */

	FreeResource (hRes_table);

	/*
	 * restore previous pointer values
	 */

	yydef = old_yydef;
	yyex = old_yyex;
	yyact = old_yyact;
	yypact = old_yypact;
	yygo = old_yygo;
	yypgo = old_yypgo;
	yyrlen = old_yyrlen;

	return (wReturnValue);
}	/* end CMDparse */

static int win_yyparse() 

#else /* YACC_WINDOWS */

/*
 * we are not compiling a windows resource
 * based parser, so call CMDparse() the old
 * standard way.
 */

int CMDparse() 

#endif /* YACC_WINDOWS */

{
#ifdef YACC_WINDOWS
	register short far	*yyp;	/* for table lookup */
	register short far	*yyq;
#else
	register short		*yyp;	/* for table lookup */
	register short		*yyq;
#endif	/* YACC_WINDOWS */
	register short		yyi;
	register short		*yyps;		/* top of state stack */
	register short		yystate;	/* current state */
	register YYSTYPE	*yypv;		/* top of value stack */
	register int		yyj;
#if YYDEBUG
	yyTraceItems	yyx;			/* trace block */
	short	* yytp;
	int	yyruletype = 0;
#endif
#ifdef YYSTATIC
	static short	yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];
#if YYDEBUG
	static short	yytypev[YYSSIZE+1];	/* type assignments */
#endif
#else /* ! YYSTATIC */
#ifdef YYALLOC
	YYSTYPE *yyv;
	short	*yys;
#if YYDEBUG
	short	*yytypev;
#endif
	YYSTYPE save_yylval;
	YYSTYPE save_yyval;
	YYSTYPE *save_yypvt;
	int save_yychar, save_yyerrflag, save_yynerrs;
	int retval; 			/* return value holder */
#else
	short		yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];	/* historically static */
#if YYDEBUG
	short	yytypev[YYSSIZE+1];		/* mirror type table */
#endif
#endif /* ! YYALLOC */
#endif /* ! YYSTATIC */
#ifdef YYDYNAMIC
	char *envp;
#endif


#ifdef YYDYNAMIC
	if ((envp = getenv("YYSTACKSIZE")) != (char *)0) {
		yyssize = atoi(envp);
		if (yyssize <= 0)
			yyssize = YYSSIZE;
	}
	if ((envp = getenv("YYSTACKINC")) != (char *)0)
		yysinc = atoi(envp);
#endif
#ifdef YYALLOC
	yys = (short *) malloc((yyssize + 1) * sizeof(short));
	yyv = (YYSTYPE *) malloc((yyssize + 1) * sizeof(YYSTYPE));
#if YYDEBUG
	yytypev = (short *) malloc((yyssize + 1) * sizeof(short));
#endif
	if (yys == (short *)0 || yyv == (YYSTYPE *)0
#if YYDEBUG
		|| yytypev == (short *) 0
#endif
	) {
		CMDerror(m_textmsg(4967, "Not enough space for parser stacks",
				  "E"));
		return 1;
	}
	save_yylval = CMDlval;
	save_yyval = yyval;
	save_yypvt = yypvt;
	save_yychar = CMDchar;
	save_yyerrflag = yyerrflag;
	save_yynerrs = yynerrs;
#endif

	yynerrs = 0;
	yyerrflag = 0;
	yyclearin;
	yyps = yys;
	yypv = yyv;
	*yyps = yystate = YYS0;		/* start state */
#if YYDEBUG
	yytp = yytypev;
	yyi = yyj = 0;			/* silence compiler warnings */
#endif

yyStack:
	yyassert((unsigned)yystate < yynstate, m_textmsg(587, "state %d\n", ""), yystate);
#ifdef YYDYNAMIC
	if (++yyps > &yys[yyssize]) {
		int yynewsize;
		int yysindex = yyps - yys;
		int yyvindex = yypv - yyv;
#if YYDEBUG
		int yytindex = yytp - yytypev;
#endif
		if (yysinc == 0) {		/* no increment */
			CMDerror(m_textmsg(4968, "Parser stack overflow", "E"));
			YYABORT;
		} else if (yysinc < 0)		/* binary-exponential */
			yynewsize = yyssize * 2;
		else				/* fixed increment */
			yynewsize = yyssize + yysinc;
		if (yynewsize < yyssize) {
			CMDerror(m_textmsg(4967,
					  "Not enough space for parser stacks",
					  "E"));
			YYABORT;
		}
		yyssize = yynewsize;
		yys = (short *) realloc(yys, (yyssize + 1) * sizeof(short));
		yyps = yys + yysindex;
		yyv = (YYSTYPE *) realloc(yyv, (yyssize + 1) * sizeof(YYSTYPE));
		yypv = yyv + yyvindex;
#if YYDEBUG
		yytypev = (short *)realloc(yytypev,(yyssize + 1)*sizeof(short));
		yytp = yytypev + yytindex;
#endif
		if (yys == (short *)0 || yyv == (YYSTYPE *)0
#if YYDEBUG
			|| yytypev == (short *) 0
#endif
		) {
			CMDerror(m_textmsg(4967, 
					  "Not enough space for parser stacks",
					  "E"));
			YYABORT;
		}
	}
#else
	if (++yyps > &yys[YYSSIZE]) {
		CMDerror(m_textmsg(4968, "Parser stack overflow", "E"));
		YYABORT;
	}
#endif /* !YYDYNAMIC */
	*yyps = yystate;	/* stack current state */
	*++yypv = yyval;	/* ... and value */
#if YYDEBUG
	*++yytp = yyruletype;	/* ... and type */

	if (CMDdebug)
		YY_TRACE(yyShowState)
#endif

	/*
	 *	Look up next action in action table.
	 */
yyEncore:
#ifdef YYSYNC
	YYREAD;
#endif

#ifdef YACC_WINDOWS
	if (yystate >= Sizeof_yypact) 	/* simple state */
#else /* YACC_WINDOWS */
	if (yystate >= sizeof yypact/sizeof yypact[0]) 	/* simple state */
#endif /* YACC_WINDOWS */
		yyi = yystate - YYDELTA;	/* reduce in any case */
	else {
		if(*(yyp = &yyact[yypact[yystate]]) >= 0) {
			/* Look for a shift on CMDchar */
#ifndef YYSYNC
			YYREAD;
#endif
			yyq = yyp;
			yyi = CMDchar;
			while (yyi < *yyp++)
				;
			if (yyi == yyp[-1]) {
				yystate = yyneg(YYQYYP);
#if YYDEBUG
				if (CMDdebug) {
					yyruletype = yyGetType(CMDchar);
					YY_TRACE(yyShowShift)
				}
#endif
				yyval = CMDlval;	/* stack what CMDlex() set */
				yyclearin;		/* clear token */
				if (yyerrflag)
					yyerrflag--;	/* successful shift */
				goto yyStack;
			}
		}

		/*
	 	 *	Fell through - take default action
	 	 */

#ifdef YACC_WINDOWS
		if (yystate >= Sizeof_yydef)
#else /* YACC_WINDOWS */
		if (yystate >= sizeof yydef /sizeof yydef[0])
#endif /* YACC_WINDOWS */
			goto yyError;
		if ((yyi = yydef[yystate]) < 0)	 { /* default == reduce? */
			/* Search exception table */
#ifdef YACC_WINDOWS
			yyassert((unsigned)yyneg(yyi) < Sizeof_yyex,
				m_textmsg(2825, "exception %d\n", "I num"), yystate);
#else /* YACC_WINDOWS */
			yyassert((unsigned)yyneg(yyi) < sizeof yyex/sizeof yyex[0],
				m_textmsg(2825, "exception %d\n", "I num"), yystate);
#endif /* YACC_WINDOWS */
			yyp = &yyex[yyneg(yyi)];
#ifndef YYSYNC
			YYREAD;
#endif
			while((yyi = *yyp) >= 0 && yyi != CMDchar)
				yyp += 2;
			yyi = yyp[1];
			yyassert(yyi >= 0,
				 m_textmsg(2826, "Ex table not reduce %d\n", "I num"), yyi);
		}
	}

	yyassert((unsigned)yyi < yynrule, m_textmsg(2827, "reduce %d\n", "I num"), yyi);
	yyj = yyrlen[yyi];
#if YYDEBUG
	if (CMDdebug)
		YY_TRACE(yyShowReduce)
	yytp -= yyj;
#endif
	yyps -= yyj;		/* pop stacks */
	yypvt = yypv;		/* save top */
	yypv -= yyj;
	yyval = yypv[1];	/* default action $ = $1 */
#if YYDEBUG
	yyruletype = yyRules[yyrmap[yyi]].type;
#endif

	switch (yyi) {		/* perform semantic action */
		
case YYr1: {	/* start :  decl_list */
#line 127 "console/gram.y"
 
} break;

case YYr2: {	/* decl_list :  */
#line 132 "console/gram.y"
 yyval.stmt = nil; 
} break;

case YYr3: {	/* decl_list :  decl_list decl */
#line 134 "console/gram.y"
 if(!statementList) { statementList = yypvt[0].stmt; } else { statementList->append(yypvt[0].stmt); } 
} break;

case YYr4: {	/* decl :  stmt */
#line 139 "console/gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr5: {	/* decl :  fn_decl_stmt */
#line 141 "console/gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr6: {	/* decl :  package_decl */
#line 143 "console/gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr7: {	/* package_decl :  rwPACKAGE IDENT '{' fn_decl_list '}' ';' */
#line 148 "console/gram.y"
 yyval.stmt = yypvt[-2].stmt; for(StmtNode *walk = (yypvt[-2].stmt);walk;walk = walk->getNext() ) walk->setPackage(yypvt[-4].s); 
} break;

case YYr8: {	/* fn_decl_list :  fn_decl_stmt */
#line 153 "console/gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr9: {	/* fn_decl_list :  fn_decl_list fn_decl_stmt */
#line 155 "console/gram.y"
 yyval.stmt = yypvt[-1].stmt; (yypvt[-1].stmt)->append(yypvt[0].stmt);  
} break;

case YYr10: {	/* statement_list :  */
#line 160 "console/gram.y"
 yyval.stmt = nil; 
} break;

case YYr11: {	/* statement_list :  statement_list stmt */
#line 162 "console/gram.y"
 if(!yypvt[-1].stmt) { yyval.stmt = yypvt[0].stmt; } else { (yypvt[-1].stmt)->append(yypvt[0].stmt); yyval.stmt = yypvt[-1].stmt; } 
} break;

case YYr17: {	/* stmt :  rwBREAK ';' */
#line 172 "console/gram.y"
 yyval.stmt = BreakStmtNode::alloc(); 
} break;

case YYr18: {	/* stmt :  rwCONTINUE ';' */
#line 174 "console/gram.y"
 yyval.stmt = ContinueStmtNode::alloc(); 
} break;

case YYr19: {	/* stmt :  rwRETURN ';' */
#line 176 "console/gram.y"
 yyval.stmt = ReturnStmtNode::alloc(NULL); 
} break;

case YYr20: {	/* stmt :  rwRETURN expr ';' */
#line 178 "console/gram.y"
 yyval.stmt = ReturnStmtNode::alloc(yypvt[-1].expr); 
} break;

case YYr21: {	/* stmt :  expression_stmt ';' */
#line 180 "console/gram.y"
 yyval.stmt = yypvt[-1].stmt; 
} break;

case YYr22: {	/* stmt :  TTAG '=' expr ';' */
#line 182 "console/gram.y"
 yyval.stmt = TTagSetStmtNode::alloc(yypvt[-3].s, yypvt[-1].expr, NULL); 
} break;

case YYr23: {	/* stmt :  TTAG '=' expr ',' expr ';' */
#line 184 "console/gram.y"
 yyval.stmt = TTagSetStmtNode::alloc(yypvt[-5].s, yypvt[-3].expr, yypvt[-1].expr); 
} break;

case YYr24: {	/* fn_decl_stmt :  rwDEFINE IDENT '(' var_list_decl ')' '{' statement_list '}' */
#line 189 "console/gram.y"
 yyval.stmt = FunctionDeclStmtNode::alloc(yypvt[-6].s, NULL, yypvt[-4].var, yypvt[-1].stmt); 
} break;

case YYr25: {	/* fn_decl_stmt :  rwDEFINE IDENT opCOLONCOLON IDENT '(' var_list_decl ')' '{' statement_list '}' */
#line 191 "console/gram.y"
 yyval.stmt = FunctionDeclStmtNode::alloc(yypvt[-6].s, yypvt[-8].s, yypvt[-4].var, yypvt[-1].stmt); 
} break;

case YYr26: {	/* var_list_decl :  */
#line 196 "console/gram.y"
 yyval.var = NULL; 
} break;

case YYr27: {	/* var_list_decl :  var_list */
#line 198 "console/gram.y"
 yyval.var = yypvt[0].var; 
} break;

case YYr28: {	/* var_list :  VAR */
#line 203 "console/gram.y"
 yyval.var = VarNode::alloc(yypvt[0].s, NULL); 
} break;

case YYr29: {	/* var_list :  var_list ',' VAR */
#line 205 "console/gram.y"
 yyval.var = yypvt[-2].var; ((StmtNode*)(yypvt[-2].var))->append((StmtNode*)VarNode::alloc(yypvt[0].s, NULL)); 
} break;

case YYr30: {	/* datablock_decl :  rwDATABLOCK IDENT '(' IDENT ')' '{' slot_assign_list '}' ';' */
#line 210 "console/gram.y"
 yyval.stmt = ObjectDeclNode::alloc(ConstantNode::alloc(yypvt[-7].s), ConstantNode::alloc(yypvt[-5].s), NULL, NULL, yypvt[-2].slist, NULL, true); 
} break;

case YYr31: {	/* datablock_decl :  rwDATABLOCK IDENT '(' IDENT ')' ':' IDENT '{' slot_assign_list '}' ';' */
#line 212 "console/gram.y"
 yyval.stmt = ObjectDeclNode::alloc(ConstantNode::alloc(yypvt[-9].s), ConstantNode::alloc(yypvt[-7].s), NULL, yypvt[-4].s, yypvt[-2].slist, NULL, true); 
} break;

case YYr32: {	/* object_decl :  rwDECLARE class_name_expr '(' object_name object_args ')' '{' object_declare_block '}' */
#line 217 "console/gram.y"
 yyval.od = ObjectDeclNode::alloc(yypvt[-7].expr, yypvt[-5].expr, yypvt[-4].expr, NULL, yypvt[-1].odcl.slots, yypvt[-1].odcl.decls, false); 
} break;

case YYr33: {	/* object_decl :  rwDECLARE class_name_expr '(' object_name object_args ')' */
#line 219 "console/gram.y"
 yyval.od = ObjectDeclNode::alloc(yypvt[-4].expr, yypvt[-2].expr, yypvt[-1].expr, NULL, NULL, NULL, false); 
} break;

case YYr34: {	/* object_name :  */
#line 224 "console/gram.y"
 yyval.expr = StrConstNode::alloc("", false); 
} break;

case YYr35: {	/* object_name :  expr */
#line 226 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr36: {	/* object_args :  */
#line 231 "console/gram.y"
 yyval.expr = NULL; 
} break;

case YYr37: {	/* object_args :  ',' expr_list */
#line 233 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr38: {	/* object_declare_block :  */
#line 238 "console/gram.y"
 yyval.odcl.slots = NULL; yyval.odcl.decls = NULL; 
} break;

case YYr39: {	/* object_declare_block :  slot_assign_list */
#line 240 "console/gram.y"
 yyval.odcl.slots = yypvt[0].slist; yyval.odcl.decls = NULL; 
} break;

case YYr40: {	/* object_declare_block :  object_decl_list */
#line 242 "console/gram.y"
 yyval.odcl.slots = NULL; yyval.odcl.decls = yypvt[0].od; 
} break;

case YYr41: {	/* object_declare_block :  slot_assign_list object_decl_list */
#line 244 "console/gram.y"
 yyval.odcl.slots = yypvt[-1].slist; yyval.odcl.decls = yypvt[0].od; 
} break;

case YYr42: {	/* object_decl_list :  object_decl ';' */
#line 249 "console/gram.y"
 yyval.od = yypvt[-1].od; 
} break;

case YYr43: {	/* object_decl_list :  object_decl_list object_decl ';' */
#line 251 "console/gram.y"
 yypvt[-2].od->append(yypvt[-1].od); yyval.od = yypvt[-2].od; 
} break;

case YYr44: {	/* stmt_block :  '{' statement_list '}' */
#line 256 "console/gram.y"
 yyval.stmt = yypvt[-1].stmt; 
} break;

case YYr45: {	/* stmt_block :  stmt */
#line 258 "console/gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr46: {	/* switch_stmt :  rwSWITCH '(' expr ')' '{' case_block '}' */
#line 263 "console/gram.y"
 yyval.stmt = yypvt[-1].ifnode; yypvt[-1].ifnode->propagateSwitchExpr(yypvt[-4].expr, false); 
} break;

case YYr47: {	/* switch_stmt :  rwSWITCHSTR '(' expr ')' '{' case_block '}' */
#line 265 "console/gram.y"
 yyval.stmt = yypvt[-1].ifnode; yypvt[-1].ifnode->propagateSwitchExpr(yypvt[-4].expr, true); 
} break;

case YYr48: {	/* case_block :  rwCASE case_expr ':' statement_list */
#line 270 "console/gram.y"
 yyval.ifnode = IfStmtNode::alloc(yypvt[-3].i, yypvt[-2].expr, yypvt[0].stmt, NULL, false); 
} break;

case YYr49: {	/* case_block :  rwCASE case_expr ':' statement_list rwDEFAULT ':' statement_list */
#line 272 "console/gram.y"
 yyval.ifnode = IfStmtNode::alloc(yypvt[-6].i, yypvt[-5].expr, yypvt[-3].stmt, yypvt[0].stmt, false); 
} break;

case YYr50: {	/* case_block :  rwCASE case_expr ':' statement_list case_block */
#line 274 "console/gram.y"
 yyval.ifnode = IfStmtNode::alloc(yypvt[-4].i, yypvt[-3].expr, yypvt[-1].stmt, yypvt[0].ifnode, true); 
} break;

case YYr51: {	/* case_expr :  expr */
#line 279 "console/gram.y"
 yyval.expr = yypvt[0].expr;
} break;

case YYr52: {	/* case_expr :  case_expr rwCASEOR expr */
#line 281 "console/gram.y"
 (yypvt[-2].expr)->append(yypvt[0].expr); yyval.expr=yypvt[-2].expr; 
} break;

case YYr53: {	/* if_stmt :  rwIF '(' expr ')' stmt_block */
#line 286 "console/gram.y"
 yyval.stmt = IfStmtNode::alloc(yypvt[-4].i, yypvt[-2].expr, yypvt[0].stmt, NULL, false); 
} break;

case YYr54: {	/* if_stmt :  rwIF '(' expr ')' stmt_block rwELSE stmt_block */
#line 288 "console/gram.y"
 yyval.stmt = IfStmtNode::alloc(yypvt[-6].i, yypvt[-4].expr, yypvt[-2].stmt, yypvt[0].stmt, false); 
} break;

case YYr55: {	/* while_stmt :  rwWHILE '(' expr ')' stmt_block */
#line 293 "console/gram.y"
 yyval.stmt = LoopStmtNode::alloc(yypvt[-4].i, nil, yypvt[-2].expr, nil, yypvt[0].stmt, false); 
} break;

case YYr56: {	/* for_stmt :  rwFOR '(' expr ';' expr ';' expr ')' stmt_block */
#line 298 "console/gram.y"
 yyval.stmt = LoopStmtNode::alloc(yypvt[-8].i, yypvt[-6].expr, yypvt[-4].expr, yypvt[-2].expr, yypvt[0].stmt, false); 
} break;

case YYr57: {	/* expression_stmt :  stmt_expr */
#line 303 "console/gram.y"
 yyval.stmt = yypvt[0].expr; 
} break;

case YYr58: {	/* expr :  stmt_expr */
#line 308 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr59: {	/* expr :  '(' expr ')' */
#line 310 "console/gram.y"
 yyval.expr = yypvt[-1].expr; 
} break;

case YYr60: {	/* expr :  expr '^' expr */
#line 312 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr61: {	/* expr :  expr '%' expr */
#line 314 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr62: {	/* expr :  expr '&' expr */
#line 316 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr63: {	/* expr :  expr '|' expr */
#line 318 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr64: {	/* expr :  expr '+' expr */
#line 320 "console/gram.y"
 yyval.expr = FloatBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr65: {	/* expr :  expr '-' expr */
#line 322 "console/gram.y"
 yyval.expr = FloatBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr66: {	/* expr :  expr '*' expr */
#line 324 "console/gram.y"
 yyval.expr = FloatBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr67: {	/* expr :  expr '/' expr */
#line 326 "console/gram.y"
 yyval.expr = FloatBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr68: {	/* expr :  '-' expr */
#line 328 "console/gram.y"
 yyval.expr = FloatUnaryExprNode::alloc(yypvt[-1].i, yypvt[0].expr); 
} break;

case YYr69: {	/* expr :  '*' expr */
#line 330 "console/gram.y"
 yyval.expr = TTagDerefNode::alloc(yypvt[0].expr); 
} break;

case YYr70: {	/* expr :  TTAG */
#line 332 "console/gram.y"
 yyval.expr = TTagExprNode::alloc(yypvt[0].s); 
} break;

case YYr71: {	/* expr :  expr '?' expr ':' expr */
#line 334 "console/gram.y"
 yyval.expr = ConditionalExprNode::alloc(yypvt[-4].expr, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr72: {	/* expr :  expr '<' expr */
#line 336 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr73: {	/* expr :  expr '>' expr */
#line 338 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr74: {	/* expr :  expr opGE expr */
#line 340 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr75: {	/* expr :  expr opLE expr */
#line 342 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr76: {	/* expr :  expr opEQ expr */
#line 344 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr77: {	/* expr :  expr opNE expr */
#line 346 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr78: {	/* expr :  expr opOR expr */
#line 348 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr79: {	/* expr :  expr opSHL expr */
#line 350 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr80: {	/* expr :  expr opSHR expr */
#line 352 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr81: {	/* expr :  expr opAND expr */
#line 354 "console/gram.y"
 yyval.expr = IntBinaryExprNode::alloc(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr82: {	/* expr :  expr opSTREQ expr */
#line 356 "console/gram.y"
 yyval.expr = StreqExprNode::alloc(yypvt[-2].expr, yypvt[0].expr, true); 
} break;

case YYr83: {	/* expr :  expr opSTRNE expr */
#line 358 "console/gram.y"
 yyval.expr = StreqExprNode::alloc(yypvt[-2].expr, yypvt[0].expr, false); 
} break;

case YYr84: {	/* expr :  expr '@' expr */
#line 360 "console/gram.y"
 yyval.expr = StrcatExprNode::alloc(yypvt[-2].expr, yypvt[0].expr, yypvt[-1].i); 
} break;

case YYr85: {	/* expr :  '!' expr */
#line 362 "console/gram.y"
 yyval.expr = IntUnaryExprNode::alloc(yypvt[-1].i, yypvt[0].expr); 
} break;

case YYr86: {	/* expr :  '~' expr */
#line 364 "console/gram.y"
 yyval.expr = IntUnaryExprNode::alloc(yypvt[-1].i, yypvt[0].expr); 
} break;

case YYr87: {	/* expr :  TAGATOM */
#line 366 "console/gram.y"
 yyval.expr = StrConstNode::alloc(yypvt[0].str, true); 
} break;

case YYr88: {	/* expr :  FLTCONST */
#line 368 "console/gram.y"
 yyval.expr = FloatNode::alloc(yypvt[0].f); 
} break;

case YYr89: {	/* expr :  INTCONST */
#line 370 "console/gram.y"
 yyval.expr = IntNode::alloc(yypvt[0].i); 
} break;

case YYr90: {	/* expr :  rwBREAK */
#line 372 "console/gram.y"
 yyval.expr = ConstantNode::alloc(StringTable->insert("break")); 
} break;

case YYr91: {	/* expr :  slot_acc */
#line 374 "console/gram.y"
 yyval.expr = SlotAccessNode::alloc(yypvt[0].slot.object, yypvt[0].slot.array, yypvt[0].slot.slotName); 
} break;

case YYr92: {	/* expr :  IDENT */
#line 376 "console/gram.y"
 yyval.expr = ConstantNode::alloc(yypvt[0].s); 
} break;

case YYr93: {	/* expr :  STRATOM */
#line 378 "console/gram.y"
 yyval.expr = StrConstNode::alloc(yypvt[0].str, false); 
} break;

case YYr94: {	/* expr :  VAR */
#line 380 "console/gram.y"
 yyval.expr = (ExprNode*)VarNode::alloc(yypvt[0].s, NULL); 
} break;

case YYr95: {	/* expr :  VAR '[' aidx_expr ']' */
#line 382 "console/gram.y"
 yyval.expr = (ExprNode*)VarNode::alloc(yypvt[-3].s, yypvt[-1].expr); 
} break;

case YYr96: {	/* slot_acc :  expr '.' IDENT */
#line 387 "console/gram.y"
 yyval.slot.object = yypvt[-2].expr; yyval.slot.slotName = yypvt[0].s; yyval.slot.array = NULL; 
} break;

case YYr97: {	/* slot_acc :  expr '.' IDENT '[' aidx_expr ']' */
#line 389 "console/gram.y"
 yyval.slot.object = yypvt[-5].expr; yyval.slot.slotName = yypvt[-3].s; yyval.slot.array = yypvt[-1].expr; 
} break;

case YYr98: {	/* class_name_expr :  IDENT */
#line 394 "console/gram.y"
 yyval.expr = ConstantNode::alloc(yypvt[0].s); 
} break;

case YYr99: {	/* class_name_expr :  '(' expr ')' */
#line 396 "console/gram.y"
 yyval.expr = yypvt[-1].expr; 
} break;

case YYr100: {	/* assign_op_struct :  opPLUSPLUS */
#line 401 "console/gram.y"
 yyval.asn.token = '+'; yyval.asn.expr = FloatNode::alloc(1); 
} break;

case YYr101: {	/* assign_op_struct :  opMINUSMINUS */
#line 403 "console/gram.y"
 yyval.asn.token = '-'; yyval.asn.expr = FloatNode::alloc(1); 
} break;

case YYr102: {	/* assign_op_struct :  opPLASN expr */
#line 405 "console/gram.y"
 yyval.asn.token = '+'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr103: {	/* assign_op_struct :  opMIASN expr */
#line 407 "console/gram.y"
 yyval.asn.token = '-'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr104: {	/* assign_op_struct :  opMLASN expr */
#line 409 "console/gram.y"
 yyval.asn.token = '*'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr105: {	/* assign_op_struct :  opDVASN expr */
#line 411 "console/gram.y"
 yyval.asn.token = '/'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr106: {	/* assign_op_struct :  opMODASN expr */
#line 413 "console/gram.y"
 yyval.asn.token = '%'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr107: {	/* assign_op_struct :  opANDASN expr */
#line 415 "console/gram.y"
 yyval.asn.token = '&'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr108: {	/* assign_op_struct :  opXORASN expr */
#line 417 "console/gram.y"
 yyval.asn.token = '^'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr109: {	/* assign_op_struct :  opORASN expr */
#line 419 "console/gram.y"
 yyval.asn.token = '|'; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr110: {	/* assign_op_struct :  opSLASN expr */
#line 421 "console/gram.y"
 yyval.asn.token = opSHL; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr111: {	/* assign_op_struct :  opSRASN expr */
#line 423 "console/gram.y"
 yyval.asn.token = opSHR; yyval.asn.expr = yypvt[0].expr; 
} break;

case YYr112: {	/* stmt_expr :  funcall_expr */
#line 428 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr113: {	/* stmt_expr :  object_decl */
#line 430 "console/gram.y"
 yyval.expr = yypvt[0].od; 
} break;

case YYr114: {	/* stmt_expr :  VAR '=' expr */
#line 432 "console/gram.y"
 yyval.expr = AssignExprNode::alloc(yypvt[-2].s, NULL, yypvt[0].expr); 
} break;

case YYr115: {	/* stmt_expr :  VAR '[' aidx_expr ']' '=' expr */
#line 434 "console/gram.y"
 yyval.expr = AssignExprNode::alloc(yypvt[-5].s, yypvt[-3].expr, yypvt[0].expr); 
} break;

case YYr116: {	/* stmt_expr :  VAR assign_op_struct */
#line 436 "console/gram.y"
 yyval.expr = AssignOpExprNode::alloc(yypvt[-1].s, NULL, yypvt[0].asn.expr, yypvt[0].asn.token); 
} break;

case YYr117: {	/* stmt_expr :  VAR '[' aidx_expr ']' assign_op_struct */
#line 438 "console/gram.y"
 yyval.expr = AssignOpExprNode::alloc(yypvt[-4].s, yypvt[-2].expr, yypvt[0].asn.expr, yypvt[0].asn.token); 
} break;

case YYr118: {	/* stmt_expr :  slot_acc assign_op_struct */
#line 440 "console/gram.y"
 yyval.expr = SlotAssignOpNode::alloc(yypvt[-1].slot.object, yypvt[-1].slot.slotName, yypvt[-1].slot.array, yypvt[0].asn.token, yypvt[0].asn.expr); 
} break;

case YYr119: {	/* stmt_expr :  slot_acc '=' expr */
#line 442 "console/gram.y"
 yyval.expr = SlotAssignNode::alloc(yypvt[-2].slot.object, yypvt[-2].slot.array, yypvt[-2].slot.slotName, yypvt[0].expr); 
} break;

case YYr120: {	/* stmt_expr :  slot_acc '=' '{' expr_list '}' */
#line 444 "console/gram.y"
 yyval.expr = SlotAssignNode::alloc(yypvt[-4].slot.object, yypvt[-4].slot.array, yypvt[-4].slot.slotName, yypvt[-1].expr); 
} break;

case YYr121: {	/* funcall_expr :  IDENT '(' expr_list_decl ')' */
#line 449 "console/gram.y"
 yyval.expr = FuncCallExprNode::alloc(yypvt[-3].s, NULL, yypvt[-1].expr, false); 
} break;

case YYr122: {	/* funcall_expr :  IDENT opCOLONCOLON IDENT '(' expr_list_decl ')' */
#line 451 "console/gram.y"
 yyval.expr = FuncCallExprNode::alloc(yypvt[-3].s, yypvt[-5].s, yypvt[-1].expr, false); 
} break;

case YYr123: {	/* funcall_expr :  expr '.' IDENT '(' expr_list_decl ')' */
#line 453 "console/gram.y"
 yypvt[-5].expr->append(yypvt[-1].expr); yyval.expr = FuncCallExprNode::alloc(yypvt[-3].s, NULL, yypvt[-5].expr, true); 
} break;

case YYr124: {	/* expr_list_decl :  */
#line 458 "console/gram.y"
 yyval.expr = NULL; 
} break;

case YYr125: {	/* expr_list_decl :  expr_list */
#line 460 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr126: {	/* expr_list :  expr */
#line 465 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr127: {	/* expr_list :  expr_list ',' expr */
#line 467 "console/gram.y"
 (yypvt[-2].expr)->append(yypvt[0].expr); yyval.expr = yypvt[-2].expr; 
} break;

case YYr128: {	/* slot_assign_list :  slot_assign */
#line 472 "console/gram.y"
 yyval.slist = yypvt[0].slist; 
} break;

case YYr129: {	/* slot_assign_list :  slot_assign_list slot_assign */
#line 474 "console/gram.y"
 yypvt[-1].slist->append(yypvt[0].slist); yyval.slist = yypvt[-1].slist; 
} break;

case YYr130: {	/* slot_assign :  IDENT '=' expr ';' */
#line 479 "console/gram.y"
 yyval.slist = SlotAssignNode::alloc(NULL, NULL, yypvt[-3].s, yypvt[-1].expr); 
} break;

case YYr131: {	/* slot_assign :  rwDATABLOCK '=' expr ';' */
#line 481 "console/gram.y"
 yyval.slist = SlotAssignNode::alloc(NULL, NULL, StringTable->insert("datablock"), yypvt[-1].expr); 
} break;

case YYr132: {	/* slot_assign :  IDENT '[' aidx_expr ']' '=' expr ';' */
#line 483 "console/gram.y"
 yyval.slist = SlotAssignNode::alloc(NULL, yypvt[-4].expr, yypvt[-6].s, yypvt[-1].expr); 
} break;

case YYr133: {	/* aidx_expr :  expr */
#line 488 "console/gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr134: {	/* aidx_expr :  aidx_expr ',' expr */
#line 490 "console/gram.y"
 yyval.expr = CommaCatExprNode::alloc(yypvt[-2].expr, yypvt[0].expr); 
} break;
#line 314 "console/yyparse.c"
	case YYrACCEPT:
		YYACCEPT;
	case YYrERROR:
		goto yyError;
	}

	/*
	 *	Look up next state in goto table.
	 */

	yyp = &yygo[yypgo[yyi]];
	yyq = yyp++;
	yyi = *yyps;
	while (yyi < *yyp++)
		;

	yystate = yyneg(yyi == *--yyp? YYQYYP: *yyq);
#if YYDEBUG
	if (CMDdebug)
		YY_TRACE(yyShowGoto)
#endif
	goto yyStack;

yyerrlabel:	;		/* come here from YYERROR	*/
/*
#pragma used yyerrlabel
 */
	yyerrflag = 1;
	if (yyi == YYrERROR) {
		yyps--;
		yypv--;
#if YYDEBUG
		yytp--;
#endif
	}

yyError:
	switch (yyerrflag) {

	case 0:		/* new error */
		yynerrs++;
		yyi = CMDchar;
		CMDerror(m_textmsg(4969, "Syntax error", "E"));
		if (yyi != CMDchar) {
			/* user has changed the current token */
			/* try again */
			yyerrflag++;	/* avoid loops */
			goto yyEncore;
		}

	case 1:		/* partially recovered */
	case 2:
		yyerrflag = 3;	/* need 3 valid shifts to recover */
			
		/*
		 *	Pop states, looking for a
		 *	shift on `error'.
		 */

		for ( ; yyps > yys; yyps--, yypv--
#if YYDEBUG
					, yytp--
#endif
		) {
#ifdef YACC_WINDOWS
			if (*yyps >= Sizeof_yypact)
#else /* YACC_WINDOWS */
			if (*yyps >= sizeof yypact/sizeof yypact[0])
#endif /* YACC_WINDOWS */
				continue;
			yyp = &yyact[yypact[*yyps]];
			yyq = yyp;
			do {
				if (YYERRCODE == *yyp) {
					yyp++;
					yystate = yyneg(YYQYYP);
					goto yyStack;
				}
			} while (*yyp++ > YYTOKEN_BASE);
		
			/* no shift in this state */
#if YYDEBUG
			if (CMDdebug && yyps > yys+1)
				YY_TRACE(yyShowErrRecovery)
#endif
			/* pop stacks; try again */
		}
		/* no shift on error - abort */
		break;

	case 3:
		/*
		 *	Erroneous token after
		 *	an error - discard it.
		 */

		if (CMDchar == 0)  /* but not EOF */
			break;
#if YYDEBUG
		if (CMDdebug)
			YY_TRACE(yyShowErrDiscard)
#endif
		yyclearin;
		goto yyEncore;	/* try again in same state */
	}
	YYABORT;

#ifdef YYALLOC
yyReturn:
	CMDlval = save_yylval;
	yyval = save_yyval;
	yypvt = save_yypvt;
	CMDchar = save_yychar;
	yyerrflag = save_yyerrflag;
	yynerrs = save_yynerrs;
	free((char *)yys);
	free((char *)yyv);
#if YYDEBUG
	free((char *)yytypev);
#endif
	return(retval);
#endif
}

		
#if YYDEBUG
/*
 * Return type of token
 */
int
yyGetType(tok)
int tok;
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->type;
	return 0;
}
/*
 * Print a token legibly.
 */
char *
yyptok(tok)
int tok;
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->name;
	return "";
}

/*
 * Read state 'num' from YYStatesFile
 */
#ifdef YYTRACE

static char *
yygetState(num)
int num;
{
	int	size;
	static FILE *yyStatesFile = (FILE *) 0;
	static char yyReadBuf[YYMAX_READ+1];

	if (yyStatesFile == (FILE *) 0
	 && (yyStatesFile = fopen(YYStatesFile, "r")) == (FILE *) 0)
		return "yyExpandName: cannot open states file";

	if (num < yynstate - 1)
		size = (int)(yyStates[num+1] - yyStates[num]);
	else {
		/* length of last item is length of file - ptr(last-1) */
		if (fseek(yyStatesFile, 0L, 2) < 0)
			goto cannot_seek;
		size = (int) (ftell(yyStatesFile) - yyStates[num]);
	}
	if (size < 0 || size > YYMAX_READ)
		return "yyExpandName: bad read size";
	if (fseek(yyStatesFile, yyStates[num], 0) < 0) {
	cannot_seek:
		return "yyExpandName: cannot seek in states file";
	}

	(void) fread(yyReadBuf, 1, size, yyStatesFile);
	yyReadBuf[size] = '\0';
	return yyReadBuf;
}
#endif /* YYTRACE */
/*
 * Expand encoded string into printable representation
 * Used to decode yyStates and yyRules strings.
 * If the expansion of 's' fits in 'buf', return 1; otherwise, 0.
 */
int
yyExpandName(num, isrule, buf, len)
int num, isrule;
char * buf;
int len;
{
	int	i, n, cnt, type;
	char	* endp, * cp;
	char	*s;

	if (isrule)
		s = yyRules[num].name;
	else
#ifdef YYTRACE
		s = yygetState(num);
#else
		s = "*no states*";
#endif

	for (endp = buf + len - 8; *s; s++) {
		if (buf >= endp) {		/* too large: return 0 */
		full:	(void) strcpy(buf, " ...\n");
			return 0;
		} else if (*s == '%') {		/* nonterminal */
			type = 0;
			cnt = yynvar;
			goto getN;
		} else if (*s == '&') {		/* terminal */
			type = 1;
			cnt = yyntoken;
		getN:
			if (cnt < 100)
				i = 2;
			else if (cnt < 1000)
				i = 3;
			else
				i = 4;
			for (n = 0; i-- > 0; )
				n = (n * 10) + *++s - '0';
			if (type == 0) {
				if (n >= yynvar)
					goto too_big;
				cp = yysvar[n];
			} else if (n >= yyntoken) {
			    too_big:
				cp = "<range err>";
			} else
				cp = yyTokenTypes[n].name;

			if ((i = strlen(cp)) + buf > endp)
				goto full;
			(void) strcpy(buf, cp);
			buf += i;
		} else
			*buf++ = *s;
	}
	*buf = '\0';
	return 1;
}
#ifndef YYTRACE
/*
 * Show current state of CMDparse
 */
void
yyShowState(tp)
yyTraceItems * tp;
{
	short * p;
	YYSTYPE * q;

	printf(
	    m_textmsg(2828, "state %d (%d), char %s (%d)\n", "I num1 num2 char num3"),
	      yysmap[tp->state], tp->state,
	      yyptok(tp->lookahead), tp->lookahead);
}
/*
 * show results of reduction
 */
void
yyShowReduce(tp)
yyTraceItems * tp;
{
	printf("reduce %d (%d), pops %d (%d)\n",
		yyrmap[tp->rule], tp->rule,
		tp->states[tp->nstates - tp->npop],
		yysmap[tp->states[tp->nstates - tp->npop]]);
}
void
yyShowRead(val)
int val;
{
	printf(m_textmsg(2829, "read %s (%d)\n", "I token num"), yyptok(val), val);
}
void
yyShowGoto(tp)
yyTraceItems * tp;
{
	printf(m_textmsg(2830, "goto %d (%d)\n", "I num1 num2"), yysmap[tp->state], tp->state);
}
void
yyShowShift(tp)
yyTraceItems * tp;
{
	printf(m_textmsg(2831, "shift %d (%d)\n", "I num1 num2"), yysmap[tp->state], tp->state);
}
void
yyShowErrRecovery(tp)
yyTraceItems * tp;
{
	short	* top = tp->states + tp->nstates - 1;

	printf(
	m_textmsg(2832, "Error recovery pops state %d (%d), uncovers %d (%d)\n", "I num1 num2 num3 num4"),
		yysmap[*top], *top, yysmap[*(top-1)], *(top-1));
}
void
yyShowErrDiscard(tp)
yyTraceItems * tp;
{
	printf(m_textmsg(2833, "Error recovery discards %s (%d), ", "I token num"),
		yyptok(tp->lookahead), tp->lookahead);
}
#endif	/* ! YYTRACE */
#endif	/* YYDEBUG */
