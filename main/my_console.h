#include "esp_console.h"
#include <stdbool.h>

typedef enum {
  console_command_arg_type_int,
  console_command_arg_type_str,
  console_command_arg_type_flag
} console_command_arg_type_t;

typedef struct {
  console_command_arg_type_t type;
  const char *shortopts;
  const char *longopts;
  const char *datatype;
  int mincount;
  int maxcount;
  const char *glossary;
} console_command_arg_t;

typedef struct {
  const char *name;
  const char *help;
  const esp_console_cmd_func_t func;
  const int argNum;
  void * pArgStrust;
  const console_command_arg_t * argInfoList;
} console_command_t;

extern void console_register_command( const console_command_t * pList, int len );

#define CONSOLE_DECL_ARG_TERM  struct arg_end * term_end


#define CONSOLE_PARSE( ARGC, ARGV, ARGSTRUST )		\
  {							\
    int nerrors = arg_parse(ARGC, ARGV, (void**)ARGSTRUST );	\
    if (nerrors != 0) {					\
      arg_print_errors(stderr, (ARGSTRUST)->term_end, argv[0]);	\
      return ESP_ERR_INVALID_ARG;			\
    }							\
  }
    

#define CONSOLE_ARG_LEN( ARGINFOLIST ) \
  ( sizeof( ARGINFOLIST ) / sizeof( ARGINFOLIST[0]) )

#define CONSOLE_ARG( TYPE, NAME, OP, LONGOP, MIN, MAX, DESCRIPTION )	\
    {						\
        TYPE,					\
        .shortopts = OP,			\
        .longopts = LONGOP,			\
        .datatype = NAME,			\
        .mincount = MIN,				\
        .maxcount = MAX,				\
        .glossary = DESCRIPTION			\
    }
/**
 * 設定必須な文字列引数
 *
 * @param NAME 引数名
 * @param DESCRIPTION 説明
 */
#define CONSOLE_ARG_STR( NAME, DESCRIPTION )		\
  CONSOLE_ARG( console_command_arg_type_str,		\
	       NAME, NULL, NULL, 1,1, DESCRIPTION )
/**
 * 省略可能な文字列引数
 *
 * @param NAME 引数名
 * @param DESCRIPTION 説明
 */
#define CONSOLE_ARG_STR_OP( NAME, DESCRIPTION ) \
  CONSOLE_ARG( console_command_arg_type_str,		\
	       NAME, NULL, NULL, 0,1, DESCRIPTION )
/**
 * オプション指定の文字列引数
 *
 * @param NAME 引数名
 * @param OP オプション名。 不要な場合 NULL。
 * @param LONGOP 長いオプション名。 不要な場合 NULL。
 *         OP か LONGOP どちらかは NULL 以外の指定が必要。
 * @param DESCRIPTION 説明
 */
#define CONSOLE_ARG_STR_OP_SET( NAME, OP, LONGOP, DESCRIPTION )	\
  CONSOLE_ARG( console_command_arg_type_str,		\
	       NAME, OP, LONGOP, 0,1, DESCRIPTION )



/**
 * 設定必須な文字列引数
 *
 * @param NAME 引数名
 * @param DESCRIPTION 説明
 */
#define CONSOLE_ARG_FLAG( NAME, DESCRIPTION )		\
  CONSOLE_ARG( console_command_arg_type_flag,		\
	       NAME, NULL, NULL, 1,1, DESCRIPTION )
/**
 * 省略可能な文字列引数
 *
 * @param NAME 引数名
 * @param DESCRIPTION 説明
 */
#define CONSOLE_ARG_FLAG_OP( NAME, DESCRIPTION ) \
  CONSOLE_ARG( console_command_arg_type_flag,		\
	       NAME, NULL, NULL, 0,1, DESCRIPTION )
/**
 * オプション指定の文字列引数
 *
 * @param NAME 引数名
 * @param OP オプション名。 不要な場合 NULL。
 * @param LONGOP 長いオプション名。 不要な場合 NULL。
 *         OP か LONGOP どちらかは NULL 以外の指定が必要。
 * @param DESCRIPTION 説明
 */
#define CONSOLE_ARG_FLAG_OP_SET( NAME, OP, LONGOP, DESCRIPTION )	\
  CONSOLE_ARG( console_command_arg_type_flag,		\
	       NAME, OP, LONGOP, 0,1, DESCRIPTION )


#define SEC_TXT_MAX 50

typedef struct {
  char ssid[SEC_TXT_MAX];
  char pass[SEC_TXT_MAX];
  char otaAuthB64[SEC_TXT_MAX];
} wifi_setting_t;

extern bool console_get_wifi_setting( wifi_setting_t * pSetting );
