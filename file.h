/*
 * file.h
 *
 *  Created on: 2009/12/26
 *      Author: takka
 */

#ifndef FILE_H_
#define FILE_H_

#include <psptypes.h>
#include <pspiofilemgr.h>

#define MAX_PATH (512)

typedef enum {
  TYPE_ISO = 0,
  TYPE_CSO,
  TYPE_DIR,
  TYPE_UMD,
  TYPE_SYS,
  TYPE_PBT,
  TYPE_ETC
} file_type;

typedef struct {
  char name[MAX_PATH];
  file_type type;
  int num;
} dir_t;

#define SECTOR_SIZE (0x800)

/*---------------------------------------------------------------------------
  指定したディレクトリ情報を読取る
---------------------------------------------------------------------------*/
int read_dir(dir_t dir[], const char *path);

int ms_read(void* buf, const char* path, int pos, int size);

int ms_write(const void* buf, const char* path, int pos, int size);


/*---------------------------------------------------------------------------
  ファイルセクタリード
---------------------------------------------------------------------------*/
int ms_sector_read(void* buf, const char* path, int sec, int num);

/*---------------------------------------------------------------------------
  ファイルライト
---------------------------------------------------------------------------*/
int ms_sector_write(const void* buf, const char* path, int sec, int num);

int file_read(void* buf, const char* path, file_type type, int sec, int num);
int file_write(const void* buf, const char* path, file_type type, int sec, int num);

/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int get_file_data(int* pos, int* size, int* size_pos, const char* path, file_type type, const char *name);
//int get_file_data_2(int* pos, int* size, int* size_pos, const char* path, file_type type, const char *name);

int set_file_mode(const char* path, int bits);
int get_file_mode(const char* path);

int up_dir(char *path);

int get_umd_sector(const char* path, file_type type);

int get_umd_id(char* id, const char* path, file_type type);

int get_umd_name(char* name, char* e_name, const char* id, int mode);

int read_line(char* str,  SceUID fp);

int get_ms_free();
int check_ms();
int check_file(const char* path);

#endif /* FILE_H_ */
