/*
 * file.c
 *
 *  Created on: 2009/12/26
 *      Author: takka
 */

#include <string.h>
#include <stdlib.h>

#include <pspkernel.h>
#include <pspmscm.h>

#include "main.h"
#include "file.h"
#include "ciso.h"
#include "umd.h"
#include "error.h"
#include "screen.h"
#include "macro.h"

#define FIO_CST_SIZE    0x0004

/*---------------------------------------------------------------------------
  ファイルサイズ変更
  const char *path : パス
  int length       : サイズ

  return int       : 変更後のファイルサイズ, エラーの場合はERR_CHG_STATを返す
---------------------------------------------------------------------------*/
int file_truncate(const char *path, int length)
{
    SceIoStat psp_stat;
    int ret;

    psp_stat.st_size = length;
    ret = sceIoChstat(path, &psp_stat, FIO_CST_SIZE);
    if(ret < 0)
      ret = ERR_CHG_STAT;

    return ret;
}

/*---------------------------------------------------------------------------
  ディレクトリ読取り
  dir_t dir[]      : dir_t配列のポインタ
  const char *path : パス

  return int       : ファイル数, dir[0].numにも保存される
---------------------------------------------------------------------------*/
int read_dir(dir_t dir[], const char *path)
{
  SceUID dp;
  SceIoDirent entry;
  int num;
  int file_num = 1;
  int ret;

  ret = check_ms();

  strcpy(dir[0].name, "[UMD DRIVE]");
  dir[0].type = TYPE_UMD;

  dp = sceIoDopen(path);
  if(dp >= 0)
  {
    memset(&entry, 0, sizeof(entry));

    while((sceIoDread(dp, &entry) > 0))
    {
      num = strlen(entry.d_name);

      strcpy(dir[file_num].name, entry.d_name);
      switch(entry.d_stat.st_mode & FIO_S_IFMT)
      {
        case FIO_S_IFREG:
          if(strncasecmp(&entry.d_name[num - 4], ".iso", 4) == 0)
          {
            dir[file_num].type = TYPE_ISO;
            file_num++;
          }
          else if(strncasecmp(&entry.d_name[num - 4], ".cso", 4) == 0)
          {
            dir[file_num].type = TYPE_CSO;
            file_num++;
          }
          else if(strncasecmp(entry.d_name, "PBOOT.PBP", 4) == 0)
          {
            dir[file_num].type = TYPE_PBT;
            file_num++;
          }
          break;

        case FIO_S_IFDIR:
          if((strcmp(&entry.d_name[0], ".") != 0) && (strcmp(&entry.d_name[0], "..") != 0))
          {
            dir[file_num].type = TYPE_DIR;
            file_num++;
          }
          break;
      }
    }
    sceIoDclose(dp);
  }

  dir[0].num = file_num;

  return file_num;
}

/*---------------------------------------------------------------------------
  MSのリード
  void* buf        : 読取りバッファ
  const char* path : パス
  int pos          : 読込み開始場所
  int size         : 読込みサイズ, 0を指定すると全てを読込む

  return int       : 読込みサイズ, エラーの場合は ERR_OPEN/ERR_READ を返す
---------------------------------------------------------------------------*/
int ms_read(void* buf, const char* path, int pos, int size)
{
  SceUID fp;
  SceIoStat stat;
  int ret = ERR_OPEN;

  if(size == 0)
  {
    pos = 0;
    sceIoGetstat(path, &stat);
    size = stat.st_size;
  }

  fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
  if(fp > 0)
  {
    sceIoLseek32(fp, pos, PSP_SEEK_SET);
    ret = sceIoRead(fp, buf, size);
    sceIoClose(fp);
    if(ret < 0)
      ret = ERR_READ;
  }
  return ret;
}

/*---------------------------------------------------------------------------
  MSへのライト
  const void* buf  : 書込みバッファ
  const char* path : パス
  int pos          : 書込み開始場所
  int size         : 書込みサイズ

  return int       : 書込んだサイズ, エラーの場合は ERR_OPEN/ERR_WRITE を返す
---------------------------------------------------------------------------*/
int ms_write(const void* buf, const char* path, int pos, int size)
{
  SceUID fp;
  int ret = ERR_OPEN;

  fp = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777);
  if(fp > 0)
  {
    sceIoLseek32(fp, pos, PSP_SEEK_SET);
    ret = sceIoWrite(fp, buf, size);
    sceIoClose(fp);
    if(ret < 0)
      ret = ERR_WRITE;
  }
  return ret;
}

/*---------------------------------------------------------------------------
  ファイルリード
---------------------------------------------------------------------------*/
int file_read(void* buf, const char* path, file_type type, int pos, int size)
{
  int ret = ERR_OPEN;

  switch(type)
  {
    case TYPE_ISO:
    case TYPE_SYS:
      ret = ms_read(buf, path, pos, size);
      break;

    case TYPE_CSO:
      ret = cso_read(buf, path, pos, size);
      break;

    case TYPE_UMD:
      ret = umd_read(buf, path, pos, size);
      break;

    default:
      break;
  }
  return ret;
}

/*---------------------------------------------------------------------------
  ファイルライト
---------------------------------------------------------------------------*/
int file_write(const void* buf, const char* path, file_type type, int pos, int size)
{
  u32 ret = ERR_OPEN;

  switch(type)
  {
    case TYPE_ISO:
      ret = ms_write(buf, path, pos, size);
      break;

    case TYPE_CSO:
      ret = cso_write(buf, path, pos, size, 9);
      break;

    case TYPE_UMD:
      break;

    default:
      break;
  }
  return ret;
}

// FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH
int set_file_mode(const char* path, int bits)
{
  SceIoStat stat;
  int ret;

  ret = sceIoGetstat(path, &stat);

  if(ret >= 0)
  {
    stat.st_mode |= (bits);
    ret = sceIoChstat(path, &stat, (FIO_S_IRWXU | FIO_S_IRWXG | FIO_S_IRWXO));
  }
  if(ret < 0)
    ret = ERR_CHG_STAT;

  return ret;
}

/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int up_dir(char *path)
{
  int loop;
  int ret = ERR_OPEN;

  loop = strlen(path) - 2;

  while(path[loop--] != '/')
    ;

  if(path[loop - 1] != ':')
  {
    path[loop + 2] = '\0';
    ret = 0;
  }

  return ret;
}

int get_file_data(int* pos, int* size, int* size_pos, const char* path, file_type type, const char *name)
{
  char *ptr;
  char work[MAX_PATH];
  char s_path[MAX_PATH];
  char s_file[MAX_PATH];
  int path_table_addr;
  int path_table_size;
  int dir_recode_addr;
  char *table_buf; // パステーブルバッファ
  char dir_buf[SECTOR_SIZE];
  short int befor_dir_num = 0x0001;
  int now_dir_num = 1;
  int tbl_ptr;
  int dir_ptr;
  unsigned char len_di;
  unsigned char len_dr;
  int ret;

  dir_recode_addr = 0;

  if(*name == '/')
    name++;

  strcpy(work, name);

  // nameのパスとファイル名を分離
  ptr = strrchr(work, '/');
  if(ptr != NULL)
  {
    *ptr++ = '\0';
    strcpy(s_path, work);
  }
  else
  {
    s_path[0] = '\0';
    ptr = (char *)work;
  }

  // ファイル名をコピー
  strcpy(s_file, ptr);

  // パステーブルの場所/サイズを読込
  file_read(&path_table_size, path, type, 0x8084, 4);
  file_read(&path_table_addr, path, type, 0x808c, 4);
  path_table_addr *= SECTOR_SIZE;
  table_buf = malloc(path_table_size);
  if(table_buf < 0)
    return ERR_NO_MEMORY;

  // パステーブルをワークエリアに読込
  ret = file_read(table_buf, path, type, path_table_addr, path_table_size);

  //  オフセット 型             内容
  //  0          uint8          ディレクトリ識別子の長さ(LEN_DI) ルートは01
  //  1          uint8          拡張属性レコードの長さ
  //  2～5       uint32le/be(*) エクステントの位置
  //  6～7       uint16le/be(*) 親ディレクトリの番号 ルートは01(自分自身)
  //  8～        fileid(LEN_DI) ディレクトリ識別子
  //  8+LEN_DI   uint8          padding / LEN_DIが奇数の場合のみ

  if(s_path[0] == '\0')
  {
    // 探すファイルがルートにある場合
    bin2int(&dir_recode_addr, &table_buf[2]);
  }
  else
  {
    // パステーブルからディレクトリレコードの場所を調べる
    befor_dir_num = 0x0001;

    tbl_ptr = 0;
    now_dir_num = 0x0001;
    ptr = s_path;

    while(tbl_ptr < path_table_size)
    {
      len_di = (unsigned char)table_buf[tbl_ptr];
      if(len_di == 0)
        break;

      // 親ディレクトリの番号を比較
      tbl_ptr += 6;

      if(befor_dir_num == *(short int *)&table_buf[tbl_ptr])
      {
        tbl_ptr += 2;

        // ディレクトリ名が一致するか確認
        if(strncasecmp(&table_buf[tbl_ptr], ptr, len_di) == 0)
        {
          // befor_dir_numを更新
          befor_dir_num = now_dir_num;
          ptr = strchr(ptr, '/');
          if(ptr != NULL)
            ptr++;
          else
          {
            bin2int(&dir_recode_addr, &table_buf[tbl_ptr - 6]);
            break;
          }
        }
      }
      else
      {
        tbl_ptr += 2;
      }

      tbl_ptr += (len_di + 1) & ~1; // padding
      now_dir_num++;
    }
  }

  free(table_buf);

  if(dir_recode_addr == 0)
    return ERR_NOT_FOUND;

  dir_recode_addr *= SECTOR_SIZE;

  ret = file_read(dir_buf, path, type, dir_recode_addr, SECTOR_SIZE);

  // エクステントの位置等を調べる

  //  オフセット 型             内容
  //  0          uint8          ディレクトリレコードの長さ(LEN_DR)
  //  1          uint8          拡張属性レコードの長さ
  //  2～9       uint32both     エクステントの位置
  //  10～17     uint32both     データ長
  //  18～24     datetime_s     記録日付及び時刻
  //  25         8ビット        ファイルフラグ
  //  26         uint8          ファイルユニットの大きさ
  //  27         uint8          インタリーブ間隙の大きさ
  //  28～31     uint16both     ボリューム順序番号
  //  32         uint8          ファイル識別子の長さ(LEN_FI)
  //  33～       fileid(LEN_FI) ファイル/ディレクトリ識別子
  //  33+LEN_FI  uint8          padding / LEN_FIが偶数の場合のみ

  dir_ptr = 0;
  ret = ERR_NOT_FOUND;

  while(dir_ptr < SECTOR_SIZE)
  {
    len_dr = (unsigned char)dir_buf[dir_ptr];

    // ディレクトリ名が一致するか確認
    if(strncasecmp(&dir_buf[dir_ptr + 33], s_file, dir_buf[dir_ptr + 32]) == 0)
    {
      bin2int(pos, &dir_buf[dir_ptr + 2]);
      *pos *= SECTOR_SIZE;

      bin2int(size, &dir_buf[dir_ptr + 10]);

      *size_pos = dir_recode_addr + dir_ptr + 10;

      ret = *pos;
      break;
    }
    dir_ptr += len_dr;
  }

  return ret;
}

#if 0
/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int get_file_data_2(int* pos, int* size, int* size_pos, const char* path, file_type type, const char *name)
{
  char buf[50 * SECTOR_SIZE];
  int ptr = 0;
  int len;
  int ret;

  // 22～63セクタ(42セクタ分)を読み込む
  ret = file_read(buf, path, type, 22 * SECTOR_SIZE, 42 * SECTOR_SIZE);

  if(ret == (42 * SECTOR_SIZE))
  {
    len = strlen(name) - 1;

    while(strncasecmp(&buf[ptr], &name[1], len) != 0)
    {
      while(buf[ptr++] != name[0])
        if(ptr > 42 * SECTOR_SIZE)
          return ERR_NOT_FOUND;
    }

    ptr--;

    // ファイル名 - 0x1f にファイル先頭セクタ
    memcpy(pos, &buf[ptr - 0x1f], 4);
    *pos *= SECTOR_SIZE;

    // ファイル名 - 0x17 にファイルサイズ
    memcpy(size, &buf[ptr - 0x17], 4);

    // ファイルサイズの位置
    *size_pos = 22 * 0x800 + ptr - 0x17;

    ret = *pos;
  }

  return ret;
}
#endif

int read_line(char* str,  SceUID fp)
{
  char buf;
  int len = 0;
  int ret;

  do{
    ret = sceIoRead(fp, &buf, 1);
    if(ret == 1)
    {
      if(buf == '\n')
      {
        str[len] = '\0';
        len++;
        break;
      }
      if(buf != '\r')
      {
        str[len] = buf;
        len++;
      }
    }
  }while(ret > 0);

  return len;
}

int get_umd_sector(const char* path, file_type type)
{
  int size = 0;
  int ret;

  ret = file_read(&size, path, type, 0x8050, 4); // 0x50から4byteがセクタ数
  if(ret < 0)
    size = ret;

  return size;
}

int get_umd_id(char* id, const char* path, file_type type)
{
  int ret;
  // 0x8373から10byteがUMD ID
  ret = file_read(id, path, type, 0x8373, 10);
  if(ret == 10)
    id[10] = '\0';
  else
    strcpy(id, "**********");

  return ret;
}

int get_umd_name(char* name, char* e_name, const char* id, int mode)
{
  static char buf[1024*256]; // 256KB
  static int init = 0;
  char *ptr;
  int ptr2 = 0;
  int ret = 0;

  if((init == 0)||(mode == 1))
  {
    ms_read(buf, "UMD_ID.csv", 0, 0);
    init = 1;
    if(mode == 1)
      return 0;
  }

  ptr = strstr((const char *)buf, id);

  if(ptr != NULL)
  {
    ptr += 11;

    while(*ptr == '\\')
      ptr++;

    while(*ptr != '\\')
    {
      name[ptr2] = *ptr;
      ptr++;
      ptr2++;
    }
    name[ptr2] = '\0';

    while(*ptr == '\\')
      ptr++;

    ptr2 = 0;
    while((*ptr != '\r') && (*ptr != '\n'))
    {
      e_name[ptr2] = *ptr;
      ptr++;
      ptr2++;
    }
    e_name[ptr2] = '\0';

  }
  else
  {
    ret = -1;
    name[0] = '\0';
    e_name[0] = '\0';
  }

  return ret;
}

int get_ms_free()
{
    unsigned int buf[5];
    unsigned int *pbuf = buf;
    int free = 0;
    int ret;

    //    buf[0] = 合計クラスタ数
    //    buf[1] = フリーなクラスタ数(ギリギリまで使いたいならこっち)
    //    buf[2] = フリーなクラスタ数(buf[3]やbuf[4]と掛けて1MB単位になるようになってる)
    //    buf[3] = セクタ当たりバイト数
    //    buf[4] = クラスタ当たりセクタ数
    ret = sceIoDevctl("ms0:", 0x02425818, &pbuf, sizeof(pbuf), 0, 0);

    if(ret >= 0)
      free = buf[1] * ((buf[3] * buf[4]) / 1024);// 空き容量取得(kb)

    return free;
}

int check_ms()
{
  SceUID ms;
  int ret = DONE;

  ms = MScmIsMediumInserted();
  if(ms <= 0)
  {
    msg_win("", 0, MSG_CLEAR, 0);
    msg_win("Memory Stickを入れて下さい", 1, MSG_WAIT, 0);

    ms = -1;
    while(ms <= 0)
    {
      sceKernelDelayThread(1000);
      ms = MScmIsMediumInserted();
    }
    msg_win("", 0, MSG_CLEAR, 0);
    msg_win("マウント中です", 1, MSG_WAIT, 0);
    ret = CANCEL;
  }
  return ret;
}

int check_file(const char* path)
{
  SceIoStat stat;

  return sceIoGetstat(path, &stat);
}
