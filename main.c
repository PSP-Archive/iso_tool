/*
	iso_tool
 */

#include <pspkernel.h>
#include <kubridge.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <systemctrl_se.h>
#include <psppower.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <psprtc.h>
#include <systemctrl.h>
#include <pspumd.h>

#include "main.h"
#include "pspdecrypt.h"
#include "screen.h"
#include "sound.h"
#include "error.h"

#define STR(num) #num
#define VER_INFO(str) "iso_tool ver "STR(str)

PSP_MODULE_INFO( "iso_tool", 0, 0, 0 );
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

char WORK[MAX_SECTOR_NUM][SECTOR_SIZE] __attribute__((aligned(0x40)));
int FLAG[MAX_SECTOR_NUM];

global_t global;
int global_running = 1;
char *global_title;
char main_path[MAX_PATH];
static SceUID decrypt_mod;

static void init_option();

// HOMEキー用の初期設定
int exit_callback(int arg1, int arg2, void *common)
{
  global_running = 0;
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

int init_module()
{
//  char path[MAX_PATH];
//
//  chdir(main_path);
//  strcpy(path, main_path);
//  strcat(path, "/pspdecrypt.prx");
  decrypt_mod = kuKernelLoadModule("pspdecrypt.prx", 0, NULL);
  if(sceKernelStartModule(decrypt_mod, 0, NULL, NULL, NULL) < 0)
  {
    err_msg(ERR_PRX);
    sceKernelExitGame();
  }

  sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
  sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

  return decrypt_mod;
}

void free_module()
{
  sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
  sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
  sceKernelUnloadModule(decrypt_mod);
}

void reboot(int mode)
{
  char path[MAX_PATH];
  struct SceKernelLoadExecVSHParam param;

  free_module();

  if(mode == 0)
  {
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(main_path)+1;
    param.argp = main_path;
    param.key = "game";

    strcpy(path, main_path);
    strcat(path, "/EBOOT.PBP");
    sctrlKernelLoadExecVSHMs1(path, &param);
  }
  else
    sceKernelExitGame();
}

void init_option()
{
  int temp;
  global.eboot_backup = 0;
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &temp);
  if(temp != 0)
    temp = 1;
  global.language = temp;

  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &temp);
  global.enter_button = temp;
  global.eboot[0] = '\0';
}

// メイン
int main(int argc, char *argv[])

{
  // HOMEボタンを使用可能にする
  SetupCallbacks();

  int ret;
  chdir(argv[0]);
  getcwd(main_path, MAX_PATH);
//  chdir(main_path);
  sceIoMkdir("BACK_UP",0777);
  sceIoMkdir("ms0:/ENC",0777);

  global_title = VER_INFO(VERSION);

  init_option();
  init_sound();
  init_screen();
  init_module();

  ret = dir_menu("ms0:/iso/", NO);

  free_sound();
  return 0;
}

void set_clock(int cpu, int bus)
{
  scePowerSetClockFrequency(cpu, cpu, bus);
}

