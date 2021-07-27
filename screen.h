/*
 * screen.h
 *
 *  Created on: 2009/12/30
 *      Author: takka
 */

#ifndef SCREEN_H_
#define SCREEN_H_

#include "menu.h"

typedef enum {
  MSG_CLEAR,
  MSG_WAIT,
  MSG_LINE,
  MSG_REDROW
} msg_win_command;

typedef enum {
  MENU_RET_START,
  MENU_RET_SELECT,
  MENU_RET_TRIANGLE,
  MENU_RET_SQUARE,
  MENU_RET_CROSS,
  MENU_RET_CIRCLE,
  MENU_RET_UP,
  MENU_RET_DOWN,
  MENU_RET_RIGHT,
  MENU_RET_LEFT,
  MENU_RET_NORMAL,
  MENU_RET_REDROW
} menu_ret_key;

typedef struct {
  int dir_num;
  int sel_num;
  menu_ret_key key;
} menu_ret_t;

/*---------------------------------------------------------------------------
  初期設定
---------------------------------------------------------------------------*/
void init_screen();

/*---------------------------------------------------------------------------
  ディレクトリメニュー
---------------------------------------------------------------------------*/
int dir_menu(const char* s_path, int redrow_flag);

/*---------------------------------------------------------------------------
  ダイアログの表示
---------------------------------------------------------------------------*/
void dialog(char *text[]);

/*---------------------------------------------------------------------------
  セレクトメニュー
---------------------------------------------------------------------------*/
int select_menu(char *title, menu_item item[], int def, int x, int y);

int msg_win(char *new_text, int beep_flg, msg_win_command command, int parm);
int select_menu_list(menu_list list[]);

int osk(char *out_text, const char *def_text, const char *title, int mode);
int net_dialog();

/*---------------------------------------------------------------------------
  ファイルステータス表示
---------------------------------------------------------------------------*/
void file_stat_print(const char *dir, const char *file, file_type type);

/*---------------------------------------------------------------------------
  メニューの表示
---------------------------------------------------------------------------*/
menu_ret_t menu(char *dir, char *file, file_type type, char *title, menu_item item[], int num, int x, int y);

#endif /* SCREEN_H_ */

