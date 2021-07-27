/* screen.c
 *
 *  Created on: 2009/10/23
 *      Author: takka
 */

#include <pspdebug.h>
#include <stdio.h>
#include <string.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>

#include "screen.h"
#include "msg.h"
#include "file.h"
#include "main.h"
#include "key.h"
#include "fnt_print.h"
#include "shnm16psp.h"
#include "sound.h"
#include "unicode.h"
#include "web.h"

char umd_id[11][1024];
char umd_name[128];

// 480x272 16x16 : 30x17

#define MAX_WIDTH  (29)
#define MAX_HEIGHT (16)

#define CIRCLE_BUTTON   "◎"
#define CROSS_BUTTON    "○"
#define SQUARE_BUTTON   "■"
#define TRIANGLE_BUTTON "▽"
#define BUTTON          "◆"

#define D_PAD           "┼"
#define D_RIGHT         "├"
#define D_LEFT          "┤"
#define D_UP            "┴"
#define D_DOWN          "┬"

#define RIGHT_TRIANGLE  "▲"

#define COLOR32(red, green, blue) (0xff000000 | ((blue & 0xff) << 16) | ((green & 0xff) << 8) | (red & 0xff))
#define TEXT    COLOR32(255, 255, 255)
#define BG      COLOR32(0, 0, 0)
#define MODE    (FNT_FONT_FILL|FNT_BACK_FILL)
#define RATE    (100)

#define DIR_MENU_X1 (00)
#define DIR_MENU_Y1 (01)
#define DIR_MENU_X2 (29)
#define DIR_MENU_Y2 (11)

#define STAT_MENU_X1 (00)
#define STAT_MENU_Y1 (12)
#define STAT_MENU_X2 (29)
#define STAT_MENU_Y2 (16)

#define MENU_X2 (27)
#define MENU_Y2 (03)

#define MSG_WIN_X1 (03)
#define MSG_WIN_Y1 (03)
#define MSG_WIN_X2 (26)
#define MSG_WIN_Y2 (10)
#define MSG_WIN_LEN (MSG_WIN_X2 - MSG_WIN_X1 - 1)
#define MSG_WIN_LINE (MSG_WIN_Y2 - MSG_WIN_Y1 - 1)

#define REPEAT_SPEED (20)

#define MID_STR(x, str, num) ((x) - (fnt_get_width(&font, str, num) / 2))
#define print_xy(x, y, str, mx, my, x_len) fnt_print_xy(&font, ((x) * 16), ((y) * 16), (str), TEXT, BG, MODE, RATE, (mx), (my), (x_len) * 16, 0)
#define print_xy_mid(x, y, str, mx, my, length) fnt_print_xy(&font, MID_STR(((x) * 16), (str), (mx)), ((y) * 16), (str), TEXT, BG, MODE, RATE, (mx), (my), (length) * 16, 0)
#define print_xy_text(x, y, str, mx, my, x_len, y_len) fnt_print_xy(&font, ((x) * 16), ((y) * 16), (str), TEXT, BG, MODE, RATE, (mx), (my), (x_len) * 16, (y_len) * 16)

static unsigned int __attribute__((aligned(16))) list[262144];

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2)

fnt_t font;

/*---------------------------------------------------------------------------
  タイトル表示
---------------------------------------------------------------------------*/
static void print_title();

/*---------------------------------------------------------------------------
  枠を描画
---------------------------------------------------------------------------*/
static void write_window(int x1, int y1, int x2, int y2, int window_only);

/*---------------------------------------------------------------------------
  ウインドウを開く
---------------------------------------------------------------------------*/
static void make_window(char* title, int x1, int y1, int x2, int y2, int anim);

/*---------------------------------------------------------------------------
  セレクトカーソルの表示
---------------------------------------------------------------------------*/
static menu_ret_t select(int x, int y1, int y2, int sel_num, int max_num, int repeat_flag);

/*---------------------------------------------------------------------------
  ステータスウインドウ
---------------------------------------------------------------------------*/
static void make_stat_win();

/*---------------------------------------------------------------------------
  ステータス表示
---------------------------------------------------------------------------*/
static void stat_print(char *str1, char *str2, char *str3);

/*---------------------------------------------------------------------------
  ディレクトリの表示
---------------------------------------------------------------------------*/
static int print_dir(dir_t dir[], int num, int x1, int y1, int x2, int y2);


/*---------------------------------------------------------------------------
  GUの初期化
    返値 なし
---------------------------------------------------------------------------*/
static void setupGu()
{
  sceGuInit();
  sceGuStart(GU_DIRECT, list);
  sceGuDrawBuffer(GU_PSM_8888, 0, BUF_WIDTH);
  sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);
  sceGuDepthBuffer((void*)0x110000, BUF_WIDTH);
  sceGuOffset(0, 0);
  sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  sceGuEnable(GU_BLEND);
  sceGuClearColor(0);
  sceGuClearDepth(0);
  sceGuClearStencil(0);
  sceGuFinish();
  sceGuSync(0, 0);
  sceDisplayWaitVblankStart();
  sceGuDisplay(GU_TRUE);
}

/*---------------------------------------------------------------------------
  OSKの表示
    char *out_text       入力されたテキスト(SHIFT-JIS)
    const char *def_text デフォルトのテキスト(UTF-8N)
    const char *title    タイトル(UTF-8N)
    int mode             漢字変換の有無

    返値 入力があった場合は PSP_UTILITY_OSK_RESULT_CHANGED を返す
---------------------------------------------------------------------------*/
int osk(char *out_text, const char *def_text, const char *title, int mode)
{
  static int done = 0;
  SceUtilityOskData data;
  unsigned short desc[MAX_PATH];
  unsigned short intext[MAX_PATH];
  unsigned short outtext[MAX_PATH];

  utf8_to_utf16(desc, title);
  utf8_to_utf16(intext, def_text);

  memset(&data, 0, sizeof(SceUtilityOskData));
  data.unk_00 = 1; //漢字変換
  data.unk_04 = 0;
  data.language =PSP_UTILITY_OSK_LANGUAGE_ENGLISH; // Use system default for text input
  data.unk_12 = 0;
  data.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL; // Allow all input types
  data.lines = 1;
  data.unk_24 = 1;
  data.desc = desc;
  data.intext = intext;
  data.outtextlength = 64;
  data.outtext = (unsigned short *)outtext;
  data.outtextlimit = 58; // Limit input to 58 characters

  if((mode == 1) && (global.language == 0))
    data.language =PSP_UTILITY_OSK_LANGUAGE_JAPANESE; // Use system default for text input

  SceUtilityOskParams params;
  memset(&params, 0, sizeof(params));
  params.base.size = sizeof(params);
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params.base.language);
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params.base.buttonSwap);
  params.base.graphicsThread = 17;
  params.base.accessThread = 19;
  params.base.fontThread = 18;
  params.base.soundThread = 16;
  params.datacount = 1;
  params.data = &data;
  params.unk_60 = 0;

  sceUtilityOskInitStart(&params);

  while(!done)
  {
      sceGuStart(GU_DIRECT,list);
      sceGuClearColor(COLOR32(64, 64, 64));
      sceGuClearDepth(0);
      sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

      sceGuFinish();
      sceGuSync(0,0);

      switch(sceUtilityOskGetStatus())
      {
          case PSP_UTILITY_DIALOG_INIT:
              break;

          case PSP_UTILITY_DIALOG_VISIBLE:
              sceUtilityOskUpdate(2);
              sceDisplayWaitVblankStart();
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

  done = 0;
  utf16_to_sjis(out_text, outtext);

  sceGuSwapBuffers();
  sceGuStart(GU_DIRECT,list);
  sceGuClearColor(0);
  sceGuClearDepth(0);
  sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
  sceGuSwapBuffers();

  sceGuFinish();
  sceGuSync(0,0);

  dir_menu(NULL, YES);

  return data.result;
}



int net_dialog()
{
  int done = 0;

  pspUtilityNetconfData data;

  memset(&data, 0, sizeof(data));
  data.base.size = sizeof(data);
  data.base.language = PSP_SYSTEMPARAM_LANGUAGE_JAPANESE;
  data.base.buttonSwap = PSP_UTILITY_ACCEPT_CIRCLE;
  data.base.graphicsThread = 17;
  data.base.accessThread = 19;
  data.base.fontThread = 18;
  data.base.soundThread = 16;
  data.action = PSP_NETCONF_ACTION_CONNECTAP;

  struct pspUtilityNetconfAdhoc adhocparam;
  memset(&adhocparam, 0, sizeof(adhocparam));
  data.adhocparam = &adhocparam;

  sceUtilityNetconfInitStart(&data);

  while(global_running)
  {

    sceGuStart(GU_DIRECT,list);
    sceGuClearColor(COLOR32(64, 64, 64));
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

    sceGuFinish();
    sceGuSync(0,0);

    switch(sceUtilityNetconfGetStatus())
    {
      case PSP_UTILITY_DIALOG_NONE:
        break;

      case PSP_UTILITY_DIALOG_VISIBLE:
        sceUtilityNetconfUpdate(1);
        break;

      case PSP_UTILITY_DIALOG_QUIT:
        sceUtilityNetconfShutdownStart();
        break;

      case PSP_UTILITY_DIALOG_FINISHED:
        done = 1;
        break;

      default:
        break;
    }

    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();

    if(done)
      break;
  }

  sceGuSwapBuffers();
  sceGuStart(GU_DIRECT,list);
  sceGuClearColor(0);
  sceGuClearDepth(0);
  sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
  sceGuSwapBuffers();

  sceGuFinish();
  sceGuSync(0,0);

  dir_menu(NULL, YES);

  return DONE;
}

/*---------------------------------------------------------------------------
  初期設定
---------------------------------------------------------------------------*/
void init_screen()
{
  pspDebugScreenInit();
  setupGu();
  sceGuSwapBuffers();
  fnt_load_mem(shnm16, &font);
  print_title();
}

/*---------------------------------------------------------------------------
  タイトル表示
---------------------------------------------------------------------------*/
void print_title()
{
  print_xy_mid(15, 0, (void*)global_title, 1, 1, 0);
  print_xy(23, 0, "kako / takka", 1, 1, 0);
}

/*---------------------------------------------------------------------------
  枠を描画
---------------------------------------------------------------------------*/
static void write_window(int x1, int y1, int x2, int y2, int window_only)
{
  int x, y;

  x = x2 - x1 - 1;
  y = y2 - y1 - 1;
  if(x < 0) x = 0;
  if(y < 0) y = 0;

  print_xy(x1    , y1    , "┏", 1, 1, 0);
  print_xy(x1 + 1, y1    , "━", x, 1, 0);
  print_xy(x2    , y1    , "┓", 1, 1, 0);

  print_xy(x1    , y1 + 1, "┃", 1, y, 0);
  print_xy(x2    , y1 + 1, "┃", 1, y, 0);

  if(window_only == 1)
    for(y = y1 + 1; y <= y2 - 1; y++)
      print_xy(x1 + 1, y   , "  ", x, 1, 0);

  print_xy(x1    , y2    , "┗", 1, 1, 0);
  print_xy(x1 + 1, y2    , "━", x, 1, 0);
  print_xy(x2    , y2    , "┛", 1, 1, 0);

}

/*---------------------------------------------------------------------------
  ウインドウを開く
---------------------------------------------------------------------------*/
static void make_window(char* title, int x1, int y1, int x2, int y2, int anim)
{
  int d, x, y;
  const int dd = 8;

  if(anim == 1)
  {
    x = (x2 - x1) / 2;
    y = (y2 - y1) / 2;

    for(d = dd; d >= 0; d--)
      write_window(x1 + (x * d / dd) , y1 + (y * d / dd), x2 - (x * d / dd), y2 - (y * d / dd), 1);
  }

  write_window(x1, y1, x2, y2, 1);
  print_xy(x1 + 1, y1, title, 1, 1, 0);
}

/*---------------------------------------------------------------------------
  メニューの表示
  char *title         タイトル
  new_menu_item item[]    表示内容
  int sel_num         最初に選択されている番号
  int x               左上のx位置(負の場合は右下のx位置) 16dot単位
  int y               左上のy位置(負の場合は右下のy位置) 16dot単位
  int x_size          最大xサイズ(0の場合は自動調整) 16dot単位
  int y_size          最大yサイズ(0の場合は自動調整) 16dot単位
  const int *callback コールバック関数

  返値 選択したメニュー番号
---------------------------------------------------------------------------*/
int new_menu(const char *title, new_menu_item item[], int sel_num, int x, int y, int x_size, int y_size, const int *callback)
{
  int x1, x2, y1, y2;
  int max_width;
  int loop = 0;
  int width;
  char line[256];

  // 表示位置計算

  // 横幅の最小値ををタイトルから取得
  max_width = fnt_get_width(&font, title, 1);

  // アイテムの数とテキストから横幅のサイズを取得
  while(**item[loop].text != '\0')
  {
    width = fnt_get_width(&font, *(item[loop].text + global.language), 1);
    max_width = max_width > width ? max_width : width;
    loop++;
  }

  //横幅決定(枠以外)
  width = (max_width + 15 ) / 16;

  // x座標確定
  if(x >= 0)
  {
    x1 = x;
    if((x_size = 0) || (x_size < width + 1))
      x_size = width;
    x2 = x + x_size + 1;
  }
  else
  {

  }



  memset(line, '-', max_width / 8);
  line[max_width / 8] = '\0';

  width = (max_width + 15 ) / 16 /*+ 1*/;

  x1 = x - width - 2;
  y1 = y;
  x2 = x;
  y2 = y1 + loop + 2;

  if(y2 > 16)
  {
    y1 = y1 - (y2 - 16);
    y2 = y1 + loop + 2;
  }

  make_window(title, x1, y1, x2, y2, 1);

  return 0;
}



/*---------------------------------------------------------------------------
  ディレクトリメニュー
---------------------------------------------------------------------------*/
int dir_menu(const char* s_path, int redrow_flag)
{
  static char path[MAX_PATH];
  static char utf8[MAX_PATH];
  static dir_t dir[MAX_PATH];
  menu_ret_t ret;
  int exit = 0;
  static int dir_num = 0;
  static int sel_num = 0;
  int num;
  static char *menu_title[] = { "ISO MENU", "CSO MENU", "DIR MENU", "UMD MENU", "SYS MENU", "PBOOT MENU" };
  static menu_item *memu_list[] = { menu_iso, menu_cso, menu_dir, menu_umd, menu_sys, menu_pbt };
  static  int redrow = 0;
  int err;

  if(redrow_flag == NO)
    strcpy(path, s_path);
  else
  {
    dir_num = 0;
    sel_num = 0;
  }

  read_dir(dir, path);
  sjis_to_utf8(utf8, path);
  make_window(utf8, DIR_MENU_X1, DIR_MENU_Y1, DIR_MENU_X2, DIR_MENU_Y2, 1);
  make_stat_win();
  file_stat_print(path, dir[dir_num + sel_num].name, dir[dir_num + sel_num].type);
  num = print_dir(dir, dir_num, DIR_MENU_X1 + 2, DIR_MENU_Y1 + 1, DIR_MENU_X2 - 1, DIR_MENU_Y2 -1);

  if(redrow_flag == YES)
  {
    print_title();
    print_xy(DIR_MENU_X1 + 1, DIR_MENU_Y1 + 1 + sel_num, RIGHT_TRIANGLE, 1, 1 ,0);
    return DONE;
  }

  while(exit == NO)
  {
    ret = select(DIR_MENU_X1 + 1, DIR_MENU_Y1 + 1, DIR_MENU_Y2 -1, sel_num, num, YES);
    sel_num = ret.sel_num;

    err = check_ms();
    if(err < 0)
      ret.key = MENU_RET_REDROW;

    switch(ret.key)
    {
      case MENU_RET_UP:
        if(dir_num > 0)
          dir_num--;
        file_stat_print(path, dir[dir_num + sel_num].name, dir[dir_num + sel_num].type);
        num = print_dir(dir, dir_num, DIR_MENU_X1 + 2, DIR_MENU_Y1 + 1, DIR_MENU_X2 - 1, DIR_MENU_Y2 -1);
        break;

      case MENU_RET_DOWN:
        if(dir_num < dir[0].num - (DIR_MENU_Y2 - DIR_MENU_Y1 - 1))
          dir_num++;
        file_stat_print(path, dir[dir_num + sel_num].name, dir[dir_num + sel_num].type);
        num = print_dir(dir, dir_num, DIR_MENU_X1 + 2, DIR_MENU_Y1 + 1, DIR_MENU_X2 - 1, DIR_MENU_Y2 -1);
        break;

      case MENU_RET_RIGHT:
        if(dir[dir_num + ret.sel_num].type == TYPE_DIR)
        {
          strcat(path, dir[dir_num + ret.sel_num].name);
          strcat(path, "/");
          beep();
          redrow = YES;
          dir_num = 0;
          sel_num = 0;
        }
        break;

      case MENU_RET_LEFT:
        if(up_dir(path) >= 0)
        {
          beep();
          redrow = YES;
          dir_num = 0;
          sel_num = 0;
        }
        break;

      case MENU_RET_START:
        break;

      case MENU_RET_SELECT:
        break;

      case MENU_RET_TRIANGLE:
        ret.key = MENU_RET_REDROW;
        ret.sel_num = 0;
        beep();
        while(ret.key == MENU_RET_REDROW)
        {
          ret = menu(path, dir[dir_num + sel_num].name, TYPE_SYS, menu_title[TYPE_SYS],
              memu_list[TYPE_SYS], ret.sel_num, MENU_X2, MENU_Y2);
          redrow = YES;
        }
        break;

      case MENU_RET_SQUARE:
        break;

      case MENU_RET_CROSS:
        break;

      case MENU_RET_CIRCLE:
        ret.key = MENU_RET_REDROW;
        ret.sel_num = 0;
        beep();
        while(ret.key == MENU_RET_REDROW)
        {
          switch(dir[dir_num + sel_num].type)
          {
            case TYPE_ISO:
            case TYPE_CSO:
            case TYPE_DIR:
            case TYPE_UMD:
            case TYPE_PBT:
              ret = menu(path, dir[dir_num + sel_num].name, dir[dir_num + sel_num].type, menu_title[dir[dir_num + sel_num].type],
                  memu_list[dir[dir_num + sel_num].type], ret.sel_num, MENU_X2, MENU_Y2);
              break;

            case TYPE_ETC:
            case TYPE_SYS:
              ret.key = MENU_RET_NORMAL;
              break;
          }
        }
        redrow = YES;
        dir_num = 0;
        sel_num = 0;
        break;

      case MENU_RET_NORMAL:
        file_stat_print(path, dir[dir_num + sel_num].name, dir[dir_num + sel_num].type);
        break;

      case MENU_RET_REDROW:
        redrow = YES;
        dir_num = 0;
        sel_num = 0;
        num = 0;
        strcpy(path, "ms0:/iso/");
        break;

    }

    if(redrow == YES)
    {
      print_title();
      sjis_to_utf8(utf8, path);
      make_window(utf8, DIR_MENU_X1, DIR_MENU_Y1, DIR_MENU_X2, DIR_MENU_Y2, 1);
      make_stat_win();
      file_stat_print(path, dir[dir_num + sel_num].name, dir[dir_num + sel_num].type);
      print_xy(DIR_MENU_X1 + 1, DIR_MENU_Y1 + 1 + sel_num, RIGHT_TRIANGLE, 1, 1 ,0);
      read_dir(dir, path);
      num = print_dir(dir, dir_num, DIR_MENU_X1 + 2, DIR_MENU_Y1 + 1, DIR_MENU_X2 - 1, DIR_MENU_Y2 -1);
      redrow = NO;
    }
  }

  return DONE;
}

/*---------------------------------------------------------------------------
  セレクトカーソルの表示
---------------------------------------------------------------------------*/
static menu_ret_t select(int x, int y1, int y2, int sel_num, int max_num, int repeat_flag)
{
  int exit = NO;
  static int repeat = NO;
  menu_ret_t ret;
  SceCtrlData key;

  ret.key = 0;
  ret.sel_num = sel_num;
  ret.dir_num = 0;

  max_num--;
  if(max_num > y2 - y1)
    max_num = y2 -y1;

  print_xy(x, y1 + sel_num, RIGHT_TRIANGLE, 1, 1, 0);

  while(exit == NO)
  {
    get_button(&key);

    switch(key.Buttons)
    {
      case PSP_CTRL_UP:
        if(sel_num > 0)
        {
          print_xy(x, y1 + sel_num, "  ", 1, 1, 0);
          sel_num--;
          print_xy(x, y1 + sel_num, RIGHT_TRIANGLE, 1, 1, 0);
          ret.key = MENU_RET_NORMAL;
        }
        else
        {
          ret.key = MENU_RET_UP;
        }
        exit = YES;
        if(repeat >= YES)
          repeat = REPEAT_SPEED;
        else
          repeat = YES;
        break;

      case PSP_CTRL_DOWN:
        if(sel_num < max_num)
        {
          print_xy(x, y1 + sel_num, "  ", 1, 1, 0);
          sel_num++;
          print_xy(x, y1 + sel_num, RIGHT_TRIANGLE, 1, 1, 0);
          ret.key = MENU_RET_NORMAL;
        }
        else
        {
          ret.key = MENU_RET_DOWN;
        }
        exit = YES;
        if(repeat >= YES)
          repeat = REPEAT_SPEED;
        else
          repeat = YES;
        break;

      case PSP_CTRL_RIGHT:
        ret.key = MENU_RET_RIGHT;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_LEFT:
        ret.key = MENU_RET_LEFT;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_START:
        ret.key = MENU_RET_START;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_SELECT:
        ret.key = MENU_RET_SELECT;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_TRIANGLE:
        ret.key = MENU_RET_TRIANGLE;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_SQUARE:
        ret.key = MENU_RET_SQUARE;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_CROSS:
        ret.key = MENU_RET_CROSS;
        exit = YES;
        repeat = NO;
        break;

      case PSP_CTRL_CIRCLE:
        ret.key = MENU_RET_CIRCLE;
        exit = YES;
        repeat = NO;
        break;

      default:
        repeat = NO;
        sceKernelDelayThread(1000);
        break;

    }
  }

  if((repeat >= YES) && (repeat_flag == YES))
    sceKernelDelayThread(500000 / repeat);
  else
    wait_button_up();

  ret.sel_num = sel_num;
  ret.dir_num = 0;
  return ret;
}

/*---------------------------------------------------------------------------
  ステータスウインドウ
---------------------------------------------------------------------------*/
static void make_stat_win()
{
  make_window(MSG_STATUS[global.language],STAT_MENU_X1, STAT_MENU_Y1, STAT_MENU_X2, STAT_MENU_Y2, YES);
}

/*---------------------------------------------------------------------------
  ステータス表示
---------------------------------------------------------------------------*/
static void stat_print(char *str1, char *str2, char *str3)
{
  char str[255];
  int max = STAT_MENU_X2 - STAT_MENU_X1 - 1;

  memset(str, ' ', max * 2);
  str[max * 2] = 0x00;
  print_xy(STAT_MENU_X1 + 1, STAT_MENU_Y1 + 1, str, 1, 1, max);
  print_xy(STAT_MENU_X1 + 1, STAT_MENU_Y1 + 2, str, 1, 1, max);
  print_xy(STAT_MENU_X1 + 1, STAT_MENU_Y1 + 3, str, 1, 1, max);
  print_xy(STAT_MENU_X1 + 1, STAT_MENU_Y1 + 1, str1, 1, 1, max);
  print_xy(STAT_MENU_X1 + 1, STAT_MENU_Y1 + 2, str2, 1, 1, max);
  print_xy(STAT_MENU_X1 + 1, STAT_MENU_Y1 + 3, str3, 1, 1, max);
}

/*---------------------------------------------------------------------------
  ファイルステータス表示
---------------------------------------------------------------------------*/
void file_stat_print(const char *dir, const char *file, file_type type)
{
  char work[128];
  char ms_info[128];
  SceIoStat stat;

  strcpy(work, dir);
  strcat(work, file);

  global.ms_free_size = get_ms_free();
  sprintf(ms_info, "MS FREE: %5.1fMiB", global.ms_free_size / 1024.0);

  global.umd_size = get_umd_sector(work, type) * SECTOR_SIZE;
  if(global.umd_size < 0)
    global.umd_size = 0;

  if((type == TYPE_ISO)||(type == TYPE_CSO)||(type == TYPE_UMD))
  {
    if((type == TYPE_ISO)||(type == TYPE_CSO))
    {
      sceIoGetstat(work, &stat);
      global.file_size = stat.st_size;
    }
    else // TYPE_UMD
      global.file_size = global.umd_size;

    get_umd_id(global.umd_id, work, type);
    sprintf(work, "ID: %s / SIZE: %5.1fMiB / FILE SIZE: %5.1fMiB", global.umd_id, global.umd_size / 1024.0 / 1024.0, global.file_size / 1024.0 / 1024.0);

    get_umd_name(global.name, global.e_name, global.umd_id, 0);

    if(global.language == 0)
      stat_print(work, global.name, ms_info);
    else
      stat_print(work, global.e_name, ms_info);
  }
  else
  {
    stat_print("", "", ms_info);
  }
}

/*---------------------------------------------------------------------------
  ディレクトリの表示
---------------------------------------------------------------------------*/
static int print_dir(dir_t dir[], int num, int x1, int y1, int x2, int y2)
{
  int y;
  int max = x2 - x1 + 1;
  int len;
  char str[255];
  char utf8[255];

  for(y = 0; y <= (y2 - y1); y++)
  {
    memset(str, ' ', max * 2);
    str[max * 2] = 0x00;

    if((num + y) < dir[0].num)
    {
      strcpy(str, dir[num + y].name);
      len = strlen(dir[num + y].name);

      if(dir[num + y].type == TYPE_DIR)
        str[len] = '/';
      else
        str[len] = ' ';
    }

    sjis_to_utf8(utf8, str);

    print_xy(x1, y1 + y, utf8, 1, 1, x2 - x1 + 1);
  }
  return dir[0].num - num;
}
/*---------------------------------------------------------------------------
  ダイアログの表示
---------------------------------------------------------------------------*/
void dialog(char *text[])
{
  SceCtrlData  data;
  int loop = 0;
  int width;
  int max_width = 0;
  int x1, x2;
  int y1, y2;

  while(*text[loop] != '\0')
  {
    width = fnt_get_width(&font, text[loop], 1);
    max_width = max_width > width ? max_width : width;
    loop++;
  }

  width = (((max_width + 15 ) / 16) + 1) / 2;
  x1 = 15 - width;
  x2 = 14 + width;

  loop--;
  y2 = 10 + (loop / 2);
  y1 = y2 - loop;

  make_window("",x1 - 1, y1 - 1, x2 + 1, y2 + 1, 1);
  loop = 0;
  while(*text[loop] != '\0')
  {
    print_xy_mid(15, y1 + loop, text[loop], 1, 1, 0);
    loop++;
  }

  beep();

  wait_button_up();

  do{
    get_button_wait(&data);
  }while(data.Buttons != PSP_CTRL_CIRCLE);

  beep();
  wait_button_up();
}

/*---------------------------------------------------------------------------
  メニューの表示
---------------------------------------------------------------------------*/
menu_ret_t menu(char *dir, char *file, file_type type, char *title, menu_item item[], int sel_num, int x, int y)
{
  int width;
  int max_width;
  int loop = 0;
  int x1, x2;
  int y1, y2;
  int exit = NO;
  menu_ret_t ret;
  int com_ret;
  char line[256];

  max_width = fnt_get_width(&font, title, 1);
  while(**item[loop].text != '\0')
  {
    width = fnt_get_width(&font, *(item[loop].text + global.language), 1);
    max_width = max_width > width ? max_width : width;
    loop++;
  }

  memset(line, '-', max_width / 8);
  line[max_width / 8] = '\0';

  width = (max_width + 15 ) / 16 /*+ 1*/;
  loop--;

  x1 = x - width - 2;
  y1 = y;
  x2 = x;
  y2 = y1 + loop + 2;

  if(y2 > 16)
  {
    y1 = y1 - (y2 - 16);
    y2 = y1 + loop + 2;
  }

  make_window(title, x1, y1, x2, y2, 1);
  loop = 0;
  while(**item[loop].text != '\0')
  {
    if(**item[loop].text == '\t')
      print_xy(x1 + 2, y1 + 1 + loop, line, 1, 1, 0);
    else
      print_xy(x1 + 2, y1 + 1 + loop, *(item[loop].text + global.language), 1, 1, 0);
    loop++;
  }

  wait_button_up();

  while(exit == NO)
  {
    ret = select(x1 + 1, y1 + 1, y2 - 1, sel_num, loop, NO);
    sel_num = ret.sel_num;

    switch(ret.key)
    {
      case MENU_RET_UP:
        break;

      case MENU_RET_DOWN:
        break;

      case MENU_RET_RIGHT:
        break;

      case MENU_RET_LEFT:
        break;

      case MENU_RET_START:
        break;

      case MENU_RET_SELECT:
        break;

      case MENU_RET_TRIANGLE:
        break;

      case MENU_RET_SQUARE:
        break;

      case MENU_RET_CROSS:
        ret.sel_num = -1;
        exit = YES;
        break;

      case MENU_RET_CIRCLE:
        switch (item[sel_num].menu_type)
        {
          case MENU_COMMAND:
            if(*item[sel_num].command != NULL)
            {
              beep();
              com_ret = (*item[sel_num].command)(dir, file, type, item[sel_num].opt_1, item[sel_num].opt_2);
              ret.sel_num = sel_num;
              if(com_ret < 0)
                ret.key = MENU_RET_REDROW;
              else
                ret.key = MENU_RET_NORMAL;
              exit = YES;
            }
            break;

          case MENU_RET_INT:
            beep();
            ret.sel_num = item[sel_num].opt_1;
            ret.key = MENU_RET_NORMAL;
            exit = YES;
            break;

          default:
            break;
        }
        break;

      case MENU_RET_NORMAL:
        break;

      case MENU_RET_REDROW:
        break;
    }
  }

    return ret;
}

/*---------------------------------------------------------------------------
  セレクトメニューの表示
---------------------------------------------------------------------------*/
int select_menu(char *title, menu_item item[], int def, int x, int y)
{
  menu_ret_t ret;

//  beep();
  ret = menu(NULL, NULL, 0, title, item, def, x, y);

  return ret.sel_num;
}

/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int select_menu_list(menu_list list[])
{
  int menu = 0;
  int ret = 0;
  int exit = NO;

  while(exit == NO)
  {
    if(list[menu].menu != NULL)
    {
      msg_win(*(list[menu].text + global.language), 0, MSG_WAIT, 1);
      ret = select_menu(*(list[menu].title + global.language) , list[menu].menu, list[menu].def, 28, 10);
      if(ret < 0)
        menu--;
      else
      {
        if(list[menu].ret != NULL)
          *(list[menu].ret) = ret;
        menu++;
      }
    }
    else
    {
      exit = YES;
    }
  }

  if(menu < 0)
    return CANCEL;
  else
    return ret; // 最後の選択が返値となる
}

/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int msg_win(char *new_text, int beep_flg, msg_win_command command, int parm)
{
  static int view = 0;
  static int line = 0;
  static char text[MSG_WIN_LINE][MSG_WIN_LEN * 3 + 1];
  int loop;
  int ret = DONE;

  switch(command)
  {
    case MSG_CLEAR:
      view = 0;
      ret = DONE;
      break;

    case MSG_LINE:
      line = parm;

      memset(text[line], ' ', MSG_WIN_LEN * 3 + 1);
      strcpy(text[line], new_text);
      ret = strlen(new_text);
      text[line][ret] = ' ';

      print_xy(MSG_WIN_X1 + 1, MSG_WIN_Y1 + 1 + line, text[line], 1, 1, MSG_WIN_LEN);

      ret = DONE;
      break;

    case MSG_WAIT:
      make_window("", MSG_WIN_X1, MSG_WIN_Y1, MSG_WIN_X2, MSG_WIN_Y2, 0);
      if(view == 0)
      {
        make_window("", MSG_WIN_X1, MSG_WIN_Y1, MSG_WIN_X2, MSG_WIN_Y2, 1);
        view = 1;
        line = 0;
        memset(text, ' ', sizeof(text));
        for(loop = 0; loop < MSG_WIN_LINE; loop++)
          text[loop][MSG_WIN_LEN * 3] = '\0';
      }

      if(line == MSG_WIN_LINE)
      {
        for(loop = 0; loop < (MSG_WIN_LINE - 1); loop++)
          strncpy(text[loop], text[loop + 1], MSG_WIN_LEN * 3 + 1);
        line--;
      }
      memset(text[line], ' ', MSG_WIN_LEN * 3 + 1);
      strcpy(text[line], new_text);
      line++;

      for(loop = 0; loop < MSG_WIN_LINE; loop++)
        print_xy(MSG_WIN_X1 + 1, MSG_WIN_Y1 + 1 + loop, text[loop], 1, 1, MSG_WIN_LEN);

      if(beep_flg == 1)
        beep();

      if(command == MSG_WAIT)
        sceKernelDelayThread(parm * 250000); // 0.25sec

      ret = DONE;
      break;

    case MSG_REDROW:
      make_window("", MSG_WIN_X1, MSG_WIN_Y1, MSG_WIN_X2, MSG_WIN_Y2, 0);

      for(loop = 0; loop < MSG_WIN_LINE; loop++)
        print_xy(MSG_WIN_X1 + 1, MSG_WIN_Y1 + 1 + loop, text[loop], 1, 1, MSG_WIN_LEN);

      if(beep_flg == 1)
        beep();

      ret = DONE;
      break;

    default:
      break;
  }

  return ret;
}
