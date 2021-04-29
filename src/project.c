
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
#include "include/memory.h"
#include "include/file.h"
#include "include/color.h"
#include "include/draw.h"
#include "include/project.h"
#include "include/map.h"
#include "include/meta.h"


extern GtkWidget *file_selection_project_open, *file_selection_project_save_as;
extern GtkWidget *file_selection_open, *file_selection_save_as, *file_selection_export, *file_selection_import_gbr;
extern GtkWidget *file_selection_map_open, *file_selection_map_save_as, *file_selection_map_export, *file_selection_import_gbm;
extern GtkWidget *file_selection_meta_open, *file_selection_meta_save_as;
extern gchar savename[256], mapsavename[256], project_name[256], meta_name[256];


void clear_project(GtkWidget *widget, gpointer data) {

  clear_memory(widget, data);
  clear_map(widget, data);
  metatile_lib_new(widget, data);
}


void file_project_save_as_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_project_save_as));
  strcpy(project_name, n);
  project_save_data(project_name);
  gtk_widget_hide(file_selection_project_save_as);
}


void file_project_save_as_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_project_save_as);
}


void file_project_open_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_project_open);
}


void file_project_open_ok(GtkWidget *widget, gpointer data) {

  char s1[256], s2[256], s3[256];
  gchar *n;
  FILE *f;
  char *b;
  int s, x, z;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_project_open));
  f = fopen(n, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open the file \"%s\".\n", n);
    return;
  }

  fseek(f, 0, SEEK_END);
  s = ftell(f);
  fseek(f, 0, SEEK_SET);

  b = malloc(s);
  if (b == NULL) {
    fclose(f);
    return;
  }

  fread(b, 1, s, f);
  fclose(f);

  if (strncmp(b, "PPMR", 4) != 0) {
    fprintf(stderr, "Unknown file format.\n");
    free(b);
    return;
  }

  /* obtain tile memory file name */
  for (x = 4; x < s && b[x] != 0; x++)
    s1[x - 4] = b[x];

  if (x == s) {
    fprintf(stderr, "Corrupted PPM project file \"%s\".\n", n);
    free(b);
    return;
  }

  s1[x - 4] = 0;

  /* obtain map file name */
  for (x++, z = 0; x < s && b[x] != 0; x++, z++)
    s2[z] = b[x];

  if (x == s) {
    fprintf(stderr, "Corrupted PPM project file \"%s\".\n", n);
    free(b);
    return;
  }

  s2[z] = 0;

  /* obtain metatile library file name */
  for (x++, z = 0; x < s && b[x] != 0; x++, z++)
    s3[z] = b[x];

  if (x == s) {
    fprintf(stderr, "Corrupted PPM project file \"%s\".\n", n);
    free(b);
    return;
  }

  s3[z] = 0;

  free(b);

  gtk_widget_hide(file_selection_project_open);

  file_open_data(s1);
  file_map_open_data(s2);
  file_meta_open_data(s3);

  draw_and_refresh_all();
  palette_mode_changed();

  strcpy(project_name, n);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_project_save_as), n);
}


void project_open(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_project_open);
}


void project_save(GtkWidget *widget, gpointer data) {

  /* nothing saved so far? */
  if (project_name[0] == 0) {
    project_save_as(widget, data);
    return;
  }
  project_save_data(project_name);
}


void project_save_as(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_project_save_as);
}


int project_save_data(char *n) {

  char s1[256], s2[256], s3[256];
  FILE *f;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\" for writing.\n", n);
    return FAILED;
  }

  fprintf(f, "PPMR");

  copy_base_name(s1, n);
  strcpy(s2, s1);
  strcpy(s3, s1);

  strcat(s1, ".mem");
  strcat(s2, ".map");
  strcat(s3, ".met");

  fprintf(f, "%s%c%s%c%s%c", s1, 0, s2, 0, s3, 0);

  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_open), s1);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_save_as), s1);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_map_open), s2);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_map_save_as), s2);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_meta_open), s3);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_meta_save_as), s3);

  strcpy(savename, s1);
  strcpy(mapsavename, s2);
  strcpy(meta_name, s3);

  fclose(f);

  file_save_data(s1);
  file_map_save_data(s2);
  file_meta_save_data(s3);

  return SUCCEEDED;
}


int copy_base_name(char *d, char *s) {

  while (*s != 0 && *s != '.') {
    *d = *s;
    d++;
    s++;
  }

  *d = 0;

  return SUCCEEDED;
}
