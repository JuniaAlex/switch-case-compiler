/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/parser.y"

#include "sqlmini.h"
#include "driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DiagnosticList *g_diagnostics = NULL;
Query *g_query = NULL;
int g_show_tokens = 0;

int yylex(void);
void yyerror(const char *message);

static ExprNode *expr_node_new(Expression *expression)
{
    ExprNode *node = (ExprNode *)malloc(sizeof(ExprNode));
    if (node == NULL) {
        fprintf(stderr, "fatal: out of memory\n");
        exit(3);
    }
    node->expression = expression;
    node->next = NULL;
    return node;
}

static void expr_node_append(ExprNode *head, Expression *expression)
{
    ExprNode *node = head;
    while (node->next != NULL) {
        node = node->next;
    }
    node->next = expr_node_new(expression);
}

static void expr_node_free(ExprNode *head)
{
    while (head != NULL) {
        ExprNode *next = head->next;
        free(head);
        head = next;
    }
}

void yyerror(const char *message)
{
    if (g_diagnostics != NULL) {
        diag_add(g_diagnostics, "P001", g_token_line, g_token_column, "%s", message);
    }
}

#line 124 "src/parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENT = 3,                      /* IDENT  */
  YYSYMBOL_STRING = 4,                     /* STRING  */
  YYSYMBOL_INT_LITERAL = 5,                /* INT_LITERAL  */
  YYSYMBOL_FLOAT_LITERAL = 6,              /* FLOAT_LITERAL  */
  YYSYMBOL_KW_SELECT = 7,                  /* KW_SELECT  */
  YYSYMBOL_KW_DISTINCT = 8,                /* KW_DISTINCT  */
  YYSYMBOL_KW_FROM = 9,                    /* KW_FROM  */
  YYSYMBOL_KW_WHERE = 10,                  /* KW_WHERE  */
  YYSYMBOL_KW_ORDER = 11,                  /* KW_ORDER  */
  YYSYMBOL_KW_BY = 12,                     /* KW_BY  */
  YYSYMBOL_KW_ASC = 13,                    /* KW_ASC  */
  YYSYMBOL_KW_DESC = 14,                   /* KW_DESC  */
  YYSYMBOL_KW_AND = 15,                    /* KW_AND  */
  YYSYMBOL_KW_OR = 16,                     /* KW_OR  */
  YYSYMBOL_KW_NOT = 17,                    /* KW_NOT  */
  YYSYMBOL_KW_AS = 18,                     /* KW_AS  */
  YYSYMBOL_KW_IN = 19,                     /* KW_IN  */
  YYSYMBOL_KW_BETWEEN = 20,                /* KW_BETWEEN  */
  YYSYMBOL_KW_LIKE = 21,                   /* KW_LIKE  */
  YYSYMBOL_KW_IS = 22,                     /* KW_IS  */
  YYSYMBOL_KW_NULL = 23,                   /* KW_NULL  */
  YYSYMBOL_KW_TRUE = 24,                   /* KW_TRUE  */
  YYSYMBOL_KW_FALSE = 25,                  /* KW_FALSE  */
  YYSYMBOL_STAR = 26,                      /* STAR  */
  YYSYMBOL_PLUS = 27,                      /* PLUS  */
  YYSYMBOL_MINUS = 28,                     /* MINUS  */
  YYSYMBOL_SLASH = 29,                     /* SLASH  */
  YYSYMBOL_EQ = 30,                        /* EQ  */
  YYSYMBOL_NE = 31,                        /* NE  */
  YYSYMBOL_LT = 32,                        /* LT  */
  YYSYMBOL_LE = 33,                        /* LE  */
  YYSYMBOL_GT = 34,                        /* GT  */
  YYSYMBOL_GE = 35,                        /* GE  */
  YYSYMBOL_COMMA = 36,                     /* COMMA  */
  YYSYMBOL_DOT = 37,                       /* DOT  */
  YYSYMBOL_LPAREN = 38,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 39,                    /* RPAREN  */
  YYSYMBOL_SEMI = 40,                      /* SEMI  */
  YYSYMBOL_YYACCEPT = 41,                  /* $accept  */
  YYSYMBOL_query = 42,                     /* query  */
  YYSYMBOL_43_1 = 43,                      /* $@1  */
  YYSYMBOL_opt_distinct = 44,              /* opt_distinct  */
  YYSYMBOL_select_list = 45,               /* select_list  */
  YYSYMBOL_select_item = 46,               /* select_item  */
  YYSYMBOL_table_list = 47,                /* table_list  */
  YYSYMBOL_table_ref = 48,                 /* table_ref  */
  YYSYMBOL_opt_where = 49,                 /* opt_where  */
  YYSYMBOL_opt_order = 50,                 /* opt_order  */
  YYSYMBOL_order_list = 51,                /* order_list  */
  YYSYMBOL_order_item = 52,                /* order_item  */
  YYSYMBOL_opt_semi = 53,                  /* opt_semi  */
  YYSYMBOL_expr = 54,                      /* expr  */
  YYSYMBOL_or_expr = 55,                   /* or_expr  */
  YYSYMBOL_and_expr = 56,                  /* and_expr  */
  YYSYMBOL_not_expr = 57,                  /* not_expr  */
  YYSYMBOL_predicate = 58,                 /* predicate  */
  YYSYMBOL_additive = 59,                  /* additive  */
  YYSYMBOL_multiplicative = 60,            /* multiplicative  */
  YYSYMBOL_unary = 61,                     /* unary  */
  YYSYMBOL_primary = 62,                   /* primary  */
  YYSYMBOL_literal = 63,                   /* literal  */
  YYSYMBOL_literal_list = 64               /* literal_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   131

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  24
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  119

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   295


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   101,   101,   100,   108,   110,   115,   122,   130,   139,
     148,   158,   171,   178,   186,   195,   205,   218,   220,   225,
     231,   236,   243,   251,   261,   271,   284,   285,   289,   294,
     296,   301,   303,   308,   310,   315,   317,   319,   321,   323,
     325,   327,   329,   331,   333,   343,   353,   355,   357,   359,
     364,   366,   368,   373,   375,   377,   382,   384,   386,   391,
     393,   395,   397,   402,   404,   406,   408,   410,   412,   417,
     419
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENT", "STRING",
  "INT_LITERAL", "FLOAT_LITERAL", "KW_SELECT", "KW_DISTINCT", "KW_FROM",
  "KW_WHERE", "KW_ORDER", "KW_BY", "KW_ASC", "KW_DESC", "KW_AND", "KW_OR",
  "KW_NOT", "KW_AS", "KW_IN", "KW_BETWEEN", "KW_LIKE", "KW_IS", "KW_NULL",
  "KW_TRUE", "KW_FALSE", "STAR", "PLUS", "MINUS", "SLASH", "EQ", "NE",
  "LT", "LE", "GT", "GE", "COMMA", "DOT", "LPAREN", "RPAREN", "SEMI",
  "$accept", "query", "$@1", "opt_distinct", "select_list", "select_item",
  "table_list", "table_ref", "opt_where", "opt_order", "order_list",
  "order_item", "opt_semi", "expr", "or_expr", "and_expr", "not_expr",
  "predicate", "additive", "multiplicative", "unary", "primary", "literal",
  "literal_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-43)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      11,    19,    23,   -43,     9,   -43,    -8,   -43,   -43,   -43,
      49,   -43,   -43,   -43,   -43,    57,    57,    49,     8,   -43,
      38,    53,    50,   -43,   -43,    72,    41,   -43,   -43,   -43,
      76,   -43,   -43,   -43,    44,    87,     9,   -43,    94,    49,
      49,    95,    63,    57,    57,     5,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,   -43,   -43,    68,    -5,
     -43,   -43,   -43,    50,   -43,    82,    57,    57,    15,    30,
      21,    96,   -43,    41,    41,    21,    21,    21,    21,    21,
      21,   -43,   -43,   -43,   118,    49,    87,   111,    15,    81,
      21,   -43,    39,    57,   -43,   -43,   -43,   -43,   112,   -43,
      74,    57,    15,   -43,    21,    49,    83,   -43,    21,   -43,
      89,   -43,    98,   -43,   -43,    49,   -43,   -43,   -43
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     4,     0,     5,     0,     1,    60,    65,    63,    64,
       0,    66,    67,    68,     8,     0,     0,     0,     0,     6,
       9,    28,    29,    31,    33,    35,    50,    53,    56,    59,
       0,    34,    57,    58,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    14,    17,
      12,     7,    11,    30,    32,     0,     0,     0,     0,     0,
      42,     0,    48,    51,    52,    36,    37,    38,    39,    40,
      41,    54,    55,    15,     0,     0,     0,    19,     0,     0,
      43,    69,     0,     0,    49,    16,    18,    13,     0,     2,
       0,     0,     0,    44,    46,     0,    26,    45,    47,    70,
      20,    21,    23,    27,     3,     0,    24,    25,    22
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -43,   -43,   -43,   -43,   -43,    90,   -43,    42,   -43,   -43,
     -43,    12,   -43,   -17,   -43,    91,     6,   -43,   -42,    71,
     -12,   -43,   -38,    43
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,   106,     4,    18,    19,    59,    60,    87,    99,
     110,   111,   114,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    92
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      34,    69,    70,    32,    33,    85,    75,    76,    77,    78,
      79,    80,     6,     7,     8,     9,    31,    35,     1,     7,
       8,     9,    71,     5,    89,    90,    10,     3,    72,    30,
      91,    86,    11,    12,    13,    14,    15,    16,    11,    12,
      13,    37,    81,    82,    36,    93,    64,    17,    46,    47,
      91,   104,     6,     7,     8,     9,    38,    46,    47,   108,
       6,     7,     8,     9,   109,    40,    10,    54,    96,    39,
      55,    83,    11,    12,    13,   102,    15,    16,   103,    56,
      11,    12,    13,    57,    15,    16,    84,    17,   112,    41,
      58,    42,    43,    44,    45,    17,   101,    62,   112,    46,
      47,    68,    48,    49,    50,    51,    52,    53,    46,    47,
     102,   116,   117,   107,    65,    66,    67,    73,    74,    94,
      88,    95,    98,   113,   105,   115,    61,   118,    97,     0,
      63,   100
};

static const yytype_int8 yycheck[] =
{
      17,    43,    44,    15,    16,    10,    48,    49,    50,    51,
      52,    53,     3,     4,     5,     6,    10,     9,     7,     4,
       5,     6,    17,     0,    66,    67,    17,     8,    23,    37,
      68,    36,    23,    24,    25,    26,    27,    28,    23,    24,
      25,     3,    54,    55,    36,    15,    40,    38,    27,    28,
      88,    93,     3,     4,     5,     6,    18,    27,    28,   101,
       3,     4,     5,     6,   102,    15,    17,    26,    85,    16,
      29,     3,    23,    24,    25,    36,    27,    28,    39,     3,
      23,    24,    25,    39,    27,    28,    18,    38,   105,    17,
       3,    19,    20,    21,    22,    38,    15,     3,   115,    27,
      28,    38,    30,    31,    32,    33,    34,    35,    27,    28,
      36,    13,    14,    39,    19,    20,    21,    46,    47,    23,
      38,     3,    11,    40,    12,    36,    36,   115,    86,    -1,
      39,    88
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     7,    42,     8,    44,     0,     3,     4,     5,     6,
      17,    23,    24,    25,    26,    27,    28,    38,    45,    46,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      37,    57,    61,    61,    54,     9,    36,     3,    18,    16,
      15,    17,    19,    20,    21,    22,    27,    28,    30,    31,
      32,    33,    34,    35,    26,    29,     3,    39,     3,    47,
      48,    46,     3,    56,    57,    19,    20,    21,    38,    59,
      59,    17,    23,    60,    60,    59,    59,    59,    59,    59,
      59,    61,    61,     3,    18,    10,    36,    49,    38,    59,
      59,    63,    64,    15,    23,     3,    54,    48,    11,    50,
      64,    15,    36,    39,    59,    12,    43,    39,    59,    63,
      51,    52,    54,    40,    53,    36,    13,    14,    52
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    41,    43,    42,    44,    44,    45,    45,    46,    46,
      46,    46,    47,    47,    48,    48,    48,    49,    49,    50,
      50,    51,    51,    52,    52,    52,    53,    53,    54,    55,
      55,    56,    56,    57,    57,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      59,    59,    59,    60,    60,    60,    61,    61,    61,    62,
      62,    62,    62,    63,    63,    63,    63,    63,    63,    64,
      64
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     9,     0,     1,     1,     3,     1,     1,
       2,     3,     1,     3,     1,     2,     3,     0,     2,     0,
       3,     1,     3,     1,     2,     2,     0,     1,     1,     1,
       3,     1,     3,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     3,     4,     5,     6,     5,     6,     3,     4,
       1,     3,     3,     1,     3,     3,     1,     2,     2,     1,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* $@1: %empty  */
#line 101 "src/parser.y"
    {
        g_query = query_create((yyvsp[-5].flag), (yyvsp[-4].select_list), (yyvsp[-2].table_list), (yyvsp[-1].expr), (yyvsp[0].order_list), (yylsp[-6]).first_line, (yylsp[-6]).first_column);
    }
#line 1647 "src/parser.tab.c"
    break;

  case 4: /* opt_distinct: %empty  */
#line 109 "src/parser.y"
    { (yyval.flag) = 0; }
#line 1653 "src/parser.tab.c"
    break;

  case 5: /* opt_distinct: KW_DISTINCT  */
#line 111 "src/parser.y"
    { (yyval.flag) = 1; }
#line 1659 "src/parser.tab.c"
    break;

  case 6: /* select_list: select_item  */
#line 116 "src/parser.y"
    {
        SelectList list;
        memset(&list, 0, sizeof(list));
        select_list_add(&list, (yyvsp[0].select_item));
        (yyval.select_list) = list;
    }
#line 1670 "src/parser.tab.c"
    break;

  case 7: /* select_list: select_list COMMA select_item  */
#line 123 "src/parser.y"
    {
        select_list_add(&(yyvsp[-2].select_list), (yyvsp[0].select_item));
        (yyval.select_list) = (yyvsp[-2].select_list);
    }
#line 1679 "src/parser.tab.c"
    break;

  case 8: /* select_item: STAR  */
#line 131 "src/parser.y"
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.star = 1;
        item.line = (yylsp[0]).first_line;
        item.column = (yylsp[0]).first_column;
        (yyval.select_item) = item;
    }
#line 1692 "src/parser.tab.c"
    break;

  case 9: /* select_item: expr  */
#line 140 "src/parser.y"
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.expression = (yyvsp[0].expr);
        item.line = (yylsp[0]).first_line;
        item.column = (yylsp[0]).first_column;
        (yyval.select_item) = item;
    }
#line 1705 "src/parser.tab.c"
    break;

  case 10: /* select_item: expr IDENT  */
#line 149 "src/parser.y"
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.expression = (yyvsp[-1].expr);
        item.alias = (yyvsp[0].text);
        item.line = (yylsp[-1]).first_line;
        item.column = (yylsp[-1]).first_column;
        (yyval.select_item) = item;
    }
#line 1719 "src/parser.tab.c"
    break;

  case 11: /* select_item: expr KW_AS IDENT  */
#line 159 "src/parser.y"
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.expression = (yyvsp[-2].expr);
        item.alias = (yyvsp[0].text);
        item.line = (yylsp[-2]).first_line;
        item.column = (yylsp[-2]).first_column;
        (yyval.select_item) = item;
    }
#line 1733 "src/parser.tab.c"
    break;

  case 12: /* table_list: table_ref  */
#line 172 "src/parser.y"
    {
        TableList list;
        memset(&list, 0, sizeof(list));
        table_list_add(&list, (yyvsp[0].table_ref));
        (yyval.table_list) = list;
    }
#line 1744 "src/parser.tab.c"
    break;

  case 13: /* table_list: table_list COMMA table_ref  */
#line 179 "src/parser.y"
    {
        table_list_add(&(yyvsp[-2].table_list), (yyvsp[0].table_ref));
        (yyval.table_list) = (yyvsp[-2].table_list);
    }
#line 1753 "src/parser.tab.c"
    break;

  case 14: /* table_ref: IDENT  */
#line 187 "src/parser.y"
    {
        TableReference table;
        memset(&table, 0, sizeof(table));
        table.name = (yyvsp[0].text);
        table.line = (yylsp[0]).first_line;
        table.column = (yylsp[0]).first_column;
        (yyval.table_ref) = table;
    }
#line 1766 "src/parser.tab.c"
    break;

  case 15: /* table_ref: IDENT IDENT  */
#line 196 "src/parser.y"
    {
        TableReference table;
        memset(&table, 0, sizeof(table));
        table.name = (yyvsp[-1].text);
        table.alias = (yyvsp[0].text);
        table.line = (yylsp[-1]).first_line;
        table.column = (yylsp[-1]).first_column;
        (yyval.table_ref) = table;
    }
#line 1780 "src/parser.tab.c"
    break;

  case 16: /* table_ref: IDENT KW_AS IDENT  */
#line 206 "src/parser.y"
    {
        TableReference table;
        memset(&table, 0, sizeof(table));
        table.name = (yyvsp[-2].text);
        table.alias = (yyvsp[0].text);
        table.line = (yylsp[-2]).first_line;
        table.column = (yylsp[-2]).first_column;
        (yyval.table_ref) = table;
    }
#line 1794 "src/parser.tab.c"
    break;

  case 17: /* opt_where: %empty  */
#line 219 "src/parser.y"
    { (yyval.expr) = NULL; }
#line 1800 "src/parser.tab.c"
    break;

  case 18: /* opt_where: KW_WHERE expr  */
#line 221 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1806 "src/parser.tab.c"
    break;

  case 19: /* opt_order: %empty  */
#line 226 "src/parser.y"
    {
        OrderList list;
        memset(&list, 0, sizeof(list));
        (yyval.order_list) = list;
    }
#line 1816 "src/parser.tab.c"
    break;

  case 20: /* opt_order: KW_ORDER KW_BY order_list  */
#line 232 "src/parser.y"
    { (yyval.order_list) = (yyvsp[0].order_list); }
#line 1822 "src/parser.tab.c"
    break;

  case 21: /* order_list: order_item  */
#line 237 "src/parser.y"
    {
        OrderList list;
        memset(&list, 0, sizeof(list));
        order_list_add(&list, (yyvsp[0].order_item));
        (yyval.order_list) = list;
    }
#line 1833 "src/parser.tab.c"
    break;

  case 22: /* order_list: order_list COMMA order_item  */
#line 244 "src/parser.y"
    {
        order_list_add(&(yyvsp[-2].order_list), (yyvsp[0].order_item));
        (yyval.order_list) = (yyvsp[-2].order_list);
    }
#line 1842 "src/parser.tab.c"
    break;

  case 23: /* order_item: expr  */
#line 252 "src/parser.y"
    {
        OrderItem item;
        memset(&item, 0, sizeof(item));
        item.expression = (yyvsp[0].expr);
        item.ascending = 1;
        item.line = (yylsp[0]).first_line;
        item.column = (yylsp[0]).first_column;
        (yyval.order_item) = item;
    }
#line 1856 "src/parser.tab.c"
    break;

  case 24: /* order_item: expr KW_ASC  */
#line 262 "src/parser.y"
    {
        OrderItem item;
        memset(&item, 0, sizeof(item));
        item.expression = (yyvsp[-1].expr);
        item.ascending = 1;
        item.line = (yylsp[-1]).first_line;
        item.column = (yylsp[-1]).first_column;
        (yyval.order_item) = item;
    }
#line 1870 "src/parser.tab.c"
    break;

  case 25: /* order_item: expr KW_DESC  */
#line 272 "src/parser.y"
    {
        OrderItem item;
        memset(&item, 0, sizeof(item));
        item.expression = (yyvsp[-1].expr);
        item.ascending = 0;
        item.line = (yylsp[-1]).first_line;
        item.column = (yylsp[-1]).first_column;
        (yyval.order_item) = item;
    }
#line 1884 "src/parser.tab.c"
    break;

  case 28: /* expr: or_expr  */
#line 290 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1890 "src/parser.tab.c"
    break;

  case 29: /* or_expr: and_expr  */
#line 295 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1896 "src/parser.tab.c"
    break;

  case 30: /* or_expr: or_expr KW_OR and_expr  */
#line 297 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_OR, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1902 "src/parser.tab.c"
    break;

  case 31: /* and_expr: not_expr  */
#line 302 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1908 "src/parser.tab.c"
    break;

  case 32: /* and_expr: and_expr KW_AND not_expr  */
#line 304 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_AND, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1914 "src/parser.tab.c"
    break;

  case 33: /* not_expr: predicate  */
#line 309 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1920 "src/parser.tab.c"
    break;

  case 34: /* not_expr: KW_NOT not_expr  */
#line 311 "src/parser.y"
    { (yyval.expr) = expression_unary(UNARY_NOT, (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1926 "src/parser.tab.c"
    break;

  case 35: /* predicate: additive  */
#line 316 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1932 "src/parser.tab.c"
    break;

  case 36: /* predicate: additive EQ additive  */
#line 318 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_EQUAL, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1938 "src/parser.tab.c"
    break;

  case 37: /* predicate: additive NE additive  */
#line 320 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_NOT_EQUAL, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1944 "src/parser.tab.c"
    break;

  case 38: /* predicate: additive LT additive  */
#line 322 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_LESS, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1950 "src/parser.tab.c"
    break;

  case 39: /* predicate: additive LE additive  */
#line 324 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_LESS_EQUAL, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1956 "src/parser.tab.c"
    break;

  case 40: /* predicate: additive GT additive  */
#line 326 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_GREATER, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1962 "src/parser.tab.c"
    break;

  case 41: /* predicate: additive GE additive  */
#line 328 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_GREATER_EQUAL, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1968 "src/parser.tab.c"
    break;

  case 42: /* predicate: additive KW_LIKE additive  */
#line 330 "src/parser.y"
    { (yyval.expr) = expression_like((yyvsp[-2].expr), (yyvsp[0].expr), 0, (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1974 "src/parser.tab.c"
    break;

  case 43: /* predicate: additive KW_NOT KW_LIKE additive  */
#line 332 "src/parser.y"
    { (yyval.expr) = expression_like((yyvsp[-3].expr), (yyvsp[0].expr), 1, (yylsp[-2]).first_line, (yylsp[-2]).first_column); }
#line 1980 "src/parser.tab.c"
    break;

  case 44: /* predicate: additive KW_IN LPAREN literal_list RPAREN  */
#line 334 "src/parser.y"
    {
        Expression *in_expression = expression_in((yyvsp[-4].expr), 0, (yylsp[-3]).first_line, (yylsp[-3]).first_column);
        ExprNode *node;
        for (node = (yyvsp[-1].expr_node); node != NULL; node = node->next) {
            expression_list_add(in_expression, node->expression);
        }
        expr_node_free((yyvsp[-1].expr_node));
        (yyval.expr) = in_expression;
    }
#line 1994 "src/parser.tab.c"
    break;

  case 45: /* predicate: additive KW_NOT KW_IN LPAREN literal_list RPAREN  */
#line 344 "src/parser.y"
    {
        Expression *in_expression = expression_in((yyvsp[-5].expr), 1, (yylsp[-4]).first_line, (yylsp[-4]).first_column);
        ExprNode *node;
        for (node = (yyvsp[-1].expr_node); node != NULL; node = node->next) {
            expression_list_add(in_expression, node->expression);
        }
        expr_node_free((yyvsp[-1].expr_node));
        (yyval.expr) = in_expression;
    }
#line 2008 "src/parser.tab.c"
    break;

  case 46: /* predicate: additive KW_BETWEEN additive KW_AND additive  */
#line 354 "src/parser.y"
    { (yyval.expr) = expression_between((yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr), 0, (yylsp[-3]).first_line, (yylsp[-3]).first_column); }
#line 2014 "src/parser.tab.c"
    break;

  case 47: /* predicate: additive KW_NOT KW_BETWEEN additive KW_AND additive  */
#line 356 "src/parser.y"
    { (yyval.expr) = expression_between((yyvsp[-5].expr), (yyvsp[-2].expr), (yyvsp[0].expr), 1, (yylsp[-4]).first_line, (yylsp[-4]).first_column); }
#line 2020 "src/parser.tab.c"
    break;

  case 48: /* predicate: additive KW_IS KW_NULL  */
#line 358 "src/parser.y"
    { (yyval.expr) = expression_is_null((yyvsp[-2].expr), 0, (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2026 "src/parser.tab.c"
    break;

  case 49: /* predicate: additive KW_IS KW_NOT KW_NULL  */
#line 360 "src/parser.y"
    { (yyval.expr) = expression_is_null((yyvsp[-3].expr), 1, (yylsp[-2]).first_line, (yylsp[-2]).first_column); }
#line 2032 "src/parser.tab.c"
    break;

  case 50: /* additive: multiplicative  */
#line 365 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2038 "src/parser.tab.c"
    break;

  case 51: /* additive: additive PLUS multiplicative  */
#line 367 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_ADD, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2044 "src/parser.tab.c"
    break;

  case 52: /* additive: additive MINUS multiplicative  */
#line 369 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_SUBTRACT, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2050 "src/parser.tab.c"
    break;

  case 53: /* multiplicative: unary  */
#line 374 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2056 "src/parser.tab.c"
    break;

  case 54: /* multiplicative: multiplicative STAR unary  */
#line 376 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_MULTIPLY, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2062 "src/parser.tab.c"
    break;

  case 55: /* multiplicative: multiplicative SLASH unary  */
#line 378 "src/parser.y"
    { (yyval.expr) = expression_binary(BINARY_DIVIDE, (yyvsp[-2].expr), (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2068 "src/parser.tab.c"
    break;

  case 56: /* unary: primary  */
#line 383 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2074 "src/parser.tab.c"
    break;

  case 57: /* unary: PLUS unary  */
#line 385 "src/parser.y"
    { (yyval.expr) = expression_unary(UNARY_PLUS, (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2080 "src/parser.tab.c"
    break;

  case 58: /* unary: MINUS unary  */
#line 387 "src/parser.y"
    { (yyval.expr) = expression_unary(UNARY_MINUS, (yyvsp[0].expr), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 2086 "src/parser.tab.c"
    break;

  case 59: /* primary: literal  */
#line 392 "src/parser.y"
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2092 "src/parser.tab.c"
    break;

  case 60: /* primary: IDENT  */
#line 394 "src/parser.y"
    { (yyval.expr) = expression_column(NULL, (yyvsp[0].text), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2098 "src/parser.tab.c"
    break;

  case 61: /* primary: IDENT DOT IDENT  */
#line 396 "src/parser.y"
    { (yyval.expr) = expression_column((yyvsp[-2].text), (yyvsp[0].text), (yylsp[-2]).first_line, (yylsp[-2]).first_column); }
#line 2104 "src/parser.tab.c"
    break;

  case 62: /* primary: LPAREN expr RPAREN  */
#line 398 "src/parser.y"
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2110 "src/parser.tab.c"
    break;

  case 63: /* literal: INT_LITERAL  */
#line 403 "src/parser.y"
    { (yyval.expr) = expression_literal_integer((yyvsp[0].integer), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2116 "src/parser.tab.c"
    break;

  case 64: /* literal: FLOAT_LITERAL  */
#line 405 "src/parser.y"
    { (yyval.expr) = expression_literal_float((yyvsp[0].number), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2122 "src/parser.tab.c"
    break;

  case 65: /* literal: STRING  */
#line 407 "src/parser.y"
    { (yyval.expr) = expression_literal_string((yyvsp[0].text), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2128 "src/parser.tab.c"
    break;

  case 66: /* literal: KW_NULL  */
#line 409 "src/parser.y"
    { (yyval.expr) = expression_literal_null((yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2134 "src/parser.tab.c"
    break;

  case 67: /* literal: KW_TRUE  */
#line 411 "src/parser.y"
    { (yyval.expr) = expression_literal_boolean(1, (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2140 "src/parser.tab.c"
    break;

  case 68: /* literal: KW_FALSE  */
#line 413 "src/parser.y"
    { (yyval.expr) = expression_literal_boolean(0, (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2146 "src/parser.tab.c"
    break;

  case 69: /* literal_list: literal  */
#line 418 "src/parser.y"
    { (yyval.expr_node) = expr_node_new((yyvsp[0].expr)); }
#line 2152 "src/parser.tab.c"
    break;

  case 70: /* literal_list: literal_list COMMA literal  */
#line 420 "src/parser.y"
    {
        expr_node_append((yyvsp[-2].expr_node), (yyvsp[0].expr));
        (yyval.expr_node) = (yyvsp[-2].expr_node);
    }
#line 2161 "src/parser.tab.c"
    break;


#line 2165 "src/parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 426 "src/parser.y"

