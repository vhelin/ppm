
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
#include "include/color.h"
#include "include/file.h"
#include "include/map.h"
#include "include/png.h"


extern unsigned char memory_data[8 * 8 * 1024];
extern unsigned char palette_colors[3 * 4 * 8];
extern unsigned char palette_colors_real[3 * 4 * 8];
extern unsigned char palette_data[1024];
extern GtkWidget *file_selection_open, *file_selection_save_as, *file_selection_save_as_png;
extern GtkWidget *file_selection_map_open, *file_selection_map_save_as;
extern gchar savename[256], mapsavename[256];

extern unsigned char *map_property_data;
extern unsigned int *map_data, memory_size;



void file_open(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_open);
}


void file_save(GtkWidget *widget, gpointer data) {

  /* nothing saved so far? */
  if (savename[0] == 0) {
    file_save_as(widget, data);
    return;
  }
  file_save_data(savename);
}


void file_save_as(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_save_as);
}


void file_save_as_png(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_save_as_png);
}


void file_save_as_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_save_as));
  strcpy(savename, n);
  file_save_data(n);
  gtk_widget_hide(file_selection_save_as);
}


void file_save_as_png_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_save_as_png));
  file_save_data_png(n);
  gtk_widget_hide(file_selection_save_as_png);
}


void file_save_as_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_save_as);
}


void file_save_as_png_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_save_as_png);
}


void file_open_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_open));
  if (file_open_data(n) == FAILED)
    return;

  gtk_widget_hide(file_selection_open);
  palette_mode_changed();

  strcpy(savename, n);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_save_as), n);
}


int file_open_data(char *n) {

  unsigned char *b;
  FILE *f;
  int s, i, p;


  f = fopen(n, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", n);
    return FAILED;
  }

  fseek(f, 0, SEEK_END);
  s = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (s != 4 + 8*4*3 + 32*32 + 256*256) {
    fprintf(stderr, "Invalid file size.\n");
    fclose(f);
    return FAILED;
  }

  b = malloc(s);
  if (b == NULL) {
    fclose(f);
    return FAILED;
  }

  fread(b, 1, s, f);
  fclose(f);

  if (strncmp(b, "PPM0", 4) != 0) {
    fprintf(stderr, "Unknown file format.\n");
    free(b);
    return FAILED;
  }

  /* collect palette colors */
  for (p = 0, i = 4; i < 4 + 8*4*3; i++, p++)
    palette_colors_real[p] = b[i];

  /* collect tile palette data */
  for (p = 0; i < 4 + 8*4*3 + 32*32; i++, p++)
    palette_data[p] = b[i];

  /* collect tile data */
  for (p = 0; i < 4 + 8*4*3 + 32*32 + 256*256; i++, p++)
    memory_data[p] = b[i];

  free(b);

  return SUCCEEDED;
}


void file_open_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_open);
}


int file_save_data_png(char *n) {

  int my, mx = 32*8, x, y, i, q, c, z;
  unsigned char *m;


  if (memory_size == MEMORY_SIZE_4KB)
    my = 8*8;
  else if (memory_size == MEMORY_SIZE_8KB)
    my = 16*8;
  else if (memory_size == MEMORY_SIZE_12KB)
    my = 24*8;
  else
    my = 32*8;

  m = malloc(mx*my*3);
  if (m == NULL) {
    fprintf(stderr, "FILE_SAVE_DATA_PNG: Out of memory error.\n");
    return FAILED;
  }

  /* convert the data into a RGB image */
  for (i = 0, y = 0; y < my; y++) {
    for (x = 0; x < mx; x++, i++) {
      c = memory_data[((my-y-1)<<8) + x];
      z = palette_data[(((my-y-1)>>3)<<5) + (x>>3)];
      q = z*12 + c*3;
      m[i*3 + 0] = palette_colors[q++];
      m[i*3 + 1] = palette_colors[q++];
      m[i*3 + 2] = palette_colors[q];
    }
  }

  if (png_save(n, mx, my, GL_RGB, m) == FAILED) {
    free(m);
    return FAILED;
  }

  free(m);

  return SUCCEEDED;
}


int file_save_data(char *n) {

  FILE *f;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\" for writing.\n", n);
    return FAILED;
  }

  fprintf(f, "PPM0");

  /* write palette data */
  fwrite(palette_colors_real, 1, 8*3*4, f);

  /* write tile palette data */
  fwrite(palette_data, 1, 32*32, f);

  /* write tile data */
  fwrite(memory_data, 1, 256*256, f);

  fclose(f);

  return SUCCEEDED;
}


void file_map_open(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_map_open);
}


void file_map_save(GtkWidget *widget, gpointer data) {

  /* nothing saved so far? */
  if (mapsavename[0] == 0) {
    file_map_save_as(widget, data);
    return;
  }
  file_map_save_data(mapsavename);
}


void file_map_save_as(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_map_save_as);
}


void file_map_save_as_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_map_save_as));
  strcpy(mapsavename, n);
  file_map_save_data(n);
  gtk_widget_hide(file_selection_map_save_as);
}


void file_map_open_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_map_open));
  if (file_map_open_data(n) == FAILED)
    return;

  gtk_widget_hide(file_selection_map_open);
  draw_and_refresh_all();

  strcpy(mapsavename, n);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_map_save_as), n);
}


int file_map_open_data(char *n) {

  unsigned char *b;
  FILE *f;
  int s, ax, ay, bx, by, i;


  f = fopen(n, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", n);
    return FAILED;
  }

  fseek(f, 0, SEEK_END);
  s = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (s <= 4) {
    fprintf(stderr, "Invalid map file size.\n");
    fclose(f);
    return FAILED;
  }

  b = malloc(s);
  if (b == NULL) {
    fclose(f);
    return FAILED;
  }

  fread(b, 1, s, f);
  fclose(f);

  if (strncmp(b, "MEL0", 4) != 0) {
    fprintf(stderr, "Unknown file format.\n");
    free(b);
    return FAILED;
  }

  i = 4;

  ax = b[i] + (b[i+1]<<8) + (b[i+2]<<16) + (b[i+3]<<24);
  i += 4;
  ay = b[i] + (b[i+1]<<8) + (b[i+2]<<16) + (b[i+3]<<24);
  i += 4;

  /* clear the old data */
  clear_map(NULL, NULL);

  /* read tile ids */
  for (by = 0; by < ay; by++)
    for (bx = 0; bx < ax; bx++) {
      map_data[(by<<10) + bx] = b[i] + (b[i+1]<<8);
      i += 2;
    }

  /* read tile property bytes */
  for (by = 0; by < ay; by++)
    for (bx = 0; bx < ax; bx++)
      map_property_data[(by<<10) + bx] = b[i++];

  free(b);

  return SUCCEEDED;
}


void file_map_open_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_map_open);
}


void file_map_save_as_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_map_save_as);
}


int file_map_save_data(char *n) {

  unsigned int x, y, i, bx, by;
  FILE *f;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\" for writing.\n", n);
    return FAILED;
  }

  fprintf(f, "MEL0");

  /* get map max x */
  for (x = 0, i = 0; i < 1024; i++)
    if (map_property_data[i] & 128)
      x = i;

  /* get map max y */
  for (y = 0, i = 0; i < 1024; i++)
    if (map_property_data[i<<10] & 128)
      y = i;

  x++;
  y++;

  fprintf(f, "%c%c%c%c", x&0xFF, (x>>8)&0xFF, (x>>16)&0xFF, (x>>24)&0xFF);
  fprintf(f, "%c%c%c%c", y&0xFF, (y>>8)&0xFF, (y>>16)&0xFF, (y>>24)&0xFF);

  /* write tile ids */
  for (by = 0; by < y; by++)
    for (bx = 0; bx < x; bx++) {
      i = map_data[(by<<10) + bx];
      fprintf(f, "%c%c", i&0xFF, (i>>8)&0xFF);
    }

  /* write tile property bytes */
  for (by = 0; by < y; by++)
    for (bx = 0; bx < x; bx++)
      fprintf(f, "%c", map_property_data[(by<<10) + bx]);

  fclose(f);

  return SUCCEEDED;
}
