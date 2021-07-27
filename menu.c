/*
 * menu.c
 *
 *  Created on: 2010/01/06
 *      Author: takka
 */

#include <pspdebug.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <psprtc.h>
#include <psputility.h>
#include <psputility_sysparam.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspctrl_kernel.h>

#include "menu.h"
#include "msg.h"

#include "main.h"
#include "screen.h"
#include "pspdecrypt.h"
#include "sound.h"
#include "ciso.h"
#include "umd.h"
#include "iso.h"
#include "unicode.h"
#include "fnt_print.h"
#include "key.h"
#include "web.h"
#include "macro.h"

#define ERR_RET(err_data, ret_code) \
if((err_data) < 0)          \
{                           \
    err_msg((err_data));    \
    return ret_code;        \
}                           \

#define ERR_RET_2(err_data, err_num) \
if((err_data) < 0)          \
{                           \
    err_msg((err_num));     \
    ret = -1;               \
    goto LABEL_ERR;         \
}                           \

int nop(char *dir, char *file, file_type type, int opt_1, int opt_2);

/*---------------------------------------------------------------------------
  EBOOT_BINの変換
---------------------------------------------------------------------------*/
int eboot_exchange(char *dir, char *file, file_type type, int opt_1, int opt_2);
int eboot_recovery(char *dir, char *file, file_type type, int opt_1, int opt_2);
int rename_file(char *dir, char *file, file_type type, int opt_1, int opt_2);
int iso2cso(char *dir, char *file, file_type type, int opt_1, int opt_2);
int umd2iso(char *dir, char *file, file_type type, int opt_1, int opt_2);
int umd2cso(char *dir, char *file, file_type type, int opt_1, int opt_2);
int cso2iso(char *dir, char *file, file_type type, int opt_1, int opt_2);
int iso_patch(char *dir, char *file, file_type type, int opt_1, int opt_2);
int file_update(char *dir, char *file, file_type type, int opt_1, int opt_2);
int soft_reboot(char *dir, char *file, file_type type, int opt_1, int opt_2);
int remove_file(char *dir, char *file, file_type type, int opt_1, int opt_2);
int fix_header(char *dir, char *file, file_type type, int opt_1, int opt_2);
int boot_iso(char *dir, char *file, file_type type, int opt_1, int opt_2);
int kernel_librarO_patch(char *dir, char *file, file_type type, int opt_1, int opt_2);
int kernel_librarZ_patch(char *dir, char *file, file_type type, int opt_1, int opt_2);

int file_trans(char* out_path, char *in_path, trans_type type, int level, int limit, int opt_1, int opt_2, char* dir);

int prometheus_patch(char *dir, char *file, file_type type, int opt_1, int opt_2);
int prometheus_recovery(char *dir, char *file, file_type type, int opt_1, int opt_2);
int libfont_patch(char *dir, char *file, file_type type, int opt_1, int opt_2);
int libfont_recovery(char *dir, char *file, file_type type, int opt_1, int opt_2);

int patch(char *dir, char *file, file_type type, int opt_1, int opt_2);
int prx_import(char *dir, char *file, file_type type, int opt_1, int opt_2);
int prx_recover(char *dir, char *file, file_type type, int opt_1, int opt_2);

int write_eboot(char *dir, char *file, file_type type, int opt_1, int opt_2);
int ebt_import(char *dir, char *file, file_type type, int opt_1, int opt_2);

menu_item menu_sys[] = {
    { MENU_COMMAND, MSG_UPDATE_UMD_ID_CSV, (file_update), NULL, (int)"UMD_ID.csv", 0 },
    { MENU_COMMAND, MSG_UPDATE_ISO_TOOL,   (file_update), NULL, (int)"EBOOT.PBP", 1 },
    { MENU_COMMAND, MSG_UPDATE_PSPDECRYPT_PRX,   (file_update), NULL, (int)"pspdecrypt.prx", 2 },
    { MENU_NOP,     MSG_SEPARATE, (NULL), NULL, 0, 0 },
    { MENU_COMMAND, MSG_REBOOT,         (soft_reboot), NULL, 0, 0 },
    { MENU_COMMAND, MSG_EXIT,           (soft_reboot), NULL, 1, 0 },
    { MENU_NOP,     MSG_NULL,                 (NULL), NULL, 0, 0 }
};

menu_item menu_iso[] = {
    { MENU_COMMAND, MSG_EBOOT_DECRYPTION, (eboot_exchange), NULL, 0, 0 },
    { MENU_COMMAND, MSG_RECOVERY_EBOOT, (eboot_recovery), NULL, 0, 0 },
    { MENU_COMMAND, MSG_RENAME, (rename_file), NULL, 0, 0 },
    { MENU_COMMAND, MSG_CSO_CONVERT, (iso2cso), NULL, 2048, 0 },
    { MENU_COMMAND, MSG_EBOOT_IMPOERT, (ebt_import), NULL, 0, 0 },
    { MENU_COMMAND, MSG_MENU_PATCH, (patch), NULL, 0, 0 },
    { MENU_COMMAND, MSG_MENU_PRX, (prx_import), NULL, 0, 0 },
    { MENU_NOP,     MSG_SEPARATE, (NULL), NULL, 0, 0 },
    { MENU_COMMAND, MSG_DELETE, (remove_file), NULL, 0, 0 },
    { MENU_NOP,     MSG_NULL,            (NULL), NULL, 0, 0 }
};

menu_item menu_patch[] = {
    { MENU_COMMAND, MSG_PATCH_KERNEL_LIBRARO, (kernel_librarO_patch), NULL, 0, 0 },
    { MENU_COMMAND, MSG_PATCH_KERNEL_LIBRARZ, (kernel_librarZ_patch), NULL, 0, 0 },
    { MENU_COMMAND, MSG_PATCH_PROMETHEUS, (prometheus_patch), NULL, 0, 0 },
    { MENU_COMMAND, MSG_RECOVERY_PROMETHEUS, (prometheus_recovery), NULL, 0, 0 },
//    { MENU_COMMAND, MSG_LIBFONT_PATCH, (libfont_patch), NULL, 0, 0 },
//    { MENU_COMMAND, MSG_LIBFONT_RECOVERY, (libfont_recovery), NULL, 0, 0 },
    { MENU_NOP,     MSG_NULL,            (NULL), NULL, 0, 0 }
};

menu_item menu_prx_import[] = {
    { MENU_RET_INT, MSG_IMPORT_LIBFONT,         (nop), NULL, 0x00, 0 },
    { MENU_RET_INT, MSG_RECOVERY_LIBFONT,       (nop), NULL, 0x10, 0 },
    { MENU_RET_INT, MSG_IMPORT_LIBPSMFPLAYER,   (nop), NULL, 0x01, 0 },
    { MENU_RET_INT, MSG_RECOVERY_LIBPSMFPLAYER, (nop), NULL, 0x11, 0 },
    { MENU_RET_INT, MSG_IMPORT_PSMF,            (nop), NULL, 0x02, 0 },
    { MENU_RET_INT, MSG_RECOVERY_PSMF,          (nop), NULL, 0x12, 0 },
    { MENU_NOP,     MSG_NULL,                 (NULL), NULL, 0, 0 }
};

menu_item menu_cso[] = {
    { MENU_COMMAND, MSG_EBOOT_DECRYPTION, (eboot_exchange), NULL, 0, 0 },
    { MENU_COMMAND, MSG_RECOVERY_EBOOT, (eboot_recovery), NULL, 0, 0 },
    { MENU_COMMAND, MSG_RENAME, (rename_file), NULL, 0, 0 },
    { MENU_COMMAND, MSG_ISO_CONVERT, (cso2iso), NULL, 0, 0 },
    { MENU_NOP,     MSG_SEPARATE, (NULL), NULL, 0, 0 },
    { MENU_COMMAND, MSG_DELETE, (remove_file), NULL, 0, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item menu_umd[] = {
    { MENU_COMMAND, MSG_ISO_CONVERT, (umd2iso), NULL, 0, 0 },
    { MENU_COMMAND, MSG_CSO_CONVERT, (umd2cso), NULL, 0, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item menu_dir[] = {
    { MENU_NOP,     MSG_UNIMPLEMENTED, (NULL), NULL, 0, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item menu_pbt[] = {
    { MENU_COMMAND, MSG_EBOOT_WRITE, (write_eboot), NULL, 0, 0 },
    { MENU_NOP,     MSG_NULL       , (NULL), NULL, 0, 0 }
};

menu_item select_yes_no[] = {
    { MENU_RET_INT, MSG_YES, (nop), NULL, YES, 0 },
    { MENU_RET_INT, MSG_NO, (nop), NULL, NO, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item select_no_yes[] = {
    { MENU_RET_INT, MSG_NO, (nop), NULL, NO, 0 },
    { MENU_RET_INT, MSG_YES, (nop), NULL, YES, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item select_rename[] = {
    { MENU_RET_INT, MSG_OSK, (nop), NULL, 0, 1 },
    { MENU_RET_INT, MSG_UMD_ID, (nop), NULL, 1, 0 },
    { MENU_RET_INT, MSG_JPANESE, (nop), NULL, 2, 1 },
    { MENU_RET_INT, MSG_ENGLISH, (nop), NULL, 3, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item select_cso_level[] = {
    { MENU_RET_INT, MSG_9, (nop), NULL, 9, 0 },
    { MENU_RET_INT, MSG_8, (nop), NULL, 8, 0 },
    { MENU_RET_INT, MSG_7, (nop), NULL, 7, 0 },
    { MENU_RET_INT, MSG_6, (nop), NULL, 6, 0 },
    { MENU_RET_INT, MSG_5, (nop), NULL, 5, 0 },
    { MENU_RET_INT, MSG_4, (nop), NULL, 4, 0 },
    { MENU_RET_INT, MSG_3, (nop), NULL, 3, 0 },
    { MENU_RET_INT, MSG_2, (nop), NULL, 2, 0 },
    { MENU_RET_INT, MSG_1, (nop), NULL, 1, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_item select_cso_threshold[] = {
    { MENU_RET_INT, MSG_100, (nop), NULL, 100, 0 },
    { MENU_RET_INT, MSG_95, (nop), NULL, 95, 0 },
    { MENU_RET_INT, MSG_90, (nop), NULL, 90, 0 },
    { MENU_RET_INT, MSG_85, (nop), NULL, 85, 0 },
    { MENU_RET_INT, MSG_80, (nop), NULL, 80, 0 },
    { MENU_RET_INT, MSG_75, (nop), NULL, 75, 0 },
    { MENU_RET_INT, MSG_70, (nop), NULL, 70, 0 },
    { MENU_RET_INT, MSG_65, (nop), NULL, 65, 0 },
    { MENU_RET_INT, MSG_60, (nop), NULL, 60, 0 },
    { MENU_RET_INT, MSG_55, (nop), NULL, 55, 0 },
    { MENU_NOP,     MSG_NULL    , (NULL), NULL, 0, 0 }
};

menu_list eboot_exchange_menu[] = {
    { MSG_BACKUP_TEXT, MSG_BACKUP, select_no_yes, &global.eboot_backup, 0 },
    { MSG_VERSION_PATCH_TEXT, MSG_VERSION_PATCH, select_no_yes, &global.auto_patch, 0 },
    { MSG_START_TEXT, MSG_START, select_no_yes, NULL, 0 },
    { MSG_NULL, MSG_NULL, NULL, NULL, 0 }
};

menu_list cso_trans_menu[] = {
    { MSG_COMPRESSION_LEVEL_TEXT, MSG_COMPRESSION_LEVEL, select_cso_level, &global.cso_level, 0 },
    { MSG_THRESHOLD_TEXT, MSG_THRESHOLD, select_cso_threshold, &global.cso_threshold, 0 },
    { MSG_NULL, MSG_NULL, NULL, NULL, 0 }
};

char *file_ext[] = { ".ISO", ".CSO" };


int write_eboot(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int o_size;
  int offset;
  int offset2;

  char path[MAX_PATH];

  int ret;
  char *read_ptr = &WORK[DECRYPT_DATA][0];

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);

  ret = ms_read(read_ptr, path, 0, 0);
  ERR_RET(ret, DONE);

  bin2int(&offset, &read_ptr[4 * 2]);
  bin2int(&offset2, &read_ptr[4 * 3]);

  o_size = offset2 - offset;

  ret = ms_write(&read_ptr[offset], "ms0:/ENC/PARAM.SFO", 0, o_size);
  ERR_RET(ret, DONE);

  bin2int(&offset, &read_ptr[4 * 8]);
  bin2int(&offset2, &read_ptr[4 * 9]);

  o_size = offset2 - offset;

  ret = ms_write(&read_ptr[offset], "ms0:/ENC/EBOOT.BIN", 0, o_size);
  ERR_RET(ret, DONE);

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 2);
  msg_win("", 0, MSG_CLEAR, 0);

  return DONE;
}

int ebt_import(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  char path[MAX_PATH];
  char ebt_path[MAX_PATH];

  int ret;
  char *read_ptr = &WORK[CRYPT_DATA][0];

  int pos;
  int size;
  int size_pos;
  SceIoStat stat;
  int eboot_size;
  int new_eboot_size;
  unsigned char *endian;
  char EBOOT_BIN_PATH[] = "ms0:/DEC/EBOOT.BIN";
  char PARAM_SFO_PATH[] = "ms0:/ENC/PARAM.SFO";
   char boot_data[8];
  int bin_flag = 0;
  char *eboot[2] = { "PSP_GAME/SYSDIR/EBOOT.OLD", "PSP_GAME/SYSDIR/EBOOT.BIN" };
  int offset;
  int param_size;

  // path 設定
  strcpy(path, dir);
  strcat(path, file);

  // バックアップ パス設定
  strcpy(ebt_path, "BACK_UP/");
  strcat(ebt_path, global.umd_id);
  strcat(ebt_path, ".EBT");

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
  ret = select_menu(MSG_START[global.language], select_no_yes, 0, 28, 10);
  if(ret != YES)
    return CANCEL;

  // 書込み設定
  ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  ERR_RET(ret, DONE);

  sceIoGetstat(PARAM_SFO_PATH, &stat);
  ERR_RET(ret, DONE);

  // バックアップ EBOOT.BINのファイルサイズ取得
  ret = sceIoGetstat(ebt_path, &stat);
  if(ret < 0)
    eboot_size = 0;
  else
    eboot_size = stat.st_size;

  // NEW EBOOT.BINのファイルサイズ取得
  sceIoGetstat(EBOOT_BIN_PATH, &stat);
  new_eboot_size = stat.st_size;

  // EBOOT.BIN読込み
  ret = get_file_data(&pos, &size, &size_pos, path, type, eboot[0]);
  if(ret < 0)
  {
    ret = get_file_data(&pos, &size, &size_pos, path, type, eboot[1]);
    bin_flag = 1;
  }

  i_size = ms_read(read_ptr, path, pos, size);
  ERR_RET(i_size, DONE);

  // バックアップ
  // 複合済みか判定
  if(strncmp(read_ptr, "~PSP" , 4) == 0)
  {
    // EBTファイルの書込み
    eboot_size = size;
    ret = ms_write(read_ptr, ebt_path, 0, i_size);
    ERR_RET(ret, DONE);
  }

  // NEW EBOOT.BIN読込み
  ret = ms_read(read_ptr, EBOOT_BIN_PATH, 0, 0);
  ERR_RET(ret, DONE);

  // GOD EATER対策
  if(strncmp(global.umd_id, "ULJS-00237", 10) == 0)
    read_ptr[0x62E24] = 0;

  eboot_size = eboot_size | 0x7ff;
  if(eboot_size >= new_eboot_size)
    iso_write(read_ptr, new_eboot_size, path, type, eboot[bin_flag]);
  else
  {
    sceIoGetstat(path, &stat);
    offset = stat.st_size;
    offset = (offset | 0x7ff) + 1;
    ret = ms_write(read_ptr, path, offset, new_eboot_size);
    ERR_RET(ret, DONE);

    offset /= SECTOR_SIZE;
    endian = (unsigned char*)&offset;
    boot_data[0x00] = endian[0];
    boot_data[0x01] = endian[1];
    boot_data[0x02] = endian[2];
    boot_data[0x03] = endian[3];
    boot_data[0x04] = endian[3];
    boot_data[0x05] = endian[2];
    boot_data[0x06] = endian[1];
    boot_data[0x07] = endian[0];
    ret = file_write(boot_data, path, type, size_pos - 8, 8);

    endian = (unsigned char*)&new_eboot_size;
    boot_data[0x00] = endian[0];
    boot_data[0x01] = endian[1];
    boot_data[0x02] = endian[2];
    boot_data[0x03] = endian[3];
    boot_data[0x04] = endian[3];
    boot_data[0x05] = endian[2];
    boot_data[0x06] = endian[1];
    boot_data[0x07] = endian[0];
    ret = file_write(boot_data, path, type, size_pos, 8);
  }

  // PARAM.SFO読込み
  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/PARAM.SFO");
  ERR_RET(ret, DONE);

  param_size = ms_read(read_ptr, PARAM_SFO_PATH, 0, 0);
  ERR_RET(param_size, DONE);

  size = size | 0x7ff;
  if(size >= param_size)
    iso_write(read_ptr, param_size, path, type, "PSP_GAME/PARAM.SFO");
  else
  {
    sceIoGetstat(path, &stat);
    offset = stat.st_size;
    offset = (offset | 0x7ff) + 1;
    ret = ms_write(read_ptr, path, offset, param_size);
    ERR_RET(ret, DONE);

    offset /= SECTOR_SIZE;
    endian = (unsigned char*)&offset;
    boot_data[0x00] = endian[0];
    boot_data[0x01] = endian[1];
    boot_data[0x02] = endian[2];
    boot_data[0x03] = endian[3];
    boot_data[0x04] = endian[3];
    boot_data[0x05] = endian[2];
    boot_data[0x06] = endian[1];
    boot_data[0x07] = endian[0];
    ret = file_write(boot_data, path, type, size_pos - 8, 8);

    endian = (unsigned char*)&param_size;
    boot_data[0x00] = endian[0];
    boot_data[0x01] = endian[1];
    boot_data[0x02] = endian[2];
    boot_data[0x03] = endian[3];
    boot_data[0x04] = endian[3];
    boot_data[0x05] = endian[2];
    boot_data[0x06] = endian[1];
    boot_data[0x07] = endian[0];
    ret = file_write(boot_data, path, type, size_pos, 8);
  }

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 2);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
}

/*---------------------------------------------------------------------------
  prx import
---------------------------------------------------------------------------*/
int prx_import(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  int o_size;
  char path[MAX_PATH];
  char tmp_path[MAX_PATH];

  int ret;
  char *read_ptr = &WORK[0][0];

  char *prx_file[] = {
      "PSP_GAME/USRDIR/MODULE/libfont.prx",
      "PSP_GAME/USRDIR/MODULE/libpsmfplayer.prx",
      "PSP_GAME/USRDIR/MODULE/psmf.prx"
  };
  SceIoStat stat;

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_MENU_PRX[global.language], 1, MSG_WAIT, 0);
  ret = select_menu(MSG_MENU_PRX[global.language], menu_prx_import, 0, 25, 8);
  if(ret < 0)
    return CANCEL;

  if(ret & 0x10)
  {
    ret = prx_recover(dir, file, type, ret & ~0x10, 0);
    return ret;
  }

  // バックアップ
  strcpy(tmp_path, "BACK_UP/");
  strcat(tmp_path, global.umd_id);
  strcat(tmp_path, "_");
  strcat(tmp_path, prx_file[ret]);

  i_size = iso_read(read_ptr, EBOOT_MAX_SIZE, path, type, prx_file[ret]);
  ERR_RET(i_size, DONE);

  ret = sceIoGetstat(tmp_path, &stat);
  if(ret < 0)
  {
    ret = ms_write(read_ptr, tmp_path, 0, i_size);
    ERR_RET(ret, DONE);
  }

  strcpy(tmp_path, "DATA/prx/");
  strcat(tmp_path, prx_file[ret]);

  o_size = ms_read(read_ptr, tmp_path, 0, 0);
  ERR_RET(o_size, DONE);

  if((o_size / SECTOR_SIZE) <= ((i_size + SECTOR_SIZE - 1) / SECTOR_SIZE))
  {
    ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
    ERR_RET(ret, DONE);
    ret = iso_write(read_ptr, o_size, path, type, prx_file[ret]);
    ERR_RET(ret, DONE);
  }
  else
  {
    err_msg(ERR_SIZE_OVER);
    return DONE;
  }

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 2);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
};

/*---------------------------------------------------------------------------
  prx recover
---------------------------------------------------------------------------*/
int prx_recover(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  char path[MAX_PATH];
  char tmp_path[MAX_PATH];

  int ret;
  char *read_ptr = &WORK[0][0];

  char *prx_file[] = {
      "libfont.prx",
      "libpsmfplayer.prx",
      "psmf.prx"
  };

  strcpy(path, dir);
  strcat(path, file);

  strcpy(tmp_path, "BACK_UP/");
  strcat(tmp_path, global.umd_id);
  strcat(tmp_path, "_");
  strcat(tmp_path, prx_file[opt_1]);

  i_size = ms_read(read_ptr, tmp_path, 0, 0);
  ERR_RET(i_size, DONE);

  ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  ERR_RET(ret, DONE);

  ret = iso_write(read_ptr, i_size, path, type, prx_file[opt_1]);
  ERR_RET(ret, DONE);

  msg_win(MSG_FINISHED_RECOVERY[global.language], 1, MSG_WAIT, 2);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
}

int patch(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  menu_ret_t ret;

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_MENU_PATCH[global.language], 1, MSG_WAIT, 0);
  ret = menu(dir, file, type, MSG_MENU_PATCH[global.language], menu_patch, 0, 25, 8);

  return 0;
};

/*---------------------------------------------------------------------------
  prometheus patch
---------------------------------------------------------------------------*/
int prometheus_patch(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  int o_size;
  char path[MAX_PATH];
  char ebt_path[MAX_PATH];

  int ret;
  char *write_ptr = &WORK[DECRYPT_DATA][0];
  char *read_ptr = &WORK[CRYPT_DATA][0];
  char *tmp_ptr;

  int pos;
  int size;
  int size_pos;
  int file_name_ptr;
  int boot_pos;
  SceIoStat stat;
  int eboot_pos;
  int eboot_size;
  int prometheus_pos;
  int prometheus_size;
  unsigned char *endian;
 int ptr;
  char EBOOT_BIN_PATH[] = "DATA/prometheus/EBOOT.BIN";
  char *PROMETHEUS_PATH[2] = {
      "DATA/prometheus/prometheus.prx",
      "DATA/prometheus/KHBBS/prometheus.prx"
  };
  int khbbs_flag = 0;

  char eboot_data[] = {
      0x38, 0x00, 0xFF, 0xEE, 0x00, 0x00, 0x00, 0x00,   0xEE, 0xFF, 0xB6, 0x0D, 0x00, 0x00, 0x00, 0x00,
      0x0D, 0xB6, 0x6D, 0x0C, 0x0E, 0x17, 0x29, 0x00,   0x24, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
      0x09, 0x45, 0x42, 0x4F, 0x4F, 0x54, 0x2E, 0x42,   0x49, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x55,
      0x58, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };


  char prometheus_data[] = {
      0x3C, 0x00, 0xFF, 0xEE, 0x00, 0x00, 0x00, 0x00,   0xEE, 0xFF, 0xF6, 0x15, 0x00, 0x00, 0x00, 0x00,
      0x15, 0xF6, 0x6D, 0x0C, 0x0E, 0x17, 0x29, 0x00,   0x24, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
      0x0E, 0x70, 0x72, 0x6F, 0x6D, 0x65, 0x74, 0x68,   0x65, 0x75, 0x73, 0x2E, 0x70, 0x72, 0x78, 0x00,
      0x00, 0x00, 0x0D, 0x55, 0x58, 0x41, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00
  };

  char boot_data[8];

  strcpy(path, dir);
  strcat(path, file);

  if(strncmp(global.umd_id, "ULJM-05600", 10) == 0)
    khbbs_flag = 1;

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
  ret = select_menu(MSG_START[global.language], select_no_yes, 0, 28, 10);
  if(ret != YES)
    return CANCEL;

  ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  ERR_RET(ret, DONE);

  // EBOOT.BINとprometheus.prxのファイルサイズ取得
  sceIoGetstat(EBOOT_BIN_PATH, &stat);
  eboot_size = stat.st_size;
  sceIoGetstat(PROMETHEUS_PATH[khbbs_flag], &stat);
  prometheus_size = stat.st_size;

  // EBOOT.BIN読込み
  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  if((ret > 0)&&(size != eboot_size))
  {
    i_size = iso_read(read_ptr, EBOOT_MAX_SIZE, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    ERR_RET(i_size, DONE);

    // バックアップ
    // 複合済みか判定
    if(strncmp(read_ptr, "~PSP" , 4) == 0)
    {
      // EBTファイルの書込み
      strcpy(ebt_path, "BACK_UP/");
      strcat(ebt_path, global.umd_id);
      strcat(ebt_path, ".EBT");

      ret = ms_write(read_ptr, ebt_path, 0, i_size);
      ERR_RET(ret, DONE);
    }

    if(strncmp(&read_ptr[1], "ELF" , 3) != 0)
    {
      // 複合化
      o_size = pspDecryptPRX((u8 *)read_ptr, (u8 *)write_ptr, i_size);
      if((o_size < 0)||(strncmp(&write_ptr[1], "ELF", 3) != 0))
      {
        err_msg(ERR_DECRYPT);
        return DONE;
      }
      tmp_ptr = write_ptr;
    }
    else
    {
      o_size = i_size;
      tmp_ptr = read_ptr;
    }

    if(strncmp(global.umd_id, "ULJS-00237", 10) == 0)
      tmp_ptr[0x62E24] = 0;

    ret = iso_write(tmp_ptr, o_size, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    ERR_RET(ret, DONE);

    //  EBOOT.OLDにリネーム
    file_name_ptr = size_pos + 0x17;
    ret = file_write("EBOOT.OLD", path, type, file_name_ptr, sizeof("EBOOT.OLD"));
    ERR_RET(ret, DONE);
  }

  // EBOOT.BINとprometheus.prxをPSPGAME/SYSDIRに入れる
  endian = (unsigned char*)&eboot_size;
  eboot_data[0x0a] = endian[0];
  eboot_data[0x0b] = endian[1];
  eboot_data[0x0c] = endian[2];
  eboot_data[0x0d] = endian[3];
  eboot_data[0x0e] = endian[3];
  eboot_data[0x0f] = endian[2];
  eboot_data[0x10] = endian[1];
  eboot_data[0x11] = endian[0];

  endian = (unsigned char*)&prometheus_size;
  prometheus_data[0x0a] = endian[0];
  prometheus_data[0x0b] = endian[1];
  prometheus_data[0x0c] = endian[2];
  prometheus_data[0x0d] = endian[3];
  prometheus_data[0x0e] = endian[3];
  prometheus_data[0x0f] = endian[2];
  prometheus_data[0x10] = endian[1];
  prometheus_data[0x11] = endian[0];

  ret = get_file_data(&boot_pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/BOOT.BIN");

  eboot_pos = (boot_pos + SECTOR_SIZE - 1) / SECTOR_SIZE;

  prometheus_pos = eboot_pos + (eboot_size + SECTOR_SIZE - 1) / SECTOR_SIZE;

  // BOOT.BINのサイズ/位置を調整
  size = size - (prometheus_pos * SECTOR_SIZE - boot_pos) - prometheus_size;
  boot_pos = (prometheus_pos * SECTOR_SIZE + prometheus_size) / SECTOR_SIZE + 1;

  endian = (unsigned char*)&boot_pos;
  boot_data[0] = endian[0];
  boot_data[1] = endian[1];
  boot_data[2] = endian[2];
  boot_data[3] = endian[3];
  boot_data[4] = endian[3];
  boot_data[5] = endian[2];
  boot_data[6] = endian[1];
  boot_data[7] = endian[0];
  ret = file_write(boot_data, path, type, size_pos - 8, 8);

  endian = (unsigned char*)&size;
  boot_data[0] = endian[0];
  boot_data[1] = endian[1];
  boot_data[2] = endian[2];
  boot_data[3] = endian[3];
  boot_data[4] = endian[3];
  boot_data[5] = endian[2];
  boot_data[6] = endian[1];
  boot_data[7] = endian[0];
  ret = file_write(boot_data, path, type, size_pos, 8);

  endian = (unsigned char*)&eboot_pos;
  eboot_data[0x02] = endian[0];
  eboot_data[0x03] = endian[1];
  eboot_data[0x04] = endian[2];
  eboot_data[0x05] = endian[3];
  eboot_data[0x06] = endian[3];
  eboot_data[0x07] = endian[2];
  eboot_data[0x08] = endian[1];
  eboot_data[0x09] = endian[0];

  endian = (unsigned char*)&prometheus_pos;
  prometheus_data[0x02] = endian[0];
  prometheus_data[0x03] = endian[1];
  prometheus_data[0x04] = endian[2];
  prometheus_data[0x05] = endian[3];
  prometheus_data[0x06] = endian[3];
  prometheus_data[0x07] = endian[2];
  prometheus_data[0x08] = endian[1];
  prometheus_data[0x09] = endian[0];

  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR");
  ERR_RET(ret, DONE);
  ret = file_read(read_ptr, path, type, pos, SECTOR_SIZE);
  ERR_RET(ret, DONE);

  ptr = 0;
  while(read_ptr[ptr] != 0)
    ptr += read_ptr[ptr];

  if(memcmp(&read_ptr[ptr - 0x1B], "prometheus.prx", 14) == 0)
    ptr -= 0x74;

  memcpy(&read_ptr[ptr], eboot_data, sizeof(eboot_data));
  memcpy(&read_ptr[ptr + 0x38], prometheus_data, sizeof(prometheus_data));

  ret = file_write(read_ptr, path, type, pos, SECTOR_SIZE);
  ERR_RET(ret, DONE);

  i_size = ms_read(read_ptr, EBOOT_BIN_PATH, 0, 0);
  ERR_RET(i_size, DONE);
  ret = file_write(read_ptr, path, type, eboot_pos * 0x800, i_size);
  ERR_RET(ret, DONE);

  i_size = ms_read(read_ptr, PROMETHEUS_PATH[khbbs_flag], 0, 0);
  ERR_RET(i_size, DONE);
  ret = file_write(read_ptr, path, type, prometheus_pos * 0x800, i_size);
  ERR_RET(ret, DONE);

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 2);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
}

/*---------------------------------------------------------------------------
  prometheus recovery
---------------------------------------------------------------------------*/
int prometheus_recovery(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char path[MAX_PATH];
  char ebt_path[MAX_PATH];

  int ret;
  char *read_ptr = &WORK[CRYPT_DATA][0];

  int pos;
  int size;
  int size_pos;
  int file_name_ptr;
  int boot_pos;
  int eboot_pos;
  int eboot_size;
  int prometheus_pos;
  int prometheus_size;
  int ptr;
  unsigned char *endian;
  char boot_data[8];

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
  ret = select_menu(MSG_START[global.language], select_no_yes, 0, 28, 10);
  if(ret != YES)
    return CANCEL;

  ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  ERR_RET(ret, DONE);

  // EBOOT.BINとprometheus.prxのファイルサイズ取得
  ret = get_file_data(&eboot_pos, &eboot_size, &size_pos, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  ret = get_file_data(&prometheus_pos, &prometheus_size, &size_pos, path, type, "PSP_GAME/SYSDIR/prometheus.prx");
  if(ret < 0)
    return DONE;

  // ファイルを0で埋める
  memset(read_ptr, 0, eboot_size);
  ret = file_write(read_ptr, path, type, eboot_pos, eboot_size);
  ERR_RET(ret, DONE);

  memset(read_ptr, 0, prometheus_size);
  ret = file_write(read_ptr, path, type, prometheus_pos, prometheus_size);
  ERR_RET(ret, DONE);

  // ディレクトリレコードを削除
  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/");
  ERR_RET(ret, DONE);
  ret = file_read(read_ptr, path, type, pos, SECTOR_SIZE);
  ERR_RET(ret, DONE);

  ptr = 0;
  while(read_ptr[ptr] != 0)
  {
    ret = read_ptr[ptr];
    if(strncasecmp(&read_ptr[ptr + 0x21], "prometheus.prx", 14) == 0)
      memset(&read_ptr[ptr], 0, ret);
    else
      if(strncasecmp(&read_ptr[ptr + 0x21], "EBOOT.BIN", 9) == 0)
        memset(&read_ptr[ptr], 0, ret);
    ptr += ret;
  }
  ret = file_write(read_ptr, path, type, pos, SECTOR_SIZE);
  ERR_RET(ret, DONE);


  // EBOOT.OLDをリネーム
  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/EBOOT.OLD");
  if(ret > 0)
  {
    //  EBOOT.BINにリネーム
    file_name_ptr = size_pos + 0x17;
    ret = file_write("EBOOT.BIN", path, type, file_name_ptr, sizeof("EBOOT.BIN"));
    ERR_RET(ret, DONE);
  }

  // BOOT.BINのサイズ/位置を調整
  ret = get_file_data(&boot_pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/BOOT.BIN");
  size = size + (prometheus_pos - eboot_pos) + prometheus_size;
  boot_pos = eboot_pos / SECTOR_SIZE;

  endian = (unsigned char*)&boot_pos;
  boot_data[0] = endian[0];
  boot_data[1] = endian[1];
  boot_data[2] = endian[2];
  boot_data[3] = endian[3];
  boot_data[4] = endian[3];
  boot_data[5] = endian[2];
  boot_data[6] = endian[1];
  boot_data[7] = endian[0];
  ret = file_write(boot_data, path, type, size_pos - 8, 8);

  endian = (unsigned char*)&size;
  boot_data[0] = endian[0];
  boot_data[1] = endian[1];
  boot_data[2] = endian[2];
  boot_data[3] = endian[3];
  boot_data[4] = endian[3];
  boot_data[5] = endian[2];
  boot_data[6] = endian[1];
  boot_data[7] = endian[0];
  ret = file_write(boot_data, path, type, size_pos, 8);

  // EBTファイルの復元
  strcpy(ebt_path, "BACK_UP/");
  strcat(ebt_path, global.umd_id);
  strcat(ebt_path, ".EBT");

  // ISOファイルへの書込み
  // ファイルサイズの変更
  ret = ms_read(read_ptr, ebt_path, 0, 0);
  if(ret > 0)
  {
    ret = iso_write(read_ptr, ret, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    ERR_RET(ret, DONE);
  }

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 2);
  msg_win("", 0, MSG_CLEAR, 0);
  return DONE;
}

/*---------------------------------------------------------------------------
  NOP
---------------------------------------------------------------------------*/
int nop(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  return 0;
}

/*---------------------------------------------------------------------------
  libfont patch
---------------------------------------------------------------------------*/
int libfont_patch(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  int o_size;
  char path[MAX_PATH];
  char patch[]   = "Kernel_Library\0";

  int num;
  int ret;
  char *write_ptr = &WORK[I_BUFFER][0];
  char *read_ptr = &WORK[O_BUFFER][0];

  int pos;
  int size;
  int size_pos;

  int boot_pos;
  int boot_size;
  int boot_size_pos;

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
  ret = select_menu(MSG_START[global.language], select_no_yes, 0, 28, 10);
  if(ret != YES)
    return CANCEL;

  // libfont.prx読込み
  i_size = iso_read(read_ptr, EBOOT_MAX_SIZE, path, type, "PSP_GAME/USRDIR/MODULE/libfont.prx");
  ERR_RET(i_size, DONE);

  // バックアップ
  // 複合済みか判定
  if(strncmp(&read_ptr[0x40], "~PSP" , 4) == 0)
  {
    // write_ptrへ複合化
    o_size = pspDecryptPRX((u8 *)&read_ptr[0x40], (u8 *)write_ptr, i_size - 0x40);

    // read_ptrへ解凍
    o_size = pspDecompress((u8 *)write_ptr, (u8 *)read_ptr, I_BUFFER *SECTOR_SIZE);

    if((o_size < 0)||(strncmp(&read_ptr[1], "ELF", 3) != 0))
    {
      err_msg(ERR_DECRYPT);
      return DONE;
    }

    memcpy(write_ptr, read_ptr, o_size);

    // パッチ
    num = 0;
    do{
      if(memcmp(&write_ptr[num], patch, 15) == 0)
      {
        write_ptr[num + 13] = 'Z';
      }
      num++;
    }while(num < o_size);

    // ISOファイルへの書込み
    ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
    ERR_RET(ret, DONE);

    // BOOT.BIN情報の取得
    ret = get_file_data(&boot_pos, &boot_size, &boot_size_pos, path, type, "PSP_GAME/SYSDIR/BOOT.BIN");

    // EBOOT.BIN情報の取得(prometheus)
    ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    if((ret > 0) && (pos >= boot_pos) && ((pos + size) <= (boot_pos + boot_size)))
      boot_pos = pos + size;

    // prometheus.prx情報の取得
    ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/prometheus.prx");
    if((ret > 0) && (pos >= boot_pos) && ((pos + size) <= (boot_pos + boot_size)))
      boot_pos = pos + size;

    boot_pos = (boot_pos + SECTOR_SIZE - 1) / SECTOR_SIZE; // 元情報の保存セクタ

    // libfont.prx情報の取得
    ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/USRDIR/MODULE/libfont.prx");

    pos /= SECTOR_SIZE;
    ret = file_write(&pos, path, type, boot_pos * SECTOR_SIZE, 4);       // オリジナルのセクタ位置を保存
    ret = file_write(&size, path, type, boot_pos * SECTOR_SIZE + 4, 4);  // オリジナルのサイズを保存

    boot_pos++;

    ret = file_write(write_ptr, path, type, boot_pos * SECTOR_SIZE, o_size); // BOOT.BINの場所に書込み

    ret = file_write(&o_size, path, type, size_pos, 4);
    ret = file_write(&boot_pos, path, type, size_pos - 8, 4);

    msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 2);
    msg_win("", 0, MSG_CLEAR, 0);
  }

  return 0;
}

int libfont_recovery(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char path[MAX_PATH];
  int ret;

  int pos;
  int size;
  int size_pos;

  int boot_pos;
  int boot_size;
  int boot_size_pos;

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);

  // ISOファイルへの書込み
  ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  ERR_RET(ret, DONE);

  // BOOT.BIN情報の取得
  ret = get_file_data(&boot_pos, &boot_size, &boot_size_pos, path, type, "PSP_GAME/SYSDIR/BOOT.BIN");

  // EBOOT.BIN情報の取得(prometheus)
  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  if((ret > 0) && (pos >= boot_pos) && ((pos + size) <= (boot_pos + boot_size)))
    boot_pos = pos + size;

  // prometheus.prx情報の取得
  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/SYSDIR/prometheus.prx");
  if((ret > 0) && (pos >= boot_pos) && ((pos + size) <= (boot_pos + boot_size)))
    boot_pos = pos + size;

  boot_pos = (boot_pos + SECTOR_SIZE - 1) / SECTOR_SIZE; // 元情報の保存セクタ

  ret = get_file_data(&pos, &size, &size_pos, path, type, "PSP_GAME/USRDIR/MODULE/libfont.prx");
  memset(&WORK[CRYPT_DATA][0], 0, size);
  ret = file_write(&WORK[CRYPT_DATA][0], path, type, pos, size);

  ret = file_read(&pos, path, type, boot_pos * SECTOR_SIZE, 4);       // オリジナルのセクタ位置を取得
  ret = file_read(&size, path, type, boot_pos * SECTOR_SIZE + 4, 4);  // オリジナルのサイズを取得

  ret = file_write(&size, path, type, size_pos, 4);
  ret = file_write(&pos, path, type, size_pos - 8, 4);

  size = 0;
  pos = 0;
  ret = file_write(&pos, path, type, boot_pos * SECTOR_SIZE, 4);       // オリジナルのセクタ位置を取得
  ret = file_write(&size, path, type, boot_pos * SECTOR_SIZE + 4, 4);  // オリジナルのサイズを取得

  msg_win(MSG_FINISHED_RECOVERY[global.language], 1, MSG_WAIT, 6);
  msg_win("", 0, MSG_CLEAR, 0);
  return DONE;
}

/*---------------------------------------------------------------------------
  Kernel_LibrarZパッチ
---------------------------------------------------------------------------*/
int kernel_librarZ_patch(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  char path[MAX_PATH];
  const unsigned char patch_1[] = "Kernel_Library";
  const unsigned char patch_5[] = "Kernel_LibrarO";
  const unsigned char patch_2[] = "Kernel_LibrarZ";
  const unsigned char patch_3[] = "sceUtility";
  const unsigned char patch_4[] = "sceUtilitO";

  int num;
  int ret;
  int mod_flag = 0;
  char *read_ptr = &WORK[DECRYPT_DATA][0];

  strcpy(path, dir);
  strcat(path, file);

  msg_win(MSG_READING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

  // EBOOT.BIN読込み
  i_size = iso_read(read_ptr, EBOOT_MAX_SIZE, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  ERR_RET(i_size, DONE);

  num = 0;
  do{
    if(memcmp(&read_ptr[num], patch_1, sizeof(patch_1)) == 0)
    {
      read_ptr[num + sizeof(patch_1) - 2] = 'Z';
      msg_win(MSG_CHANGED_KERNEL_LIBRARZ[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    else if(memcmp(&read_ptr[num], patch_2, sizeof(patch_2)) == 0)
    {
      read_ptr[num + sizeof(patch_2) - 2] = 'y';
      msg_win(MSG_CHANGED_KERNEL_LIBRARY[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    else if(memcmp(&read_ptr[num], patch_5, sizeof(patch_5)) == 0)
    {
      read_ptr[num + sizeof(patch_5) - 2] = 'Z';
      msg_win(MSG_CHANGED_KERNEL_LIBRARZ[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    if(memcmp(&read_ptr[num], patch_3, sizeof(patch_3)) == 0)
    {
      read_ptr[num + sizeof(patch_3) - 2] = 'O';
      msg_win(MSG_CHANGED_SCEUTILITO[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    else if(memcmp(&read_ptr[num], patch_4, sizeof(patch_4)) == 0)
    {
      read_ptr[num + sizeof(patch_4) - 2] = 'y';
      msg_win(MSG_CHANGED_SCEUTILITY[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    num++;
  }while(num < i_size);

  // ISOファイルへの書込み
  // ファイルサイズの変更
  if(mod_flag == 1)
  {
    msg_win(MSG_WRITING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

    ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
    ERR_RET(ret, DONE);

    ret = iso_write(read_ptr, i_size, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    ERR_RET(ret, DONE);
  }
  else
    msg_win(OBJECT_NOT_FOUND[global.language], 1, MSG_WAIT, 1);

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 6);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
}

/*---------------------------------------------------------------------------
  Kernel_LibrarOパッチ
---------------------------------------------------------------------------*/
int kernel_librarO_patch(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  char path[MAX_PATH];
  const unsigned char patch_1[] = "Kernel_Library";
  const unsigned char patch_2[] = "Kernel_LibrarO";

  int num;
  int ret;
  int mod_flag = 0;
  char *read_ptr = &WORK[DECRYPT_DATA][0];

  strcpy(path, dir);
  strcat(path, file);

  msg_win(MSG_READING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

  // EBOOT.BIN読込み
  i_size = iso_read(read_ptr, EBOOT_MAX_SIZE, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  ERR_RET(i_size, DONE);

  num = 0;
  do{
    if(memcmp(&read_ptr[num], patch_1, sizeof(patch_1)) == 0)
    {
      read_ptr[num + sizeof(patch_1) - 2] = 'O';
      msg_win(MSG_CHANGED_KERNEL_LIBRARO[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    else if(memcmp(&read_ptr[num], patch_2, sizeof(patch_2)) == 0)
    {
      read_ptr[num + sizeof(patch_2) - 2] = 'y';
      msg_win(MSG_CHANGED_KERNEL_LIBRARY[global.language], 1, MSG_WAIT, 0);
      mod_flag = 1;
    }
    num++;
  }while(num < i_size);

  // ISOファイルへの書込み
  // ファイルサイズの変更
  if(mod_flag == 1)
  {
    msg_win(MSG_WRITING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

    ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
    ERR_RET(ret, DONE);

    ret = iso_write(read_ptr, i_size, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    ERR_RET(ret, DONE);
  }
  else
    msg_win(OBJECT_NOT_FOUND[global.language], 1, MSG_WAIT, 1);

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 6);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
}

/*---------------------------------------------------------------------------
  EBOOT_BINの変換
---------------------------------------------------------------------------*/
int eboot_exchange(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  int i_size;
  int o_size;
  char path[MAX_PATH];
  char ebt_path[MAX_PATH];
  const unsigned char patch_1[] = { 0xE0, 0xFF, 0xBD, 0x27, 0x05, 0x05, 0x02, 0x3C, 0x0C, 0x00, 0xB3, 0xAF};
  const unsigned char patch_2[] = { 0x00, 0x40, 0x05, 0x34, 0x05, 0x05, 0x04, 0x3C, 0x99, 0x81, 0x05, 0x0C};

  int num;
  int ret;
  int mod_flag = 0;
  char *write_ptr = &WORK[DECRYPT_DATA][0];
  char *read_ptr = &WORK[CRYPT_DATA][0];

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);
  ret = select_menu_list(eboot_exchange_menu);
  if(ret != YES)
    return CANCEL;

  msg_win(MSG_READING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

  // EBOOT.BIN読込み
  i_size = iso_read(read_ptr, EBOOT_MAX_SIZE, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  ERR_RET(i_size, DONE);

  // バックアップ
  // 複合済みか判定
  if(strncmp(read_ptr, "~PSP" , 4) == 0)
  {
    if(global.eboot_backup == YES)
    {
      // EBTファイルの書込み
      msg_win(MSG_WRITING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

      strcpy(ebt_path, "BACK_UP/");
      strcat(ebt_path, global.umd_id);
      strcat(ebt_path, ".EBT");

      ret = ms_write(read_ptr, ebt_path, 0, i_size);
      ERR_RET(ret, DONE);
    }
  }

  if(strncmp(&read_ptr[1], "ELF" , 3) == 0)
  {
    // 複合化ずみ
    msg_win(MSG_ALREADY_DECRYPT_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);
    o_size = i_size;
    write_ptr = read_ptr;
  }
  else
  {
    // 複合化
    msg_win(MSG_DECRYPTING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);
    o_size = pspDecryptPRX((u8 *)read_ptr, (u8 *)write_ptr, i_size);
    if((o_size < 0)||(strncmp(&write_ptr[1], "ELF", 3) != 0))
    {
      err_msg(ERR_DECRYPT);
      return DONE;
    }

    mod_flag = 1;
  }

  // パッチ
  //    addiu  r29,r29,-$20                       ;0000010C[27BDFFE0,'...''] E0 FF BD 27
  // +  lui    r2,$0505                           ;00000110[3C020505,'...<'] 05 05 02 3C
  //    sw     r19,$c(r29)                        ;00000114[AFB3000C,'....'] 0C 00 B3 AF

  //47 81 05 0C 00 40 05 34 05 05 04 3C 99 81 05 0C

  if(global.auto_patch == 1)
  {
    num = 0;
    do{
      if(memcmp(&write_ptr[num], patch_1, 12) == 0)
      {
        write_ptr[num + 4] = 0;
        msg_win(MSG_PATCHED_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);
        mod_flag = 1;
      }
      if(memcmp(&write_ptr[num], patch_2, 12) == 0)
      {
        write_ptr[num + 4] = 0;
        msg_win(MSG_PATCHED_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);
        mod_flag = 1;
      }
      num++;
    }while(num < o_size);
  }

  // ISOファイルへの書込み
  // ファイルサイズの変更
  if(mod_flag == 1)
  {
    msg_win(MSG_WRITING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

    ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
    ERR_RET(ret, DONE);

    ret = iso_write(write_ptr, o_size, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
    ERR_RET(ret, DONE);
  }

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 6);
  msg_win("", 0, MSG_CLEAR, 0);
  return 0;
}

/*---------------------------------------------------------------------------
  EBOOT_BINのリカバリ
---------------------------------------------------------------------------*/
int eboot_recovery(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char path[MAX_PATH];
  char ebt_path[4][MAX_PATH];
  int ebt_size;
  int loop;
  int ret;

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_READING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);

  // EBOOT.BIN読込み
  strcpy(ebt_path[0], "BACK_UP/");
  strcat(ebt_path[0], global.umd_id);
  strcat(ebt_path[0], ".EBT");

  strcpy(ebt_path[1], "BACK_UP/");
  strcat(ebt_path[1], file);
  strcpy(&ebt_path[1][strlen(ebt_path[1]) - 3], "EBT");

  strcpy(ebt_path[2], dir);
  strcat(ebt_path[2], global.umd_id);
  strcat(ebt_path[2], ".EBT");

  strcpy(ebt_path[3], path);
  strcpy(&ebt_path[3][strlen(ebt_path[3]) - 3], "EBT");

  for(loop = 0; loop < 4; loop++)
  {
    ebt_size = ms_read(&WORK[CRYPT_DATA][0], ebt_path[loop], 0, 0);
    if(ebt_size > 0)
      break;
  }
  ERR_RET(ebt_size, DONE);

  // ISOファイルへの書込み
  // ファイルサイズの変更
  msg_win(MSG_RECOVERING_EBOOT_BIN[global.language], 1, MSG_WAIT, 1);
  ret = set_file_mode(path, FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  ERR_RET(ret, DONE);

  ret = iso_write(&WORK[CRYPT_DATA][0], ebt_size, path, type, "PSP_GAME/SYSDIR/EBOOT.BIN");
  ERR_RET(ret, DONE);

  msg_win(MSG_FINISHED[global.language], 1, MSG_WAIT, 6);
  msg_win("", 0, MSG_CLEAR, 0);
  return DONE;
}

int rename_file(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char path[MAX_PATH];
  char new_path[MAX_PATH];
  char id[MAX_PATH];
  char j_name[MAX_PATH];
  char e_name[MAX_PATH];
  char *name = NULL;
  char work[MAX_PATH];
  char work2[MAX_PATH];
  char *text[15];
  int name_type;
  int ret;
  int mode = 0;
  char *ptr;
  int loop;
  char bad_char[] = "<>:\"/\\|?*";
  char good_char[] = "()_\'_____";

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_RENAME[global.language], 1, MSG_WAIT, 1);
  name_type = select_menu(MSG_TYPE[global.language], select_rename, 0, 25, 8);

  if(name_type < 0)
    return -1;
  else
  {
    strcpy(path, dir);
    strcat(path, file);
    get_umd_id(id, path, type);
    get_umd_name(j_name, e_name, id, 0);

    switch(name_type)
    {
      case 0: // リネーム
        strcpy(j_name, file);
        j_name[strlen(j_name) - 4] = '\0';
        sjis_to_utf8(e_name, j_name);
        name = e_name;
        mode = 1;
        break;

      case 1: // UMD ID
        name = id;
        mode = 0;
        break;

      case 2: // 日本語名
        name = j_name;
        mode = 1;
        break;

      case 3: // 英語名
        name = e_name;
        mode = 0;
        break;

    }

    ret = osk(work, name, MSG_RENAME[global.language], mode);

    if(ret == PSP_UTILITY_OSK_RESULT_CHANGED)
    {
      // 駄目文字チェック
      // < > : " / \ | ? *
      sjis_to_utf8(work2, work);

      for(loop = strlen(bad_char); loop > 0; loop--)
      {
        ptr =strchr(work2, bad_char[loop - 1]);
        if(ptr != NULL)
        {
          *ptr = good_char[loop - 1];
          loop++;
        }
      }

      // 先頭の「.」対策
      if(work2[0] == '.')
      {
        strcpy(work, work2);
        work2[0] = ' ';
        strcpy(&work2[1], work);
      }

      utf8_to_utf16((u16*)work, work2);
      utf16_to_sjis(work2, work);

      strcpy(new_path, dir);
      strcat(new_path, work2);
      strcat(new_path, file_ext[type]);

      ret = sceIoRename(path, new_path);
      if(ret < 0)
      {
          err_msg(ERR_RENAME);
          return DONE;
      }

      text[0] = MSG_FINISHED[global.language];
      text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
      text[2] = "\0";
      dialog(text);
    }
  }

  return DONE;
}

// TODO
int iso_patch(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  return CANCEL;
}


#define file_trans_macro_init(buf_num, size)                        \
    char in_path[MAX_PATH];                                             \
    char out_path[MAX_PATH];                                            \
    char ren_path[MAX_PATH];                                            \
    char work[MAX_PATH];                                                \
    char work2[MAX_PATH];                                               \
    char *read_buf[buf_num];              /* 読込バッファ ポインタ */ \
    char *write_buf[buf_num];             /* 書込バッファ ポインタ */ \
                                                                         \
    SceUID fp_in = 0;                       /* 読込ファイル ポインタ */  \
    SceUID fp_out = 0;                      /* 書込ファイル ポインタ */  \
    SceInt64 res = -1;                      /* 非同期ファイルIO用 */     \
                                                                         \
    const int MAX_READ_SIZE = (size);       /* 一回の読込サイズ */      \
                                                                         \
    int in_sec_num = 0;                     /* 総セクタ数 */            \
    int read_size[3];                       /* 読込んだサイズ */        \
    int write_size = 0;                     /* 一回の書込サイズ */      \
    int write_sector = 0;                   /* 処理したセクタ数 */      \
    int read_sector = 0;                    /* 処理したセクタ数 */      \
                                                                        \
    int num;                                /* 汎用 */                  \
    int ret;                                                            \
                                                                         \
    char *text[15];                         /* テキスト表示用 */        \
    char msg[256];                          /* テキスト表示用 */        \
                                                                        \
    SceCtrlData  data;                                                  \
    u64 start_tick;                                                     \
    u64 now_tick;                                                       \
    u64 old_tick = 0;                                                   \
    pspTime date1;                                                      \
    pspTime date2;                                                      \
    pspTime date3;                                                      \
    int first_wait = 0;                                                 \


#define file_trans_macro_cso_init()                                 \
    CISO_H header;                        /* CSOヘッダ */                \
    int *tag = NULL;                     /* CSOセクタタグ ポインタ */    \
    int tag_size = 0;                     /* CSOセクタタグ サイズ */    \
    char cso_buf[SECTOR_SIZE * 3 / 2];    /* CSO変換バッファ */         \
    int write_ptr = 0;                    /* 書込バッファへの転送位置 */    \
    int cso_sec_ptr = 0;                  /* CSOセクタ位置 */            \
    int cso_sec_size;                     /* 圧縮後のセクタサイズ */      \
    int cso_sector = 0;              /* 処理したセクタ数 */         \
    int loop;                                                   \

#define file_trans_ms_umd_check() \
    /* MSチェック */    \
    ret = check_ms();   \
    if(ret < 0)         \
      return CANCEL;    \
                        \
    /* UMDチェック */   \
    ret = check_umd();  \
    if(ret < 0)         \
      return CANCEL;   \

// TODO
int umd2iso(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  file_trans_macro_init(3, 512);

  // buff 設定
  read_buf[0] = &WORK[O_BUFFER][0];
  read_buf[1] = &WORK[O_BUFFER + MAX_READ_SIZE][0];
  read_buf[2] = &WORK[O_BUFFER + MAX_READ_SIZE * 2][0];
  write_buf[0] = &WORK[O_BUFFER][0];
  write_buf[1] = &WORK[O_BUFFER + MAX_READ_SIZE][0];
  write_buf[2] = &WORK[O_BUFFER + MAX_READ_SIZE * 2][0];

  file_trans_ms_umd_check();

  // 空き容量チェック
  if((global.umd_size / 1024) > global.ms_free_size)
  {
    err_msg(ERR_SIZE_OVER);
    return CANCEL;
  }

  // ファイル名設定
  strcpy(in_path, dir);
  strcat(in_path, file);

  strcpy(out_path, dir);
  ret = osk(work, global.umd_id, MSG_FILE_NAME[global.language], 0);
  if(ret == PSP_UTILITY_OSK_RESULT_CHANGED)
    strcat(out_path, work);
  else
    strcat(out_path, global.umd_id);
  strcat(out_path, ".ISO");

  strcpy(ren_path, dir);
  strcat(ren_path, "01234565789012345678901234567890123456789012345678901234567890123456789");

  msg_win("", 0, MSG_CLEAR, 0);
  sjis_to_utf8(work2, strrchr(out_path, '/') + 1);
  sprintf(msg, MSG_OUTPUT_NAME[global.language], work2);
  msg_win(msg, 0, MSG_WAIT, 0);

  if(check_file(out_path) < 0)
  {
    msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_START[global.language], select_yes_no, 0, 28, 10);
  }
  else
  {
    msg_win(MSG_THERE_ARE_FILES_WITH_THE_SAME_NAME[global.language], 0, MSG_WAIT, 0);
    msg_win(MSG_OVERWRITE_[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_OVERWRITE[global.language], select_no_yes, 0, 28, 10);
  }

  if(ret != YES)
    return CANCEL;

  in_sec_num = get_umd_sector("umd:", TYPE_UMD);

  // open設定
  fp_in  = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
  ERR_RET_2(fp_in, ERR_OPEN);
  sceIoLseek32(fp_in, 0, SEEK_SET);

  fp_out = sceIoOpen(ren_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  ERR_RET_2(fp_out, ERR_OPEN);
  sceIoLseek32(fp_out, 0, SEEK_SET);

  // msg
  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_UMD_ISO_CONVERT[global.language], 0, MSG_WAIT, 0);
  sprintf(msg, MSG_SECTOR_COMPLETE[global.language], write_sector, in_sec_num);
  msg_win(msg, 0, MSG_LINE, 1);

  sceRtcGetCurrentTick(&start_tick);
  old_tick = start_tick;

  set_clock(333, 166);

  // 2ブロック分先行読込
  read_size[0] = sceIoRead(fp_in, read_buf[0], MAX_READ_SIZE);
  ERR_RET_2(read_size[0], ERR_READ);
  read_sector += read_size[0];
  read_size[1] = sceIoReadAsync(fp_in, read_buf[1], MAX_READ_SIZE);
  ERR_RET_2(read_size[1], ERR_READ);

  // loop前
  write_sector = 0;
  num = 0;

  // loop TODO
  while(in_sec_num > write_sector)
  {
    // 遅延書込み終了
    msg_win("Wait WRITE", 0, MSG_LINE, 4);
    ret = sceIoWaitAsync(fp_out, &res);
    if(res < 0)
    {
      if(first_wait == 1)
      {
        err_msg(ERR_WRITE);
        return CANCEL;
      }
      first_wait = 1;
    }

    write_sector += read_size[num];

    // 遅延書込み開始
    write_size = read_size[num] << 11;
    msg_win("WRITE", 0, MSG_LINE, 4);
    ret = sceIoWriteAsync(fp_out, write_buf[num], write_size);
    ERR_RET_2(ret, ERR_WRITE);

    // 遅延読込み終了
    if(read_sector < in_sec_num)
    {
      msg_win("Wait READ", 0, MSG_LINE, 4);
      ret = sceIoWaitAsync(fp_in, &res);
      ERR_RET_2(res, ERR_READ);
      read_size[(num + 1) % 3] = res;
      read_sector += res;
    }
    // 遅延読込み開始
    if(read_sector < in_sec_num)
    {
      msg_win("READ", 0, MSG_LINE, 4);
      ret = sceIoReadAsync(fp_in, read_buf[(num + 2) % 3], MAX_READ_SIZE);
      if(ret < 0)
      {
        err_msg(ERR_READ);
        return CANCEL;
      }
    }

    sprintf(msg, MSG_SECTOR_COMPLETE[global.language], write_sector, in_sec_num);
    msg_win(msg, 0, MSG_LINE, 1);
    sceRtcGetCurrentTick(&now_tick);
    now_tick -= start_tick;
    ret = (MAX_READ_SIZE << 11) / 1024 * 1000 /((now_tick - old_tick) / 1000);
    old_tick = now_tick;
    sceRtcSetTick(&date1, &now_tick);
    now_tick = now_tick * in_sec_num / write_sector;
    sceRtcSetTick(&date2, &now_tick);
    now_tick -= old_tick;
    sceRtcSetTick(&date3, &now_tick);
    sprintf(msg, MSG_NOW_EST_LEFT[global.language],
        date1.hour, date1.minutes, date1.seconds,
        date2.hour, date2.minutes, date2.seconds,
        date3.hour, date3.minutes, date3.seconds);
    msg_win(msg, 0, MSG_LINE, 2);
    sprintf(msg, MSG_SPEED[global.language], ret);
    msg_win(msg, 0, MSG_LINE, 3);

    get_button(&data);
    if((data.Buttons) == PSP_CTRL_CROSS)
    {
      msg_win(MSG_STOP[global.language], 0, MSG_LINE, 2);
      ret = select_menu(MSG_STOP_TEXT[global.language], select_no_yes, 0, 28, 10);
      if(ret == 1)
      {
        text[0] = MSG_CONVERSION_STOP[global.language];
        text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
        text[2] = "\0";
        dialog(text);
        ret = -1;
        goto LABEL_ERR;
      }
      dir_menu(NULL, YES);
      msg_win("", 0, MSG_REDROW, 0);
    }
    num = (num + 1) % 3;
  }

  // msg
  sprintf(msg, MSG_SECTOR_COMPLETE[global.language], in_sec_num, in_sec_num);
  msg_win(msg, 0, MSG_LINE, 1);

  sceRtcGetCurrentTick(&now_tick);
  now_tick -= start_tick;
  sceRtcSetTick(&date3, &now_tick);
  ret = (in_sec_num * SECTOR_SIZE) / 1024 * 1000 / (now_tick / 1000);
  sprintf(msg, MSG_TOTAL_TIME_AVERAGE_SPEED[global.language], date3.hour, date3.minutes, date3.seconds, ret);
  msg_win(msg, 0, MSG_LINE, 0);
  sprintf(msg, MSG_NOW_EST_LEFT_FINISH[global.language],
      date1.hour, date1.minutes, date1.seconds,
      date2.hour, date2.minutes, date2.seconds);
  msg_win(msg, 0, MSG_LINE, 2);
  msg_win("", 0, MSG_LINE, 3);
  msg_win("", 0, MSG_LINE, 4);
  msg_win("", 0, MSG_LINE, 5);
  ret = 0;

LABEL_ERR:
  // close
  sceIoWaitAsync(fp_in, &res);
  sceIoWaitAsync(fp_out, &res);
  sceIoClose(fp_in);
  sceIoClose(fp_out);

  if(ret != -1)
  {
    sceIoRemove(out_path);
    sceIoRename(ren_path, out_path);

    text[0] = MSG_FINISHED[global.language];
    text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
    text[2] = "\0";
    dialog(text);
  }
  else
    sceIoRemove(ren_path);

  set_clock(222, 111);
  return DONE;
}

// TODO
int umd2cso(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  file_trans_macro_init(3, 512);
  file_trans_macro_cso_init();

  // buff 設定
  read_buf[0] = &WORK[O_BUFFER][0];
  read_buf[1] = &WORK[O_BUFFER + MAX_READ_SIZE][0];
  read_buf[2] = &WORK[O_BUFFER + MAX_READ_SIZE * 2][0];
  write_buf[0] = &WORK[O_BUFFER + MAX_READ_SIZE * 3][0];
  write_buf[1] = &WORK[O_BUFFER + MAX_READ_SIZE * 4][0];
  write_buf[2] = &WORK[O_BUFFER + MAX_READ_SIZE * 5][0];

  // tag設定
  tag = (int *)&WORK[O_BUFFER + MAX_READ_SIZE * 6][0];

  file_trans_ms_umd_check();

  // 空き容量チェック
  if((global.umd_size / 1024) > global.ms_free_size)
  {
    msg_win(MSG_FREE_SPACE_IS_LESS_THAN_ISO_SIZE[global.language], 0, MSG_WAIT, 0);
    msg_win(MSG_THERE_IS_A_POSSIBILITY_OF_FAILING[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_CONTINUE[global.language], select_yes_no, 0, 28, 10);
    if(ret != YES)
      return CANCEL;
  }

  // パラメータ設定
  msg_win("", 0, MSG_CLEAR, 0);
  ret = select_menu_list(cso_trans_menu);
  if(ret == CANCEL )
    return CANCEL;

  // ファイル名設定
  strcpy(in_path, dir);
  strcat(in_path, file);

  strcpy(out_path, dir);
  strcpy(work2, global.umd_id);
  sprintf(work, "_%d_%03d", global.cso_level, global.cso_threshold);
  strcat(work2, work);

  ret = osk(work, work2, MSG_FILE_NAME[global.language], 0);
  if(ret == PSP_UTILITY_OSK_RESULT_CHANGED)
    strcat(out_path, work);
  else
    strcat(out_path, work2);

  strcat(out_path, ".CSO");

  strcpy(ren_path, dir);
  strcat(ren_path, "01234565789012345678901234567890123456789012345678901234567890123456789");

  msg_win("", 0, MSG_CLEAR, 0);
  sjis_to_utf8(work, strrchr(out_path, '/') + 1);
  sprintf(msg, MSG_OUTPUT_NAME[global.language], work);
  msg_win(msg, 0, MSG_WAIT, 0);

  if(check_file(out_path) < 0)
  {
    msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_START[global.language], select_yes_no, 0, 28, 10);
  }
  else
  {
    msg_win(MSG_THERE_ARE_FILES_WITH_THE_SAME_NAME[global.language], 0, MSG_WAIT, 0);
    msg_win(MSG_OVERWRITE_[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_OVERWRITE[global.language], select_no_yes, 0, 28, 10);
  }

  if(ret != YES)
    return CANCEL;

  in_sec_num = get_umd_sector("umd:", TYPE_UMD);
  tag_size = in_sec_num + 1;

  // open設定
  fp_in  = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
  ERR_RET_2(fp_in, ERR_OPEN);
  sceIoLseek32(fp_in, 0, SEEK_SET);

  fp_out = sceIoOpen(ren_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  ERR_RET_2(fp_out, ERR_OPEN);
  sceIoLseek32(fp_out, 0, SEEK_SET);

  // msg
  msg_win("", 0, MSG_CLEAR, 0);
  sprintf(msg, MSG_UMD_CSO_CONVERT[global.language], global.cso_level, global.cso_threshold);
  msg_win(msg, 0, MSG_WAIT, 0);
  sprintf(msg, MSG_SECTOR_COMPLETE[global.language], write_sector, in_sec_num);
  msg_win(msg, 0, MSG_LINE, 1);

  sceRtcGetCurrentTick(&start_tick);
  old_tick = start_tick;

  set_clock(333, 166);

  // 2ブロック分先行読込
  read_size[0] = sceIoRead(fp_in, read_buf[0], MAX_READ_SIZE);
  ERR_RET_2(read_size[0], ERR_READ);
  read_sector += read_size[0];
  read_size[1] = sceIoReadAsync(fp_in, read_buf[1], MAX_READ_SIZE);
  ERR_RET_2(read_size[1], ERR_READ);

  // header
  header.magic[0] = 'C';
  header.magic[1] = 'I';
  header.magic[2] = 'S';
  header.magic[3] = 'O';
  header.header_size = 0x00; /* 本来 0x18 であるが、PSP Filer 6.6で読み込めなくなるので一時的に修正*/
  header.total_bytes = in_sec_num * SECTOR_SIZE;
  header.block_size = SECTOR_SIZE;
  header.ver = 0x01;
  header.align = 0x00;
  header.rsv_06[0] = 0x00;
  header.rsv_06[1] = 0x00;

  // ヘッダ書込み
  ret = sceIoWrite(fp_out, &header, CISO_HEADER_SIZE);
  ERR_RET_2(ret, ERR_WRITE);
  cso_sec_ptr = CISO_HEADER_SIZE + (tag_size * 4);
  tag[0] = cso_sec_ptr;
  ret = sceIoLseek32(fp_out, cso_sec_ptr, PSP_SEEK_SET);
  ERR_RET_2(ret, ERR_SEEK);

  // loop前
  cso_sector = 0;
  write_sector = 0;
  num = 0;

  // loop
  while(in_sec_num > write_sector)
  {
    write_sector += read_size[num];

    // CSO変換
    msg_win("DEFLATE", 0, MSG_LINE, 4);
    loop = 0;
    write_size = 0;
    write_ptr = 0;
    while(read_size[num] > 0)
    {
      cso_sec_size = deflate_cso(cso_buf, sizeof(cso_buf), (read_buf[num] + (loop * SECTOR_SIZE)), SECTOR_SIZE, global.cso_level);
      if(cso_sec_size < 0)
      {
        text[0] = MSG_CSO_CONVERSION_ERROR[global.language];
        text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
        text[2] = "\0";
        dialog(text);
        ret = -1;
        goto LABEL_ERR;
      }
      if(cso_sec_size < (SECTOR_SIZE * global.cso_threshold / 100))
      {
        write_size += cso_sec_size;
        cso_sec_ptr += cso_sec_size;
        tag[cso_sector + 1] = cso_sec_ptr;
        memcpy(write_buf[num] + write_ptr, cso_buf, cso_sec_size);
      }
      else
      {
        cso_sec_size = SECTOR_SIZE;
        write_size += SECTOR_SIZE;
        cso_sec_ptr += SECTOR_SIZE;
        tag[cso_sector] |= 0x80000000;
        tag[cso_sector + 1] = cso_sec_ptr;
        memcpy(write_buf[num] + write_ptr, read_buf[num] + loop * SECTOR_SIZE, SECTOR_SIZE);
      }
      write_ptr += cso_sec_size;
      read_size[num]--;
      loop++;
      cso_sector++;
    }

    // 遅延書込み終了
    msg_win("Wait WRITE", 0, MSG_LINE, 4);
    ret = sceIoWaitAsync(fp_out, &res);
    if(res < 0)
    {
      if(first_wait == 1)
      {
        err_msg(ERR_WRITE);
        return CANCEL;
      }
      first_wait = 1;
    }

    // 遅延書込み開始
    msg_win("WRITE", 0, MSG_LINE, 4);
    ret = sceIoWriteAsync(fp_out, write_buf[num], write_size);
    ERR_RET_2(ret, ERR_WRITE);

    // 遅延読込み終了
    if(read_sector < in_sec_num)
    {
      msg_win("Wait READ", 0, MSG_LINE, 4);
      ret = sceIoWaitAsync(fp_in, &res);
      ERR_RET_2(res, ERR_READ);
      read_size[(num + 1) % 3] = res;
      read_sector += res;
    }
    // 遅延読込み開始
    if(read_sector < in_sec_num)
    {
      msg_win("READ", 0, MSG_LINE, 4);
      ret = sceIoReadAsync(fp_in, read_buf[(num + 2) % 3], MAX_READ_SIZE);
      if(ret < 0)
      {
        err_msg(ERR_READ);
        return CANCEL;
      }
    }

    sprintf(msg, MSG_SECTOR_COMPLETE[global.language], write_sector, in_sec_num);
    msg_win(msg, 0, MSG_LINE, 1);
    sceRtcGetCurrentTick(&now_tick);
    now_tick -= start_tick;
    ret = (MAX_READ_SIZE << 11) / 1024 * 1000 /((now_tick - old_tick) / 1000);
    old_tick = now_tick;
    sceRtcSetTick(&date1, &now_tick);
    now_tick = now_tick * in_sec_num / write_sector;
    sceRtcSetTick(&date2, &now_tick);
    now_tick -= old_tick;
    sceRtcSetTick(&date3, &now_tick);
    sprintf(msg, MSG_NOW_EST_LEFT[global.language],
        date1.hour, date1.minutes, date1.seconds,
        date2.hour, date2.minutes, date2.seconds,
        date3.hour, date3.minutes, date3.seconds);
    msg_win(msg, 0, MSG_LINE, 2);
    sprintf(msg, MSG_SPEED[global.language], ret);
    msg_win(msg, 0, MSG_LINE, 3);

    get_button(&data);
    if((data.Buttons) == PSP_CTRL_CROSS)
    {
      msg_win(MSG_STOP[global.language], 0, MSG_LINE, 2);
      ret = select_menu(MSG_STOP_TEXT[global.language], select_no_yes, 0, 28, 10);
      if(ret == 1)
      {
        text[0] = MSG_CONVERSION_STOP[global.language];
        text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
        text[2] = "\0";
        dialog(text);
        ret = -1;
        goto LABEL_ERR;
      }
      dir_menu(NULL, YES);
      msg_win("", 0, MSG_REDROW, 0);
    }
    num = (num + 1) % 3;
  }

  // msg
  sprintf(msg, MSG_SECTOR_COMPLETE[global.language], in_sec_num, in_sec_num);
  msg_win(msg, 0, MSG_LINE, 1);

  sceRtcGetCurrentTick(&now_tick);
  now_tick -= start_tick;
  sceRtcSetTick(&date3, &now_tick);
  ret = (in_sec_num * SECTOR_SIZE) / 1024 * 1000 / (now_tick / 1000);
  sprintf(msg, MSG_TOTAL_TIME_AVERAGE_SPEED[global.language], date3.hour, date3.minutes, date3.seconds, ret);
  msg_win(msg, 0, MSG_LINE, 0);
  sprintf(msg, MSG_NOW_EST_LEFT_FINISH[global.language],
      date1.hour, date1.minutes, date1.seconds,
      date2.hour, date2.minutes, date2.seconds);
  msg_win(msg, 0, MSG_LINE, 2);
  msg_win("", 0, MSG_LINE, 3);
  msg_win("", 0, MSG_LINE, 4);
  msg_win("", 0, MSG_LINE, 5);
  ret = 0;

LABEL_ERR:
  // close
  sceIoWaitAsync(fp_in, &res);
  sceIoWaitAsync(fp_out, &res);

  // header
  res = sceIoLseek32(fp_out, CISO_HEADER_SIZE, PSP_SEEK_SET);
  ERR_RET_2(res, ERR_SEEK);
  res = sceIoWrite(fp_out, tag, tag_size * sizeof(unsigned int));
  ERR_RET_2(res, ERR_WRITE);

  sceIoClose(fp_in);
  sceIoClose(fp_out);

  if(ret != -1)
  {
    sceIoRemove(out_path);
    sceIoRename(ren_path, out_path);

    text[0] = MSG_FINISHED[global.language];
    text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
    text[2] = "\0";
    dialog(text);
  }
  else
    sceIoRemove(ren_path);

  set_clock(222, 111);
  return DONE;
}

int iso2cso(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char iso_path[MAX_PATH];
  char cso_path[MAX_PATH];
  char work[MAX_PATH];
  char work2[MAX_PATH];
  char work3[MAX_PATH];
  int ret;
  char *ptr;

  ret = check_ms();
  if(ret < 0)
    return CANCEL;

  if((global.umd_size / 1024) > global.ms_free_size)
  {
    msg_win(MSG_FREE_SPACE_IS_LESS_THAN_ISO_SIZE[global.language], 0, MSG_WAIT, 0);
    msg_win(MSG_THERE_IS_A_POSSIBILITY_OF_FAILING[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_CONTINUE[global.language], select_yes_no, 0, 28, 10);
    if(ret != YES)
      return CANCEL;
  }

  msg_win("", 0, MSG_CLEAR, 0);
  ret = select_menu_list(cso_trans_menu);
  if(ret == CANCEL)
    return CANCEL;

  strcpy(iso_path, dir);
  strcat(iso_path, file);

  strcpy(cso_path, dir);
  strcpy(work3, file);
  ptr = strrchr(work3, '.');
  *ptr = '\0';
  sprintf(work, "_%d_%03d", global.cso_level, global.cso_threshold);
  strcat(work3, work);

  sjis_to_utf8(work2, work3);

  ret = osk(work, work2, MSG_FILE_NAME[global.language], 0);
  if(ret == PSP_UTILITY_OSK_RESULT_CHANGED)
    strcat(cso_path, work);
  else
    strcat(cso_path, work3);

  strcat(cso_path, ".CSO");

  set_clock(333, 166);
  file_trans(cso_path, iso_path, TRANS_ISO_CSO, 0, 0, opt_1, opt_2, dir);
  set_clock(222, 111);
  return DONE;
}

int cso2iso(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char iso_path[MAX_PATH];
  char cso_path[MAX_PATH];
  char work[MAX_PATH];
  char work2[MAX_PATH];
  char work3[MAX_PATH];
  int ret;
  char *ptr;

  ret = check_ms();
  if(ret < 0)
    return CANCEL;

  if((global.umd_size / 1024) > global.ms_free_size)
  {
    err_msg(ERR_SIZE_OVER);
    return CANCEL;
  }

  strcpy(cso_path, dir);
  strcat(cso_path, file);

  strcpy(iso_path, dir);
  strcpy(work3, file);
  ptr = strrchr(work3, '.');
  *ptr = '\0';
  sjis_to_utf8(work2, work3);

  ret = osk(work, work2, MSG_FILE_NAME[global.language], 0);
  if(ret == PSP_UTILITY_OSK_RESULT_CHANGED)
    strcat(iso_path, work);
  else
    strcat(iso_path, work3);
  strcat(iso_path, ".ISO");

  set_clock(333, 166);
  file_trans(iso_path, cso_path, TRANS_CSO_ISO, 0, 0, opt_1, opt_2, dir);
  set_clock(222, 111);
  return DONE;
}

int file_trans(char* out_path, char *in_path, trans_type type, int level, int limit, int opt_1, int opt_2, char *dir)
{
  char *read_buf[2];        // 読込バッファ ポインタ
  char *write_buf[2];       // 書込バッファ ポインタ

  SceUID fp_in = 0;         // 読込ファイル ポインタ
  SceUID fp_out = 0;        // 書込ファイル ポインタ
  SceIoStat stat;           // ファイルステータス
  SceInt64 res = -1;        // 非同期ファイルIO用

  CISO_H header;            // CSOヘッダ
  int *tag = NULL;          // CSOセクタタグ ポインタ
  int tag_size = 0;         // CSOセクタタグ サイズ
  char cso_buf[0x1000];     // CSO変換バッファ
  int write_ptr = 0;        // 書込バッファへの転送位置
  int cso_sec_ptr = 0;          // CSOセクタ位置
  int cso_sec_size;         // 圧縮後のセクタサイズ
  int out_cso_flag = 0;

  int in_sec_num = 0;       // 読込セクタ数

  int max_read_size = 0;    // 一回の読込サイズ
  int read_size = 0;        // 読込んだサイズ
  int write_size = 0;       // 一回の書込サイズ
  int now_sector = 0;       // 処理したセクタ数
  int write_block_shift = 0;    // 処理単位(シフト数)
  int read_block_shift = 0;    // 処理単位(シフト数)

  int num;                  // 汎用
  int loop;                 // ループ用

  char *text[15];           // テキスト表示用
  char msg[256];            // テキスト表示用
  char work[256];            // テキスト表示用
  char ren_path[MAX_PATH];

  int ret;
  SceCtrlData  data;
  u64 start_tick;
  u64 now_tick;
  u64 old_tick = 0;
  pspTime date1;
  pspTime date2;
  pspTime date3;
  int first_wait = 0;

  strcpy(ren_path, dir);
  strcat(ren_path, "01234565789012345678901234567890123456789012345678901234567890123456789");

  msg_win("", 0, MSG_CLEAR, 0);
  sjis_to_utf8(work, strrchr(out_path, '/') + 1);
  sprintf(msg, MSG_OUTPUT_NAME[global.language], work);
  msg_win(msg, 0, MSG_WAIT, 0);

  if(check_file(out_path) < 0)
  {
    msg_win(MSG_START_TEXT[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_START[global.language], select_yes_no, 0, 28, 10);
  }
  else
  {
    msg_win(MSG_THERE_ARE_FILES_WITH_THE_SAME_NAME[global.language], 0, MSG_WAIT, 0);
    msg_win(MSG_OVERWRITE_[global.language], 0, MSG_WAIT, 0);
    ret = select_menu(MSG_OVERWRITE[global.language], select_no_yes, 0, 28, 10);
  }

  if(ret != YES)
    return CANCEL;

  // buff 設定
  switch(type)
  {
    case TRANS_UMD_ISO:
      strcpy(msg, MSG_UMD_ISO_CONVERT[global.language]);

      // buff 設定
      read_buf[0] = &WORK[O_BUFFER][0];
      read_buf[1] = &WORK[I_BUFFER][0];
      write_buf[0] = &WORK[O_BUFFER][0];
      write_buf[1] = &WORK[I_BUFFER][0];

      // tag設定
      out_cso_flag = 0;

      // block設定
      max_read_size = 512;
      write_block_shift = 11; /* SECTOR_SIZE */
      read_block_shift = 0;

      in_sec_num = get_umd_sector("umd:", TYPE_UMD);
      tag_size = 0;

      // open設定
      fp_in  = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
      ERR_RET_2(fp_in, ERR_OPEN);
      fp_out = sceIoOpen(ren_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
      ERR_RET_2(fp_out, ERR_OPEN);

      break;

    case TRANS_UMD_CSO:
      sprintf(msg, MSG_UMD_CSO_CONVERT[global.language], global.cso_level, global.cso_threshold);

      // buff 設定
      read_buf[0] = &WORK[O_BUFFER][0];
      read_buf[1] = &WORK[O_BUFFER + 2048][0];
      write_buf[0] = &WORK[O_BUFFER + 4096][0];
      write_buf[1] = &WORK[O_BUFFER + 6144][0];

      // tag設定
      tag = (int *)&WORK[O_BUFFER + 8192][0];
      out_cso_flag = 1;

      // block設定
      max_read_size = 512;
      write_block_shift = 11;
      read_block_shift = 0;

      in_sec_num = get_umd_sector("umd:", TYPE_UMD);
      tag_size = in_sec_num + 1;

      // open設定
      fp_in  = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
      ERR_RET_2(fp_in, ERR_OPEN);
      fp_out = sceIoOpen(ren_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
      ERR_RET_2(fp_out, ERR_OPEN);

      break;

    case TRANS_ISO_CSO:
      sprintf(msg, MSG_ISO_CSO_CONVERT[global.language], global.cso_level, global.cso_threshold);

      // buff 設定
      read_buf[0] = &WORK[O_BUFFER][0];
      read_buf[1] = &WORK[O_BUFFER + 2048][0];
      write_buf[0] = &WORK[O_BUFFER + 4096][0];
      write_buf[1] = &WORK[O_BUFFER + 6144][0];

      // tag設定
      tag = (int *)&WORK[O_BUFFER + 8192][0]; // 最大4MB
      out_cso_flag = 1;

      // block設定
      max_read_size = 2048 * SECTOR_SIZE;
      write_block_shift = 0;
      read_block_shift = 11;

      ret = sceIoGetstat(in_path, &stat);
      ERR_RET_2(ret, ERR_READ);
      in_sec_num = (stat.st_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
      tag_size = in_sec_num + 1;

      // open設定
      fp_in  = sceIoOpen(in_path, PSP_O_RDONLY, 0777);
      ERR_RET_2(fp_in, ERR_OPEN);
      fp_out = sceIoOpen(ren_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
      ERR_RET_2(fp_out, ERR_OPEN);

      break;

    case TRANS_CSO_ISO:
      strcpy(msg, MSG_CSO_ISO_CONVERT[global.language]);

      // buff 設定
      read_buf[0] = &WORK[I_BUFFER][0];
      read_buf[1] = &WORK[O_BUFFER][0];
      write_buf[0] = &WORK[I_BUFFER][0];
      write_buf[1] = &WORK[O_BUFFER][0];

      // tag設定
      out_cso_flag = 0;

      // block設定
      max_read_size = 2048 * SECTOR_SIZE;
      write_block_shift = 0;
      read_block_shift = 11;

      tag_size = 0;

      // open設定
      fp_in  = sceIoOpen(in_path, PSP_O_RDONLY, 0777);
      ERR_RET_2(fp_in, ERR_OPEN);
      fp_out = sceIoOpen(ren_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
      ERR_RET_2(fp_out, ERR_OPEN);

      ret = sceIoRead(fp_in, &header, CISO_HEADER_SIZE);
      ERR_RET_2(ret, ERR_READ);
      in_sec_num = header.total_bytes / SECTOR_SIZE;

      break;

    default:
      // buff 設定
      // tag設定
      // block設定
      // open設定
      return DONE;
      break;
  }

  // msg
  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(msg, 1, MSG_WAIT, 1);

  if(out_cso_flag == 1)
  {
    // header
    header.magic[0] = 'C';
    header.magic[1] = 'I';
    header.magic[2] = 'S';
    header.magic[3] = 'O';
    header.header_size = 0x00; /* 本来 0x18 であるが、PSP Filer 6.6で読み込めなくなるので一時的に修正*/
    header.total_bytes = in_sec_num * SECTOR_SIZE;
    header.block_size = SECTOR_SIZE;
    header.ver = 0x01;
    header.align = 0x00;
    header.rsv_06[0] = 0x00;
    header.rsv_06[1] = 0x00;

    ret = sceIoWrite(fp_out, &header, CISO_HEADER_SIZE);
    ERR_RET_2(ret, ERR_WRITE);
    cso_sec_ptr = CISO_HEADER_SIZE + (tag_size * 4);
    tag[0] = cso_sec_ptr;
    ret = sceIoLseek32(fp_out, cso_sec_ptr, PSP_SEEK_SET);
    ERR_RET_2(ret, ERR_SEEK);
  }


  // loop前
  now_sector = 0;
  num = 0;

  sceRtcGetCurrentTick(&start_tick);

  // msg
  msg_win("", 0, MSG_WAIT, 0);
  sprintf(msg, MSG_SECTOR_COMPLETE[global.language], now_sector, in_sec_num);
  msg_win(msg, 0, MSG_LINE, 1);

  if(type == TRANS_CSO_ISO)
    read_size = cso_read_fp(read_buf[0], fp_in, now_sector << read_block_shift, max_read_size);
  else
  {
    read_size = sceIoRead(fp_in, read_buf[0], max_read_size);
    ERR_RET_2(read_size, ERR_READ);
  }

  // loop
  while(read_size > 0)
  {
    if(out_cso_flag == 1)
    {
      msg_win("DEFLATE", 0, MSG_LINE, 4);
      loop = 0;
      write_size = 0;
      write_ptr = 0;
      while(read_size > 0)
      {
        cso_sec_size = deflate_cso(cso_buf, sizeof(cso_buf), (read_buf[num] + (loop * SECTOR_SIZE)), SECTOR_SIZE, global.cso_level);
        if(cso_sec_size < 0)
        {
          text[0] = MSG_CSO_CONVERSION_ERROR[global.language];
          text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
          text[2] = "\0";
          dialog(text);
          ret = -1;
          goto LABEL_ERR;
        }
        if(cso_sec_size < (SECTOR_SIZE * global.cso_threshold / 100))
        {
          write_size += cso_sec_size;
          cso_sec_ptr += cso_sec_size;
          tag[now_sector + 1] = cso_sec_ptr;
          memcpy(write_buf[num] + write_ptr, cso_buf, cso_sec_size);
        }
        else
        {
          cso_sec_size = SECTOR_SIZE;
          write_size += SECTOR_SIZE;
          cso_sec_ptr += SECTOR_SIZE;
          tag[now_sector] |= 0x80000000;
          tag[now_sector + 1] = cso_sec_ptr;
          memcpy(write_buf[num] + write_ptr, read_buf[num] + loop * SECTOR_SIZE, SECTOR_SIZE);
        }
        write_ptr += cso_sec_size;
        read_size -= SECTOR_SIZE >> write_block_shift;
        loop++;
        now_sector++;
      }
    }
    else
    {
      now_sector += max_read_size >> read_block_shift;
      write_size = read_size << write_block_shift;
    }

    msg_win("Wait WRITE", 0, MSG_LINE, 4);
    ret = sceIoWaitAsync(fp_out, &res);
    if(ret < 0)
    {
      if(first_wait == 1)
      {
        err_msg(ERR_WRITE);
        return CANCEL;
      }
      first_wait = 1;
    }

    if(type != TRANS_CSO_ISO)
    {
      msg_win("READ", 0, MSG_LINE, 4);
      ret = sceIoReadAsync(fp_in, read_buf[(num + 1) % 2], max_read_size);
      if(ret < 0)
      {
        err_msg(ERR_READ);
        return CANCEL;
      }
    }

    msg_win("WRITE", 0, MSG_LINE, 4);
    ret = sceIoWriteAsync(fp_out, write_buf[num], write_size);
    ERR_RET_2(ret, ERR_WRITE);

    num = (num + 1) % 2;
    sprintf(msg, MSG_SECTOR_COMPLETE[global.language], now_sector, in_sec_num);
    msg_win(msg, 0, MSG_LINE, 1);
    sceRtcGetCurrentTick(&now_tick);
    now_tick -= start_tick;
    ret = (max_read_size << write_block_shift) / 1024 * 1000 / ((now_tick - old_tick) / 1000);
    old_tick = now_tick;
    sceRtcSetTick(&date1, &now_tick);
    now_tick = now_tick * in_sec_num / now_sector;
    sceRtcSetTick(&date2, &now_tick);
    now_tick -= old_tick;
    sceRtcSetTick(&date3, &now_tick);
    sprintf(msg, MSG_NOW_EST_LEFT[global.language],
        date1.hour, date1.minutes, date1.seconds,
        date2.hour, date2.minutes, date2.seconds,
        date3.hour, date3.minutes, date3.seconds);
    msg_win(msg, 0, MSG_LINE, 2);
    sprintf(msg, MSG_SPEED[global.language], ret);
    msg_win(msg, 0, MSG_LINE, 3);

    get_button(&data);
    if((data.Buttons) == PSP_CTRL_CROSS)
    {
      msg_win(MSG_STOP[global.language], 0, MSG_LINE, 2);
      ret = select_menu(MSG_STOP_TEXT[global.language], select_no_yes, 0, 28, 10);
      if(ret == 1)
      {
        text[0] = MSG_CONVERSION_STOP[global.language];
        text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
        text[2] = "\0";
        dialog(text);
        ret = -1;
        goto LABEL_ERR;
      }
      dir_menu(NULL, YES);
      msg_win("", 0, MSG_REDROW, 0);
    }

    msg_win("Wait READ", 0, MSG_LINE, 4);
    if(type == TRANS_CSO_ISO)
      res = cso_read_fp(read_buf[num], fp_in, now_sector << read_block_shift, max_read_size);
    else
    {
      ret = sceIoWaitAsync(fp_in, &res);
      ERR_RET_2(ret, ERR_READ);
      msg_win("READ", 0, MSG_LINE, 4);
    }
    read_size = res;
  }

  // msg
  sprintf(msg, MSG_SECTOR_COMPLETE[global.language], in_sec_num, in_sec_num);
  msg_win(msg, 0, MSG_LINE, 1);

  sceRtcGetCurrentTick(&now_tick);
  now_tick -= start_tick;
  sceRtcSetTick(&date3, &now_tick);
  ret = (in_sec_num * SECTOR_SIZE) / 1024 * 1000 / (now_tick / 1000);
  sprintf(msg, MSG_TOTAL_TIME_AVERAGE_SPEED[global.language], date3.hour, date3.minutes, date3.seconds, ret);
  msg_win(msg, 0, MSG_LINE, 0);
  sprintf(msg, MSG_NOW_EST_LEFT_FINISH[global.language],
      date1.hour, date1.minutes, date1.seconds,
      date2.hour, date2.minutes, date2.seconds);
  msg_win(msg, 0, MSG_LINE, 2);

  // header
  if(out_cso_flag == 1)
  {
    ret = sceIoWaitAsync(fp_out, &res);
    ERR_RET_2(ret, ERR_READ);
    ret = sceIoLseek32(fp_out, CISO_HEADER_SIZE, PSP_SEEK_SET);
    ERR_RET_2(ret, ERR_SEEK);
    ret = sceIoWriteAsync(fp_out, tag, tag_size * sizeof(unsigned int));
    ERR_RET_2(ret, ERR_WRITE);
  }

LABEL_ERR:
  // close
  sceIoWaitAsync(fp_in, &res);
  sceIoWaitAsync(fp_out, &res);
  sceIoClose(fp_in);
  sceIoClose(fp_out);

  if(ret != -1)
  {
    sceIoRename(ren_path, out_path);

    text[0] = MSG_FINISHED[global.language];
    text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
    text[2] = "\0";
    dialog(text);
  }
  else
  {
    sceIoRemove(ren_path);
  }

  return DONE;
}

// TODO 別関数にまとめる
/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int file_update(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char *msg[3][2] = {
      { "UMD_ID.csvを更新しますか?", "Update UMD_ID.csv?"},
      { "iso_toolを更新しますか?", "Update iso_tool?" },
      { "pspdecrypt.prxを更新しますか?",  "Update pspdecrypt.prx?" }
  };
  char *url[3] = { "http://isotool.tfact.net/UMD_ID.csv", "http://isotool.tfact.net/EBOOT.PBP", "http://isotool.tfact.net/pspdecrypt.prx" };
  char old_path[MAX_PATH];
  char new_path[MAX_PATH];
  time_t old_time;
  time_t new_time;
  int tz;
  SceIoStat old_stat;
  int ret;
  char *text[15];           // テキスト表示用
  int mod = 0;

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(msg[opt_2][global.language], 1, MSG_WAIT, 1);
  ret = select_menu(MSG_UPDATE[global.language], select_no_yes, 0, 25, 8);

  if(ret < 0)
    return -1;

  if(ret == 1)
  {
    // 接続
    msg_win(MSG_CONNECTION[global.language], 0, MSG_WAIT, 0);
    net_connect();

    msg_win(MSG_UPDATE_CHECK[global.language], 0, MSG_WAIT, 0);
    // パス設定
    strcpy(old_path, main_path);
    strcat(old_path, "/");
    strcat(old_path, (char *)opt_1);

    strcpy(new_path, old_path);
    strcat(new_path, ".new");

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_TIMEZONE, &tz);

    // 旧ファイルの時刻取得
    ret = sceIoGetstat(old_path, &old_stat);
    if(ret < 0)
      old_time = 0;
    else
    {
      sceRtcGetTime_t((pspTime*)&old_stat.st_mtime, &old_time);
      old_time -= tz * 60; // local -> UTC
    }
    // 新ファイルの時刻取得
    ret = web_get_file_time(&new_time, url[opt_2]);
    // 取得エラー、更新なしの場合はメッセージを表示して終了
    if(ret < 0)
      msg_win(MSG_GET_UPDATE_TIME_ERROR[global.language], 1, MSG_WAIT, 0);
    else if(new_time < (old_time + 60)) // 誤差を考えて1分以内の変更は変更なしとする
      msg_win(MSG_NOT_UPDATE[global.language], 1, MSG_WAIT, 0);
    else
    {
      msg_win(MSG_FOUND_UPDATE_DO_YOU_WANT_TO_UPDATE[global.language], 1, MSG_WAIT, 1);
      ret = select_menu(MSG_CONFIRM[global.language], select_no_yes, 0, 25, 8);
      if(ret == 1)
      {
        msg_win(MSG_GET_FILE[global.language], 0, MSG_WAIT, 0);
        // 新ファイル取得
        ret = web_get_file(new_path, url[opt_2]);
        if(ret < 0)
          msg_win(MSG_GET_FILE_ERROR[global.language], 1, MSG_WAIT, 0);
        else
        {
          msg_win(MSG_FILE_UPDATE[global.language], 0, MSG_WAIT, 0);
          // エラーが無ければ、旧ファイル削除、新ファイルリネーム
          ret = sceIoRemove(old_path);
          if(ret < 0)
            msg_win(MSG_OLD_FILE_DELETE_ERROR[global.language], 1, MSG_WAIT, 0);
          ret = sceIoRename(new_path, old_path);
          if(ret < 0)
            msg_win(MSG_RENAME_ERROR[global.language], 1, MSG_WAIT, 0);
          else
            mod = 1;
        }
      }
    }
    // 切断
    msg_win(MSG_DISCONNECT[global.language], 0, MSG_WAIT, 0);
    net_disconnect();

    // 状況に応じて再起動/UMD_ID再読込
    if(mod == 1)
    {
      switch(opt_2)
      {
        case 0:
          msg_win(MSG_RELOAD_DATA[global.language], 0, MSG_WAIT, 0);
          get_umd_name(NULL, NULL, NULL, 1);
          text[0] = MSG_FINISHED[global.language];
          text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
          text[2] = "\0";
          dialog(text);
          break;

        case 1:
        case 2:
          text[0] = MSG_FINISHED[global.language];
          text[1] = MSG_PUSH_BUTTON[global.language][global.enter_button];
          text[2] = MSG_RESTART[global.language];
          text[3] = "\0";
          dialog(text);
          reboot(0);
          break;
      }
    }
    else
    {
      text[0] = MSG_PUSH_BUTTON[global.language][global.enter_button];
      text[1] = "\0";
      dialog(text);
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int soft_reboot(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char *msg[2][2] = {
      { "再起動しますか?", "Reboot?" },
      { "終了しますか?", "Exit?" }
  };
  int ret;

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(msg[opt_1][global.language], 1, MSG_WAIT, 1);
  ret = select_menu(MSG_CONFIRM[global.language], select_no_yes, 0, 25, 8);
  if(ret == -1)
    return CANCEL;

  if(ret == 1)
    reboot(opt_1);

  return DONE;
}

int remove_file(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char path[MAX_PATH];
  int ret;

  strcpy(path, dir);
  strcat(path, file);

  msg_win("", 0, MSG_CLEAR, 0);
  msg_win(MSG_DELETE_[global.language], 0, MSG_WAIT, 0);
  ret = select_menu(MSG_DELETE[global.language], select_no_yes, 0, 28, 10);
  if(ret == CANCEL)
    return CANCEL;
  if(ret == YES)
    ret = sceIoRemove(path);
  return DONE;
}

//int fix_header(char *dir, char *file, file_type type, int opt_1, int opt_2)
//{
//  char path[MAX_PATH];
//  int ret;
//  char header_size[1];
//
//  header_size[0] = 0x00;
//
//  strcpy(path, dir);
//  strcat(path, file);
//
//  msg_win("", 0, MSG_CLEAR, 0);
//  msg_win("ヘッダー調整しますか？", 0, MSG_WAIT, 0);
//  ret = select_menu("調整", select_no_yes, 0, 28, 10);
//  if(ret == CANCEL)
//    return CANCEL;
//  if(ret == YES)
//    ret = ms_write(header_size, path, 0x04, 1);
//  return DONE;
//}

int boot_iso(char *dir, char *file, file_type type, int opt_1, int opt_2)
{
  char path[MAX_PATH];
  SEConfig config;
  int ret;

  strcpy(path, dir);
  strcat(path, file);

  sctrlSEUmountUmd();
  sctrlSESetDiscOut(1);
  sctrlSEGetConfig(&config);
  switch(config.umdmode)
  {
    // Normal
    case 0:
      sctrlSEMountUmdFromFile(path, 0, config.useisofsonumdinserted);
      break;
    // OE isofs
    case 1:
      sctrlSEMountUmdFromFile(path, 1, 1);
      break;
    // M33 Driver
    case 2:
      sctrlSEMountUmdFromFile(path, 1, 1);
      break;
    // Sony NP9660
    case 3:
      sctrlSEMountUmdFromFile(path, 1, 1);
      break;
  }

//  sctrlSESetUmdFile(path);

//  sceKernelDelayThread(5000000);
//  sceUmdActivate(1, "disc0:");
//  sceKernelDelayThread(5000000);
//  sceUmdWaitDriveStat(PSP_UMD_READY);
//  ModuleMgrForKernel_1B91F6EC("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", 0, NULL);
//  sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", NULL);
//  sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", NULL);
//  sctrlKernelLoadExecVSHWithApitype();
  ret = sceKernelLoadModule("loadprx.prx", 0, NULL);
  ret = sceKernelStartModule(ret, strlen(path)+1, path, NULL, NULL);
  return DONE;
}

#if 0
static int lua_runiso(lua_State *L)
{
    char* isopath = luaL_checkstring(L, 1);
    struct SceKernelLoadExecVSHParam param;
    SEConfig config;
    int apitype = 0x120;
    char *program = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
    char *mode = "game";
    SetUmdFile(isopath);
    sctrlSEGetConfigEx(&config, sizeof(config));
    if (config.umdmode == MODE_MARCH33)
    {
        SetConfFile(1);
    }
    else if (config.umdmode == MODE_NP9660)
    {
        SetConfFile(2);
    }
    else
    {
        SetConfFile(0);
    }
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(program)+1;
    param.argp = program;
    param.key = mode;
    sctrlKernelLoadExecVSHWithApitype(apitype, program, &param);
    return 0;
}

#endif
