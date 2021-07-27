/*
 * umd.c
 *
 *  Created on: 2009/10/23
 *      Author: takka
 */

#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <pspumd.h>

#include "umd.h"
#include "error.h"
#include "screen.h"
#include "key.h"
#include "main.h"

#define SECTOR_SIZE (0x800)

/*---------------------------------------------------------------------------
  ファイルリード
---------------------------------------------------------------------------*/
// ファイル名、セクタ番号、読取り長さを指定して、CSOから読込む
// 返値 実際に読み込んだ長さ / エラーの場合は-1を返す
int umd_read(char *buf, const char *path, int pos, int size)
{
  SceUID fp;
  int start_sec;
  int end_sec;
  int sector_num;
  int read_size = 0;
  char tmp_buf[SECTOR_SIZE * 2]; // 展開済みデータバッファ
  int ret;
  int start_pos;
  int end_pos;
  int err;

  err = sceUmdCheckMedium();
  if(err == 0)
    return ERR_NO_UMD;

  fp = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
  if(fp < 0)
    return ERR_OPEN;

  // 読込セクタ数を計算
  start_sec = pos / SECTOR_SIZE;
  end_sec = (pos + size - 1) / SECTOR_SIZE;
  sector_num = start_sec;

  ret = 0;
  while(sector_num <= end_sec)
  {
    // １セクタを読込
    err = sceIoLseek32(fp, sector_num, PSP_SEEK_SET);
    if(err < 0)
      return ERR_SEEK;

    err = sceIoRead(fp, tmp_buf, 1);
    if(err < 0)
      return ERR_READ;

    // 指定バッファに転送
    if((sector_num > start_sec) && (sector_num < end_sec))
    {
      // 全転送
      memcpy(buf, tmp_buf, SECTOR_SIZE);
      read_size = SECTOR_SIZE;
    }
    else if((sector_num == start_sec) || (sector_num == end_sec))
    {
      // 部分転送
      start_pos = 0;
      end_pos = SECTOR_SIZE;
      if(sector_num == start_sec)
        start_pos = pos - (start_sec * SECTOR_SIZE);
      if(sector_num == end_sec)
        end_pos = (pos + size) - (end_sec * SECTOR_SIZE);
      read_size = end_pos - start_pos;
      memcpy(buf, &tmp_buf[start_pos], read_size);
    }

    buf += read_size;
    ret += read_size;
    sector_num++;
  }

  sceIoClose(fp);
  return ret;
}

int check_umd()
{
  SceUID umd;
  SceCtrlData button;

  umd = sceUmdCheckMedium();
  if(umd == 0)
  {
    msg_win("", 0, MSG_CLEAR, 0);
    msg_win("UMDを入れて下さい", 1, MSG_WAIT, 0);
    msg_win("○でキャンセル", 0, MSG_WAIT, 1);

    umd = -1;
    while(umd < 0)
    {
      get_button(&button);
      if(button.Buttons == PSP_CTRL_CROSS)
        return CANCEL;
      umd = sceUmdWaitDriveStatWithTimer(PSP_UMD_PRESENT, 100);
    }
    msg_win("", 0, MSG_CLEAR, 0);
    msg_win("マウント中です", 1, MSG_WAIT, 0);

    umd = sceUmdActivate(1, "disc0:");
    umd = sceUmdWaitDriveStat(PSP_UMD_READY);

    file_stat_print("", "", TYPE_UMD);
  }
  return DONE;
}
