
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

#include "include/refresh.h"
#include "include/defines.h"

extern guchar rgb_buffer[EDIT_WIDTH * EDIT_HEIGHT * 3];
extern guchar memory_buffer[512 * 512 * 3];
extern guchar map_buffer[512 * 512 * 3];
extern guchar tile_buffer[96 * 2 * 96 * 2 * 3];
extern guchar meta_buffer[64 * 64 * 3];
extern guchar palette_buffer_1[PBX * PBY * 3], palette_buffer_2[PBX * PBY * 3], palette_buffer_3[PBX * PBY * 3], palette_buffer_4[PBX * PBY * 3];
extern guchar palette_buffer_5[PBX * PBY * 3], palette_buffer_6[PBX * PBY * 3], palette_buffer_7[PBX * PBY * 3], palette_buffer_8[PBX * PBY * 3];
extern GtkWidget *palette_draw_area_1, *palette_draw_area_2, *palette_draw_area_3, *palette_draw_area_4;
extern GtkWidget *palette_draw_area_5, *palette_draw_area_6, *palette_draw_area_7, *palette_draw_area_8;
extern GtkWidget *edit_draw_area, *memory_draw_area, *tile_draw_area, *map_draw_area, *meta_da;
extern unsigned int tile_zoom_size, zoom_size, edit_grid, tile_window_status, edit_mode, map_zoom_size, memory_size;
extern unsigned int map_window_status, metatile_window_status;
extern unsigned int edit_size_x, edit_size_y;



gboolean draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  if (edit_grid == ON)
    gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
		       0, 0, EDIT_WIDTH, EDIT_HEIGHT, GDK_RGB_DITHER_NORMAL, rgb_buffer, EDIT_WIDTH * 3);
  else
    gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
		       0, 0, EDIT_WIDTH - 1, EDIT_HEIGHT - 1, GDK_RGB_DITHER_NORMAL, rgb_buffer, EDIT_WIDTH * 3);
  return FALSE;
}


gboolean memory_draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
  		     0, 0, zoom_size<<8, zoom_size * memory_size, GDK_RGB_DITHER_NORMAL, memory_buffer, zoom_size * 256 * 3);
  return FALSE;
}


gboolean tile_draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
		     0, 0, tile_zoom_size * 96, tile_zoom_size * 96, GDK_RGB_DITHER_NORMAL, tile_buffer, tile_zoom_size * 96 * 3);
  return FALSE;
}


gboolean map_draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
		     0, 0, map_zoom_size<<8, map_zoom_size<<8, GDK_RGB_DITHER_NORMAL, map_buffer, map_zoom_size * 256 * 3);
  return FALSE;
}


gboolean meta_da_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
		     0, 0, 64, 64, GDK_RGB_DITHER_NORMAL, meta_buffer, 64 * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_1(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_1, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_2(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_2, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_3(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_3, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_4(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_4, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_5(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_5, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_6(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_6, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_7(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_7, PBX * 3);
  return FALSE;
}


gboolean palette_draw_area_expose_8(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, PBX, PBY, GDK_RGB_DITHER_NORMAL, palette_buffer_8, PBX * 3);
  return FALSE;
}


void refresh_draw_data(void) {

  refresh_edit_draw_area();
  refresh_memory_draw_area();
  refresh_tile_draw_area();
  refresh_map_draw_area();
  refresh_meta_draw_area();
}


void refresh_edit_draw_area(void) {

  gtk_widget_queue_draw(edit_draw_area);
}


void refresh_memory_draw_area(void) {

  gtk_widget_queue_draw(memory_draw_area);
}


void refresh_meta_draw_area(void) {

  if (metatile_window_status == OFF)
    return;

  gtk_widget_queue_draw(meta_da);
}


void refresh_tile_draw_area(void) {

  if (tile_window_status == OFF)
    return;

  gtk_widget_queue_draw(tile_draw_area);
}


void refresh_map_draw_area(void) {

  if (map_window_status == OFF)
    return;

  gtk_widget_queue_draw(map_draw_area);
}


void refresh_palette_1(void) {

  gtk_widget_queue_draw(palette_draw_area_1);
}


void refresh_palette_2(void) {

  gtk_widget_queue_draw(palette_draw_area_2);
}


void refresh_palette_3(void) {

  gtk_widget_queue_draw(palette_draw_area_3);
}


void refresh_palette_4(void) {

  gtk_widget_queue_draw(palette_draw_area_4);
}


void refresh_palette_5(void) {

  gtk_widget_queue_draw(palette_draw_area_5);
}


void refresh_palette_6(void) {

  gtk_widget_queue_draw(palette_draw_area_6);
}


void refresh_palette_7(void) {

  gtk_widget_queue_draw(palette_draw_area_7);
}


void refresh_palette_8(void) {

  gtk_widget_queue_draw(palette_draw_area_8);
}
