
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
#include "include/draw.h"
#include "include/mouse.h"
#include "include/color.h"
#include "include/refresh.h"
#include "include/map.h"
#include "include/edit.h"


extern guchar palette_buffer_1[PBX * PBY * 3], palette_buffer_2[PBX * PBY * 3], palette_buffer_3[PBX * PBY * 3], palette_buffer_4[PBX * PBY * 3];
extern guchar palette_buffer_5[PBX * PBY * 3], palette_buffer_6[PBX * PBY * 3], palette_buffer_7[PBX * PBY * 3], palette_buffer_8[PBX * PBY * 3];
extern unsigned char memory_data[8 * 8 * 1024]; /* one byte / pixel */
extern unsigned char undo_buffer[32 * 32], undo_palette_buffer[4 * 4];
extern unsigned char palette_colors[3 * 4 * 8]; /* eight palettes to choose from */
extern unsigned char palette_colors_real[3 * 4 * 8]; /* eight palettes to choose from */
extern unsigned char palette_data[1024]; /* one byte / palette number */
extern unsigned int color_one, color_two, color_three, palette_one, palette_two, palette_three;
extern unsigned int active_press, active_color, active_palette, active_x, active_y;
extern unsigned int latest_color, latest_palette;
extern unsigned int x, y;
extern unsigned int edit_mode;
extern unsigned int edit_grid, memory_size;
extern unsigned int edit_size_x, edit_size_y, zoom_size;
extern unsigned int memory_check_status;
extern unsigned int shift_status;
extern GtkWidget *edit_draw_area, *memory_draw_area, *color_selection;

unsigned int old_memory_rec_x = 0, old_memory_rec_y = 0;




gint button_release(GtkWidget *widget, GdkEventButton *event) {

  active_press = OFF;
  return FALSE;
}


gint button_press(GtkWidget *widget, GdkEventButton *event) {

  unsigned char c, p;
  int a, b;


  a = event->x;
  b = event->y;
  if (edit_mode == EDIT_MODE_8) {
    a /= 36;
    b /= 36;
    if (a >= 8 || b >= 8)
      return FALSE;
  }
  else if (edit_mode == EDIT_MODE_8x16) {
    a /= 18;
    b /= 18;
    if (a >= 8 || b >= 16)
      return FALSE;
  }
  else if (edit_mode == EDIT_MODE_16) {
    a /= 18;
    b /= 18;
    if (a >= 16 || b >= 16)
      return FALSE;
  }
  else {
    a /= 9;
    b /= 9;
    if (a >= 32 || b >= 32)
      return FALSE;
  }

  store_undo_tile_data();

  if (event->button == 1) {
    c = color_one;
    p = palette_one;
  }
  else if (event->button == 2) {
    c = color_two;
    p = palette_two;
  }
  else {
    c = color_three;
    p = palette_three;
  }

  /* one pixel paint? */
  if (shift_status == 0) {
    memory_data[(y + b) * 256 + x + a] = c;
    if (palette_data[((y + b)>>3) * 32 + ((x + a)>>3)] == p) {
      draw_edit_pixel(a, b, x + a, y + b);
      draw_memory_pixel(x + a, y + b);
    }
    else {
      palette_data[((y + b)>>3) * 32 + ((x + a)>>3)] = p;
      draw_edit_pixel_8(a, b, x, y);
      draw_memory_pixel_8(x + a, y + b);
    }

    draw_grid_rectangle();
    draw_tile_area();
    draw_meta_area();
    update_map();
    refresh_draw_data();
    refresh_meta_draw_area();
  }
  /* flood fill with shift down? */
  else {
    flood_fill((unsigned int)a, (unsigned int)b, memory_data[(((int)y) + b) * 256 + x + a], palette_data[((((int)y) + b) / 8) * 32 + (x + a) / 8], c, p);
    update_map();
    draw_and_refresh_all();
  }

  active_color = c;
  active_palette = p;
  active_press = ON;
  active_x = a;
  active_y = b;

  return FALSE;
}


gint motion_notify(GtkWidget *widget, GdkEventMotion *event) {

  unsigned int a, b;


  if (active_press == OFF)
    return FALSE;

  a = event->x;
  b = event->y;

  if (edit_mode == EDIT_MODE_8) {
    a /= 36;
    b /= 36;
    if (a >= 8 || b >= 8)
      return FALSE;
  }
  else if (edit_mode == EDIT_MODE_8x16) {
    a /= 18;
    b /= 18;
    if (a >= 8 || b >= 16)
      return FALSE;
  }
  else if (edit_mode == EDIT_MODE_16) {
    a /= 18;
    b /= 18;
    if (a >= 16 || b >= 16)
      return FALSE;
  }
  else {
    a /= 9;
    b /= 9;
    if (a >= 32 || b >= 32)
      return FALSE;
  }

  if (active_x == a && active_y == b)
    return FAILED;

  active_x = a;
  active_y = b;

  memory_data[(y + b) * 256 + x + a] = active_color;
  if (palette_data[((y + b)>>3) * 32 + ((x + a)>>3)] == active_palette) {
    draw_edit_pixel(a, b, x + a, y + b);
    draw_memory_pixel(x + a, y + b);
  }
  else {
    palette_data[((y + b)>>3) * 32 + ((x + a)>>3)] = active_palette;
    draw_edit_pixel_8(a, b, x, y);
    draw_memory_pixel_8(x + a, y + b);
  }

  draw_grid_rectangle();
  draw_tile_area();
  draw_meta_area();
  update_map();
  refresh_draw_data();
  refresh_meta_draw_area();

  return FALSE;
}


gint memory_button_press(GtkWidget *widget, GdkEventButton *event) {

  active_press = ON;

  if (adjust_memory_rectangle_position((int)(event->x / zoom_size), (int)(event->y / zoom_size)) == FAILED)
    return FALSE;

  set_edit_window_title();
  set_memory_window_title();

  draw_edit_area();
  draw_tile_area();
  memory_rectangle_draw();
  refresh_edit_draw_area();
  refresh_tile_draw_area();
  refresh_memory_draw_area();

  return FALSE;
}


int adjust_memory_rectangle_position(int nx, int ny) {

  int a, b;


  if (nx < 0)
    a = 0;
  else if (nx >= (int)(256 - edit_size_x))
    a = 256 - edit_size_x;
  else
    a = nx;

  if (ny < 0)
    b = 0;
  else if (ny >= (int)(memory_size - edit_size_y))
    b = memory_size - edit_size_y;
  else
    b = ny;

  if (memory_check_status == ON) {
    a = (a / edit_size_x) * edit_size_x;
    b = (b / edit_size_y) * edit_size_y;
  }

  /* was there a change? */
  if (a == (int)x && b == (int)y)
    return FAILED;

  x = a;
  y = b;

  return SUCCEEDED;
}


gint memory_motion_notify(GtkWidget *widget, GdkEventMotion *event) {

  if (active_press == OFF)
    return FALSE;

  if (adjust_memory_rectangle_position((int)(event->x / zoom_size), (int)(event->y / zoom_size)) == FAILED)
    return FALSE;

  set_edit_window_title();
  set_memory_window_title();

  draw_edit_area();
  draw_tile_area();
  memory_rectangle_draw();
  refresh_edit_draw_area();
  refresh_tile_draw_area();
  refresh_memory_draw_area();

  return FALSE;
}


gint memory_button_release(GtkWidget *widget, GdkEventButton *event) {

  active_press = OFF;
  return FALSE;
}


gint memory_check_press(GtkWidget *widget, GdkEventButton *event) {

  unsigned int a, b;


  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
    memory_check_status = ON;
  else
    memory_check_status = OFF;

  if (memory_check_status == ON) {
    a = ((int)x / edit_size_x) * edit_size_x;
    b = ((int)y / edit_size_y) * edit_size_y;

    /* was there a change? */
    if (a == x && b == y)
      return FALSE;

    x = a;
    y = b;

    set_edit_window_title();
    set_memory_window_title();
    draw_and_refresh_all();
  }

  return FALSE;
}


gint palette_1_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 0);
  return FALSE;
}


gint palette_2_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 1);
  return FALSE;
}


gint palette_3_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 2);
  return FALSE;
}


gint palette_4_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 3);
  return FALSE;
}


gint palette_5_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 4);
  return FALSE;
}


gint palette_6_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 5);
  return FALSE;
}


gint palette_7_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 6);
  return FALSE;
}


gint palette_8_button(GtkWidget *widget, GdkEventButton *event) {

  palette_x_button(widget, event, 7);
  return FALSE;
}


gint palette_x_button(GtkWidget *widget, GdkEventButton *event, unsigned int palette) {

  unsigned int a;


  if (event->x < PE || event->x > (PBX - PE - 1))
    return FALSE;
  if (event->y < PE || event->y > (PE + PY - 1))
    return FALSE;

  if (event->x >= PE && event->x < PE + PX)
    a = 0;
  else if (event->x >= PE + PX + PM && event->x < PE + PX + PM + PX)
    a = 1;
  else if (event->x >= PE + PX + PM + PX + PM && event->x < PE + PX + PM + PX + PM + PX)
    a = 2;
  else if (event->x >= PE + PX + PM + PX + PM + PX + PM && event->x < PE + PX + PM + PX + PM + PX + PM + PX)
    a = 3;
  else
    return FALSE;

  set_new_button_color(palette, event->button, a);

  return FALSE;
}


int set_new_button_color(unsigned int palette, unsigned int button, unsigned int color) {

  gdouble c[3];


  c[0] = ((gdouble)palette_colors_real[palette * 3 * 4 + color * 3]) / 255;
  c[1] = ((gdouble)palette_colors_real[palette * 3 * 4 + color * 3 + 1]) / 255;
  c[2] = ((gdouble)palette_colors_real[palette * 3 * 4 + color * 3 + 2]) / 255;
  gtk_color_selection_set_color(GTK_COLOR_SELECTION(color_selection), c);

  if (button == 1) {
    draw_color_box(palette_one, color_one);
    palette_one = palette;
    color_one = color;
  }
  else if (button == 2) {
    draw_color_box(palette_two, color_two);
    palette_two = palette;
    color_two = color;
  }
  else {
    draw_color_box(palette_three, color_three);
    palette_three = palette;
    color_three = color;
  }

  latest_color = color;
  latest_palette = palette;

  draw_button_color_indicator(1, palette_one, color_one);
  draw_button_color_indicator(2, palette_two, color_two);
  draw_button_color_indicator(3, palette_three, color_three);
  refresh_palette_all();

  return SUCCEEDED;
}


unsigned int flood_fill(unsigned int ax, unsigned int ay, unsigned char old_c, unsigned char old_p, unsigned char c, unsigned char p) {

  if (c == old_c && p == old_p)
    return FAILED;

  if (memory_data[(y + ay) * 256 + x + ax] == old_c && palette_data[((y + ay) / 8) * 32 + (x + ax) / 8] == old_p) {
    memory_data[(y + ay) * 256 + x + ax] = c;
    palette_data[((y + ay) / 8) * 32 + (x + ax) / 8] = p;
  }
  else
    return FAILED;

  if (ax > 0)
    flood_fill(ax - 1, ay, old_c, old_p, c, p);
  if (ay > 0)
    flood_fill(ax, ay - 1, old_c, old_p, c, p);
  if (ax < edit_size_x - 1)
    flood_fill(ax + 1, ay, old_c, old_p, c, p);
  if (ay < edit_size_y - 1)
    flood_fill(ax, ay + 1, old_c, old_p, c, p);

  return SUCCEEDED;
}
