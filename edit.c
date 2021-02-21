
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


extern unsigned char copy_data_buffer[32 * 32], copy_palette_buffer[4 * 4];
extern unsigned char memory_data[8 * 8 * 1024];
extern unsigned char palette_data[1024];
extern unsigned char palette_colors[3 * 4 * 8];
extern unsigned int edit_size_x, edit_size_y, edit_mode;
extern unsigned int zoom_size, zoom_mode;
extern unsigned int color_one, color_two, color_three, palette_one, palette_two, palette_three;
extern unsigned int x, y;

unsigned char undo_palette_buffer[4 * 4];
unsigned int paste_status = OFF, undo_x = 0, undo_y = 0, undo_size_x, undo_size_y, copy_size_x, copy_size_y;
unsigned int undo_buffer[32 * 32];



void undo_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b, ub[32 * 32];
  unsigned char upb[4 * 4];


  /* copy old data to a tmp undo buffer */
  for (a = 0; a < (undo_size_y<<3); a++)
    for (b = 0; b < (undo_size_x<<3); b++)
      ub[a * 32 + b] = memory_data[(undo_y + a) * 256 + undo_x + b];

  for (a = 0; a < undo_size_y; a++)
    for (b = 0; b < undo_size_x; b++)
      upb[a * 4 + b] = palette_data[(undo_y / 8 + a) * 32 + undo_x / 8 + b];

  /* return undo buffer */
  for (a = 0; a < (undo_size_y<<3); a++)
    for (b = 0; b < (undo_size_x<<3); b++)
      memory_data[(undo_y + a) * 256 + undo_x + b] = undo_buffer[a * 32 + b];

  for (a = 0; a < undo_size_y; a++)
    for (b = 0; b < undo_size_x; b++)
      palette_data[(undo_y / 8 + a) * 32 + undo_x / 8 + b] = undo_palette_buffer[a * 4 + b];

  /* copy old data to undo buffer */
  for (a = 0; a < (undo_size_y<<3); a++)
    for (b = 0; b < (undo_size_x<<3); b++)
      undo_buffer[a * 32 + b] = ub[a * 32 + b];

  for (a = 0; a < undo_size_y; a++)
    for (b = 0; b < undo_size_x; b++)
      undo_palette_buffer[a * 4 + b] = upb[a * 4 + b];

  draw_and_refresh_all();
}


void copy_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      copy_data_buffer[a * 32 + b] = memory_data[(y + a) * 256 + x + b];

  for (a = 0; a < edit_size_y / 8; a++)
    for (b = 0; b < edit_size_x / 8; b++)
      copy_palette_buffer[a * 4 + b] = palette_data[(y / 8 + a) * 32 + x / 8 + b];

  copy_size_x = edit_size_x>>3;
  copy_size_y = edit_size_y>>3;

  paste_status = ON;
}


void cut_tile(GtkWidget *widget, gpointer data) {

  copy_tile(widget, data);
  clear_tile(widget, data);
}


void paste_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  if (paste_status == OFF)
    return;

  store_undo_tile_data();

  /* paste from copy buffer */
  for (a = 0; a < copy_size_y; a++)
    for (b = 0; b < copy_size_x; b++)
      palette_data[(y / 8 + a) * 32 + x / 8 + b] = copy_palette_buffer[a * 4 + b];

  for (a = 0; a < (copy_size_y<<3); a++)
    for (b = 0; b < (copy_size_x<<3); b++)
      memory_data[(y + a) * 256 + x + b] = copy_data_buffer[a * 32 + b];

  draw_and_refresh_all();
}


void clear_tile(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  store_undo_tile_data();

  /* clear the tile */
  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      memory_data[(y + a) * 256 + x + b] = color_one;

  for (a = 0; a < edit_size_y / 8; a++)
    for (b = 0; b < edit_size_x / 8; b++)
      palette_data[(y / 8 + a) * 32 + x / 8 + b] = palette_one;

  draw_and_refresh_all();
}


void store_undo_tile_data(void) {

  unsigned int a, b;


  undo_x = x;
  undo_y = y;
  undo_size_x = edit_size_x>>3;
  undo_size_y = edit_size_y>>3;

  /* copy old data to undo buffer */
  for (a = 0; a < edit_size_y; a++)
    for (b = 0; b < edit_size_x; b++)
      undo_buffer[a * 32 + b] = memory_data[(y + a) * 256 + x + b];

  for (a = 0; a < undo_size_y; a++)
    for (b = 0; b < undo_size_x; b++)
      undo_palette_buffer[a * 4 + b] = palette_data[(y / 8 + a) * 32 + x / 8 + b];
}
