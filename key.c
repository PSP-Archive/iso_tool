/*
 * key.c
 *
 *  Created on: 2009/10/23
 *      Author: takka
 */

#include <pspkernel.h>

#include "key.h"
#include "main.h"
#include "error.h"

#define CHEACK_KEY (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_RIGHT | \
    PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_TRIANGLE | \
    PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_SQUARE | PSP_CTRL_NOTE | PSP_CTRL_SELECT)

/*---------------------------------------------------------------------------
  キー読取り(入力があるまでループ)
---------------------------------------------------------------------------*/
void get_button_wait(SceCtrlData  *data)
{
  int temp = 0;
  do{
    sceKernelDelayThread(1000);
    sceCtrlPeekBufferPositive( data, 1 );
  }while((data->Buttons & CHEACK_KEY) == 0);

  if(global.enter_button == 1)
  {
    if(data->Buttons & PSP_CTRL_CIRCLE)
      temp |= PSP_CTRL_CROSS;
    if(data->Buttons & PSP_CTRL_CROSS)
      temp |= PSP_CTRL_CIRCLE;
    data->Buttons = (data->Buttons & ~(PSP_CTRL_CROSS | PSP_CTRL_CIRCLE)) | temp;
  }
}

/*---------------------------------------------------------------------------
  キー読取り(リアルタイム)
---------------------------------------------------------------------------*/
void get_button(SceCtrlData  *data)
{
  int temp = 0;
  sceCtrlPeekBufferPositive( data, 1 );
  data->Buttons &= CHEACK_KEY;

  if(global.enter_button == 1)
  {
    if(data->Buttons & PSP_CTRL_CIRCLE)
      temp |= PSP_CTRL_CROSS;
    if(data->Buttons & PSP_CTRL_CROSS)
      temp |= PSP_CTRL_CIRCLE;
    data->Buttons = (data->Buttons & ~(PSP_CTRL_CROSS | PSP_CTRL_CIRCLE)) | temp;
  }
}

/*---------------------------------------------------------------------------
  キーが放されるまで待つ
---------------------------------------------------------------------------*/
void wait_button_up(void)
{
  SceCtrlData  data;

  do{
    sceKernelDelayThread(1000);
    sceCtrlPeekBufferPositive( &data, 1 );
  }while((data.Buttons & CHEACK_KEY) != 0);
}

/*---------------------------------------------------------------------------
  キーが押されるまで待つ
---------------------------------------------------------------------------*/
void wait_button_down(void)
{
  SceCtrlData  data;

  do{
    sceKernelDelayThread(1000);
    sceCtrlPeekBufferPositive( &data, 1 );
  }while((data.Buttons & CHEACK_KEY) != 1);
}
