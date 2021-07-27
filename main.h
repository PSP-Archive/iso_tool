/*
 * main.h
 *
 *  Created on: 2010/01/03
 *      Author: takka
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "file.h"

#define DECRYPT_DATA (0)
#define CRYPT_DATA (512)

// 512 * 2KB(0x800) * 20 = 21MB
#define MAX_SECTOR_NUM (512 * 20)
// 21MB - 1MB = 20MB
#define EBOOT_MAX_SIZE (SECTOR_SIZE * (MAX_SECTOR_NUM  - CRYPT_DATA))

#define O_BUFFER (0)
#define I_BUFFER (MAX_SECTOR_NUM / 2)


extern char WORK[MAX_SECTOR_NUM][SECTOR_SIZE] __attribute__((aligned(0x40)));

typedef struct {
  int eboot_backup;
  int auto_patch;
  char umd_id[11];
  char name[128];
  char e_name[128];
  int umd_size;
  int file_size;
  int ms_free_size;
  int cso_level;
  int cso_threshold;
  int language;
  int enter_button;
  char eboot[MAX_PATH];
} global_t;

extern global_t global;
extern int global_running;
extern char *global_title;
extern char main_path[MAX_PATH];

void set_clock(int cpu, int bus);
void reboot(int mode);

#endif /* MAIN_H_ */
