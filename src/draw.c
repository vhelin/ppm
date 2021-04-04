
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkkeysyms.h>

#include "include/defines.h"
#include "include/main.h"
#include "include/prefs.h"
#include "include/draw.h"
#include "include/color.h"
#include "include/refresh.h"
#include "include/map.h"
#include "include/mouse.h"
#include "include/export.h"
#include "include/meta.h"


extern struct metatile_16x16 *mt_first, *mt_last;
extern unsigned char memory_data[8 * 8 * 1024];
extern unsigned char palette_data[1024];
extern unsigned char palette_colors[3 * 4 * 8];
extern guchar rgb_buffer[EDIT_WIDTH * EDIT_HEIGHT * 3];
extern guchar memory_buffer[512 * 512 * 3];
extern guchar tile_buffer[96 * 2 * 96 * 2 * 3];
extern guchar meta_buffer[64 * 64 * 3];
extern guchar palette_buffer_1[PBX * PBY * 3], palette_buffer_2[PBX * PBY * 3], palette_buffer_3[PBX * PBY * 3], palette_buffer_4[PBX * PBY * 3];
extern guchar palette_buffer_5[PBX * PBY * 3], palette_buffer_6[PBX * PBY * 3], palette_buffer_7[PBX * PBY * 3], palette_buffer_8[PBX * PBY * 3];
extern unsigned int x, y;
extern unsigned int edit_mode, edit_size_x, edit_size_y;
extern unsigned int edit_grid, memory_size;
extern unsigned int zoom_mode, zoom_size, tile_zoom_mode, tile_zoom_size;
extern unsigned int tile_window_status, metatile_window_status;
extern unsigned int old_memory_rec_x, old_memory_rec_y;
extern int metatile, metatiles, tile_source;

unsigned char tile_data[96 * 96], tile_palette_data[12 * 12];



int draw_edit_pixel(int x, int y, int a, int b) {

  unsigned char cr, cg, cb;
  unsigned int c, d, z, p, k;
  guchar *q;


  c = memory_data[b * 256 + a];
  z = palette_data[(b >> 3) * 32 + (a >> 3)];
  cr = palette_colors[z * 3 * 4 + c * 3];
  cg = palette_colors[z * 3 * 4 + c * 3 + 1];
  cb = palette_colors[z * 3 * 4 + c * 3 + 2];

  if (edit_grid == ON) {
    if (edit_mode == EDIT_MODE_8) {
      p = (1 + y * 36) * 3 * EDIT_WIDTH + (1 + x * 36) * 3;
      c = 35;
      d = 35 * 3;
    }
    else if (edit_mode == EDIT_MODE_8x16) {
      p = (1 + y * 18) * 3 * EDIT_WIDTH + (1 + x * 18) * 3;
      c = 17;
      d = 17 * 3;
    }
    else if (edit_mode == EDIT_MODE_16) {
      p = (1 + y * 18) * 3 * EDIT_WIDTH + (1 + x * 18) * 3;
      c = 17;
      d = 17 * 3;
    }
    else {
      p = (1 + y * 9) * 3 * EDIT_WIDTH + (1 + x * 9) * 3;
      c = 8;
      d = 8 * 3;
    }
  }
  else {
    if (edit_mode == EDIT_MODE_8) {
      p = (y * 36) * 3 * EDIT_WIDTH + (x * 36) * 3;
      c = 36;
      d = 36 * 3;
    }
    else if (edit_mode == EDIT_MODE_8x16) {
      p = (y * 18) * 3 * EDIT_WIDTH + (x * 18) * 3;
      c = 18;
      d = 18 * 3;
    }
    else if (edit_mode == EDIT_MODE_16) {
      p = (y * 18) * 3 * EDIT_WIDTH + (x * 18) * 3;
      c = 18;
      d = 18 * 3;
    }
    else {
      p = (y * 9) * 3 * EDIT_WIDTH + (x * 9) * 3;
      c = 9;
      d = 9 * 3;
    }
  }

  q = rgb_buffer + p;

  for (z = 0; z < c; z++) {
    for (k = 0; k < d; ) {
      q[k++] = cr;
      q[k++] = cg;
      q[k++] = cb;
    }
    q += 3 * EDIT_WIDTH;
  }

  return SUCCEEDED;
}


int draw_edit_pixel_8(int a, int b, int x, int y) {

  int c, d;


  c = (a / edit_size_x) * edit_size_x;
  d = (b / edit_size_y) * edit_size_y;

  for (b = d; b < (d + (int)edit_size_y); b++)
    for (a = c; a < (c + (int)edit_size_x); a++)
      draw_edit_pixel(a, b, a + x, b + y);

  return SUCCEEDED;
}


int draw_memory_pixel_8(int x, int y) {

  int a, b;


  x = (x>>3)<<3;
  y = (y>>3)<<3;

  for (a = y; a < y + 8; a++)
    for (b = x; b < x + 8; b++)
      draw_memory_pixel(b, a);

  draw_grid_rectangle();

  return SUCCEEDED;
}


int draw_edit_area(void) {

  unsigned int a, b;


  if (edit_grid == ON)
    memset(rgb_buffer, 0, EDIT_WIDTH * EDIT_HEIGHT * 3);

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      draw_edit_pixel(b, a, b + x, a + y);

  return SUCCEEDED;
}


int draw_memory_pixel(int x, int y) {

  unsigned char r, g, b;
  unsigned int c, z, q;


  c = memory_data[(y<<8) + x];
  z = palette_data[((y>>3)<<5) + (x>>3)];
  q = z * 12 + c * 3;
  r = palette_colors[q++];
  g = palette_colors[q++];
  b = palette_colors[q];

  draw_memory_pixel_plot(x, y, r, g, b);

  return SUCCEEDED;
}


int draw_memory_pixel_plot(int x, int y, int r, int g, int b) {

  unsigned int q;


  if (zoom_mode == ZOOM_MODE_2) {
    q = y * 512 * 6 + x * 6;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q++] = b;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q] = b;
    q += 512 * 3 - 5;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q++] = b;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q] = b;
  }
  else if (zoom_mode == ZOOM_MODE_3) {
    q = y * 512 * 6 + x * 6;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q++] = b;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q] = b;
    q += 512 * 3 - 5;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q++] = b;
    memory_buffer[q++] = r - (r/5);
    memory_buffer[q++] = g - (g/5);
    memory_buffer[q] = b - (b/5);
  }
  else {
    q = y * 256 * 3 + x * 3;
    memory_buffer[q++] = r;
    memory_buffer[q++] = g;
    memory_buffer[q] = b;
  }

  return SUCCEEDED;
}


int draw_memory_area(void) {

  unsigned int a;
  int b, c, q, z, nr, ng, nb, zx, zy;


  for (a = 0; a < memory_size; a += 8) {
    for (b = 0; b < 256; b += 8) {
      z = palette_data[(a<<2) + (b>>3)] * 12;
      for (zy = 0; zy < 8; zy++)
	for (zx = 0; zx < 8; zx++) {
	  c = memory_data[((a+zy)<<8) + b+zx];
	  q = z + c*3;
	  nr = palette_colors[q++];
	  ng = palette_colors[q++];
	  nb = palette_colors[q];
	  draw_memory_pixel_plot(b+zx, a+zy, nr, ng, nb);
	}
    }
  }

  if (adjust_memory_rectangle_position(x, y) == SUCCEEDED) {
    set_edit_window_title();
    set_memory_window_title();
    draw_tile_area();
    draw_edit_area();
    refresh_edit_draw_area();
    refresh_tile_draw_area();
  }

  draw_grid_rectangle();

  return SUCCEEDED;
}


void memory_rectangle_draw(void) {

  unsigned int a, b;


  /* clear the old rectangle */
  for (b = old_memory_rec_y; b < old_memory_rec_y + edit_size_y; b++) {
    draw_memory_pixel(old_memory_rec_x, b);
    draw_memory_pixel(old_memory_rec_x + edit_size_x - 1, b);
  }
  for (a = old_memory_rec_x + 1; a < old_memory_rec_x + edit_size_x - 1; a++) {
    draw_memory_pixel(a, old_memory_rec_y);
    draw_memory_pixel(a, old_memory_rec_y + edit_size_y - 1);
  }

  /* draw the new rectangle */
  draw_grid_rectangle();
}


int draw_grid_rectangle(void) {

  unsigned int a;


  for (a = y; a < y + edit_size_y; a++) {
    draw_memory_pixel_plot(x, a, 0, 0, 0);
    draw_memory_pixel_plot(x + edit_size_x - 1, a, 0, 0, 0);
  }
  for (a = x + 1; a < x + edit_size_x - 1; a++) {
    draw_memory_pixel_plot(a, y, 0, 0, 0);
    draw_memory_pixel_plot(a, y + edit_size_y - 1, 0, 0, 0);
  }

  old_memory_rec_x = x;
  old_memory_rec_y = y;

  return SUCCEEDED;
}


int draw_tile_area(void) {

  unsigned int a, c;
  int b, d, e, f;


  if (tile_window_status == OFF)
    return FAILED;

  /* tile memory blocks */
  if (tile_source == TILE_SOURCE_MEM) {
    for (a = 96 / edit_size_y, b = 0; a > 0; a--, b += edit_size_y) {
      for (c = 96 / edit_size_x, d = 0; c > 0; c--, d += edit_size_x) {
	for (e = 0; e < ((int)edit_size_y); e++) {
	  for (f = 0; f < ((int)edit_size_x); f++) {
	    tile_data[(e + b) * 96 + f + d] = memory_data[((y + e)<<8) + x + f];
	    tile_palette_data[((e + b)>>3) * 12 + ((f + d)>>3)] = palette_data[(((y + e)>>3)<<5) + ((x + f)>>3)];
	  }
	}
      }
    }
  }
  /* TO DO: tile metaile blocks */
  else {

  }

  for (a = 0; a < 96; a++)
    for (b = 0; b < 96; b++)
      draw_tile_pixel(b, a);

  return SUCCEEDED;
}


int draw_tile_pixel(int x, int y) {

  unsigned char r, g, b;
  unsigned int c, z, q;


  c = tile_data[y * 96 + x];
  z = tile_palette_data[(y / 8) * 12 + (x / 8)];
  q = z * 12 + c * 3;
  r = palette_colors[q++];
  g = palette_colors[q++];
  b = palette_colors[q];

  if (tile_zoom_mode == ZOOM_MODE_3) {
    q = y * 96 * 12 + x * 6;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q++] = b;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q] = b;
    q += 96 * 6 - 5;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q++] = b;
    tile_buffer[q++] = r - (r/5);
    tile_buffer[q++] = g - (g/5);
    tile_buffer[q] = b - (b/5);
  }
  else if (tile_zoom_mode == ZOOM_MODE_2) {
    q = y * 96 * 12 + x * 6;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q++] = b;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q] = b;
    q += 96 * 6 - 5;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q++] = b;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q] = b;
  }
  else {
    q = y * 96 * 3 + x * 3;
    tile_buffer[q++] = r;
    tile_buffer[q++] = g;
    tile_buffer[q] = b;
  }

  return SUCCEEDED;
}


int draw_and_refresh_all(void) {

  draw_edit_area();
  draw_memory_area();
  draw_tile_area();
  draw_map_area();
  draw_meta_area();
  refresh_draw_data();

  return SUCCEEDED;
}


int draw_color_boxes(void) {

  unsigned int a, b;


  for (a = 0; a < 8; a++)
    for (b = 0; b < 4; b++)
      draw_color_box(a, b);

  return SUCCEEDED;
}


int draw_color_box(int palette, int color) {

  unsigned char r, g, b;
  unsigned int i, k, l;
  guchar *p = NULL;


  p = get_palette_buffer(palette);
  i = (PE + color * (PM + PX)) * 3 + PE * PBX * 3;

  r = palette_colors[palette * 3 * 4 + color * 3 + 0];
  g = palette_colors[palette * 3 * 4 + color * 3 + 1];
  b = palette_colors[palette * 3 * 4 + color * 3 + 2];

  for (k = 0; k < PY; k++)
    for (l = 0; l < PX; l++) {
      p[k * PBX * 3 + l * 3 + i + 0] = r;
      p[k * PBX * 3 + l * 3 + i + 1] = g;
      p[k * PBX * 3 + l * 3 + i + 2] = b;
    }

  return SUCCEEDED;
}


int draw_button_color_indicator(unsigned int button, unsigned int palette, unsigned int color) {

  unsigned int a, b, c, d;
  guchar *p = NULL;


  p = get_palette_buffer(palette);

  if (color == 0)
    a = PE;
  else if (color == 1)
    a = PE + PX + PM;
  else if (color == 2)
    a = PE + PX + PM + PX + PM;
  else
    a = PE + PX + PM + PX + PM + PX + PM;

  b = PE + 2;

  if (button == 1) {
    a += (PX>>1) - 1;
    for (c = 200; b < PE + PY - 2; b++, c -= 8) {
      p[b * PBX * 3 + a * 3 + 0] = c;
      p[b * PBX * 3 + a * 3 + 1] = c;
      p[b * PBX * 3 + a * 3 + 2] = c;
      p[b * PBX * 3 + a * 3 + 3] = c;
      p[b * PBX * 3 + a * 3 + 4] = c;
      p[b * PBX * 3 + a * 3 + 5] = c;
    }
  }
  else if (button == 2) {
    a += (PX>>1) - 1 - 2;
    d = b;
    for (c = 200; b < PE + PY - 2; b++, c -= 8) {
      p[b * PBX * 3 + a * 3 + 0] = c;
      p[b * PBX * 3 + a * 3 + 1] = c;
      p[b * PBX * 3 + a * 3 + 2] = c;
      p[b * PBX * 3 + a * 3 + 3] = c;
      p[b * PBX * 3 + a * 3 + 4] = c;
      p[b * PBX * 3 + a * 3 + 5] = c;
    }
    a += 3;
    b = d;
    for (c = 200; b < PE + PY - 2; b++, c -= 8) {
      p[b * PBX * 3 + a * 3 + 0] = c;
      p[b * PBX * 3 + a * 3 + 1] = c;
      p[b * PBX * 3 + a * 3 + 2] = c;
      p[b * PBX * 3 + a * 3 + 3] = c;
      p[b * PBX * 3 + a * 3 + 4] = c;
      p[b * PBX * 3 + a * 3 + 5] = c;
    }
  }
  else {
    a += (PX>>1) - 2 - 2;
    d = b;
    for (c = 200; b < PE + PY - 2; b++, c -= 8) {
      p[b * PBX * 3 + a * 3 + 0] = c;
      p[b * PBX * 3 + a * 3 + 1] = c;
      p[b * PBX * 3 + a * 3 + 2] = c;
      p[b * PBX * 3 + a * 3 + 3] = c;
      p[b * PBX * 3 + a * 3 + 4] = c;
      p[b * PBX * 3 + a * 3 + 5] = c;
    }
    a += 3;
    b = d;
    for (c = 200; b < PE + PY - 2; b++, c -= 8) {
      p[b * PBX * 3 + a * 3 + 0] = c;
      p[b * PBX * 3 + a * 3 + 1] = c;
      p[b * PBX * 3 + a * 3 + 2] = c;
      p[b * PBX * 3 + a * 3 + 3] = c;
      p[b * PBX * 3 + a * 3 + 4] = c;
      p[b * PBX * 3 + a * 3 + 5] = c;
    }
    a += 3;
    b = d;
    for (c = 200; b < PE + PY - 2; b++, c -= 8) {
      p[b * PBX * 3 + a * 3 + 0] = c;
      p[b * PBX * 3 + a * 3 + 1] = c;
      p[b * PBX * 3 + a * 3 + 2] = c;
      p[b * PBX * 3 + a * 3 + 3] = c;
      p[b * PBX * 3 + a * 3 + 4] = c;
      p[b * PBX * 3 + a * 3 + 5] = c;
    }
  }

  return SUCCEEDED;
}


gchar *get_palette_buffer(int palette) {

  switch (palette) {
  case 0:
    return palette_buffer_1;
  case 1:
    return palette_buffer_2;
  case 2:
    return palette_buffer_3;
  case 3:
    return palette_buffer_4;
  case 4:
    return palette_buffer_5;
  case 5:
    return palette_buffer_6;
  case 6:
    return palette_buffer_7;
  case 7:
    return palette_buffer_8;
  }

  return NULL;
}


int draw_meta_area(void) {

  struct metatile_16x16 *m;
  int i, k, p;


  if (metatile_window_status == OFF)
    return FAILED;

  if (metatiles == 0) {
    for (k = 0, i = 0; i < 16; i++, k++) {
      for (p = 0; p < 16; p++, k++) {
	if ((k & 1) == 0)
	  draw_meta_pixel_plot(p, i, 100, 100, 100);
	else
	  draw_meta_pixel_plot(p, i, 70, 70, 70);
      }
    }
  }
  else {
    m = mt_first;
    for (i = 1; i != metatile; i++)
      m = m->next;
    draw_meta_block(m->t1, m->p1, 0, 0);
    draw_meta_block(m->t2, m->p2, 8, 0);
    draw_meta_block(m->t3, m->p3, 0, 8);
    draw_meta_block(m->t4, m->p4, 8, 8);
  }

  return SUCCEEDED;
}


void draw_meta_pixel_plot(int x, int y, int r, int g, int b) {

  unsigned int q, i;


  q = y * 16 * 3 * 4 * 4 + x * 3 * 4;
  for (i = 0; i < 4; i++) {
    meta_buffer[q++] = r;
    meta_buffer[q++] = g;
    meta_buffer[q++] = b;
    meta_buffer[q++] = r;
    meta_buffer[q++] = g;
    meta_buffer[q++] = b;
    meta_buffer[q++] = r;
    meta_buffer[q++] = g;
    meta_buffer[q++] = b;
    meta_buffer[q++] = r;
    meta_buffer[q++] = g;
    meta_buffer[q] = b;
    q += 16 * 3 * 4 - 11;
  }
}


void draw_meta_block(int a, int b, int x, int y) {

  unsigned char nr, ng, nb;
  unsigned int c, i, p, t, k;


  /* block data start */
  for (t = 0, i = a; i > 31; ) {
    i -= 32;
    t += 8 * 256;
  }
  t += i<<3;

  /* palette */
  c = (b & 7) * 12;

  /* x-flip + y-flip? */
  if (b & 8 && b & 16) {
    for (i = 0; i < 8; i++) {
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_meta_pixel_plot(x + (7 - p), y + (7 - i), nr, ng, nb);
      }
    }
  }
  /* x-flip + no y-flip? */
  else if (b & 8 && !(b & 16)) {
    for (i = 0; i < 8; i++) {
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_meta_pixel_plot(x + (7 - p), y + i, nr, ng, nb);
      }
    }
  }
  /* no x-flip + no y-flip? */
  else if (!(b & 24)) {
    for (i = 0; i < 8; i++) {
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_meta_pixel_plot(x + p, y + i, nr, ng, nb);
      }
    }
  }
  /* no x-flip + y-flip? */
  else {
    for (i = 0; i < 8; i++) {
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_meta_pixel_plot(x + p, y + (7 - i), nr, ng, nb);
      }
    }
  }
}
