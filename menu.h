/*
 * menu.h
 *
 *  Created on: 2010/01/06
 *      Author: takka
 */

#ifndef MENU_H_
#define MENU_H_

#include "file.h"
#include "error.h"

// メニュー
typedef enum {
  MENU_COMMAND,
  MENU_RET_INT,
  MENU_NOP
} menu_type;

typedef enum {
  TRANS_UMD_ISO,
  TRANS_UMD_CSO,
  TRANS_ISO_CSO,
  TRANS_CSO_ISO
} trans_type;

typedef struct {
  int menu_type;
  char **text;
  int (*command)();
  void *var;
  int opt_1;
  int opt_2;
} menu_item;

typedef struct {
  int (*command)(); // コマンド
  char **text;      // 表示テキスト
  void *var;        // 汎用ポインタ
  int opt_1;        // オプション1
  int opt_2;        // オプション2
} new_menu_item;

typedef struct {
  char **text;
  char **title;
  menu_item *menu;
  int *ret;
  int def;
} menu_list;

extern menu_item menu_iso[];
extern menu_item menu_cso[];
extern menu_item menu_umd[];
extern menu_item menu_dir[];
extern menu_item menu_pbt[];
extern menu_item menu_sys[];

#endif /* MENU_H_ */
