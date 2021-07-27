/*
 * key.h
 *
 *  Created on: 2009/10/23
 *      Author: takka
 */

#ifndef KEY_H_
#define KEY_H_

#include <pspctrl.h>

/*---------------------------------------------------------------------------
  キー読取り(入力があるまでループ)
---------------------------------------------------------------------------*/
void get_button_wait(SceCtrlData  *data);

/*---------------------------------------------------------------------------
  キー読取り(リアルタイム)
---------------------------------------------------------------------------*/
void get_button(SceCtrlData  *data);

/*---------------------------------------------------------------------------
  キーが放されるまで待つ
---------------------------------------------------------------------------*/
void wait_button_up(void);

/*---------------------------------------------------------------------------
  キーが押されるまで待つ
---------------------------------------------------------------------------*/
void wait_button_down(void);

#endif /* KEY_H_ */