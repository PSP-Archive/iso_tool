// fnt_print.c

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <pspiofilemgr.h>
#include "fnt_print.h"

#define MAX_STR (2048)

static u32 nextx;
static u32 nexty;
static int max_x_length;
static int max_y_length;

static void *fnt_malloc(u32 size);
static s32 fnt_mfree(void *ptr);

static int (*fnt_print_sub)(const fnt_t* font, u32 ucs2, void *vram, u32 bufferwidth, u32 color, u32 back, u32 fill, u32 rate, u32 mx, u32 my);
static u32 (*fnt_colmix)(void *vram, u32 color, s32 rate);

static void fnt_read_header(const u8* fnt_ptr, fnt_t *font);

static int fnt_print_sub16(const fnt_t* font, u32 ucs2, void *vram, u32 bufferwidth, u32 color, u32 back, u32 fill, u32 rate, u32 mx, u32 my);
static int fnt_print_sub32(const fnt_t* font, u32 ucs2, void *vram, u32 bufferwidth, u32 color, u32 back, u32 fill, u32 rate, u32 mx, u32 my);

static u32 fnt_colmix_565(void *vram, u32 color, s32 rate);
static u32 fnt_colmix_5551(void *vram, u32 color, s32 rate);
static u32 fnt_colmix_4444(void *vram, u32 color, s32 rate);
static u32 fnt_colmix_8888(void *vram, u32 color, s32 rate);
static u32 fnt_colmixrev_565(void *vram, u32 color, s32 rate);
static u32 fnt_colmixrev_5551(void *vram, u32 color, s32 rate);
static u32 fnt_colmixrev_4444(void *vram, u32 color, s32 rate);
static u32 fnt_colmixrev_8888(void *vram, u32 color, s32 rate);

/*---------------------------------------------------------------------------
  フォントをファイルからロード
    *path: フォントファイルのパス
    *font: font_tへのポインタ
    return: ファイルサイズ
            -1 指定ファイル無し / -2 メモリ確保失敗 / -3 読込失敗
---------------------------------------------------------------------------*/
s32 fnt_load_file(const void* path, fnt_t *font)
{
  SceIoStat stat;
  SceUID fp;
  u32 size;

  if(sceIoGetstat(path, &stat) < 0)
    return -1;

  font->fnt_ptr = fnt_malloc(stat.st_size);
  if(font->fnt_ptr <0)
    return -2;

  font->file_flag = 1;

  fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
  if(fp < 0)
    return -3;

  size = sceIoRead(fp, font->fnt_ptr, stat.st_size);
  sceIoClose(fp);

  if(size != stat.st_size)
    return -3;

  fnt_read_header(font->fnt_ptr, font);
  return stat.st_size;
}

/*---------------------------------------------------------------------------
  メモリ上のフォントをロード
    *ptr: フォントデータへのポインタ
    *font: fnt_tへのポインタ
---------------------------------------------------------------------------*/
void fnt_load_mem(const void* ptr, fnt_t *font)
{
  font->file_flag = 0;

  font->fnt_ptr = (void *)ptr;

  fnt_read_header(font->fnt_ptr, font);
}

/*---------------------------------------------------------------------------
  フォントの解放
  ファイルからロードした場合は、メモリも解放する
    *font: fnt_tへのポインタ
    return: エラーの場合は負を返す
---------------------------------------------------------------------------*/
s32 fnt_free(fnt_t *font)
{
  if(font->file_flag == 1)
    return fnt_mfree(font->fnt_ptr);

  return 0;
}

/*---------------------------------------------------------------------------
  メモリ確保
    size: 利用メモリサイズ
    return: 確保したメモリへのポインタ
            エラーの場合はNULLを返す
---------------------------------------------------------------------------*/
static void *fnt_malloc(u32 size)
{
  u32 *p;
  u32 h_block;

  if(size == 0) return NULL;

  h_block = sceKernelAllocPartitionMemory(2, "block", 0, size + sizeof(h_block), NULL);

  if(h_block < 0) return NULL;

  p = (u32 *)sceKernelGetBlockHeadAddr(h_block);
  *p = h_block;

  return (void *)(p + 1);
}

/*---------------------------------------------------------------------------
  メモリ解放
    *ptr: 確保したメモリへのポインタ
    return: エラーの場合は負の値を返す
---------------------------------------------------------------------------*/
static s32 fnt_mfree(void *ptr)
{
  return sceKernelFreePartitionMemory((SceUID)*((u32 *)ptr - 1));
}

/*---------------------------------------------------------------------------
  フォント ヘッダ読込み
    *fnt_ptr: フォントデーターへのポインタ
    *font: fnt_tへのポインタ
---------------------------------------------------------------------------*/
static void fnt_read_header(const u8* fnt_ptr, fnt_t *font)
{
  u32 *u32_ptr;
  u16 *u16_ptr;
  u32 pad;
  u32 shift;

  u16_ptr = (u16 *)&fnt_ptr[4];
  font->maxwidth    = *u16_ptr;
  u16_ptr = (u16 *)&fnt_ptr[6];
  font->height      = *u16_ptr;
  u16_ptr = (u16 *)&fnt_ptr[8];
  font->ascent      = *u16_ptr;
  u32_ptr = (u32 *)&fnt_ptr[12];
  font->firstchar   = *u32_ptr;
  u32_ptr = (u32 *)&fnt_ptr[16];
  font->defaultchar = *u32_ptr;
  u32_ptr = (u32 *)&fnt_ptr[20];
  font->size        = *u32_ptr;
  u32_ptr = (u32 *)&fnt_ptr[24];
  font->nbits     = *u32_ptr;
  u32_ptr = (u32 *)&fnt_ptr[28];
  font->noffset   = *u32_ptr;
  u32_ptr = (u32 *)&fnt_ptr[32];
  font->nwidth    = *u32_ptr;
  font->bits = (const u8 *)(&fnt_ptr[36]);

  if(font->nbits < 0xFFDB)
  {
    pad = 1;
    font->long_offset = 0;
    shift = 1;
  }
  else
  {
    pad = 3;
    font->long_offset = 1;
    shift = 2;
  }

#ifdef USE_FIX
  font->fix_flag = 0;
#endif

  if(font->noffset != 0)
    font->offset = (u32 *)(((u32)font->bits + font->nbits + pad) & ~pad);
  else
  {
    font->offset = NULL;
#ifdef USE_FIX
    font->fix_flag = 1;
#endif
  }

  if(font->nwidth != 0)
    font->width = (u8 *)((u32)font->offset + (font->noffset << shift));
  else
  {
    font->width = NULL;
#ifdef USE_FIX
    font->fix_flag = 1;
#endif
  }

#ifdef USE_FIX
  font->halfchar = 0xFF; // 0～0xFF までは半角として扱う
  font->fix_wide = font->maxwidth;
  font->fix_half = font->fix_wide / 2;
#endif
}

/*---------------------------------------------------------------------------
  文字列の幅を得る
    *font: fnt_tへのポインタ
    *str: 文字列へのポインタ(UTF-8N)
    return: 文字列の幅
---------------------------------------------------------------------------*/
u32 fnt_get_width(const fnt_t* font, const char *str, int mx)
{
  u16 i;
  u16 len;
  u16 width = 0;
  u16 ucs[MAX_STR];

  utf8_to_utf16(ucs, str);
  len = utf16len(ucs);

  for (i = 0; i < len; i++)
    width += fnt_get_width_ucs2(font ,ucs[i], 1);

  return width * mx;
}

/*---------------------------------------------------------------------------
  文字の幅を得る
    *font: fnt_tへのポインタ
    ucs2: 文字(UCS2)
    return: 文字の幅
---------------------------------------------------------------------------*/
u32 fnt_get_width_ucs2(const fnt_t* font, u32 ucs2, int mx)
{
  u16 width = 0;

  if((ucs2 < font->firstchar) || (ucs2 >= font->firstchar + font->size))
    ucs2 = font->defaultchar;

  if((font->nwidth != 0)
#ifdef USE_FIX
      && (font->fix_flag == 0)
#endif
      )
    width = font->width[ucs2 - font->firstchar];
  else
  {
#ifdef USE_FIX
    if(font->nwidth == 0)
    {
      if(ucs2 > font->halfchar)
#endif
        width = font->maxwidth;
#ifdef USE_FIX
      else
        width = font->maxwidth / 2;
    }
    else
    {
      if(ucs2 > font->halfchar)
        width = font->fix_wide;
      else
        width = font->fix_half;
    }
#endif
  }

  return width * mx;
}

/*---------------------------------------------------------------------------
  グリフデータを得る
    *font: fnt_tへのポインタ
    ucs2: 文字(UCS2)
    return: グリフデータへのポインタ
---------------------------------------------------------------------------*/
u8* fnt_get_bits(const fnt_t* font, u32 ucs2)
{
    u8* bits;

    if((ucs2 < font->firstchar) || (ucs2 >= font->firstchar + font->size))
      ucs2 = font->defaultchar;

    ucs2 -= font->firstchar;

    if(font->long_offset == 0)
    {
      bits = (u8 *)font->bits + (font->offset?
          ((u16*)(font->offset))[ucs2]:
          (((font->height + 7) / 8) * font->maxwidth * ucs2));
    }
    else
    {
      bits = (u8 *)font->bits + (font->offset?
          ((u32*)(font->offset))[ucs2]:
          (((font->height + 7) / 8) * font->maxwidth * ucs2));
    }

    return bits;
}

/*---------------------------------------------------------------------------
  fnt_print_sub用のマクロ定義
---------------------------------------------------------------------------*/
#ifndef USE_FIX
#define fnt_print_sub_body(depth)                                          \
{                                                                          \
  u32 dx, dy;                                                              \
  u32 lx, ly;                                                              \
  u8* index;                                                               \
  u8* index_tmp;                                                           \
  u16 shift;                                                               \
  u8 pt;                                                                   \
  depth *vptr_tmp;                                                         \
  depth *vptr;                                                             \
  u32 temp = 0;                                                            \
  u32 width;                                                               \
                                                                           \
  width = fnt_get_width_ucs2(font, ucs2, 1);                               \
  if(nextx + width * mx > max_x_length)                                    \
    return -1;                                                            \
                                                                           \
  index = fnt_get_bits(font, ucs2);                                        \
  vptr_tmp = (depth *)vram + nextx + nexty * bufferwidth;                  \
                                                                           \
  for (dx = 0; dx < width; dx++) /* x loop */                              \
  {                                                                        \
    for(lx = 0; lx < mx; lx++) /* mx loop */                               \
    {                                                                      \
      index_tmp = index;                                                   \
      shift = 0;                                                           \
      vptr = vptr_tmp;                                                     \
      pt = *index;                                                         \
                                                                           \
      for(dy = 0; dy < font->height; dy++) /* y loop */                    \
      {                                                                    \
        if(shift >= 8)                                                     \
        {                                                                  \
          shift = 0;                                                       \
          index_tmp += width;                                              \
          pt = *index_tmp;                                                 \
        }                                                                  \
                                                                           \
        if(pt & 0x01)                                                      \
        {                                                                  \
          if((fill & 0x01) && (rate > 0))                                  \
            temp = color;                                                  \
        }                                                                  \
        else                                                               \
        {                                                                  \
          if(fill & 0x10)                                                  \
            temp = back;                                                   \
        }                                                                  \
                                                                           \
        for(ly = 0; ly < my; ly++) /* my loop */                           \
        {                                                                  \
          *vptr = (rate < 100) ? fnt_colmix(vptr, temp, rate) : temp;      \
          vptr += bufferwidth;                                             \
        } /* my loop */                                                    \
        shift++;                                                           \
        pt >>= 1;                                                          \
      } /* y loop */                                                       \
      vptr_tmp++;                                                          \
    } /* mx loop */                                                        \
    index++;                                                               \
  } /* x loop */                                                           \
  nextx = nextx + width * mx;                                              \
  return 0;                                                                \
}                                                                          \

#else
#define fnt_print_sub_body(depth)                                          \
{                                                                          \
  u32 dx, dy;                                                              \
  u32 lx, ly;                                                              \
  u8* index;                                                               \
  u8* index_tmp;                                                           \
  u16 shift;                                                               \
  u8 pt;                                                                   \
  depth *vptr_tmp;                                                         \
  depth *vptr;                                                             \
  u32 temp = 0;                                                            \
  fnt_t font_temp;                                                         \
  u32 width,width_2;                                                       \
  u32 b_1, b_2;                                                            \
  u32 mode;                                                                \
                                                                           \
  if(font->fix_flag == 0)                                                  \
  {                                                                        \
    width = fnt_get_width_ucs2(font, ucs2);                                \
    width_2 = width;                                                       \
  }                                                                        \
  else                                                                     \
  {                                                                        \
    font_temp.fix_flag = 0;                                                \
    font_temp.nwidth = font->nwidth;                                       \
    font_temp.width = font->width;                                         \
    font_temp.firstchar = font->firstchar;                                 \
    font_temp.halfchar = font->halfchar;                                   \
    font_temp.maxwidth = font->maxwidth;                                   \
    font_temp.fix_half = font->fix_half;                                   \
    font_temp.fix_wide = font->fix_wide;                                   \
    width = fnt_get_width_ucs2(&font_temp, ucs2);                          \
    width_2 = fnt_get_width_ucs2(font, ucs2);                              \
    if(width_2 < width)                                                    \
      width_2 = width;                                                     \
  }                                                                        \
                                                                           \
  b_1 = (width_2 - width) / 2;                                             \
  b_2 = b_1 + width;                                                       \
                                                                           \
  index = fnt_get_bits(font, ucs2);                                        \
  vptr_tmp = (depth *)vram + nextx + nexty * bufferwidth;                  \
                                                                           \
  for (dx = 0; dx < width_2; dx++) /* x loop */                            \
  {                                                                        \
    if((b_1 <= dx)&&(dx < b_2))                                            \
      mode = 1; /* フォント部 */                                           \
    else                                                                   \
      mode = 0; /* 余白部 */                                               \
                                                                           \
    for(lx = 0; lx < mx; lx++) /* mx loop */                               \
    {                                                                      \
      index_tmp = index;                                                   \
      shift = 0;                                                           \
      vptr = vptr_tmp;                                                     \
      if(mode == 1)                                                        \
        pt = *index;                                                       \
      else                                                                 \
        pt = 0;                                                            \
                                                                           \
      for(dy = 0; dy < font->height; dy++) /* y loop */                    \
      {                                                                    \
        if(shift >= 8)                                                     \
        {                                                                  \
          shift = 0;                                                       \
          index_tmp += width;                                              \
          pt = *index_tmp;                                                 \
        }                                                                  \
                                                                           \
        if(pt & 0x01)                                                      \
        {                                                                  \
          if((fill & 0x01) && (rate > 0))                                  \
            temp = color;                                                  \
        }                                                                  \
        else                                                               \
        {                                                                  \
          if(fill & 0x10)                                                  \
            temp = back;                                                   \
        }                                                                  \
                                                                           \
        for(ly = 0; ly < my; ly++) /* my loop */                           \
        {                                                                  \
          *vptr = (rate < 100) ? fnt_colmix(vptr, temp, rate) : temp;      \
          vptr += bufferwidth;                                             \
        } /* my loop */                                                    \
                                                                           \
        if(mode == 1)                                                      \
        {                                                                  \
          shift++;                                                         \
          pt >>= 1;                                                        \
        }                                                                  \
      } /* y loop */                                                       \
      vptr_tmp++;                                                          \
    } /* mx loop */                                                        \
    if(mode == 1)                                                          \
      index++;                                                             \
  } /* x loop */                                                           \
  nextx = nextx + width_2 * mx;                                            \
}                                                                          \

#endif

/*---------------------------------------------------------------------------
  一文字表示
    *font: fnt_tへのポインタ
    ucs2: 文字(UCS2)
    *vram: VRAMアドレス
    bufferwidth: VRAM横幅
    color: 文字色
    back: 背景色
    fill: 書込フラグ 0x01 文字部 0x10 背景部 0x11 両方
    rate: 混合比(-100～100)
    mx: 横方向倍率
    my: 縦方向倍率
---------------------------------------------------------------------------*/
static int fnt_print_sub16(const fnt_t* font, u32 ucs2, void *vram, u32 bufferwidth, u32 color, u32 back, u32 fill, u32 rate, u32 mx, u32 my)
fnt_print_sub_body(u16);

static int fnt_print_sub32(const fnt_t* font, u32 ucs2, void *vram, u32 bufferwidth, u32 color, u32 back, u32 fill, u32 rate, u32 mx, u32 my)
fnt_print_sub_body(u32);



/*---------------------------------------------------------------------------
  文字列表示(表示VRAMへの描画)
    *font: fnt_tへのポインタ
    x: 横方向位置
    y: 縦方向位置
    *str: 文字列(UTF8-N)へのポインタ('\n'は改行を行う)
    color: 文字色
    back: 背景色
    fill: 書込フラグ FNT_FONT_FILL(0x01) 文字部, FNT_BACK_FILL(0x10) 背景部
    rate: 混合比(-100～100)
    mx: 横方向倍率
    my: 縦方向倍率
---------------------------------------------------------------------------*/
u32 fnt_print_xy(const fnt_t* font, u16 x, u16 y, void *str, u32 color, u32 back, u8 fill, s16 rate, u16 mx, u16 my, int x_length, int y_length)
{
  void *vram;
  int  bufferwidth;
  int  pixelformat;
  int  pwidth;
  int  pheight;
  int  unk;

  sceDisplayGetMode(&unk, &pwidth, &pheight);
  sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, unk);

  if(vram == NULL)
    vram = (void*) (0x40000000 | (u32) sceGeEdramGetAddr());

  return fnt_print_vram(font, vram, bufferwidth, pixelformat, x, y, str, color, back, fill, rate, mx, my, x_length, y_length);
}

/*---------------------------------------------------------------------------
  文字列表示(指定したRAM/VRAMへの描画)
    *font: fnt_tへのポインタ
    *vram: VRAMアドレス
    bufferwidth: VRAM横幅
    pixelformat: ピクセルフォーマット (0=16bit, 1=15bit, 2=12bit, 3=32bit)
    x: 横方向位置
    y: 縦方向位置
    *str: 文字列(UTF8-N)へのポインタ('\n'は改行を行う)
    color: 文字色
    back: 背景色
    fill: 書込フラグ FNT_FONT_FILL(0x01) 文字部, FNT_BACK_FILL(0x10) 背景部
    rate: 混合比(-100～100) ※負の場合は減色
    mx: 横方向倍率
    my: 縦方向倍率
    return: エラー時は-1
---------------------------------------------------------------------------*/
u32 fnt_print_vram(const fnt_t* font, void *vram, u16 bufferwidth, u16 pixelformat, u16 x, u16 y, const void *str, u32 color, u32 back, u8 fill, s16 rate, u16 mx, u16 my, int x_length, int y_length)
{
  u16 i;
  u16 len;
  u16 ucs[MAX_STR];
  int ret = 0;

  if (bufferwidth == 0) return -1;

  if(x_length == 0)
    max_x_length = 480;
  else
    max_x_length = x + x_length;

  if(y_length == 0)
    max_y_length = 272;
  else
    max_y_length = y + y_length;

  nextx = x;
  nexty = y;

  switch (pixelformat)
  {
    case PSP_DISPLAY_PIXEL_FORMAT_565:
      fnt_print_sub = fnt_print_sub16;
      fnt_colmix = (rate < 0) ? fnt_colmixrev_565 : fnt_colmix_565;
      break;

    case PSP_DISPLAY_PIXEL_FORMAT_5551:
      fnt_print_sub = fnt_print_sub16;
      fnt_colmix = (rate < 0) ? fnt_colmixrev_5551 : fnt_colmix_5551;
      break;

    case PSP_DISPLAY_PIXEL_FORMAT_4444:
      fnt_print_sub = fnt_print_sub16;
      fnt_colmix = (rate < 0) ? fnt_colmixrev_4444 : fnt_colmix_4444;
      break;

    case PSP_DISPLAY_PIXEL_FORMAT_8888:
      fnt_print_sub = fnt_print_sub32;
      fnt_colmix = (rate < 0) ? fnt_colmixrev_8888 : fnt_colmix_8888;
      break;

    default:
      return -1;
  }

  if (rate < 0) rate = -rate - 1;
  if (rate > 100) rate = 100;

  // utf-8nをUCS2に変換
  utf8_to_utf16(ucs, str);
  len = utf16len(ucs);

  for (i = 0; i < len; i++)
  {
    if (ucs[i] == '\n')
    {
      nextx = x;
      nexty += font->height;
      if(nexty > max_y_length)
        return 0;
      ret = 0;
    }
    else
    {
      if(ret >= 0)
        ret = fnt_print_sub(font, ucs[i], vram, bufferwidth, color, back, fill, rate, mx, my);
    }
  }

  return 0;
}

/*---------------------------------------------------------------------------
  fnt_colmix～用のマクロ定義
---------------------------------------------------------------------------*/
#define fnt_colmix_BODY(TYPE, REV, Rm, Gm, Bm, Gs, Bs) \
{                                                      \
  u16 r1, g1, b1;                                      \
  u16 r2, g2, b2;                                      \
                                                       \
  r1 = color & Rm;                                     \
  g1 = (color >> Gs) & Gm;                             \
  b1 = (color >> Bs) & Bm;                             \
                                                       \
  r2 = REV(*(TYPE *)vr) & Rm;                          \
  g2 = REV(*(TYPE *)vr >> Gs) & Gm;                    \
  b2 = REV(*(TYPE *)vr >> Bs) & Bm;                    \
                                                       \
  r1 = ((r1 * rate) + (r2 * (100 - rate)) + 50) / 100; \
  g1 = ((g1 * rate) + (g2 * (100 - rate)) + 50) / 100; \
  b1 = ((b1 * rate) + (b2 * (100 - rate)) + 50) / 100; \
                                                       \
  return r1 | (g1 << Gs) | (b1 << Bs);                 \
}                                                      \

/*---------------------------------------------------------------------------
  VRAM カラー合成
    vr: VRAMアドレス
    color: 合成色
    rate: 混合比(-100～100)
    return: 合成色
---------------------------------------------------------------------------*/
static u32 fnt_colmix_565(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u16, +, 0x1f, 0x3f, 0x1f, 5, 11)

static u32 fnt_colmix_5551(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u16, +, 0x1f, 0x1f, 0x1f, 5, 10)

static u32 fnt_colmix_4444(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u16, +, 0x0f, 0x0f, 0x0f, 4, 8)

static u32 fnt_colmix_8888(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u32, +, 0xff, 0xff, 0xff, 8, 16)

static u32 fnt_colmixrev_565(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u16, ~, 0x1f, 0x3f, 0x1f, 5, 11)

static u32 fnt_colmixrev_5551(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u16, ~, 0x1f, 0x1f, 0x1f, 5, 10)

static u32 fnt_colmixrev_4444(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u16, ~, 0x0f, 0x0f, 0x0f, 4, 8)

static u32 fnt_colmixrev_8888(void *vr, u32 color, s32 rate)
fnt_colmix_BODY(u32, ~, 0xff, 0xff, 0xff, 8, 16)

/*---------------------------------------------------------------------------
  UFT-8Nの一文字をUTF-16に変換する
    *utf16: 変換後の文字(UTF-16)へのポインタ
    *utf8: UFT-8N文字へのポインタ
    return: UTF-8Nの次の文字へのポインタ
---------------------------------------------------------------------------*/
char* utf8_utf16(u16 *utf16, const char *utf8)
{
  u8 c = *utf8++;
  u16 code;
  s32 tail = 0;

  if((c <= 0x7f) || (c >= 0xc2))
  {
    /* Start of new character. */
    if(c < 0x80)
    {
      /* U-00000000 - U-0000007F, 1 byte */
      code = c;
    }
    else if(c < 0xe0)   /* U-00000080 - U-000007FF, 2 bytes */
    {
      tail = 1;
      code = c & 0x1f;
    }
    else if(c < 0xf0)   /* U-00000800 - U-0000FFFF, 3 bytes */
    {
      tail = 2;
      code = c & 0x0f;
    }
    else if(c < 0xf5)   /* U-00010000 - U-001FFFFF, 4 bytes */
    {
      tail = 3;
      code = c & 0x07;
    }
    else                /* Invalid size. */
    {
      code = 0xfffd;
    }

    while(tail-- && ((c = *utf8++) != 0))
    {
      if((c & 0xc0) == 0x80)
      {
        /* Valid continuation character. */
        code = (code << 6) | (c & 0x3f);

      }
      else
      {
        /* Invalid continuation char */
        code = 0xfffd;
        utf8--;
        break;
      }
    }
  }
  else
  {
    /* Invalid UTF-8 char */
    code = 0xfffd;
  }
  /* currently we don't support chars above U-FFFF */
  *utf16 = (code < 0x10000) ? code : 0xfffd;
  return (char*)utf8;
}

/*---------------------------------------------------------------------------
  UFT-8Nの文字列をUTF-16に変換する
    *utf16: 変換後の文字列(UTF-16)へのポインタ
    *utf8: UFT-8N文字列へのポインタ
---------------------------------------------------------------------------*/
void utf8_to_utf16(u16 *utf16, const char *utf8)
{
  while(*utf8 !='\0')
  {
    utf8 = utf8_utf16(utf16++, utf8);
  }
  *utf16 = '\0';
}

/*---------------------------------------------------------------------------
  UTF-16の文字列の長さを得る
    *utf16: 文字列(UTF-16)へのポインタ
    return: 文字数
---------------------------------------------------------------------------*/
u16 utf16len(const u16 *utf16)
{
  u16 len = 0;
  while(utf16[len] != '\0')
    len++;
  return len;
}

