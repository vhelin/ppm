
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
#include "include/draw.h"
#include "include/main.h"
#include "include/meta.h"
#include "include/export.h"
#include "include/refresh.h"


extern GtkWidget *meta_cb_x1, *meta_cb_x2, *meta_cb_x3, *meta_cb_x4, *meta_cb_y1, *meta_cb_y2, *meta_cb_y3, *meta_cb_y4;
extern GtkWidget *meta_spin_t1, *meta_spin_t2, *meta_spin_t3, *meta_spin_t4;
extern GtkWidget *meta_spin_p1, *meta_spin_p2, *meta_spin_p3, *meta_spin_p4;
extern GtkItemFactory *map_item_factory, *tile_item_factory;
extern GtkWidget *file_selection_meta_open, *file_selection_meta_save_as;
extern unsigned int map_end_x, map_end_y;
extern unsigned char *map_property_data;
extern unsigned int *map_data;
extern gchar meta_name[256];
extern int metatile, metatiles;

struct metatile_16x16 *mt_first, *mt_last, *m_first, *m_last;
int mt, mx, my;
char *map;


void metatile_lib_new(GtkWidget *widget, gpointer data) {

  struct metatile_16x16 *mt1, *mt2;


  mt1 = mt_first;
  while (mt1 != NULL) {
    mt2 = mt1;
    mt1 = mt1->next;
    free(mt2);
  }
  mt_first = NULL;
  mt_last = NULL;

  metatile = 0;
  metatiles = 0;

  /* all metatiles are gone, they can't be used in drawing any more */
  gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(map_item_factory, "/Draw/Memory"), TRUE);

  gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(tile_item_factory, "/Source/Memory"), TRUE);

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();
}


void metatile_lib_new_from_map(GtkWidget *widget, gpointer data) {

  struct metatile_16x16 *mt1, *mt2;


  mt1 = mt_first;
  while (mt1 != NULL) {
    mt2 = mt1;
    mt1 = mt1->next;
    free(mt2);
  }
  mt_first = NULL;
  mt_last = NULL;

  get_map_dimensions();
  build_metatile_map();

  mt_first = m_first;
  mt_last = m_last;
  metatiles = mt;
  if (mt > 0)
    metatile = 1;
  else
    metatile = 0;

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();
}


void metatile_lib_add_from_map(GtkWidget *widget, gpointer data) {

  get_map_dimensions();

  m_first = mt_first;
  m_last = mt_last;
  mt = metatiles;

  new_metatiles_from_map(0);

  metatiles = mt;
  mt_last = m_last;
  mt_first = m_first;

  if (metatile == 0 && metatiles > 0)
    metatile = 1;

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();
}


gint meta_press_left(GtkWidget *widget, GdkEventButton *event) {

  if (metatile <= 1)
    return FALSE;

  metatile--;

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();

  return FALSE;
}


gint meta_press_right(GtkWidget *widget, GdkEventButton *event) {

  if (metatile == metatiles)
    return FALSE;

  metatile++;

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();

  return FALSE;
}


gint meta_flip_press(GtkWidget *widget, GdkEventButton *event) {

  struct metatile_16x16 *m;
  int i;


  if (metatiles == 0)
    return FALSE;

  m = mt_first;
  for (i = 0; i < metatile - 1; i++)
    m = m->next;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == TRUE)
    i = 0;
  else
    i = 1;

  if (widget == meta_cb_x1)
    m->p1 = (m->p1 & 247) | (i << 3);
  else if (widget == meta_cb_x2)
    m->p2 = (m->p2 & 247) | (i << 3);
  else if (widget == meta_cb_x3)
    m->p3 = (m->p3 & 247) | (i << 3);
  else if (widget == meta_cb_x4)
    m->p4 = (m->p4 & 247) | (i << 3);
  else if (widget == meta_cb_y1)
    m->p1 = (m->p1 & 239) | (i << 4);
  else if (widget == meta_cb_y2)
    m->p2 = (m->p2 & 239) | (i << 4);
  else if (widget == meta_cb_y3)
    m->p3 = (m->p3 & 239) | (i << 4);
  else if (widget == meta_cb_y4)
    m->p4 = (m->p4 & 239) | (i << 4);

  draw_meta_area();
  refresh_meta_draw_area();

  return FALSE;
}


gint meta_tile_changed(GtkWidget *widget, GdkEventButton *event) {

  struct metatile_16x16 *m;
  int i;


  if (metatiles == 0)
    return FALSE;

  m = mt_first;
  for (i = 0; i < metatile - 1; i++)
    m = m->next;

  i = GTK_ADJUSTMENT(widget)->value;

  if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t1)))
    m->t1 = i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t2)))
    m->t2 = i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t3)))
    m->t3 = i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t4)))
    m->t4 = i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p1)))
    m->p1 = (m->p1 & 248) | i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p2)))
    m->p2 = (m->p2 & 248) | i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p3)))
    m->p3 = (m->p3 & 248) | i;
  else if (GTK_ADJUSTMENT(widget) == gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p4)))
    m->p4 = (m->p4 & 248) | i;

  draw_meta_area();
  refresh_meta_draw_area();

  return FALSE;
}


void meta_add(GtkWidget *widget, gpointer data) {

  struct metatile_16x16 *m;


  m = malloc(sizeof(struct metatile_16x16));
  if (m == NULL) {
    fprintf(stderr, "Out of memory while creating a new metatile.\n");
    return;
  }
  metatiles++;
  metatile = metatiles;
  m->next = NULL;
  m->t1 = 0;
  m->t2 = 0;
  m->t3 = 0;
  m->t4 = 0;
  m->p1 = 0;
  m->p2 = 0;
  m->p3 = 0;
  m->p4 = 0;
  m->x = 0;
  m->y = 0;
  if (mt_first == NULL) {
    mt_first = m;
    mt_last = m;
    m->prev = NULL;
  }
  else {
    mt_last->next = m;
    m->prev = mt_last;
    mt_last = m;
  }

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();
}


void meta_delete(GtkWidget *widget, gpointer data) {

  struct metatile_16x16 *m;
  int i;


  if (metatiles == 0)
    return;

  m = mt_first;
  for (i = 1; i != metatile; i++)
    m = m->next;

  if (metatiles == metatile)
    metatile--;
  metatiles--;

  if (metatiles == 0) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(map_item_factory, "/Draw/Memory"), TRUE);
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(tile_item_factory, "/Source/Memory"), TRUE);
  }

  if (m->prev != NULL)
    (m->prev)->next = m->next;
  if (m->next != NULL)
    (m->next)->prev = m->prev;

  if (m == mt_first)
    mt_first = m->next;
  if (m == mt_last)
    mt_last = m->prev;

  free(m);

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();
}


int free_metatile_map(void) {

  struct metatile_16x16 *m1;


  free(map);
  m1 = m_first;
  while (m1 != NULL) {
    m_last = m1;
    m1 = m1->next;
    free(m_last);
  }

  m_first = NULL;
  m_last = NULL;
  mt = 0;

  return SUCCEEDED;
}


int build_metatile_map(void) {

  mt = 0;
  mx = map_end_x>>1;
  my = map_end_y>>1;
  if (mx * my == 0)
    return FAILED;
  m_first = NULL;
  m_last = NULL;

  map = malloc(mx * my);

  if (map == NULL) {
    fprintf(stderr, "Out of memory while creating metatile map.\n");
    return FAILED;
  }

  if (new_metatiles_from_map(1) == FAILED) {
    free(map);
    return FAILED;
  }

  return SUCCEEDED;
}
 

int new_metatiles_from_map(int mark) {

  struct metatile_16x16 *m1;
  int i, a, b, c, res;


  for (i = 0, a = 0; a < map_end_y; a += 2) {
    for (b = 0; b < map_end_x; b += 2, i++) {
      c = (a<<10) + b;
      m1 = m_first;
      res = 0;
      /* try to find a match in the metatile list */
      while (m1 != NULL) {
	if (m1->t1 == map_data[c] && m1->t2 == map_data[c+1] && m1->t3 == map_data[c+1024] && m1->t4 == map_data[c+1025] &&
	    m1->p1 == (map_property_data[c] & 127) && m1->p2 == (map_property_data[c+1] & 127) && m1->p3 == (map_property_data[c+1024] & 127) &&
	    m1->p4 == (map_property_data[c+1025] & 127))
	  break;
	m1 = m1->next;
	res++;
      }
      /* no match in the metatile list */
      if (m1 == NULL) {
	mt++;
	m1 = malloc(sizeof(struct metatile_16x16));
	if (m1 == NULL) {
	  fprintf(stderr, "Out of memory while creating metatiles.\n");
	  if (mark == 1)
	    free(map);
	  return FAILED;
	}
	m1->next = NULL;
	m1->t1 = map_data[c];
	m1->t2 = map_data[c+1];
	m1->t3 = map_data[c+1024];
	m1->t4 = map_data[c+1025];
	m1->p1 = map_property_data[c] & 127;
	m1->p2 = map_property_data[c+1] & 127;
	m1->p3 = map_property_data[c+1024] & 127;
	m1->p4 = map_property_data[c+1025] & 127;
	m1->x = b;
	m1->y = a;
	if (m_first == NULL) {
	  m_first = m1;
	  m_last = m1;
	  m1->prev = NULL;
	}
	else {
	  m_last->next = m1;
	  m1->prev = m_last;
	  m_last = m1;
	}
      }

      if (mark == 1)
	map[i] = res;
    }
  }

  return SUCCEEDED;
}


int update_meta_spin_buttons(void) {

  struct metatile_16x16 *m;
  int i;


  if (metatile == 0)
    return SUCCEEDED;

  m = mt_first;
  for (i = 1; i != metatile; i++)
    m = m->next;

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_t1), m->t1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_t2), m->t2);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_t3), m->t3);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_t4), m->t4);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_p1), m->p1 & 7);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_p2), m->p2 & 7);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_p3), m->p3 & 7);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(meta_spin_p4), m->p4 & 7);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_x1), m->p1 & 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_x2), m->p2 & 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_x3), m->p3 & 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_x4), m->p4 & 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_y1), m->p1 & 16);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_y2), m->p2 & 16);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_y3), m->p3 & 16);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(meta_cb_y4), m->p4 & 16);

  gtk_widget_queue_draw(meta_spin_t1);
  gtk_widget_queue_draw(meta_spin_t2);
  gtk_widget_queue_draw(meta_spin_t3);
  gtk_widget_queue_draw(meta_spin_t4);
  gtk_widget_queue_draw(meta_spin_p1);
  gtk_widget_queue_draw(meta_spin_p2);
  gtk_widget_queue_draw(meta_spin_p3);
  gtk_widget_queue_draw(meta_spin_p4);

  return SUCCEEDED;
}


void file_meta_save_as_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_meta_save_as);
}


void file_meta_open_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_meta_open);
}


void file_meta_open_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_meta_open));
  if (file_meta_open_data(n) == FAILED)
    return;

  gtk_widget_hide(file_selection_meta_open);

  strcpy(meta_name, n);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_meta_save_as), n);
}


void file_meta_save_as_ok(GtkWidget *widget, gpointer data) {

  gchar *n;


  n = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_meta_save_as));
  strcpy(meta_name, n);
  file_meta_save_data(n);
  gtk_widget_hide(file_selection_meta_save_as);
}


void file_meta_open(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_meta_open);
}


void file_meta_save(GtkWidget *widget, gpointer data) {

  /* nothing saved so far? */
  if (meta_name[0] == 0) {
    file_meta_save_as(widget, data);
    return;
  }
  file_meta_save_data(meta_name);
}


void file_meta_save_as(GtkWidget *widget, gpointer data) {

  gtk_widget_show(file_selection_meta_save_as);
}


int file_meta_open_data(char *n) {

  struct metatile_16x16 *m, *mp;
  unsigned char *b, *c;
  FILE *f;
  int s, i;


  f = fopen(n, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", n);
    return FAILED;
  }

  fseek(f, 0, SEEK_END);
  s = ftell(f);
  fseek(f, 0, SEEK_SET);

  b = malloc(s);
  if (b == NULL) {
    fclose(f);
    return FAILED;
  }

  fread(b, 1, s, f);
  fclose(f);

  if (strncmp(b, "PPMM", 4) != 0) {
    fprintf(stderr, "Unknown file format.\n");
    free(b);
    return FAILED;
  }

  c = b + 4;

  metatile_lib_new(NULL, NULL);

  i = (c[0]<<24) + (c[1]<<16) + (c[2]<<8) + c[3];
  metatiles = i;
  if (i > 0)
    metatile = 1;
  else
    metatile = 0;
  mt_first = NULL;
  mt_last = NULL;
  mp = NULL;

  c += 4;
  for ( ; i > 0; i--) {
    m = malloc(sizeof(struct metatile_16x16));
    if (m == NULL) {
      fprintf(stderr, "Out of memory while allocating a new metatile.\n");
      free(b);
      return FAILED;
    }
    m->prev = mp;
    if (mp != NULL)
      mp->next = m;
    m->next = NULL;
    m->t1 = *c++;
    m->t2 = *c++;
    m->t3 = *c++;
    m->t4 = *c++;
    m->p1 = *c++;
    m->p2 = *c++;
    m->p3 = *c++;
    m->p4 = *c++;
    m->x = 0;
    m->y = 0;
    if (mt_first == NULL) {
      mt_first = m;
      mt_last = m;
    }
    else {
      mt_last->next = m;
      mt_last = m;
    }
    mp = m;
  }

  free(b);

  draw_meta_area();
  refresh_meta_draw_area();
  update_meta_spin_buttons();
  set_metatile_window_title();

  return SUCCEEDED;
}


int file_meta_save_data(char *n) {

  struct metatile_16x16 *m;
  FILE *f;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", n);
    return FAILED;
  }

  fprintf(f, "PPMM");
  fprintf(f, "%c%c%c%c", (metatiles>>24)&255, (metatiles>>16)&255, (metatiles>>8)&255, metatiles&255);

  m = mt_first;
  while (m != NULL) {
    fprintf(f, "%c%c%c%c%c%c%c%c", m->t1, m->t2, m->t3, m->t4, m->p1, m->p2, m->p3, m->p4);
    m = m->next;
  }

  fclose(f);

  return SUCCEEDED;
}
