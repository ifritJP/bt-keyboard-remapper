#include "esp_console.h"
#include <stdbool.h>
#include <bluetooth.h>
#include <hid_keyboard_ctrl.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
  

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


#define CONSOLE_PARSE( ARGC, ARGV, ARGSTRUST )			\
  {								\
    int nerrors = arg_parse(ARGC, ARGV, (void**)ARGSTRUST );	\
    if (nerrors != 0) {						\
      arg_print_errors(stderr, (ARGSTRUST)->term_end, argv[0]);	\
      return ESP_ERR_INVALID_ARG;				\
    }								\
  }
    

#define CONSOLE_ARG_LEN( ARGINFOLIST )			\
  ( sizeof( ARGINFOLIST ) / sizeof( ARGINFOLIST[0]) )

#define CONSOLE_ARG( TYPE, NAME, OP, LONGOP, MIN, MAX, DESCRIPTION )	\
  {									\
    TYPE,								\
      .shortopts = OP,							\
      .longopts = LONGOP,						\
      .datatype = NAME,							\
      .mincount = MIN,							\
      .maxcount = MAX,							\
      .glossary = DESCRIPTION						\
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
#define CONSOLE_ARG_STR_OP( NAME, DESCRIPTION )		\
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
  CONSOLE_ARG( console_command_arg_type_str,			\
	       NAME, OP, LONGOP, 0,1, DESCRIPTION )


  /**
   * 設定必須な文字列引数
   *
   * @param NAME 引数名
   * @param DESCRIPTION 説明
   */
#define CONSOLE_ARG_INT( NAME, DESCRIPTION )		\
  CONSOLE_ARG( console_command_arg_type_int,		\
	       NAME, NULL, NULL, 1,1, DESCRIPTION )
  /**
   * 省略可能な文字列引数
   *
   * @param NAME 引数名
   * @param DESCRIPTION 説明
   */
#define CONSOLE_ARG_INT_OP( NAME, DESCRIPTION )		\
  CONSOLE_ARG( console_command_arg_type_int,		\
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
#define CONSOLE_ARG_INT_OP_SET( NAME, OP, LONGOP, DESCRIPTION )	\
  CONSOLE_ARG( console_command_arg_type_int,			\
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
#define CONSOLE_ARG_FLAG_OP( NAME, DESCRIPTION )	\
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
  CONSOLE_ARG( console_command_arg_type_flag,				\
	       NAME, OP, LONGOP, 0,1, DESCRIPTION )


#define SEC_TXT_MAX 50

  typedef struct {
    char ssid[SEC_TXT_MAX];
    char pass[SEC_TXT_MAX];
    char otaAuthB64[SEC_TXT_MAX];
  } wifi_setting_t;

  extern bool console_get_wifi_setting( wifi_setting_t * pSetting );


  typedef enum {
    bt_type_classic,
    bt_type_le,
  } bt_type_t;

  typedef enum {
    bt_roleType_device,
    bt_roleType_host,
  } bt_roleType_t;

#define BT_NAME_MAX 50

  typedef struct {
    int version;
    bt_type_t type;
    bt_roleType_t role;
    bd_addr_t addr;
    bd_addr_t pairAddr;
    bd_addr_type_t pairAddrType;
    char name[ BT_NAME_MAX + 1 ];
  } my_bt_info_t;


  typedef enum {
    my_config_mode_setup,
    my_config_mode_normal,
  } my_config_mode_t;
  typedef struct {
    int version;

    my_config_mode_t mode;
    bd_addr_t toHostAddr;
    hid_device_mode_t hid_device_mode;
    bool isEnableDemo;
  } my_config_t;

  typedef enum {
    console_blob_id_remap,
  } console_blob_id_t;
  
  extern bool console_get_bt_info( const bd_addr_t addr, my_bt_info_t * pInfo );
  extern void console_hid_packet_handler_meta_bt(
    uint16_t channel, const uint8_t * packet, bt_roleType_t roleType );
  extern void console_hid_packet_handler_meta_le(
    uint16_t channel, const uint8_t * packet, bt_roleType_t roleType );
  extern my_config_t * console_get_config( void );
  extern my_config_mode_t console_get_config_mode( void );
  extern hid_device_mode_t console_get_hid_device_mode( void );
  extern void console_loadConfig( void );
  extern void console_save_blob(
    console_blob_id_t id, const void * pBuf, int size );
  extern int console_load_blob(
    console_blob_id_t id, void * pBuf, int size );
  extern bool console_get_isEnableDemo( void );

#ifdef __cplusplus
}
#endif
