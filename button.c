
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

#include "defines.h"
#include "main.h"
#include "draw.h"
#include "edit.h"


extern unsigned char memory_data[8 * 8 * 1024]; /* one byte / pixel */
extern unsigned char palette_data[1024]; /* one byte / palette number */
extern unsigned int color_one, color_two, color_three;
extern unsigned int active_press, active_color, active_x, active_y;
extern unsigned int x, y;
extern unsigned int edit_mode, edit_size_x, edit_size_y;
extern unsigned int edit_grid, memory_size, memory_control_value;

extern GtkWidget *edit_draw_area, *memory_draw_area;

unsigned int shift_status = 0;



gint button_press_right(GtkWidget *widget, GdkEventButton *event) {

  if (x + edit_size_x >= 256) {
    if (y + edit_size_y >= memory_size)
      return FALSE;
    x += edit_size_x - 256;
    y += edit_size_y;
  }
  else
    x += edit_size_x;

  set_edit_window_title();
  set_memory_window_title();
  draw_and_refresh_all();

  return FALSE;
}


gint button_press_left(GtkWidget *widget, GdkEventButton *event) {

  if ((int)(x - edit_size_x) < 0) {
    if ((int)(y - edit_size_y) < 0)
      return FALSE;
    x += 256 - edit_size_x;
    y -= edit_size_y;
  }
  else
    x -= edit_size_x;

  set_edit_window_title();
  set_memory_window_title();
  draw_and_refresh_all();

  return FALSE;
}


gint button_press_up(GtkWidget *widget, GdkEventButton *event) {

  if ((int)(y - edit_size_y) < 0)
    return FALSE;
  else
    y -= edit_size_y;

  set_edit_window_title();
  set_memory_window_title();
  draw_and_refresh_all();

  return FALSE;
}


gint button_press_down(GtkWidget *widget, GdkEventButton *event) {

  if (y + edit_size_y >= memory_size)
    return FALSE;
  else
    y += edit_size_y;

  set_edit_window_title();
  set_memory_window_title();
  draw_and_refresh_all();

  return FALSE;
}


gint draw_area_shift_right(GtkWidget *widget, GdkEventButton *event) {

  unsigned char d[32 * 32];
  unsigned int a, b;


  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      d[(a<<5) + b] = memory_data[((y + a)<<8) + x + b];

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      memory_data[((y + a)<<8) + x + b] = d[(a<<5) + ((b - 1) % edit_size_x)];

  draw_and_refresh_all();

  return FALSE;
}


gint draw_area_shift_left(GtkWidget *widget, GdkEventButton *event) {

  unsigned char d[32 * 32];
  unsigned int a, b;


  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      d[(a<<5) + b] = memory_data[((y + a)<<8) + x + b];

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      memory_data[((y + a)<<8) + x + b] = d[(a<<5) + ((b + 1) % edit_size_x)];

  draw_and_refresh_all();

  return FALSE;
}


gint draw_area_shift_up(GtkWidget *widget, GdkEventButton *event) {

  unsigned char d[32 * 32];
  unsigned int a, b;


  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      d[(a<<5) + b] = memory_data[((y + a)<<8) + x + b];

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      memory_data[((y + a)<<8) + x + b] = d[(((a + 1) % edit_size_y)<<5) + b];

  draw_and_refresh_all();

  return FALSE;
}


gint draw_area_shift_down(GtkWidget *widget, GdkEventButton *event) {

  unsigned char d[32 * 32];
  unsigned int a, b;


  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      d[(a<<5) + b] = memory_data[((y + a)<<8) + x + b];

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      memory_data[((y + a)<<8) + x + b] = d[(((a - 1) % edit_size_y)<<5) + b];

  draw_and_refresh_all();

  return FALSE;
}


gint flip_y(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b, c, d;
  unsigned char t;


  store_undo_tile_data();

  for (a = 0; a < edit_size_x; a++)
    for (b = 0; b < (edit_size_y>>1); b++) {
      t = memory_data[((y + b)<<8) + x + a];
      memory_data[((y + b)<<8) + x + a] = memory_data[((y - b + edit_size_y - 1)<<8) + x + a];
      memory_data[((y - b + edit_size_y - 1)<<8) + x + a] = t;
    }

  c = y>>3;
  d = x>>3;

  for (a = 0; a < (edit_size_x>>3); a++)
    for (b = 0; b < (edit_size_y>>4); b++) {
      t = palette_data[((c + b)<<5) + d + a];
      palette_data[((c + b)<<5) + d + a] = palette_data[((c + (edit_size_y>>3) - 1 - b)<<5) + d + a];
      palette_data[((c + (edit_size_y>>3) - 1 - b)<<5) + d + a] = t;
    }

  draw_and_refresh_all();

  return FALSE;
}


gint flip_x(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b, c, d;
  unsigned char t;


  if (memory_control_value != 0)
    return FALSE;

  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < (edit_size_x>>1); b++) {
      t = memory_data[((y + a)<<8) + x + b];
      memory_data[((y + a)<<8) + x + b] = memory_data[((y + a)<<8) + x + edit_size_x - 1 - b];
      memory_data[((y + a)<<8) + x + edit_size_x - 1 - b] = t;
    }

  c = y>>3;
  d = x>>3;

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>4); b++) {
      t = palette_data[((c + a)<<5) + d + b];
      palette_data[((c + a)<<5) + d + b] = palette_data[((c + a)<<5) + d - b + (edit_size_x>>3) - 1];
      palette_data[((c + a)<<5) + d - b + (edit_size_x>>3) - 1] = t;
    }

  draw_and_refresh_all();

  return FALSE;
}


gint key_press(GtkWidget *widget, GdkEventKey *event) {

  if (event->keyval == GDK_Right)
    button_press_right(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Left)
    button_press_left(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Up)
    button_press_up(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Down)
    button_press_down(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_x)
    flip_x(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_y)
    flip_y(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_l)
    draw_area_shift_left(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_r)
    draw_area_shift_right(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_u)
    draw_area_shift_up(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_d)
    draw_area_shift_down(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R)
    shift_status = 1;

  return FALSE;
}


gint key_release(GtkWidget *widget, GdkEventKey *event) {

  if (event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R)
    shift_status = 0;

  return FALSE;
}


gint rotate_l(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b, c, d;
  unsigned char t[32 * 32];


  if (edit_mode == EDIT_MODE_8x16)
    return FALSE;

  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < (edit_size_x); b++)
      t[((edit_size_y - b - 1)<<5)+a] = memory_data[((y + a)<<8) + x + b];

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < (edit_size_x); b++)
      memory_data[((y + a)<<8) + x + b] = t[(a<<5)+b];

  c = y>>3;
  d = x>>3;

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++)
      t[(((edit_size_y>>3) - b - 1)<<5)+a] = palette_data[((c + a)<<5) + d + b];

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++)
      palette_data[((c + a)<<5) + d + b]= t[(a<<5)+b];

  draw_and_refresh_all();

  return FALSE;
}


gint rotate_r(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b, c, d;
  unsigned char t[32 * 32];


  if (edit_mode == EDIT_MODE_8x16)
    return FALSE;

  store_undo_tile_data();

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < (edit_size_x); b++)
      t[((b)<<5)-a+edit_size_x-1] = memory_data[((y + a)<<8) + x + b];

  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < (edit_size_x); b++)
      memory_data[((y + a)<<8) + x + b] = t[(a<<5)+b];

  c = y>>3;
  d = x>>3;

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++)
      t[(b<<5)+-a+(edit_size_x>>3)-1] = palette_data[((c + a)<<5) + d + b];

  for (a = 0; a < (edit_size_y>>3); a++)
    for (b = 0; b < (edit_size_x>>3); b++)
      palette_data[((c + a)<<5) + d + b]= t[(a<<5)+b];

  draw_and_refresh_all();

  return FALSE;
}
