/*
	eboot_exchange ver 2.7
 */

#include <pspkernel.h>
#include <kubridge.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <systemctrl_se.h>
#include <psppower.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <psprtc.h>

#include "pspdecrypt.h"
#include "ciso.h"
#include "umd.h"
#include "key.h"

#include "fbm_print.h"
#include "shnm8x16r.c"
#include "shnmk16.c"



PSP_MODULE_INFO( "iso_tool", 0, 1, 1 );
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define MID_STR(x, str, num) (x - (fbm_getwidth(str, num) / 2))

#define CLEAR_LINE "                                    "
#define MENU_TITLE " iso_tool cso_test "
#define MENU_MSG_1 " U/D/R/L ... SELECT ISO / ○ ... RIP / × ... EXIT "
#define MENU_MSG_2 " △ ... EXCHANGE / □ ... REPAIR "
#define MENU_MSG_3 " SELECT + △/□ ... TARGET ALL ISO FILE "

#define MSG_LINE 150

#define READ_EBOOT_BIN        " READ EBOOT.BIN "
#define NOT_CRYPT_EBOOT_BIN   " NOT CRYPT EBOOT.BIN "
#define DECRYPT_ERROR         " DECRYPT EBOOT.BIN ERROR "
#define WRITE_EBOOT_BIN       " WRITE EBOOT.BIN "
#define EXCHANGE_DONE         " EXCHANGE DONE "
//
#define READ_BACKUP         " READ BACKUP FILE "
#define WRITE_BACKUP        " WRITE BACKUP FILE "
#define REPAIR_EBOOT_BIN    " REPAIR EBOOT.BIN "
#define REPAIR_DONE         " REPAIR DONE "
#define NO_BACKUP           " NO BACKUP "
#define NOT_BACKUP_FILE     " NOT BACKUP FILE "

#define ALL_ECHANGE         " ALL EXCHANGE "
#define ALL_REPAIR          " ALL REPAIR "
#define ALL_ECHANGE_DONE    " ALL EXCHANGE DONE "
#define ALL_REPAIR_DONE     " ALL REPAIR DONE "

#define EBOOT_BIN_SIZE_OVER " EBOOT.BIN SIZE OVER "

#define MENU_XY(x, y, str, num) fbm_printXY(MID_STR(x, str, num), y, str, 0xFFFFFFFF, 0, FBM_FONT_FILL | FBM_BACK_FILL, 100, num, num);

#define ISO_PATH        "ms0:/iso/"
#define EBOOT_NAME      "EBOOT.BIN"
#define EBOOT_NAME_LEN  9

int exit_callback(int arg1, int arg2, void *common);
int CallbackThread(SceSize args, void *argp);
int SetupCallbacks(void);
int LoadStartModule(char *module, int partition);

int main(void);

int read_dir(char *dir_name);
void set_clock(int cpu, int bus);
void wait_button_up(void);
void wait_button_down(void);
void print_menu(char *iso_name, int backup_mode, int patch_mode);

int get_eboot(char* path, int* pos, int* size, int* size_pos, int* mode);
void eboot_exchange(char* path, int backup_mode, int patch_mode);
void eboot_repair(char* path);

void chg_file_mode(char* path);

int umd(void);
int umd_cso(void);
int umd_test(void);

#define MAX_SECTOR_NUM (4096)
#define EBOOT_MAX_SIZE (UMD_SEC_SIZE * MAX_SECTOR_NUM) // 8MB

static char I_BUFFER[MAX_SECTOR_NUM][UMD_SEC_SIZE] __attribute__((aligned(0x40)));
static char O_BUFFER[MAX_SECTOR_NUM][UMD_SEC_SIZE] __attribute__((aligned(0x40)));
static int  B_FLAG[MAX_SECTOR_NUM];
static int  INDEX[1024 * 1024];

static unsigned int __attribute__((aligned(16))) list[262144];

#define BUF_WIDTH   (512)
#define SCR_WIDTH   (480)
#define SCR_HEIGHT  (272)
#define PIXEL_SIZE  (4)
#define FRAME_SIZE  (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE   (BUF_WIDTH SCR_HEIGHT * 2)

// HOMEキー用の初期設定
int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void *argp)
{
    int cbid;

    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}

int SetupCallbacks(void)
{
    int thid = 0;

    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

int LoadStartModule(char *module, int partition)
{
    SceUID mod = kuKernelLoadModule(module, 0, NULL);

    if (mod < 0)
        return mod;

    return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

char iso_name[256][256];
int dir_flag[256];

int read_dir(char *dir_name)
{
  DIR *dp;
  struct dirent *entry;
  int num;
  int file_num = 0;

  // ディレクトリデータの取得
  dp = opendir(dir_name);

  // ISOフォルダがない場合
  if(dp == NULL)
    return -1;

  // ファイルデータの取得
  while((entry = readdir(dp)) != NULL){
    num = strlen(entry->d_name);

    switch(entry->d_stat.st_mode & FIO_S_IFMT)
    {

      // ISOファイルの場合
      case FIO_S_IFREG:
        if(strncasecmp(&entry->d_name[num - 4], ".iso", 4) == 0)
        {
          strcpy(&iso_name[file_num][0], entry->d_name);
          dir_flag[file_num] = 0;
          file_num++;
        }
        if(strncasecmp(&entry->d_name[num - 4], ".cso", 4) == 0)
        {
          strcpy(&iso_name[file_num][0], entry->d_name);
          dir_flag[file_num] = 0;
          file_num++;
        }

        break;

      // ディレクトリの場合
      case FIO_S_IFDIR:
        if((strcmp(&entry->d_name[0], ".") != 0) && (strcmp(&entry->d_name[0], "..") != 0))
        {
          strcpy(&iso_name[file_num][0], entry->d_name);
          dir_flag[file_num] = 1;
          file_num++;
        }
        break;

    }
  }

  closedir(dp);

  return file_num;
}

// メイン
int main(void)
{
  char now_path[1024] = ISO_PATH;
  char path[1024];
  int dir_level = 1;

  int exit_flag = 0;

  int file_num = 0;
  int select_num = 0;

  int loop;

  int backup_mode;
  int patch_mode;
  int mode;

  static SceCtrlData  data;

  // HOMEボタンを使用可能にする
  SetupCallbacks();

//  SceUID mod = LoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
//  if (mod < 0)
//  {
//    MENU_XY(240, 110, (s8*)" NOT LOAD PSPDECRYPT.PRX ", 1);
//    MENU_XY(240, MSG_LINE, (s8*)" PUSH ANY BUTTON ", 1);
//
//    wait_button_down();
//    wait_button_up();
//
//    sceKernelExitGame();
//  }

  backup_mode = 0;
  patch_mode = 0;
  mode = 0;

  fbm_init(shnm8x16r, 0, shnmk16, 0);


//  sceGuInit();
//  sceGuStart(GU_DIRECT,list);
//  sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
//  sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
//  sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
//  sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
//  sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
//  sceGuDepthRange(0xc350,0x2710);
//  sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
//  sceGuEnable(GU_SCISSOR_TEST);
//  sceGuDepthFunc(GU_GEQUAL);
//  sceGuEnable(GU_DEPTH_TEST);
//  sceGuFrontFace(GU_CW);
//  sceGuShadeModel(GU_FLAT);
//  sceGuEnable(GU_CULL_FACE);
//  sceGuEnable(GU_TEXTURE_2D);
//  sceGuEnable(GU_CLIP_PLANES);
//  sceGuFinish();
//  sceGuSync(0,0);
//  sceDisplayWaitVblankStart();
//  sceGuDisplay(GU_TRUE);


  pspDebugScreenInit();
  print_menu("", backup_mode, patch_mode);

//  cso_read(o_buff, 0x100, 1024 * 1024, "ms0:/ISO/igo.cso");
//
//  FILE* fp;
//  fp = fopen("ms0:/PSP/GAME/ebt_exchange/igo.tst", "wb");
//  fwrite(o_buff, 1024*1024, 1, fp);
//  fclose(fp);
//
//  cso_write(o_buff, 0x100, 1024 * 1024, 9, "ms0:/PSP/GAME/ebt_exchange/igo.cso");
//
//  cso_read(o_buff, 0x100, 1024 * 1024, "ms0:/PSP/GAME/ebt_exchange/igo.cso");
//
//  fp = fopen("ms0:/PSP/GAME/ebt_exchange/igo_ext.tst", "wb");
//  fwrite(o_buff, 1024*1024, 1, fp);
//  fclose(fp);
//
//
//  sceKernelExitGame();

  // ディレクトリデータの取得
  file_num = read_dir(now_path);

  // ISOフォルダがない場合
  if(file_num < 1)
  {
    MENU_XY(240, 110, (s8*)" NO ISO FOLDER or ISO/CSO DATA ", 1);
    MENU_XY(240, MSG_LINE, (s8*)" PUSH ANY BUTTON ", 1);

    wait_button_down();
    wait_button_up();

    sceKernelExitGame();
  }

  print_menu(&iso_name[0][0], backup_mode, patch_mode);
  MENU_XY(240, MSG_LINE, (s8*)now_path, 1);

  // ボタンから手が離れるまで待つ
  wait_button_up();

  // メニューのメインループ
  while(exit_flag == 0)
  {
    get_button_wait(&data);
    switch(data.Buttons)
    {
      case PSP_CTRL_START:
        wait_button_up();
        mode = (mode + 1) % 4;
        patch_mode = mode & 0x01;
        backup_mode = (mode >> 1) & 0x01;
        print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)now_path, 1);
        break;

      case PSP_CTRL_TRIANGLE:
        if(dir_flag[select_num] == 0)
        {
          wait_button_up();
          set_clock(333,166);
          sprintf(path, "%s/%s", now_path, &iso_name[select_num][0]);
          eboot_exchange(path, backup_mode, patch_mode);
          set_clock(222,111);
        }
        break;

      case (PSP_CTRL_TRIANGLE | PSP_CTRL_SELECT):
        set_clock(333,166);
        MENU_XY(240, 90, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, 90, (s8*)ALL_ECHANGE, 1);
        for(loop = 0; loop < file_num; loop++)
        {
          if(dir_flag[loop] == 0)
          {
            MENU_XY(240, 110, (s8*)CLEAR_LINE, 1);
            MENU_XY(240, 110, (s8*)&iso_name[loop][0], 1);
            sprintf(path, "%s/%s", now_path, &iso_name[loop][0]);
            eboot_exchange(path, backup_mode, patch_mode);
          }
        }
        print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
        MENU_XY(240, 90, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)ALL_ECHANGE_DONE, 1);
        set_clock(222,111);
        wait_button_up();
        break;

      case PSP_CTRL_SQUARE:
        if(dir_flag[select_num] == 0)
        {
          wait_button_up();
          set_clock(333,166);
          sprintf(path, "%s/%s", now_path, &iso_name[select_num][0]);
          eboot_repair(path);
          set_clock(222,111);
        }
        break;

      case (PSP_CTRL_SQUARE | PSP_CTRL_SELECT):
        set_clock(333,166);
        MENU_XY(240, 90, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, 90, (s8*)ALL_REPAIR, 1);
        for(loop = 0; loop < file_num; loop++)
        {
          if(dir_flag[loop] == 0)
          {
            MENU_XY(240, 110, (s8*)CLEAR_LINE, 1);
            MENU_XY(240, 110, (s8*)&iso_name[loop][0], 1);
            sprintf(path, "%s/%s", now_path, &iso_name[loop][0]);
            eboot_repair(path);
          }
        }
        print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
        MENU_XY(240, 90, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)ALL_REPAIR_DONE, 1);
        set_clock(222,111);
        wait_button_up();
        break;

      case PSP_CTRL_CIRCLE:
        wait_button_up();
        set_clock(333,166);
        umd_test();
        set_clock(222,111);
        break;

      case PSP_CTRL_CROSS:
        exit_flag = 1;
        wait_button_up();
        break;

      case PSP_CTRL_LTRIGGER:
      case PSP_CTRL_LEFT:
        wait_button_up();
        select_num--;
        if(select_num < 0) select_num = file_num - 1;
        print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)now_path, 1);
        break;

      case PSP_CTRL_RTRIGGER:
      case PSP_CTRL_RIGHT:
        wait_button_up();
        select_num++;
        if(select_num >= file_num) select_num = 0;
        print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)now_path, 1);
        break;

      case PSP_CTRL_UP:
        if(dir_level != 0)
        {
          wait_button_up();
          strcpy(path, now_path);

          loop = strlen(path) - 2;
          while(path[loop] != '/')
            loop--;
          path[loop + 1] = '\0';

          file_num = read_dir(path);
          if(file_num < 1)
          {
            file_num = read_dir(now_path);
          }
          else
          {
            select_num = 0;
            dir_level--;
            strcpy(now_path, path);
          }

          print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
          MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
          MENU_XY(240, MSG_LINE, (s8*)now_path, 1);
        }
        break;

      case PSP_CTRL_DOWN:
        if(dir_flag[select_num] == 1)
        {
          wait_button_up();
          sprintf(path, "%s%s/", now_path, &iso_name[select_num][0]);
          file_num = read_dir(path);
          if(file_num < 1)
          {
            file_num = read_dir(now_path);
          }
          else
          {
            dir_level++;
            select_num = 0;
            strcpy(now_path, path);
          }
          print_menu(&iso_name[select_num][0], backup_mode, patch_mode);
          MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
          MENU_XY(240, MSG_LINE, (s8*)now_path, 1);
        }
        break;

    }
  }

  fbm_freeall();
  sceKernelExitGame();
  return 0;
}

void set_clock(int cpu, int bus)
{
  scePowerSetClockFrequency(cpu, cpu, bus);
}


void print_menu(char *iso_name, int backup_mode, int patch_mode)
{
  MENU_XY(240, 10, (s8*)MENU_TITLE, 1);
  MENU_XY(240, 50, (s8*)CLEAR_LINE, 1);
  if(backup_mode == 0)
  {
    MENU_XY(240, 50, (s8*)" NO BACKUP MODE ", 1);
  }
  else
  {
    MENU_XY(240, 50, (s8*)" BACKUP MODE ", 1);
  }

  MENU_XY(240, 70, (s8*)CLEAR_LINE, 1);
  if(patch_mode == 0)
  {
    MENU_XY(240, 70, (s8*)" NO PATCH MODE ", 1);
  }
  else
  {
    MENU_XY(240, 70, (s8*)" PATCH MODE ", 1);
  }

  MENU_XY(240, 110, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, 110, (s8*)iso_name, 1);

  MENU_XY(240, 200, (s8*)MENU_MSG_1, 1);
  MENU_XY(240, 220, (s8*)MENU_MSG_2, 1);
  MENU_XY(240, 240, (s8*)MENU_MSG_3, 1);
  sceKernelDcacheWritebackAll();
}

int get_eboot(char* path, int* pos, int* size, int* size_pos, int* mode)
{
  FILE* fp;
  int ptr;
  int num;

  num = strlen(path);
  // 20～63セクタ(44セクタ分)を読み込む //Ys7のエラーの原因
  if(strncasecmp(&path[num - 4], ".iso", 4) == 0)
  {
    fp = fopen(path, "rb");
    fseek( fp, 20 * 0x800, SEEK_SET);
    fread(O_BUFFER, 0x800, 44, fp);
    fclose(fp);
    *mode = 0;
  }

  if(strncasecmp(&path[num - 4], ".cso", 4) == 0)
  {
    cso_read(&O_BUFFER[0][0], 20, 44 * 0x800, path);
    *mode = 1;
  }

  ptr = 0;

  do{
    while(O_BUFFER[0][ptr++] != 'E');
  }while(strncasecmp(&O_BUFFER[0][ptr], "BOOT.BIN", 8) != 0);

  ptr--;

  // ファイル名 - 0x1f にファイル先頭セクタ
  memcpy(pos, &O_BUFFER[0][ptr - 0x1f], 4);

  // ファイル名 - 0x17 にファイルサイズ
  memcpy(size, &O_BUFFER[0][ptr - 0x17], 4);

  // ファイルサイズの位置
  *size_pos = 20 * 0x800 + ptr - 0x17;

  return 0;
}

void eboot_exchange(char* path, int backup_mode, int patch_mode)
{
  int mode;
  int pos;
  int size_pos;
  int i_size;
  int o_size;

  FILE* fp;
  char ebt_path[1024];
  const unsigned char patch_1[] = { 0xE0, 0xFF, 0xBD, 0x27, 0x05, 0x05, 0x02, 0x3C, 0x0C, 0x00, 0xB3, 0xAF};
  const unsigned char patch_2[] = { 0x00, 0x40, 0x05, 0x34, 0x05, 0x05, 0x04, 0x3C, 0x99, 0x81, 0x05, 0x0C};

  // パッチ
  //    addiu  r29,r29,-$20                       ;0000010C[27BDFFE0,'...''] E0 FF BD 27
  // +  lui    r2,$0505                           ;00000110[3C020505,'...<'] 05 05 02 3C
  //    sw     r19,$c(r29)                        ;00000114[AFB3000C,'....'] 0C 00 B3 AF
  // アンデットナイツ
  //47 81 05 0C 00 40 05 34 05 05 04 3C 99 81 05 0C

  int num;

  // ISOからの読込み
  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)READ_EBOOT_BIN, 1);

  // EBOOT.BINの位置/サイズ/サイズデータの位置を取得
  get_eboot(path, &pos, &i_size, &size_pos, &mode);

  if(i_size > EBOOT_MAX_SIZE)
  {
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)EBOOT_BIN_SIZE_OVER, 1);
    return;
  }

  if(mode == 0)
  {
    fp = fopen(path, "rb");
    fseek( fp, pos * 0x800, SEEK_SET);
    fread(I_BUFFER, i_size, 1, fp);
    fclose(fp);
  }
  else
  {
    cso_read(&I_BUFFER[0][0], pos, i_size, path);
  }

  if(backup_mode != 0)
  {
    // 複合済みか判定
    if(strncmp(&I_BUFFER[0][0], "~PSP" , 4) == 0)
    {

      // EBTファイルの書込み
      MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
      MENU_XY(240, MSG_LINE, (s8*)WRITE_BACKUP, 1);

      strcpy(ebt_path, path);
      num = strlen(ebt_path);
      strcpy(&ebt_path[num - 4], ".EBT");
      fp = fopen(ebt_path, "wb");
      fwrite(I_BUFFER, i_size, 1, fp);
      fclose(fp);
    }
    else
    {
      // 複合済み
      MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
      MENU_XY(240, MSG_LINE, (s8*)NOT_CRYPT_EBOOT_BIN, 1);
    }
  }

  if(strncmp(&I_BUFFER[0][1], "ELF" , 3) == 0)
  {
    // 複合化ずみ
    o_size = i_size;
    memcpy(O_BUFFER, I_BUFFER, o_size);
  }
  else
  {
#if 0
    // 複合化
    o_size = pspDecryptPRX((u8 *)I_BUFFER, (u8 *)O_BUFFER, i_size);
    if((o_size <= 0) || (strncmp(&O_BUFFER[0][1], "ELF", 3) != 0))
    {
      MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
      MENU_XY(240, MSG_LINE, (s8*)DECRYPT_ERROR, 1);
      return;
    }
#else
    int addr = 0xD9160aF0;
    char msg[256];
    FILE *fp;

    do
    {
      addr = addr + 0x100;
      // prx 書換え
      fp = fopen("ms0:/PSP/GAME/TEST/pspdecrypt.prx", "r+b");
      fseek(fp, 0x39e0, SEEK_SET);
      fwrite(&addr, 4, 1, fp);
      fclose(fp);

      // prx 読込み
      SceUID mod = kuKernelLoadModule("ms0:/PSP/GAME/TEST/pspdecrypt.prx", 0, NULL);
      sceKernelStartModule(mod, 0, NULL, NULL, NULL);

      sprintf(msg, "DECRYPT %x", addr);
      MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
      MENU_XY(240, MSG_LINE, (s8*)msg, 1);

      // 復号化
      o_size = pspDecryptPRX((u8 *)I_BUFFER, (u8 *)O_BUFFER, i_size);

      // prx 解放
      sceKernelStopModule(mod, 0, NULL, NULL, NULL);
      sceKernelUnloadModule(mod);
    }while((o_size <= 0) || (strncmp(&O_BUFFER[0][1], "ELF", 3) != 0));


    sprintf(msg, "DECRYPT DONE %x", addr);
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)msg, 1);

    wait_button_down();
    wait_button_up();

#endif
  }

  // パッチ
  //    addiu  r29,r29,-$20                       ;0000010C[27BDFFE0,'...''] E0 FF BD 27
  // +  lui    r2,$0505                           ;00000110[3C020505,'...<'] 05 05 02 3C
  //    sw     r19,$c(r29)                        ;00000114[AFB3000C,'....'] 0C 00 B3 AF

  //47 81 05 0C 00 40 05 34 05 05 04 3C 99 81 05 0C

  if(patch_mode != 0)
  {
    num = 0;
    do{
      if(memcmp(&O_BUFFER[0][num], patch_1, 12) == 0)
        O_BUFFER[0][num + 4] = 0;
      if(memcmp(&O_BUFFER[0][num], patch_2, 12) == 0)
        O_BUFFER[0][num + 4] = 0;
      num++;
    }while(num < o_size);
  }

  // ISOファイルへの書込み
  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)WRITE_EBOOT_BIN, 1);

  chg_file_mode(path);
//  if(mode == 0)
//  {
//    fp = fopen(path, "r+b");
//    fseek( fp, pos * 0x800, SEEK_SET);
//    fwrite(O_BUFFER, o_size, 1, fp);
//    // ファイルサイズの変更
//    fseek( fp, size_pos, SEEK_SET);
//    fwrite(&o_size, 4, 1, fp);
//    fclose(fp);
//  }
//  else
//  {
//    cso_write(&O_BUFFER[0][0], pos, o_size, 9, path);
//
//    // ファイルサイズの変更
//    pos = size_pos / 0x800;
//    size_pos %= 0x800;
//    cso_read(&I_BUFFER[0][0], pos, 0x800, path);
//
//    memcpy(&I_BUFFER[size_pos], &o_size, 4);
//
//    cso_write(&I_BUFFER[0][0], pos, 0x800, 9, path);
//  }

  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)EXCHANGE_DONE, 1);
}

void eboot_repair(char* path)
{
  int mode;
  int pos;
  int size;
  int size_pos;
  get_eboot(path, &pos, &size, &size_pos, &mode);

  FILE* fp;
  char ebt_path[1024];

  int num;

  // BACKUPの読込み
  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)READ_BACKUP, 1);

  strcpy(ebt_path, path);
  num = strlen(ebt_path);
  strcpy(&ebt_path[num - 4], ".EBT");
  fp = fopen(ebt_path, "rb");
  if(fp == NULL)
  {
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)NO_BACKUP, 1);
    return;
  }
  fseek( fp, 0, SEEK_END );
  size = ftell( fp );
  fseek( fp, 0, SEEK_SET );
  fread(I_BUFFER, size, 1, fp);
  fclose(fp);

  // BACKUPファイルの判定
  if(strncmp(&I_BUFFER[0][0], "~PSP" , 4) == 0)
  {

    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)REPAIR_EBOOT_BIN, 1);

    // ISOファイルへの書込み
    if(mode == 0)
    {
      fp = fopen(path, "r+b");
      fseek( fp, pos * 0x800, SEEK_SET);
      fwrite(I_BUFFER, size, 1, fp);
      // ファイルサイズの変更
      fseek( fp, size_pos, SEEK_SET);
      fwrite(&size, 4, 1, fp);
      fclose(fp);
    }
    else
    {
      cso_write(&I_BUFFER[0][0], pos, size, 9, path);

      // ファイルサイズの変更
      pos = size_pos / 0x800;
      size_pos %= 0x800;
      cso_read(&I_BUFFER[0][0], pos, 0x800, path);

      memcpy(&I_BUFFER[size_pos], &size, 4);

      cso_write(&I_BUFFER[0][0], pos, 0x800, 9, path);
    }

    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)REPAIR_DONE, 1);
  }
  else
  {
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)NOT_BACKUP_FILE, 1);
  }

}

void chg_file_mode(char* path)
{
  SceIoStat stat;

  sceIoGetstat(path, &stat);
  stat.st_mode |= (FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH);
  sceIoChstat(path, &stat, (FIO_S_IRWXU | FIO_S_IRWXG | FIO_S_IRWXO));
}

int umd()
{
  SceUID umd;
  SceUID ms0;

  int num;
  char str[1024];
  int size;
  int total;
  char name[16];
  char path[1024];
  int loop;

  total = 0;
  num = 0;
  SceCtrlData  data;

  get_umd_id(name);
  sprintf(path, "ms0:/iso/%s.ISO", name);

  umd = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
  ms0 = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

  while(size = sceIoRead(umd, I_BUFFER, 1024/*4096*/), size > 0)
  {
    total += size;
    for(loop = 0; loop < size; loop++)
    {
      deflate_cso(&O_BUFFER[0][loop * 0x800], sizeof(O_BUFFER), &I_BUFFER[0][loop * 0x800], 0x800, 1);
    }
    sceIoWrite(ms0, I_BUFFER, 0x800 * size);
    sprintf(str, "%s WRITE %dMB", name, total / 512);
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)str, 1);
    num = (num + 1) % 2;
    get_button(&data);
    if((data.Buttons) == PSP_CTRL_CROSS)
    {
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)"STOP", 1);
        wait_button_down();
        wait_button_up();
        sceIoClose(umd);
        sceIoClose(ms0);
        return -1;
    }
  }
  sceIoClose(umd);
  sceIoClose(ms0);

  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)" DONE ", 1);

  return 0;
}


int umd_cso()
{
  SceUID umd;
  SceUID ms0;

  char *buff[2] = { &I_BUFFER[0][0], &O_BUFFER[0][0] };
  int num;
  char str[1024];
  int size;
  int total;
  SceInt64 res;
  char name[16];
  char path[1024];

  total = 0;
  num = 0;
  SceCtrlData  data;

  CISO_H ciso;

  get_umd_id(name);
  sprintf(path, "ms0:/iso/%s.CSO", name);

  umd = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
  ms0 = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);


  // init ciso header
  int index_size;
  unsigned int *index_buf = NULL;
  unsigned int *crc_buf = NULL;
  int ciso_total_block;
  int write_pos;

  memset(&ciso,0,sizeof(ciso));

  ciso.magic[0] = 'C';
  ciso.magic[1] = 'I';
  ciso.magic[2] = 'S';
  ciso.magic[3] = 'O';
  ciso.ver      = 0x01;

  ciso.block_size  = 0x800; /* ISO9660 one of sector */

  ciso.total_bytes = get_umd_size();
  ciso_total_block = ciso.total_bytes / ciso.block_size ;

  // allocate index block
  index_size = (ciso_total_block + 1 ) * sizeof(unsigned long);
  index_buf  = malloc(index_size);
  crc_buf    = malloc(index_size);

  memset(index_buf,0,index_size);

  // write header block
  sceIoWrite(ms0, &ciso, sizeof(ciso));

  // dummy write index block
  sceIoWrite(ms0, index_buf, sizeof(index_size));

  write_pos = sizeof(ciso) + index_size;

  while(size = sceIoRead(umd, buff[num], 4096), size > 0)
  {
    total += size;
    sceIoWaitAsync(ms0, &res);
    sceIoWriteAsync(ms0, buff[num], 0x800 * size);
    sprintf(str, "%s WRITE %dMB", name, total / 512);
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)str, 1);
    num = (num + 1) % 2;
    get_button(&data);
    if((data.Buttons) == PSP_CTRL_CROSS)
    {
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)"STOP", 1);
        wait_button_down();
        wait_button_up();
        sceIoWaitAsync(ms0, &res);
        sceIoClose(umd);
        sceIoClose(ms0);
        return -1;
    }
  }
  sceIoWaitAsync(ms0, &res);
  sceIoClose(umd);
  sceIoClose(ms0);

  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)" DONE ", 1);

  return 0;
}

#define NUM_INPUT_FIELDS    (3)
#define TEXT_LENGTH         (128)

int osk()
{
  static int done = 0;

  unsigned short intext[NUM_INPUT_FIELDS][TEXT_LENGTH];
  unsigned short outtext[NUM_INPUT_FIELDS][TEXT_LENGTH];
  unsigned short desc[NUM_INPUT_FIELDS][TEXT_LENGTH];

  memset(&intext, 0, NUM_INPUT_FIELDS * TEXT_LENGTH * sizeof(unsigned short));
  memset(&outtext, 0, NUM_INPUT_FIELDS * TEXT_LENGTH * sizeof(unsigned short));
  memset(&desc, 0, NUM_INPUT_FIELDS * TEXT_LENGTH * sizeof(unsigned short));

  int i;

  for(i = 0;i < NUM_INPUT_FIELDS;i++)
  {
      desc[i][0] = 'F';
      desc[i][1] = 'i';
      desc[i][2] = 'e';
      desc[i][3] = 'l';
      desc[i][4] = 'd';
      desc[i][5] = ' ';
      desc[i][6] = i + 48 + 1; // Convert i to ASCII value.
      desc[i][7] = 0;

      intext[i][0] = 'T';
      intext[i][1] = 'e';
      intext[i][2] = 'x';
      intext[i][3] = 't';
      intext[i][4] = ' ';
      intext[i][5] = i + 48 + 1; // Convert i to ASCII value.
      intext[i][6] = 0;

  }

  SceUtilityOskData data[NUM_INPUT_FIELDS];

  for(i = 0; i < NUM_INPUT_FIELDS;i++)
  {
      memset(&data[i], 0, sizeof(SceUtilityOskData));
      data[i].language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT; // Use system default for text input
      data[i].lines = 1;
      data[i].unk_24 = 1;
      data[i].inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL; // Allow all input types
      data[i].desc = desc[i];
      data[i].intext = intext[i];
      data[i].outtextlength = TEXT_LENGTH;
      data[i].outtextlimit = 32; // Limit input to 32 characters
      data[i].outtext = outtext[i];
  }

  SceUtilityOskParams params;
  memset(&params, 0, sizeof(params));
  params.base.size = sizeof(params);
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params.base.language);
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params.base.buttonSwap);
  params.base.graphicsThread = 17;
  params.base.accessThread = 19;
  params.base.fontThread = 18;
  params.base.soundThread = 16;
  params.datacount = NUM_INPUT_FIELDS;
  params.data = data;

  sceUtilityOskInitStart(&params);

  while(!done)
  {
      sceGuStart(GU_DIRECT,list);
      sceGuClearColor(0);
      sceGuClearDepth(0);
      sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

      sceGuFinish();
      sceGuSync(0,0);

      switch(sceUtilityOskGetStatus())
      {
          case PSP_UTILITY_DIALOG_INIT:
              break;

          case PSP_UTILITY_DIALOG_VISIBLE:
              sceUtilityOskUpdate(1);
              break;

          case PSP_UTILITY_DIALOG_QUIT:
              sceUtilityOskShutdownStart();
              break;

          case PSP_UTILITY_DIALOG_FINISHED:
              break;

          case PSP_UTILITY_DIALOG_NONE:
              done = 1;

          default :
              break;
      }

      sceDisplayWaitVblankStart();
      sceGuSwapBuffers();
  }

  return 0;
}


int umd_test()
{
  SceUID umd;
  SceUID ms0;

  int num;
  char str[1024];
  int size;
  int total;
  char name[16];
  char path[1024];
  int loop;

  total = 0;
  num = 0;
  SceCtrlData  data;

  unsigned int par;
  u64 tick_1;
  u64 tick_2;

  par = sceRtcGetTickResolution();

  get_umd_id(name);
  sprintf(path, "ms0:/iso/%s.ISO", name);

  umd = sceIoOpen("umd:", PSP_O_RDONLY, 0777);
  ms0 = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);


  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)"┏ PUSH  BUTTON ┓", 1);
  wait_button_down();
  wait_button_up();

  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)"┏━━━━━━━┓", 1);

  sceRtcGetCurrentTick(&tick_1);

//  size = sceIoRead(umd, i_buff, 512 * 2); // 512セクタ読込 1024KB
//  total += size;

  while(size = sceIoRead(umd, I_BUFFER, 1024), size > 0) // 2KB 639.05s 4KB 348.20s 8KB 200.02s 16KB 166.74s 32KB 160.12s 64KB 158.14s 128KB 159.14s 256KB 158.16s 1MB 157.87 2MB 159.46s 2m40s 4MB 165.01s 8MB 162.21s
//  while(total < 512 * 100)
  {
    total += size;
//    size = 2048 / 2; //(1024KB) 1024KB 11.18s 512KB 11.70s 2048KB 11.14s
//    total += size;
    for(loop = 0; loop < size; loop++)
    {
      deflate_cso(&O_BUFFER[0][loop * 0x800], sizeof(O_BUFFER), &I_BUFFER[0][loop * 0x800], 0x800, 9);
    }
    sceIoWrite(ms0, I_BUFFER, 0x800 * size);
    sprintf(str, "%s WRITE %dMB", name, total / 512);
    MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
    MENU_XY(240, MSG_LINE, (s8*)str, 1);
    num = (num + 1) % 2;
    get_button(&data);
    if((data.Buttons) == PSP_CTRL_CROSS)
    {
        MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
        MENU_XY(240, MSG_LINE, (s8*)"STOP", 1);
        wait_button_down();
        wait_button_up();
        sceIoClose(umd);
        sceIoClose(ms0);
        return -1;
    }
  }


  sceRtcGetCurrentTick(&tick_2);

  sceIoClose(umd);
  sceIoClose(ms0);

  sprintf(str, "SIZE:%d , TIME:%.2fs", total, (float)((float)(tick_2 - tick_1) / (float)(par)));
  MENU_XY(240, MSG_LINE, (s8*)CLEAR_LINE, 1);
  MENU_XY(240, MSG_LINE, (s8*)str, 1);

  return 0;
}

#if 0
/*
解説

 buf[0] = 合計クラスタ数
 buf[1] = フリーなクラスタ数(ギリギリまで使いたいならこっち)
 buf[2] = フリーなクラスタ数(buf[3]やbuf[4]と掛けて1MB単位になるようになってる)
 buf[3] = セクタ当たりバイト数
 buf[4] = クラスタ当たりセクタ数

 0*3*4でメモステ容量
 2*3*4で空き容量 (1*3*4 でも ok)

*/

#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <pspdisplay_kernel.h>
#include <pspctrl.h>

#include "blit.h"

PSP_MODULE_INFO("MsFreeSpace", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#define RED 0x0000ff
#define GREEN 0x00ff00
#define BLUE 0xff0000
#define BLACK 0x000000
#define WHITE 0xffffff

int main_thread(SceSize args, void* argp)
{
    unsigned int buf[5];
    unsigned int *pbuf = buf;
    char str[50], str1[50], str2[50];
    unsigned int free, all, use;
    int flag = 0;

    SceCtrlData key;

    while(1)
    {
    sceIoDevctl("ms0:", 0x02425818, &pbuf, sizeof(pbuf), 0, 0);

     all = buf[0] * buf[3] * buf[4];// MS容量取得
    free = buf[2] * buf[3] * buf[4];// 空き容量取得
     use = all - free;              // 使用している容量取得

    if(flag==0)
    {
        all = all / 1024 / 1024;
        sprintf(str, "%d MB", all);

        free = free / 1024 / 1024;
        sprintf(str1, "%d MB", free);

        use = use / 1024 / 1024;
        sprintf(str2, "%d MB", use);
    }

    if(flag==1)
    {
        all = all / 1024;
        sprintf(str, "%d KB", all);

        free = free / 1024;
        sprintf(str1, "%d KB", free);

        use = use / 1024;
        sprintf(str2, "%d KB", use);
    }

    sceCtrlReadBufferPositive(&key, 1);

    if(key.Buttons & PSP_CTRL_LTRIGGER && key.Buttons & PSP_CTRL_RTRIGGER)
    {
        flag += 1;
        if(flag > 1)flag = 0;
#endif
