/*
 * iso.c
 *
 *  Created on: 2009/12/29
 *      Author: takka
 */

#include "file.h"
#include "error.h"

int iso_read(void *buf, int max_buf, const char* path, file_type type, const char* file)
{
  int pos;
  int size;
  int size_pos;
  int ret;

  ret = get_file_data(&pos, &size, &size_pos, path, type, file);
  if(ret < 0)
    return ret;
  else if(size > max_buf)
    return ERR_SIZE_OVER;

  ret = file_read(buf, path, type, pos, size);

  return ret;
}

int iso_write( void *buf, int size, const char* path, file_type type, const char* file)
{
  int pos;
  int orig_size;
  int size_pos;
  int ret;
  unsigned char *endian;
  char prx_data[8];

  ret = get_file_data(&pos, &orig_size, &size_pos, path, type, file);
  if(ret < 0)
    return ret;

  endian = (unsigned char*)&size;
  prx_data[0] = endian[0];
  prx_data[1] = endian[1];
  prx_data[2] = endian[2];
  prx_data[3] = endian[3];
  prx_data[4] = endian[3];
  prx_data[5] = endian[2];
  prx_data[6] = endian[1];
  prx_data[7] = endian[0];
  ret = file_write(prx_data, path, type, size_pos, 8);
  if(ret < 0)
    return ret;

  ret = file_write(buf, path, type, pos, size);

  return ret;
}

