/*
 * sound.c
 *
 *  Created on: 2010/01/05
 *      Author: takka
 */

#include <pspaudio.h>
#include <pspaudiolib.h>

#include "beep_data.c"

static int snd = 0;

/*---------------------------------------------------------------------------
  サウンドの初期化
---------------------------------------------------------------------------*/
void init_sound()
{
  snd = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_AUDIO_SAMPLE_ALIGN(size_beep_data / 4), PSP_AUDIO_FORMAT_MONO);
}

/*---------------------------------------------------------------------------
  サウンドの解放
---------------------------------------------------------------------------*/
void free_sound()
{
  sceAudioChRelease(snd);
}

/*---------------------------------------------------------------------------
  例の音を鳴らす
---------------------------------------------------------------------------*/
void beep()
{
  sceAudioOutputBlocking(snd, PSP_AUDIO_VOLUME_MAX, beep_data);
}
