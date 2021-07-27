/*
 * iso.h
 *
 *  Created on: 2009/12/29
 *      Author: takka
 */

#ifndef ISO_H_
#define ISO_H_

//int eboot_read(void *buf, int max_buf, const char* path, file_type type);
//int eboot_write( void *buf, int size, const char* path, file_type type);
int iso_read(void *buf, int max_buf, const char* path, file_type type, const char* file);
int iso_write(void *buf, int max_buf, const char* path, file_type type, const char* file);

#endif /* ISO_H_ */
