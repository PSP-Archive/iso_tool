/*
 * web.h
 *
 *  Created on: 2010/01/10
 *      Author: takka
 */

#ifndef WEB_H_
#define WEB_H_

int web_get_file(const char *path, const char *url);
int web_get_file_time(time_t *utc, const char *url);
int net_connect();
int net_disconnect();

#endif /* WEB_H_ */
