
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
#include "color.h"
#include "file.h"
#include "export.h"
#include "meta.h"


extern unsigned char memory_data[8 * 8 * 1024]; /* one byte / pixel */
extern unsigned char palette_colors[3 * 4 * 8]; /* eight palettes to choose from */
extern unsigned char palette_data[1024]; /* one byte / palette number */
extern unsigned int export_format_format, export_format_palette, memory_size, export_format_tile_data;
extern GtkWidget *export_window, *file_selection_export, *file_selection_map_export;
extern GtkWidget *export_entry_data, *export_entry_palette, *export_entry_tile_palette, *export_table;
extern GtkWidget *export_check_button_data, *export_check_button_palette, *export_check_button_tile_palette;
extern GtkWidget *export_spin_end_x, *export_spin_end_y;
extern GtkWidget *export_radio_button_1, *export_radio_button_2, *export_radio_button_3;
extern GtkWidget *export_clist_format, *export_clist_palette, *map_export_radio_button_4;
extern GtkWidget *export_radio_button_tile_size, *map_export_spin_offset;
extern GtkWidget *map_export_window, *map_export_entry_map_data, *map_export_entry_map_property_data;
extern GtkWidget *map_export_check_button_map_data, *map_export_check_button_map_property_data;
extern GtkWidget *map_export_radio_button_1, *map_export_radio_button_2, *map_export_radio_button_3;
extern unsigned char *map_property_data;
extern unsigned int *map_data, map_export_format_format, map_export_format_size;

static unsigned int export_counter, export_counter_max;
unsigned int export_selection_status, end_x, end_y, tile, export_size, edit_size_x, edit_size_y;
unsigned int map_export_selection_status, map_end_x, map_end_y, eff;
int tile_offset, map_mode;

extern struct metatile_16x16 *m_first, *m_last, *mt_first, *mt_last;
extern int mt, mx, my, metatiles;
extern char *map;



void export(GtkWidget *widget, gpointer data) {

  gtk_widget_show_all(export_window);
}


void export_ok(GtkWidget *widget, gpointer data) {

  if (export_selection_status == EXPORT_SELECTION_DATA)
    gtk_entry_set_text(GTK_ENTRY(export_entry_data), gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_export)));
  else if (export_selection_status == EXPORT_SELECTION_PALETTE)
    gtk_entry_set_text(GTK_ENTRY(export_entry_palette), gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_export)));
  else
    gtk_entry_set_text(GTK_ENTRY(export_entry_tile_palette), gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_export)));

  gtk_widget_hide(file_selection_export);
}


void export_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_export);
}


gint export_button_data_get(GtkWidget *widget, GdkEventButton *event) {

  export_selection_status = EXPORT_SELECTION_DATA;
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_export), gtk_entry_get_text(GTK_ENTRY(export_entry_data)));
  gtk_widget_show(file_selection_export);
  return FALSE;
}


gint export_button_palette_get(GtkWidget *widget, GdkEventButton *event) {

  export_selection_status = EXPORT_SELECTION_PALETTE;
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_export), gtk_entry_get_text(GTK_ENTRY(export_entry_palette)));
  gtk_widget_show(file_selection_export);
  return FALSE;
}


gint export_button_tile_palette_get(GtkWidget *widget, GdkEventButton *event) {

  export_selection_status = EXPORT_SELECTION_TILE_PALETTE;
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_export), gtk_entry_get_text(GTK_ENTRY(export_entry_tile_palette)));
  gtk_widget_show(file_selection_export);
  return FALSE;
}


gint export_export_pressed(GtkWidget *widget, GdkEventButton *event) {

  export_selected_data();
  gtk_widget_hide(export_window);
  return FALSE;
}


gint export_cancel_pressed(GtkWidget *widget, GdkEventButton *event) {

  gtk_widget_hide_all(export_window);
  return FALSE;
}


int export_selected_data(void) {

  end_x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(export_spin_end_x));
  end_y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(export_spin_end_y));

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_radio_button_1)))
    tile = 1;
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_radio_button_2)))
    tile = 2;
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_radio_button_3)))
    tile = 4;
  else tile = 8;

  /* tile data format */
  eff = export_format_format;

  /* export tile data */
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_check_button_data)))
    if (export_tile_data(gtk_entry_get_text(GTK_ENTRY(export_entry_data))) == FAILED)
      return FAILED;

  /* export tile palette data */
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_check_button_tile_palette)))
    if (export_tile_palette_data(gtk_entry_get_text(GTK_ENTRY(export_entry_tile_palette))) == FAILED)
      return FAILED;

  /* export palette data */
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_check_button_palette)))
    if (export_palette_data(gtk_entry_get_text(GTK_ENTRY(export_entry_palette))) == FAILED)
      return FAILED;

  return SUCCEEDED;
}


int export_tile_palette_data(char *n) {

  FILE *f;
  int a, b, c, d;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  a = end_x;
  b = end_y;
  if (tile == 8)
    init_export_data((a + 1) * (b + 1) * 4);
  else
    init_export_data((a + 1) * (b + 1) * tile);
  export_counter_max = 7;

  /* GBDK header for tile palette data */
  if (export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC tile palette data\n");
    fprintf(f, "   Tile size: %d x %d\n", edit_size_x, edit_size_y);
    fprintf(f, "   Tiles X:   %d\n", a + 1);
    fprintf(f, "   Tiles Y:   %d\n", b + 1);
    fprintf(f, "   Tiles:     %d */\n\n", (a + 1) * (b + 1));
    fprintf(f, "unsigned char %s_tpd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }

  if (export_format_format == FORMAT_RGBDS || export_format_format == FORMAT_WLA || export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC tile palette data\n");
    fprintf(f, "; Tile size: %d x %d\n", edit_size_x, edit_size_y);
    fprintf(f, "; Tiles X:   %d\n", a + 1);
    fprintf(f, "; Tiles Y:   %d\n", b + 1);
    fprintf(f, "; Tiles:     %d\n\n", (a + 1) * (b + 1));
  }

  for (d = 0; d <= b; d++)
    for (c = 0; c <= a; c++) {
      /* 8x8 */
      if (tile == 1)
	export_write_byte(f, palette_data[d * 32 + c]);
      /* 8x16 */
      else if (tile == 2) {
	export_write_byte(f, palette_data[d * 32 * 2 + c]);
	export_write_byte(f, palette_data[d * 32 * 2 + 32 + c]);
      }
      /* 16x16 H */
      else if (tile == 4) {
	export_write_byte(f, palette_data[d * 32 * 2 + c * 2]);
	export_write_byte(f, palette_data[d * 32 * 2 + c * 2 + 1]);
	export_write_byte(f, palette_data[d * 32 * 2 + 32 + c * 2]);
	export_write_byte(f, palette_data[d * 32 * 2 + 32 + c * 2 + 1]);
      }
      /* 16x16 V */
      else if (tile == 8) {
	export_write_byte(f, palette_data[d * 32 * 2 + c * 2]);
	export_write_byte(f, palette_data[d * 32 * 2 + 32 + c * 2]);
	export_write_byte(f, palette_data[d * 32 * 2 + c * 2 + 1]);
	export_write_byte(f, palette_data[d * 32 * 2 + 32 + c * 2 + 1]);
      }
    }

  /* GBDK footer for tile palette data */
  if (export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);

  return SUCCEEDED;
}


int export_palette_data(char *n) {

  FILE *f;
  int a, b;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  if (export_format_palette == PALETTE_16BIT)
    init_export_data(8 * 4 * 2);
  else
    init_export_data(8 * 4 * 3);
  export_counter_max = 7;

  /* header for palette data */
  if (export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC palette data\n");
    fprintf(f, "   Palettes: 8\n");
    fprintf(f, "   Colors:   8 x 4\n");
    fprintf(f, "   Format:   ");
    if (export_format_palette == PALETTE_16BIT)
      fprintf(f, "16bit */\n\n");
    else
      fprintf(f, "24bit */\n\n");
    fprintf(f, "unsigned char %s_pd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }

  if (export_format_format == FORMAT_RGBDS || export_format_format == FORMAT_WLA || export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC palette data\n");
    fprintf(f, "; Palettes: 8\n");
    fprintf(f, "; Colors:   8 x 4\n");
    fprintf(f, "; Format:   ");
    if (export_format_palette == PALETTE_16BIT)
      fprintf(f, "16bit\n\n");
    else
      fprintf(f, "24bit\n\n");
  }

  for (a = 0; a < 8; a++) {
    for (b = 0; b < 4; b++) {
      export_write_palette_byte(f, (unsigned char)(palette_colors[a * 4 * 3 + b * 3] / 8.2));
      export_write_palette_byte(f, (unsigned char)(palette_colors[a * 4 * 3 + b * 3 + 1] / 8.2));
      export_write_palette_byte(f, (unsigned char)(palette_colors[a * 4 * 3 + b * 3 + 2] / 8.2));
    }
  }

  /* GBDK footer for palette data */
  if (export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);

  return SUCCEEDED;
}


int export_tile_data(char *n) {

  FILE *f;
  int a, b, c, d, e, g;


  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  a = end_x;
  b = end_y;
  if (export_format_tile_data == EXPORT_FORMAT_8BIT_CHUNKY) {
    if (tile == 8)
      init_export_data((a + 1) * (b + 1) * 8 * 8 * 4);
    else
      init_export_data((a + 1) * (b + 1) * 8 * 8 * tile);
  }
  else {
    if (tile == 8)
      init_export_data((a + 1) * (b + 1) * 8 * 2 * 4);
    else
      init_export_data((a + 1) * (b + 1) * 8 * 2 * tile);
  }
  export_counter_max = 7;

  /* GBDK header for tile data */
  if (export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GB(C) tile data\n");
    fprintf(f, "   Tile order: ");
    if (tile == 1)
      fprintf(f, "8x8\n");
    else if (tile == 2)
      fprintf(f, "8x16\n");
    else if (tile == 4)
      fprintf(f, "16x16 Horizontal\n");
    else
      fprintf(f, "16x16 Vertical\n");
    fprintf(f, "   Tiles X:    %d\n", a + 1);
    fprintf(f, "   Tiles Y:    %d\n", b + 1);
    fprintf(f, "   Tiles:      %d\n", (a + 1) * (b + 1));
    fprintf(f, "   Format:     ");
    if (export_format_tile_data == EXPORT_FORMAT_2BIT_PLANAR_INTERLEAVED)
      fprintf(f, "2bit planar interleaved */\n\n");
    else if (export_format_tile_data == EXPORT_FORMAT_2BIT_PLANAR_NONINTERLEAVED)
      fprintf(f, "2bit planar non interleaved */\n\n");
    else
      fprintf(f, "8bit chunky */\n\n");
    fprintf(f, "unsigned char %s_td[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }

  if (export_format_format == FORMAT_RGBDS || export_format_format == FORMAT_WLA || export_format_format == FORMAT_TASM) {
    fprintf(f, "; GB(C) tile data\n");
    fprintf(f, "; Tile order: ");
    if (tile == 1)
      fprintf(f, "8x8\n");
    else if (tile == 2)
      fprintf(f, "8x16\n");
    else if (tile == 4)
      fprintf(f, "16x16 Horizontal\n");
    else
      fprintf(f, "16x16 Vertical\n");
    fprintf(f, "; Tiles X:    %d\n", a + 1);
    fprintf(f, "; Tiles Y:    %d\n", b + 1);
    fprintf(f, "; Tiles:      %d\n", (a + 1) * (b + 1));
    fprintf(f, "; Format:     ");
    if (export_format_tile_data == EXPORT_FORMAT_2BIT_PLANAR_INTERLEAVED)
      fprintf(f, "2bit planar interleaved\n\n");
    else if (export_format_tile_data == EXPORT_FORMAT_2BIT_PLANAR_INTERLEAVED)
      fprintf(f, "2bit planar non interleaved\n\n");
    else
      fprintf(f, "8bit chunky\n\n");
  }

  for (d = 0; d <= b; d++) {
    for (c = 0; c <= a; c++) {
      /* 8x8 */
      if (tile == 1)
	export_8x8_tile(f, &memory_data[(d<<11) + (c<<3)], palette_data[(d<<5) + c]);
      /* 8x16 */
      else if (tile == 2) {
	e = (d<<12) + (c<<3);
	g = (d<<6) + c;
	export_8x8_tile(f, &memory_data[e], palette_data[g]);
	export_8x8_tile(f, &memory_data[e + 2048], palette_data[g + 32]);
      }
      /* 16x16 H */
      else if (tile == 4) {
	e = (d<<12) + (c<<4);
	g = (d<<6) + (c<<1);
	export_8x8_tile(f, &memory_data[e], palette_data[g]);
	export_8x8_tile(f, &memory_data[e + 8], palette_data[g + 1]);
	export_8x8_tile(f, &memory_data[e + 2048], palette_data[g + 32]);
	export_8x8_tile(f, &memory_data[e + 2048 + 8], palette_data[g + 32 + 1]);
      }
      /* 16x16 V */
      else if (tile == 8) {
	e = (d<<12) + (c<<4);
	g = (d<<6) + (c<<1);
	export_8x8_tile(f, &memory_data[e], palette_data[g]);
	export_8x8_tile(f, &memory_data[e + 2048], palette_data[g + 32]);
	export_8x8_tile(f, &memory_data[e + 8], palette_data[g + 1]);
	export_8x8_tile(f, &memory_data[e + 2048 + 8], palette_data[g + 32 + 1]);
      }
    }
  }

  /* GBDK footer for tile data */
  if (export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);

  return SUCCEEDED;
}


int export_8x8_tile(FILE *f, unsigned char *d, unsigned char e) {

  unsigned char c;
  unsigned int a, b;


  if (export_format_tile_data == EXPORT_FORMAT_2BIT_PLANAR_INTERLEAVED) {
    for (a = 0; a < 8; a++) {
      for (c = 0, b = 0; b < 8; b++)
	c |= (d[(a<<8) + b] & 1) << (7 - b);
      export_write_byte(f, c);
      for (c = 0, b = 0; b < 8; b++)
	c |= ((d[(a<<8) + b] & 2) >> 1) << (7 - b);
      export_write_byte(f, c);
    }
  }
  else if (export_format_tile_data == EXPORT_FORMAT_2BIT_PLANAR_NONINTERLEAVED) {
    for (a = 0; a < 8; a++) {
      for (c = 0, b = 0; b < 8; b++)
	c |= (d[(a<<8) + b] & 1) << (7 - b);
      export_write_byte(f, c);
    }
    for (a = 0; a < 8; a++) {
      for (c = 0, b = 0; b < 8; b++)
	c |= ((d[(a<<8) + b] & 2) >> 1) << (7 - b);
      export_write_byte(f, c);
    }
  }
  else {
    for (a = 0; a < 8; a++)
      for (b = 0; b < 8; b++)
	export_write_byte(f, e + d[(a<<8) + b]);
  }

  return SUCCEEDED;
}


int export_write_byte(FILE *f, int c) {

  c &= 0xFF;

  if (eff == FORMAT_RAW)
    fprintf(f, "%c", c);
  else if (eff == FORMAT_GBDK) {
    fprintf(f, "0x%.2x", c);

    if (export_size == 1)
      fprintf(f, "\n");
    else {
      if (export_counter != export_counter_max) {
	fprintf(f, ", ");
	export_counter++;
      }
      else {
	fprintf(f, ",\n");
	export_counter = 0;
      }
    }

    export_size--;
  }
  else {
    /* line header */
    if (export_counter == 0) {
      if (eff == FORMAT_RGBDS)
	fprintf(f, "DB ");
      else if (eff == FORMAT_TASM)
	fprintf(f, ".byte ");
      else
	fprintf(f, ".DB ");
    }

    fprintf(f, "$%.2x", c);

    if (export_size == 1)
      fprintf(f, "\n");
    else {
      if (export_counter != export_counter_max) {
	fprintf(f, ", ");
	export_counter++;
      }
      else {
	fprintf(f, "\n");
	export_counter = 0;
      }
    }

    export_size--;
  }

  return SUCCEEDED;
}


int export_write_word(FILE *f, int c) {

  export_write_byte(f, c & 255);
  export_write_byte(f, (c>>8) & 255);

  return SUCCEEDED;
}


int export_write_palette_byte(FILE *f, unsigned char c) {

  static unsigned int a = 0;
  static unsigned char l, h;


  if (export_format_palette == PALETTE_24BIT)
    export_write_byte(f, ((int)c) & 31);
  else {
    /* R */
    if (a == 0) {
      l = c & 31;
      a++;
    }
    /* G */
    else if (a == 1) {
      l |= (c & 7) << 5;
      h = (c>>3) & 3;
      a++;
    }
    /* B */
    else {
      h |= (c & 31) << 2;
      a = 0;
      export_write_byte(f, l);
      export_write_byte(f, h);
    }
  }
  return SUCCEEDED;
}


gint export_button_tile_size(GtkWidget *widget, GdkEvent *event) {

  export_radio_button_tile_size = widget;

  export_adjust_end_spin_buttons(widget);
  return FALSE;
}


gint map_export_button_tile_size(GtkWidget *widget, GdkEvent *event) {

  export_adjust_end_spin_buttons(widget);
  return FALSE;
}


void export_adjust_end_spin_buttons(GtkWidget *widget) {

  GtkAdjustment *x, *y;


  if (widget == NULL)
    return;

  end_x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(export_spin_end_x));
  end_y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(export_spin_end_y));
  x = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(export_spin_end_x));
  y = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(export_spin_end_y));

  /* 8x8 */
  if (widget == export_radio_button_1) {
    x->upper = 31;

    if (memory_size == MEMORY_SIZE_4KB)
      y->upper = 7;
    else if (memory_size == MEMORY_SIZE_8KB)
      y->upper = 15;
    else if (memory_size == MEMORY_SIZE_12KB)
      y->upper = 23;
    else
      y->upper = 31;
  }
  /* 8x16 */
  else if (widget == export_radio_button_2) {
    x->upper = 31;

    if (memory_size == MEMORY_SIZE_4KB)
      y->upper = 3;
    else if (memory_size == MEMORY_SIZE_8KB)
      y->upper = 7;
    else if (memory_size == MEMORY_SIZE_12KB)
      y->upper = 11;
    else
      y->upper = 15;
  }
  /* 16x16 */
  else {
    x->upper = 15;

    if (memory_size == MEMORY_SIZE_4KB)
      y->upper = 3;
    else if (memory_size == MEMORY_SIZE_8KB)
      y->upper = 7;
    else if (memory_size == MEMORY_SIZE_12KB)
      y->upper = 11;
    else
      y->upper = 15;
  }

  if (end_x > x->upper)
    end_x = x->upper;
  if (end_y > y->upper)
    end_y = y->upper;

  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(export_spin_end_x), x);
  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(export_spin_end_y), y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(export_spin_end_x), end_x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(export_spin_end_y), end_y);
  gtk_widget_queue_draw(export_spin_end_x);
  gtk_widget_queue_draw(export_spin_end_y);
}


void export_format_tile_data_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data) {

  export_format_tile_data = row;
}


void export_format_format_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data) {

  export_format_format = row;
}


void export_format_palette_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data) {

  export_format_palette = row;
}


int init_export_data(unsigned int x) {

  export_counter = 0;
  export_size = x;

  return SUCCEEDED;
}


char *export_resolve_name(char *n) {

  static char o[256];
  int i, x, a;


  x = strlen(n);
  for (i = x - 1; i >= 0; i--) {
    if (n[i] == '/' || n[i] == '\\') {
      i++;
      break;
    }
  }

  if (i == -1)
    i = 0;

  for (a = 0; i < x; i++, a++) {
    if (n[i] == '.')
      break;
    o[a] = n[i];
  }

  o[a] = 0;

  return o;
}


void map_export(GtkWidget *widget, gpointer data) {

  gtk_widget_show_all(map_export_window);
}


gint map_export_button_map_data_get(GtkWidget *widget, GdkEventButton *event) {

  map_export_selection_status = MAP_EXPORT_SELECTION_MAP_DATA;
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_map_export), gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_data)));
  gtk_widget_show(file_selection_map_export);
  return FALSE;
}


gint map_export_button_map_property_data_get(GtkWidget *widget, GdkEventButton *event) {

  map_export_selection_status = MAP_EXPORT_SELECTION_MAP_PROPERTY_DATA;
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_map_export), gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_property_data)));
  gtk_widget_show(file_selection_map_export);
  return FALSE;
}


void map_export_ok(GtkWidget *widget, gpointer data) {

  if (map_export_selection_status == MAP_EXPORT_SELECTION_MAP_DATA)
    gtk_entry_set_text(GTK_ENTRY(map_export_entry_map_data), gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_map_export)));
  else if (map_export_selection_status == MAP_EXPORT_SELECTION_MAP_PROPERTY_DATA)
    gtk_entry_set_text(GTK_ENTRY(map_export_entry_map_property_data), gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_map_export)));

  gtk_widget_hide(file_selection_map_export);
}


void map_export_cancel(GtkWidget *widget, gpointer data) {

  gtk_widget_hide(file_selection_map_export);
}


gint map_export_export_pressed(GtkWidget *widget, GdkEventButton *event) {

  tile_offset = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(map_export_spin_offset));

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(map_export_radio_button_1)))
    map_mode = MAP_EXPORT_MODE_NORMAL;
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(map_export_radio_button_2)))
    map_mode = MAP_EXPORT_MODE_16x16_METATILE_F;
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(map_export_radio_button_3)))
    map_mode = MAP_EXPORT_MODE_16x16_METATILE_D;
  else
    map_mode = MAP_EXPORT_MODE_16x16_METATILE_L;

  if (map_export_selected_data() == FAILED)
    return FAILED;
  gtk_widget_hide(map_export_window);
  return FALSE;
}


gint map_export_cancel_pressed(GtkWidget *widget, GdkEventButton *event) {

  gtk_widget_hide_all(map_export_window);
  return FALSE;
}


int map_export_selected_data(void) {

  get_map_dimensions();

  /* map data format */
  eff = map_export_format_format;

  if (map_mode == MAP_EXPORT_MODE_16x16_METATILE_D)
    if (build_metatile_map() == FAILED)
      return FAILED;

  /* export map data */
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(map_export_check_button_map_data))) {

    if (map_mode != MAP_EXPORT_MODE_NORMAL) {
      if (map_end_x & 1 || map_end_y & 1) {
	fprintf(stderr, "If you wish to export 16x16 metatiles you must build the map from 16x16 blocks.\n");
	return FAILED;
      }
    }

    if (map_mode == MAP_EXPORT_MODE_NORMAL) {
      if (map_export_map_data(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_data))) == FAILED)
	return FAILED;
    }
    else if (map_mode == MAP_EXPORT_MODE_16x16_METATILE_F) {
      if (map_export_map_data_16x16_f(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_data))) == FAILED)
	return FAILED;
    }
    else if (map_mode == MAP_EXPORT_MODE_16x16_METATILE_L) {
      if (map_export_map_data_16x16_l(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_data))) == FAILED)
	return FAILED;
    }
    else {
      if (map_export_map_data_16x16_d(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_data))) == FAILED) {
	free_metatile_map();
	return FAILED;
      }
    }
  }

  /* export map property data */
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(map_export_check_button_map_property_data))) {
    if (map_mode == MAP_EXPORT_MODE_NORMAL) {
      if (map_export_map_property_data(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_property_data))) == FAILED)
	return FAILED;
    }
    else if (map_mode == MAP_EXPORT_MODE_16x16_METATILE_F) {
      if (map_export_map_property_data_16x16_f(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_property_data))) == FAILED)
	return FAILED;
    }
    else if (map_mode == MAP_EXPORT_MODE_16x16_METATILE_L) {
      m_first = mt_first;
      mt = metatiles;
      if (map_export_map_property_data_16x16_d(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_property_data))) == FAILED)
	return FAILED;
    }
    else {
      if (map_export_map_property_data_16x16_d(gtk_entry_get_text(GTK_ENTRY(map_export_entry_map_property_data))) == FAILED) {
	free_metatile_map();
	return FAILED;
      }
    }
  }

  if (map_mode == MAP_EXPORT_MODE_16x16_METATILE_D)
    free_metatile_map();

  return SUCCEEDED;
}


int map_export_map_data_16x16_f(char *n) {

  FILE *f;
  int k, a, b;


  init_export_data((map_end_x * map_end_y)>>2);
  export_counter_max = (map_end_x>>1) - 1;

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC 16x16 metatile map data\n");
    fprintf(f, "   Tiles X: %d\n", map_end_x>>1);
    fprintf(f, "   Tiles Y: %d\n", map_end_y>>1);
    fprintf(f, "   Tiles:   %d */\n\n", (map_end_x * map_end_y)>>2);
    fprintf(f, "unsigned char %s_mmd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC 16x16 metatile map data\n");
    fprintf(f, "; Tiles X: %d\n", map_end_x>>1);
    fprintf(f, "; Tiles Y: %d\n", map_end_y>>1);
    fprintf(f, "; Tiles:   %d\n\n", (map_end_x * map_end_y)>>2);
  }

  /* write data */
  for (a = 0; a < map_end_y; a += 2) {
    for (b = 0; b < map_end_x; b += 2) {
      k = map_data[(a<<10) + b];
      if (!(map_data[(a<<10) + b + 1] == (k+1) && map_data[((a+1)<<10) + b] == (k+32) &&
	    map_data[((a+1)<<10) + b + 1] == (k+33)) || k & 1 || k & 32) {
	fprintf(stderr, "You must build the map from fixed 16x16 memory tiles. 16x16 map tile (X,Y) == (%d..%d, %d..%d) is incorrect.\n",
		b, b+1, a, a+1);
	fclose(f);
	return FAILED;
      }
      k = ((k>>6)<<4) + ((k&31)>>1);
      export_write_byte(f, k);
    }
  }

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);
  return SUCCEEDED;
}


int map_export_map_data_16x16_d(char *n) {

  FILE *f;
  int i;


  init_export_data(mx * my);
  export_counter_max = mx - 1;

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC 16x16 metatile map data\n");
    fprintf(f, "   Tiles X: %d\n", mx);
    fprintf(f, "   Tiles Y: %d\n", my);
    fprintf(f, "   Tiles:   %d */\n\n", mx*my);
    fprintf(f, "unsigned char %s_mmd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC 16x16 metatile map data\n");
    fprintf(f, "; Tiles X: %d\n", mx);
    fprintf(f, "; Tiles Y: %d\n", my);
    fprintf(f, "; Tiles:   %d\n\n", mx*my);
  }

  for (i = 0; i < mx*my; i++)
    export_write_byte(f, map[i]);

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);
  return SUCCEEDED;
}


int map_export_map_data(char *n) {

  unsigned int a, b;
  FILE *f;


  if (map_export_format_size == SIZE_8BIT) {
    init_export_data(map_end_x * map_end_y);
    export_counter_max = map_end_x - 1;
  }
  else {
    init_export_data((map_end_x * map_end_y)<<1);
    export_counter_max = (map_end_x<<1) - 1;
  }

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC map data\n");
    fprintf(f, "   Tiles X:         %d\n", map_end_x);
    fprintf(f, "   Tiles Y:         %d\n", map_end_y);
    fprintf(f, "   Tiles:           %d\n", map_end_x * map_end_y);
    fprintf(f, "   Tile index size: %dbit */\n\n", (map_export_format_size + 1) * 8);
    fprintf(f, "unsigned char %s_md[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC map data\n");
    fprintf(f, "; Tiles X:         %d\n", map_end_x);
    fprintf(f, "; Tiles Y:         %d\n", map_end_y);
    fprintf(f, "; Tiles:           %d\n", map_end_x * map_end_y);
    fprintf(f, "; Tile index size: %dbit\n\n", (map_export_format_size + 1) * 8);
  }

  /* bytes */
  if (map_export_format_size == SIZE_8BIT) {
    for (a = 0; a < map_end_y; a++)
      for (b = 0; b < map_end_x; b++)
	export_write_byte(f, (unsigned char)(map_data[(a<<10) + b] + tile_offset));
  }
  /* words */
  else {
    for (a = 0; a < map_end_y; a++)
      for (b = 0; b < map_end_x; b++)
	export_write_word(f, map_data[(a<<10) + b] + tile_offset);
  }

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);
  return SUCCEEDED;
}


int map_export_map_property_data_16x16_f(char *n) {

  int a, b, c, d, q, k;
  FILE *f;


  if (map_export_format_size == SIZE_8BIT) {
    init_export_data(memory_size<<3);
    export_counter_max = 7;
  }
  else {
    init_export_data(memory_size*12);
    export_counter_max = 11;
  }

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC 16x16 metatile map property data\n");
    fprintf(f, "   Metatiles:       %d\n", memory_size);
    fprintf(f, "   Tile index size: %dbit */\n\n", (map_export_format_size + 1) * 8);
    fprintf(f, "unsigned char %s_mmpd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC 16x16 metatile map property data\n");
    fprintf(f, "; Metatiles:       %d\n", memory_size);
    fprintf(f, "; Tile index size: %dbit\n\n", (map_export_format_size + 1) * 8);
  }

  for (q = 0, a = 0; a < memory_size>>3; a += 2, q += 32) {
    for (b = 0; b < 32; b += 2, q += 2) {
      if (map_export_format_size == SIZE_8BIT) {
	export_write_byte(f, q);
	export_write_byte(f, q+1);
	export_write_byte(f, q+32);
	export_write_byte(f, q+33);
      }
      else {
	export_write_word(f, q);
	export_write_word(f, q+1);
	export_write_word(f, q+32);
	export_write_word(f, q+33);
      }

      k = (a<<5) + b;
      c = palette_data[k];
      /* tile bank */
      d = ((q>>8) & 1)<<3;
      /* palette */
      d |= (c & 7);
      export_write_byte(f, d);

      k = (a<<5) + b + 1;
      c = palette_data[k];
      /* tile bank */
      d = (((q+1)>>8) & 1)<<3;
      /* palette */
      d |= (c & 7);
      export_write_byte(f, d);

      k = ((a+1)<<5) + b;
      c = palette_data[k];
      /* tile bank */
      d = (((q+32)>>8) & 1)<<3;
      /* palette */
      d |= (c & 7);
      export_write_byte(f, d);

      k = ((a+1)<<5) + b + 1;
      c = palette_data[k];
      /* tile bank */
      d = (((q+33)>>8) & 1)<<3;
      /* palette */
      d |= (c & 7);
      export_write_byte(f, d);
    }
  }

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);
  return SUCCEEDED;
}


int map_export_map_property_data_16x16_d(char *n) {

  struct metatile_16x16 *m1;
  FILE *f;
  int i, d;


  if (map_export_format_size == SIZE_8BIT) {
    init_export_data(mt<<3);
    export_counter_max = 7;
  }
  else {
    init_export_data(mt*12);
    export_counter_max = 11;
  }

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC 16x16 metatile map property data\n");
    fprintf(f, "   Metatiles:       %d\n", mt);
    fprintf(f, "   Tile index size: %dbit */\n\n", (map_export_format_size + 1) * 8);
    fprintf(f, "unsigned char %s_mmpd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC 16x16 metatile map property data\n");
    fprintf(f, "; Metatiles:       %d\n", mt);
    fprintf(f, "; Tile index size: %dbit\n\n", (map_export_format_size + 1) * 8);
  }

  m1 = m_first;
  for (i = 0; i < mt; i++) {
    if (map_export_format_size == SIZE_8BIT) {
      export_write_byte(f, m1->t1);
      export_write_byte(f, m1->t2);
      export_write_byte(f, m1->t3);
      export_write_byte(f, m1->t4);
    }
    else {
      export_write_word(f, m1->t1);
      export_write_word(f, m1->t2);
      export_write_word(f, m1->t3);
      export_write_word(f, m1->t4);
    }

    d = ppm_to_cgb(m1->t1, m1->p1);
    export_write_byte(f, d);
    d = ppm_to_cgb(m1->t2, m1->p2);
    export_write_byte(f, d);
    d = ppm_to_cgb(m1->t3, m1->p3);
    export_write_byte(f, d);
    d = ppm_to_cgb(m1->t4, m1->p4);
    export_write_byte(f, d);

    m1 = m1->next;
  }

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);
  return SUCCEEDED;
}


int map_export_map_property_data(char *n) {

  unsigned int a, b;
  unsigned char d;
  FILE *f;


  init_export_data(map_end_x * map_end_y);
  export_counter_max = map_end_x - 1;

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC map property data\n");
    fprintf(f, "   Tiles X: %d\n", map_end_x);
    fprintf(f, "   Tiles Y: %d\n", map_end_y);
    fprintf(f, "   Tiles:   %d */\n\n", map_end_x * map_end_y);
    fprintf(f, "unsigned char %s_mpd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC map property data\n");
    fprintf(f, "; Tiles X: %d\n", map_end_x);
    fprintf(f, "; Tiles Y: %d\n", map_end_y);
    fprintf(f, "; Tiles:   %d\n\n", map_end_x * map_end_y);
  }

  /* write property data */
  for (a = 0; a < map_end_y; a++)
    for (b = 0; b < map_end_x; b++) {
      d = ppm_to_cgb(map_data[(a<<10)+b], map_property_data[(a<<10)+b]);
      export_write_byte(f, d);
    }

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);
  return SUCCEEDED;
}


void map_export_format_format_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data) {

  map_export_format_format = row;
}


void map_export_format_size_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data) {

  map_export_format_size = row;
}


int get_map_dimensions(void) {

  int i;


  /* get map max x */
  for (map_end_x = 0, i = 0; i < 1024; i++)
    if (map_property_data[i] & 128)
      map_end_x = i;

  /* get map max y */
  for (map_end_y = 0, i = 0; i < 1024; i++)
    if (map_property_data[i<<10] & 128)
      map_end_y = i;

  map_end_x++;
  map_end_y++;

  return SUCCEEDED;
}


int ppm_to_cgb(int t, int p) {

  int d;


  d = 0;
  /* tile bank */
  d = ((t>>8) & 1)<<3;
  /* palette */
  d |= (p & 7);
  /* x-flip */
  d |= ((p & 8)<<2);
  /* y-flip */
  d |= ((p & 16)<<2);

  return d;
}


int map_export_map_data_16x16_l(char *n) {

  struct metatile_16x16 *m1;
  FILE *f;
  int i, a, b, c, res;


  mx = map_end_x>>1;
  my = map_end_y>>1;
  if (mx * my == 0)
    return FAILED;

  init_export_data(mx * my);
  export_counter_max = mx - 1;

  f = fopen(n, "wb");
  if (f == NULL) {
    fprintf(stderr, "Could not write to \"%s\".\n", n);
    return FAILED;
  }

  /* GBDK header */
  if (map_export_format_format == FORMAT_GBDK) {
    fprintf(f, "/* GBC 16x16 metatile map data\n");
    fprintf(f, "   Tiles X: %d\n", mx);
    fprintf(f, "   Tiles Y: %d\n", my);
    fprintf(f, "   Tiles:   %d */\n\n", mx*my);
    fprintf(f, "unsigned char %s_mmd[] =\n", export_resolve_name(n));
    fprintf(f, "{\n");
  }
  /* asm header */
  else if (map_export_format_format == FORMAT_RGBDS || map_export_format_format == FORMAT_WLA || map_export_format_format == FORMAT_TASM) {
    fprintf(f, "; GBC 16x16 metatile map data\n");
    fprintf(f, "; Tiles X: %d\n", mx);
    fprintf(f, "; Tiles Y: %d\n", my);
    fprintf(f, "; Tiles:   %d\n\n", mx*my);
  }

  /* match tiles */
  for (i = 0, a = 0; a < map_end_y; a += 2) {
    for (b = 0; b < map_end_x; b += 2, i++) {
      c = (a<<10) + b;
      m1 = mt_first;
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
      if (m1 == NULL)
	fprintf(stderr, "Unknown metatile found at (%d, %d).\n", b, a);

      export_write_byte(f, res);
    }
  }

  /* GBDK footer */
  if (map_export_format_format == FORMAT_GBDK)
    fprintf(f, "};\n");

  fclose(f);

  return SUCCEEDED;
}
