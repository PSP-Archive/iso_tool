/*
 * web.c
 *
 *  Created on: 2010/2/23
 *      Author: takka
 */

#include <pspiofilemgr.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <psputility.h>

#include <curl/curl.h>
#include "web.h"
#include "screen.h"
#include "sound.h"

static int web_write(void* buf, size_t size, size_t num, int fp)
{
  int ret;
  ret = sceIoWrite(fp, buf, (int)size * num);
  return ret;
}

static void netInit(void)
{
    sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
    sceNetInetInit();
    sceNetApctlInit(0x8000, 48);
}

static void netTerm(void)
{
    sceNetApctlTerm();
    sceNetInetTerm();
    sceNetTerm();
}

int net_connect()
{
  netInit();
  net_dialog();

  return 0;
}

int net_disconnect()
{
  netTerm();
  return 0;
}

int web_get_file(const char *path, const char *url)
{
  SceUID fp;
  CURL *curl;
  CURLcode res;
  long long int http_code = 0;

  //ファイルのオープン
  fp = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

  //初期化
  curl = curl_easy_init();
  //URL設定
  curl_easy_setopt(curl, CURLOPT_URL, url);

  //関数を登録
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, web_write);
  curl_easy_setopt(curl, CURLOPT_CRLF, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

  //実行
  res = curl_easy_perform(curl);

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  curl_easy_cleanup(curl);

  sceIoClose(fp);

  if((res == CURLE_OK)&&(http_code == 200))
//    res = 0;
    beep();
  else
    res = -1;

  return res;
}

int web_get_file_time(time_t *utc, const char *url)
{
  CURL *curl;
  CURLcode res;

  //初期化
  *utc = 0;
  curl = curl_easy_init();

  //URL設定
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FILETIME, 1);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
  //実行
  res = curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_FILETIME, utc);
  curl_easy_cleanup(curl);

  if(res == CURLE_OK)
    res = 0;
  else
    res = -1;

  return res;
}

