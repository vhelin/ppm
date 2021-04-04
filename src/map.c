
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
#include "include/refresh.h"
#include "include/map.h"
#include "include/main.h"
#include "include/draw.h"


extern GtkAdjustment *map_adj1, *map_adj2;
extern GtkWidget *map_draw_area, *map_window;
extern guchar map_buffer[512 * 512 * 3];
extern unsigned char memory_data[8 * 8 * 1024];
extern unsigned char palette_data[1024];
extern unsigned char palette_colors[3 * 4 * 8];
extern unsigned char *map_property_data;
extern unsigned int map_zoom_size, x, y, map_zoom_mode, edit_size_x, edit_size_y, map_window_status, edit_mode, map_check_status;
extern unsigned int *map_data, memory_size;
extern struct metatile_16x16 *mt_first, *mt_last;
extern int metatiles, metatile, map_draw_mode;

unsigned char map_copy_property_buffer[4 * 4], map_undo_property_buffer[4 * 4];
unsigned int map_x = 0, map_y = 0, map_active_press = OFF, map_rec_x = 0, map_rec_y = 0, old_map_rec_x = 0, old_map_rec_y = 0;
unsigned int map_active_button, map_undo_x, map_undo_y, map_undo_size_x, map_undo_size_y, map_copy_size_x, map_copy_size_y;
unsigned int map_copy_tile_buffer[4 * 4], map_undo_tile_buffer[4 * 4], map_paste_status = OFF, map_control_value = 0;



void draw_map_area(void) {

  unsigned int a, b;


  if (map_window_status == OFF)
    return;

  for (a = 0; a < 32; a++)
    for (b = 0; b < 32; b++)
      draw_map_block(b, a);

  map_draw_rectangle();
}


void draw_map_block(unsigned int sx, unsigned int sy) {

  unsigned char nr, ng, nb;
  unsigned int a, b, c, i, p, t, k;


  /* touched? */
  if (!(map_property_data[(sy + map_y) * 1024 + sx + map_x] & 128)) {
    for (k = 0, i = 0; i < 8; i++, k++)
      for (p = 0; p < 8; p++, k++) {
	if ((k & 1) == 0)
	  draw_map_pixel_plot(sx * 8 + p, sy * 8 + i, 100, 100, 100);
	else
	  draw_map_pixel_plot(sx * 8 + p, sy * 8 + i, 70, 70, 70);
      }
    return;
  }

  /* block data id */
  a = map_data[(sy + map_y) * 1024 + sx + map_x];

  /* block data start */
  for (t = 0, b = a; b > 31; ) {
    b -= 32;
    t += 8 * 256;
  }
  t += b * 8;

  /* block properties */
  b = map_property_data[(sy + map_y) * 1024 + sx + map_x];

  /* palette */
  c = (b & 7) * 12;

  sx = sx<<3;
  sy = sy<<3;

  /* x-flip + y-flip? */
  if (b & 8 && b & 16) {
    for (i = 0; i < 8; i++)
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_map_pixel_plot(sx + (7 - p), sy + (7 - i), nr, ng, nb);
      }
  }
  /* x-flip + no y-flip? */
  else if (b & 8 && !(b & 16)) {
    for (i = 0; i < 8; i++)
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_map_pixel_plot(sx + (7 - p), sy + i, nr, ng, nb);
      }
  }
  /* no x-flip + no y-flip? */
  else if (!(b & 24)) {
    for (i = 0; i < 8; i++)
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_map_pixel_plot(sx + p, sy + i, nr, ng, nb);
      }
  }
  /* no x-flip + y-flip? */
  else {
    for (i = 0; i < 8; i++)
      for (p = 0; p < 8; p++) {
	k = memory_data[t + (i<<8) + p];
	k = c + k * 3;
	nr = palette_colors[k++];
	ng = palette_colors[k++];
	nb = palette_colors[k];
	draw_map_pixel_plot(sx + p, sy + (7 - i), nr, ng, nb);
      }
  }
}


void draw_map_pixel_plot(int x, int y, int r, int g, int b) {

  unsigned int q;


  if (map_zoom_mode == ZOOM_MODE_2) {
    q = y * 512 * 6 + x * 6;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q++] = b;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q] = b;
    q += 512 * 3 - 5;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q++] = b;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q] = b;
  }
  else if (map_zoom_mode == ZOOM_MODE_3) {
    q = y * 512 * 6 + x * 6;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q++] = b;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q] = b;
    q += 512 * 3 - 5;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q++] = b;
    map_buffer[q++] = r - (r/5);
    map_buffer[q++] = g - (g/5);
    map_buffer[q] = b - (b/5);
  }
  else {
    q = y * 256 * 3 + x * 3;
    map_buffer[q++] = r;
    map_buffer[q++] = g;
    map_buffer[q] = b;
  }
}


void update_map(void) {

  unsigned int a, b, c, d, e;


  if (map_window_status == OFF)
    return;

  c = 0;
  d = ((y>>3)<<5) + (x>>3);

  for (a = 0; a < 32; a++)
    for (b = 0; b < 32; b++) {
      e = ((a + map_y)<<10) + b + map_x;
      if (find_tile(map_data[e], d) == SUCCEEDED && (map_property_data[e] & 128)) {
	draw_map_block(b, a);
	c++;
      }
    }

  if (c != 0) {
    map_draw_rectangle();
    refresh_map_draw_area();
  }
}


int find_tile(unsigned int d, unsigned int a) {

  if (edit_mode == EDIT_MODE_8) {
    if (d == a)
      return SUCCEEDED;
    return FAILED;
  }
  if (edit_mode == EDIT_MODE_8x16) {
    if (d == a || d == (a + 32))
      return SUCCEEDED;
    return FAILED;
  }
  if (edit_mode == EDIT_MODE_16) {
    if (d == a || d == (a + 32) || d == (a + 1) || d == (a + 33))
      return SUCCEEDED;
    return FAILED;
  }

  if (d == a || d == (a + 32) || d == (a + 1) || d == (a + 33))
    return SUCCEEDED;
  if (d == (a + 64) || d == (a + 32 + 64) || d == (a + 1 + 64) || d == (a + 33 + 64))
    return SUCCEEDED;
  if (d == (a + 2) || d == (a + 32 + 2) || d == (a + 1 + 2) || d == (a + 33 + 2))
    return SUCCEEDED;
  if (d == (a + 64 + 2) || d == (a + 32 + 64 + 2) || d == (a + 1 + 64 + 2) || d == (a + 33 + 64 + 2))
    return SUCCEEDED;

  return FAILED;
}


void map_zoom_1(GtkWidget *widget, gpointer data) {

  map_zoom_mode = ZOOM_MODE_1;
  draw_map_area();
  gtk_drawing_area_size(GTK_DRAWING_AREA(map_draw_area), 256, 256);
  set_map_window_title();
}


void map_zoom_2(GtkWidget *widget, gpointer data) {

  map_zoom_mode = ZOOM_MODE_2;
  draw_map_area();
  gtk_drawing_area_size(GTK_DRAWING_AREA(map_draw_area), 512, 512);
  set_map_window_title();
}


void map_zoom_3(GtkWidget *widget, gpointer data) {

  map_zoom_mode = ZOOM_MODE_3;
  draw_map_area();
  gtk_drawing_area_size(GTK_DRAWING_AREA(map_draw_area), 512, 512);
  set_map_window_title();
}


gint map_scrollbar_clicked(GtkWidget *widget, GdkEventButton *event) {

  map_x = (int)map_adj1->value;
  map_y = (int)map_adj2->value;

  draw_map_area();
  refresh_map_draw_area();
  set_map_window_title();

  return FALSE;
}


void clear_map(GtkWidget *widget, gpointer data) {

  memset(map_property_data, 0, 1024*1024);
  draw_map_area();
  refresh_map_draw_area();
}


gint map_button_press(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b;


  map_active_press = ON;
  map_active_button = event->button;

  adjust_map_rectangle((int)event->x, (int)event->y, &a, &b);

  map_rec_x = a;
  map_rec_y = b;

  set_map_window_title();
  map_mouse_button_pressed();
  draw_map_area();
  refresh_map_draw_area();

  return FALSE;
}


gint map_button_release(GtkWidget *widget, GdkEventButton *event) {

  map_active_press = OFF;
  return FALSE;
}


void adjust_map_rectangle(int x, int y, unsigned int *a, unsigned int *b) {

  if (x < 0)
    *a = 0;
  else if (x >= (int)(map_zoom_size * (256 - edit_size_x)))
    *a = 256 - edit_size_x;
  else
    *a = x / map_zoom_size;

  if (y < 0)
    *b = 0;
  else if (y >= (int)(map_zoom_size * (256 - edit_size_y)))
    *b = 256 - edit_size_y;
  else
    *b = y / map_zoom_size;

  if (map_check_status == ON) {
    *a = (*a / edit_size_x) * edit_size_x;
    *b = (*b / edit_size_y) * edit_size_y;
  }
  else {
    *a = (*a>>3)<<3;
    *b = (*b>>3)<<3;
  }
}


gint map_motion_notify(GtkWidget *widget, GdkEventMotion *event) {

  unsigned int a, b;


  if (map_active_press == OFF)
    return FALSE;

  adjust_map_rectangle((int)event->x, (int)event->y, &a, &b);

  /* was there a change? */
  if (a == map_rec_x && b == map_rec_y)
    return FALSE;

  map_rec_x = a;
  map_rec_y = b;

  set_map_window_title();
  map_mouse_button_pressed();
  draw_map_area();
  refresh_map_draw_area();

  return FALSE;
}


void map_draw_rectangle(void) {

  unsigned int a;


  for (a = map_rec_y; a < map_rec_y + edit_size_y; a++) {
    draw_map_pixel_plot(map_rec_x, a, 0, 0, 0);
    draw_map_pixel_plot(map_rec_x + edit_size_x - 1, a, 0, 0, 0);
  }
  for (a = map_rec_x + 1; a < map_rec_x + edit_size_x - 1; a++) {
    draw_map_pixel_plot(a, map_rec_y, 0, 0, 0);
    draw_map_pixel_plot(a, map_rec_y + edit_size_y - 1, 0, 0, 0);
  }

  old_map_rec_x = map_rec_x;
  old_map_rec_y = map_rec_y;
}


void map_mouse_button_pressed(void) {

  struct metatile_16x16 *m;
  unsigned int a, b, c, d;


  /* position? */
  if (map_active_button == 1)
    return;

  /* draw tiles? */
  if (map_active_button == 3) {

    map_store_undo_tile_data();

    c = ((y>>3)<<5) + (x>>3);

    if (map_draw_mode == MAP_DRAW_META) {
      m = mt_first;
      for (a = 0; a < metatile - 1; a++)
	m = m->next;

      map_data[((map_y + (map_rec_y>>3))<<10) + map_x + (map_rec_x>>3)] = m->t1;
      map_data[((map_y + (map_rec_y>>3))<<10) + map_x + 1 + (map_rec_x>>3)] = m->t2;
      map_data[((1 + map_y + (map_rec_y>>3))<<10) + map_x + (map_rec_x>>3)] = m->t3;
      map_data[((1 + map_y + (map_rec_y>>3))<<10) + map_x + 1 + (map_rec_x>>3)] = m->t4;
      map_property_data[((map_y + (map_rec_y>>3))<<10) + map_x + (map_rec_x>>3)] = m->p1 | 128;
      map_property_data[((map_y + (map_rec_y>>3))<<10) + map_x + 1 + (map_rec_x>>3)] = m->p2 | 128;
      map_property_data[((1 + map_y + (map_rec_y>>3))<<10) + map_x + (map_rec_x>>3)] = m->p3 | 128;
      map_property_data[((1 + map_y + (map_rec_y>>3))<<10) + map_x + 1 + (map_rec_x>>3)] = m->p4 | 128;
    }
    else {
      for (a = 0; a < (edit_size_y>>3); a++, c += 32)
	for (d = c, b = 0; b < (edit_size_x>>3); b++, d++) {
	  map_data[((a + map_y + (map_rec_y>>3))<<10) + b + map_x + (map_rec_x>>3)] = d;
	  map_property_data[((a + map_y + (map_rec_y>>3))<<10) + b + map_x + (map_rec_x>>3)] = palette_data[d] | 128;
	}
    }
  }
  /* untouch? */
  else {

    map_store_undo_tile_data();

    for (a = 0, c = 0; a < (edit_size_y>>3); a++, c += 32)
      for (d = c, b = 0; b < (edit_size_x>>3); b++, d++)
	map_property_data[((a + map_y + (map_rec_y>>3))<<10) + b + map_x + (map_rec_x>>3)] &= 127;
  }
}


gint map_key_release(GtkWidget *widget, GdkEventKey *event) {

  if (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R)
    map_control_value--;

  return FALSE;
}


gint map_key_press(GtkWidget *widget, GdkEventKey *event) {

  unsigned int i = 8, a, b, c, d, e, f;
  unsigned char t;


  if (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R)
    map_control_value++;
  /* paint? */
  else if (event->keyval == GDK_p) {
    map_active_button = 3;
    map_mouse_button_pressed();
    draw_map_area();
    refresh_map_draw_area();
    return FALSE;
  }
  /* delete? */
  else if (event->keyval == GDK_o) {
    map_active_button = 2;
    map_mouse_button_pressed();
    draw_map_area();
    refresh_map_draw_area();
    return FALSE;
  }
  else if (event->keyval == GDK_Right) {
    adjust_map_rectangle((int)(map_rec_x + edit_size_x) * map_zoom_size, (int)map_rec_y * map_zoom_size, &a, &b);

    /* was there a change? */
    if (a == map_rec_x && b == map_rec_y)
      return FALSE;

    map_rec_x = a;
    map_rec_y = b;

    set_map_window_title();
    draw_map_area();
    refresh_map_draw_area();
    return FALSE;
  }
  else if (event->keyval == GDK_Left) {
    adjust_map_rectangle((int)(map_rec_x - edit_size_x) * map_zoom_size, (int)map_rec_y * map_zoom_size, &a, &b);

    /* was there a change? */
    if (a == map_rec_x && b == map_rec_y)
      return FALSE;

    map_rec_x = a;
    map_rec_y = b;

    set_map_window_title();
    draw_map_area();
    refresh_map_draw_area();
    return FALSE;
  }
  else if (event->keyval == GDK_Up) {
    adjust_map_rectangle((int)map_rec_x * map_zoom_size, (int)(map_rec_y - edit_size_y) * map_zoom_size, &a, &b);

    /* was there a change? */
    if (a == map_rec_x && b == map_rec_y)
      return FALSE;

    map_rec_x = a;
    map_rec_y = b;

    set_map_window_title();
    draw_map_area();
    refresh_map_draw_area();
    return FALSE;
  }
  else if (event->keyval == GDK_Down) {
    adjust_map_rectangle((int)map_rec_x * map_zoom_size, (int)(map_rec_y + edit_size_y) * map_zoom_size, &a, &b);

    /* was there a change? */
    if (a == map_rec_x && b == map_rec_y)
      return FALSE;

    map_rec_x = a;
    map_rec_y = b;

    set_map_window_title();
    draw_map_area();
    refresh_map_draw_area();
    return FALSE;
  }
  else if (event->keyval == GDK_1)
    i = 0;
  else if (event->keyval == GDK_2)
    i = 1;
  else if (event->keyval == GDK_3)
    i = 2;
  else if (event->keyval == GDK_4)
    i = 3;
  else if (event->keyval == GDK_5)
    i = 4;
  else if (event->keyval == GDK_6)
    i = 5;
  else if (event->keyval == GDK_7)
    i = 6;
  else if (event->keyval == GDK_8)
    i = 7;
  /* x-flip!? */
  else if (event->keyval == GDK_x) {

    /* cut was selected, not flip */
    if (map_control_value != 0)
      return FALSE;

    map_store_undo_tile_data();

    d = (map_rec_x>>3) + map_x;
    e = (map_rec_y>>3) + map_y;

    for (a = 0; a < (edit_size_y>>3); a++) {
      f = ((e + a)<<10) + d;
      for (b = 0; b < (edit_size_x>>3); b++) {
	c = map_property_data[f + b];
	if (c & 8)
	  map_property_data[f + b] = (c & (23+128));
	else
	  map_property_data[f + b] = (c | 8);
      }
    }

    for (a = 0; a < edit_size_y>>3; a++) {
      f = ((e + a)<<10) + d;
      for (b = 0; b < edit_size_x>>4; b++) {
	t = map_data[f + b];
	map_data[f + b] = map_data[f + (edit_size_x>>3) - 1 - b];
	map_data[f + (edit_size_x>>3) - 1 - b] = t;

	t = map_property_data[f + b];
	map_property_data[f + b] = map_property_data[f + (edit_size_x>>3) - 1 - b];
	map_property_data[f + (edit_size_x>>3) - 1 - b] = t;
      }
    }

    draw_map_area();
    refresh_map_draw_area();

    return FALSE;
  }
  /* y-flip!? */
  else if (event->keyval == GDK_y) {

    map_store_undo_tile_data();

    d = (map_rec_x>>3) + map_x;
    e = (map_rec_y>>3) + map_y;

    for (a = 0; a < (edit_size_y>>3); a++) {
      f = ((a + e)<<10) + d;
      for (b = 0; b < (edit_size_x>>3); b++) {
	c = map_property_data[f + b];
	if (c & 16)
	  map_property_data[f + b] = (c & (15+128));
	else
	  map_property_data[f + b] = (c | 16);
      }
    }

    for (a = 0; a < (edit_size_x>>3); a++)
      for (b = 0; b < (edit_size_y>>4); b++) {
	t = map_data[((b + e)<<10) + d + a];
	map_data[((b + e)<<10) + d + a] = map_data[((e + (edit_size_y>>3) - 1 - b)<<10) + d + a];
	map_data[((e + (edit_size_y>>3) - 1 - b)<<10) + d + a] = t;

	t = map_property_data[((e + b)<<10) + d + a];
	map_property_data[((e + b)<<10) + d + a] = map_property_data[((e + (edit_size_y>>3) - 1 - b)<<10) + d + a];
	map_property_data[((e + (edit_size_y>>3) - 1 - b)<<10) + d + a] = t;
      }

    draw_map_area();
    refresh_map_draw_area();

    return FALSE;
  }

  if (i == 8)
    return FALSE;

  map_store_undo_tile_data();

  /* properties changed */
  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++) {
      c = map_property_data[((a + map_y + (map_rec_y>>3))<<10) + b + map_x + (map_rec_x>>3)];
      map_property_data[((a + map_y + (map_rec_y>>3))<<10) + b + map_x + (map_rec_x>>3)] = ((c & (24+128)) | i);
    }

  draw_map_area();
  refresh_map_draw_area();

  return FALSE;
}


gint map_check_press(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b;


  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
    map_check_status = ON;
  else
    map_check_status = OFF;

  adjust_map_rectangle(map_rec_x * map_zoom_size, map_rec_y * map_zoom_size, &a, &b);

  /* was there a change? */
  if (a == map_rec_x && b == map_rec_y)
    return FALSE;

  map_rec_x = a;
  map_rec_y = b;

  draw_map_area();
  refresh_map_draw_area();

  return FALSE;
}


void map_copy_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  map_store_undo_tile_data();

  map_copy_size_x = edit_size_x>>3;
  map_copy_size_y = edit_size_y>>3;

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++) {
      map_copy_tile_buffer[(a<<2) + b] = map_data[((((map_y + map_rec_y)>>3) + a)<<10) + ((map_x + map_rec_x)>>3) + b];
      map_copy_property_buffer[(a<<2) + b] = map_property_data[((((map_y + map_rec_y)>>3) + a)<<10) + ((map_x + map_rec_x)>>3) + b];
    }

  map_paste_status = ON;
}


void map_cut_tile(GtkWidget *widget, gpointer data) {

  map_copy_tile(widget, data);
  map_clear_tile(widget, data);
}


void map_clear_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  map_store_undo_tile_data();

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++)
      map_property_data[((((map_y + map_rec_y)>>3) + a)<<10) + ((map_x + map_rec_x)>>3) + b] = 0;

  draw_and_refresh_all();
}


void map_paste_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  map_store_undo_tile_data();

  if (map_paste_status == OFF)
    return;

  for (a = 0; a < map_copy_size_y; a++)
    for (b = 0; b < map_copy_size_x; b++) {
      map_data[((((map_y + map_rec_y)>>3) + a)<<10) + ((map_x + map_rec_x)>>3) + b] = map_copy_tile_buffer[(a<<2) + b];
      map_property_data[((((map_y + map_rec_y)>>3) + a)<<10) + ((map_x + map_rec_x)>>3) + b] = map_copy_property_buffer[(a<<2) + b];
    }

  draw_and_refresh_all();
}


void map_store_undo_tile_data(void) {

  unsigned int a, b;


  map_undo_x = (map_x + map_rec_x)>>3;
  map_undo_y = (map_y + map_rec_y)>>3;
  map_undo_size_x = edit_size_x>>3;
  map_undo_size_y = edit_size_y>>3;

  for (a = 0; a < map_undo_size_y; a++)
    for (b = 0; b < map_undo_size_x; b++) {
      map_undo_tile_buffer[(a<<2) + b] = map_data[((map_undo_y + a)<<10) + map_undo_x + b];
      map_undo_property_buffer[(a<<2) + b] = map_property_data[((map_undo_y + a)<<10) + map_undo_x + b];
    }
}


void map_undo_tile(GtkWidget *widget, gpointer data) {

  unsigned char mupb[4 * 4];
  unsigned int mutb[4 * 4], a, b;


  /* store current data */
  for (a = 0; a < map_undo_size_y; a++)
    for (b = 0; b < map_undo_size_x; b++) {
      mutb[(a<<2) + b] = map_data[((map_undo_y + a)<<10) + map_undo_x + b];
      mupb[(a<<2) + b] = map_property_data[((map_undo_y + a)<<10) + map_undo_x + b];
    }

  /* return old undo data */
  for (a = 0; a < map_undo_size_y; a++)
    for (b = 0; b < map_undo_size_x; b++) {
      map_data[((map_undo_y + a)<<10) + map_undo_x + b] = map_undo_tile_buffer[(a<<2) + b];
      map_property_data[((map_undo_y + a)<<10) + map_undo_x + b] = map_undo_property_buffer[(a<<2) + b];
    }

  /* copy new undo data */
  for (a = 0; a < map_undo_size_y; a++)
    for (b = 0; b < map_undo_size_x; b++) {
      map_undo_tile_buffer[(a<<2) + b] = mutb[(a<<2) + b];
      map_undo_property_buffer[(a<<2) + b] = mupb[(a<<2) + b];
    }

  draw_and_refresh_all();
}


void map_new_tile_memory_map(GtkWidget *widget, gpointer data) {

  int x, y, i, c;


  memset(map_property_data, 0, 1024*1024);

  if (memory_size == MEMORY_SIZE_4KB)
    i = 8;
  else if (memory_size == MEMORY_SIZE_8KB)
    i = 16;
  else if (memory_size == MEMORY_SIZE_12KB)
    i = 24;
  else
    i = 32;

  for (c = 0, y = 0; y < i; y++) {
    for (x = 0; x < 32; x++, c++) {
      map_property_data[(y<<10) + x] = 128 | palette_data[(y<<5) + x];
      map_data[(y<<10) + x] = c;
    }
  }

  draw_map_area();
  map_draw_rectangle();
  refresh_map_draw_area();
}
