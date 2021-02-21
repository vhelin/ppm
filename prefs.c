
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


extern unsigned char palette_colors[3 * 4 * 8];
extern unsigned char palette_colors_real[3 * 4 * 8];
extern unsigned int memory_window_x, memory_window_y, edit_window_x, edit_window_y, tile_window_x, tile_window_y, palette_window_x, palette_window_y;
extern unsigned int map_window_x, map_window_y, map_window_loaded;
extern unsigned int metatile_window_x, metatile_window_y, metatile_window_loaded;
extern unsigned int memory_window_loaded, edit_window_loaded, tile_window_loaded, palette_window_loaded;
extern unsigned int x, y, palette_mode, memory_check_status;
extern unsigned int edit_mode, edit_grid, edit_grid_loaded, palette_mode_loaded;
extern unsigned int zoom_mode, tile_zoom_mode, memory_size, metatile_window_status;
extern unsigned int tile_window_status, palette_window_status, map_window_status, map_zoom_mode, map_check_status;

extern GtkWidget *edit_window, *memory_window, *tile_window, *palette_window, *map_window, *metatile_window;




int load_prefs(void) {

  unsigned char t[256];
  FILE *f;
  char *e;
  int i, k, v;


  i = SUCCEEDED;

#ifdef WIN32
  strcpy(t, "ppmconfig");
#else

  e = getenv("HOME");
  if (e == NULL)
    return FAILED;

  strcpy(t, e);
  strcat(t, "/.ppm/config");
#endif // WIN32

  f = fopen(t, "rb");

  if (f == NULL)
    return FAILED;

  while (fscanf(f, "%s", t) != EOF) {

    /* EditGridStatus */
    if (strcmp(t, "EditGridStatus") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0) {
	edit_grid = ON;
	edit_grid_loaded = ON;
      }
      else if (strcmp(t, "OFF") == 0) {
	edit_grid = OFF;
	edit_grid_loaded = OFF;
	}
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* EditWindow */
    if (strcmp(t, "EditWindow") == 0) {
      if (fscanf(f, "%d %d", &edit_window_x, &edit_window_y) == EOF)
	return FAILED;
      edit_window_loaded = ON;
      continue;
    }

    /* PaletteWindow */
    if (strcmp(t, "PaletteWindow") == 0) {
      if (fscanf(f, "%d %d", &palette_window_x, &palette_window_y) == EOF)
	return FAILED;
      palette_window_loaded = ON;
      continue;
    }

    /* MemoryWindow */
    if (strcmp(t, "MemoryWindow") == 0) {
      if (fscanf(f, "%d %d", &memory_window_x, &memory_window_y) == EOF)
	return FAILED;
      memory_window_loaded = ON;
      continue;
    }

    /* TileWindow */
    if (strcmp(t, "TileWindow") == 0) {
      if (fscanf(f, "%d %d", &tile_window_x, &tile_window_y) == EOF)
	return FAILED;
      tile_window_loaded = ON;
      continue;
    }

    /* MapWindow */
    if (strcmp(t, "MapWindow") == 0) {
      if (fscanf(f, "%d %d", &map_window_x, &map_window_y) == EOF)
	return FAILED;
      map_window_loaded = ON;
      continue;
    }

    /* MetatileWindow */
    if (strcmp(t, "MetatileWindow") == 0) {
      if (fscanf(f, "%d %d", &metatile_window_x, &metatile_window_y) == EOF)
	return FAILED;
      metatile_window_loaded = ON;
      continue;
    }

    /* TileWindowStatus */
    if (strcmp(t, "TileWindowStatus") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0)
	tile_window_status = ON;
      else if (strcmp(t, "OFF") == 0)
	tile_window_status = OFF;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* MapWindowStatus */
    if (strcmp(t, "MapWindowStatus") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0)
	map_window_status = ON;
      else if (strcmp(t, "OFF") == 0)
	map_window_status = OFF;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* MetatileWindowStatus */
    if (strcmp(t, "MetatileWindowStatus") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0)
	metatile_window_status = ON;
      else if (strcmp(t, "OFF") == 0)
	metatile_window_status = OFF;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* PaletteX */
    if (strcmp(t, "Palette0") == 0 || strcmp(t, "Palette1") == 0 || strcmp(t, "Palette2") == 0 || strcmp(t, "Palette3") == 0 ||
	strcmp(t, "Palette4") == 0 || strcmp(t, "Palette5") == 0 || strcmp(t, "Palette6") == 0 || strcmp(t, "Palette7") == 0) {
      k = t[7] - '0';
      for (v = 0; v < 4 * 3; v++)
	if (fscanf(f, "%d", (int *)&palette_colors_real[k * 4 * 3 + v]) == EOF)
	  return FAILED;
      continue;
    }

    /* MemoryTileAdjust */
    if (strcmp(t, "MemoryTileAdjust") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0)
	memory_check_status = ON;
      else if (strcmp(t, "OFF") == 0)
	memory_check_status = OFF;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* MapTileAdjust */
    if (strcmp(t, "MapTileAdjust") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0)
	map_check_status = ON;
      else if (strcmp(t, "OFF") == 0)
	map_check_status = OFF;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* PaletteWindowStatus */
    if (strcmp(t, "PaletteWindowStatus") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "ON") == 0)
	palette_window_status = ON;
      else if (strcmp(t, "OFF") == 0)
	palette_window_status = OFF;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* PaletteMode */
    if (strcmp(t, "PaletteMode") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "PC") == 0)
	palette_mode = PALETTE_MODE_PC;
      else if (strcmp(t, "GBC") == 0)
	palette_mode = PALETTE_MODE_RUS;
      else if (strcmp(t, "JED") == 0)
	palette_mode = PALETTE_MODE_JED;
      else {
	i = FAILED;
	break;
      }
      if (i != FAILED)
	palette_mode_loaded = ON;
      continue;
    }

    /* MemorySize */
    if (strcmp(t, "MemorySize") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "4KB") == 0)
	memory_size = MEMORY_SIZE_4KB;
      else if (strcmp(t, "8KB") == 0)
	memory_size = MEMORY_SIZE_8KB;
      else if (strcmp(t, "12KB") == 0)
	memory_size = MEMORY_SIZE_12KB;
      else if (strcmp(t, "16KB") == 0)
	memory_size = MEMORY_SIZE_16KB;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* EditMode */
    if (strcmp(t, "EditMode") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "8x8") == 0)
	edit_mode = EDIT_MODE_8;
      else if (strcmp(t, "8x16") == 0)
	edit_mode = EDIT_MODE_8x16;
      else if (strcmp(t, "16x16") == 0)
	edit_mode = EDIT_MODE_16;
      else if (strcmp(t, "32x32") == 0)
	edit_mode = EDIT_MODE_32;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* MemoryZoom */
    if (strcmp(t, "MemoryZoom") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "1:1") == 0)
	zoom_mode = ZOOM_MODE_1;
      else if (strcmp(t, "2:1") == 0)
	zoom_mode = ZOOM_MODE_2;
      else if (strcmp(t, "2:1GBC") == 0)
	zoom_mode = ZOOM_MODE_3;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* TileZoom */
    if (strcmp(t, "TileZoom") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "1:1") == 0)
	tile_zoom_mode = ZOOM_MODE_1;
      else if (strcmp(t, "2:1") == 0)
	tile_zoom_mode = ZOOM_MODE_2;
      else if (strcmp(t, "2:1GBC") == 0)
	tile_zoom_mode = ZOOM_MODE_3;
      else {
	i = FAILED;
	break;
      }
      continue;
    }

    /* MapZoom */
    if (strcmp(t, "MapZoom") == 0) {
      if (fscanf(f, "%s", t) == EOF)
	return FAILED;
      if (strcmp(t, "1:1") == 0)
	map_zoom_mode = ZOOM_MODE_1;
      else if (strcmp(t, "2:1") == 0)
	map_zoom_mode = ZOOM_MODE_2;
      else if (strcmp(t, "2:1GBC") == 0)
	map_zoom_mode = ZOOM_MODE_3;
      else {
	i = FAILED;
	break;
      }
      continue;
    }
  }

  if (i == FAILED) {
    fprintf(stderr, "LOAD_PREFS: Unknown symbol \"%s\".\n", t);
    return FAILED;
  }

  return SUCCEEDED;
}


void prefs_save(GtkWidget *widget, gpointer data) {

  unsigned char t[256];
  FILE *f;
  char *e;
  int i, a;
  gint x, y;


#ifdef WIN32
  strcpy(t, "ppmconfig");
#else

  e = getenv("HOME");
  if (e == NULL)
    return;

  strcpy(t, e);
  strcat(t, "/.ppm");
  mkdir(t, 477);

  strcat(t, "/config");
#endif

  f = fopen(t, "wb");

  if (f == NULL)
    return;

  gdk_window_get_position(edit_window->window, &x, &y);
  fprintf(f, "EditWindow %d %d\n", x, y);

  gdk_window_get_position(memory_window->window, &x, &y);
  fprintf(f, "MemoryWindow %d %d\n", x, y);

  if (tile_window_status == ON)
    gdk_window_get_position(tile_window->window, &tile_window_x, &tile_window_y);
  fprintf(f, "TileWindow %d %d\n", tile_window_x, tile_window_y);

  if (palette_window_status == ON)
    gdk_window_get_position(palette_window->window, &palette_window_x, &palette_window_y);
  fprintf(f, "PaletteWindow %d %d\n", palette_window_x, palette_window_y);

  if (map_window_status == ON)
    gdk_window_get_position(map_window->window, &map_window_x, &map_window_y);
  fprintf(f, "MapWindow %d %d\n", map_window_x, map_window_y);

  if (metatile_window_status == ON)
    gdk_window_get_position(metatile_window->window, &metatile_window_x, &metatile_window_y);
  fprintf(f, "MetatileWindow %d %d\n", metatile_window_x, metatile_window_y);

  fprintf(f, "PaletteWindowStatus ");
  if (palette_window_status == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "TileWindowStatus ");
  if (tile_window_status == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "MapWindowStatus ");
  if (map_window_status == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "MetatileWindowStatus ");
  if (metatile_window_status == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "EditGridStatus ");
  if (edit_grid == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "MemoryTileAdjust ");
  if (memory_check_status == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "MapTileAdjust ");
  if (map_check_status == OFF)
    fprintf(f, "OFF\n");
  else
    fprintf(f, "ON\n");

  fprintf(f, "PaletteMode ");
  if (palette_mode == PALETTE_MODE_PC)
    fprintf(f, "PC\n");
  else if (palette_mode == PALETTE_MODE_RUS)
    fprintf(f, "GBC\n");
  else
    fprintf(f, "JED\n");

  fprintf(f, "TileZoom ");
  if (tile_zoom_mode == ZOOM_MODE_1)
    fprintf(f, "1:1\n");
  else if (tile_zoom_mode == ZOOM_MODE_2)
    fprintf(f, "2:1\n");
  else
    fprintf(f, "2:1GBC\n");

  fprintf(f, "MemoryZoom ");
  if (zoom_mode == ZOOM_MODE_1)
    fprintf(f, "1:1\n");
  else if (zoom_mode == ZOOM_MODE_2)
    fprintf(f, "2:1\n");
  else
    fprintf(f, "2:1GBC\n");

  fprintf(f, "MapZoom ");
  if (map_zoom_mode == ZOOM_MODE_1)
    fprintf(f, "1:1\n");
  else if (map_zoom_mode == ZOOM_MODE_2)
    fprintf(f, "2:1\n");
  else
    fprintf(f, "2:1GBC\n");

  fprintf(f, "EditMode ");
  if (edit_mode == EDIT_MODE_8)
    fprintf(f, "8x8\n");
  else if (edit_mode == EDIT_MODE_8x16)
    fprintf(f, "8x16\n");
  else if (edit_mode == EDIT_MODE_16)
    fprintf(f, "16x16\n");
  else
    fprintf(f, "32x32\n");

  fprintf(f, "MemorySize ");
  if (memory_size == MEMORY_SIZE_4KB)
    fprintf(f, "4KB\n");
  else if (memory_size == MEMORY_SIZE_8KB)
    fprintf(f, "8KB\n");
  else if (memory_size == MEMORY_SIZE_12KB)
    fprintf(f, "12KB\n");
  else
    fprintf(f, "16KB\n");

  for (i = 0; i < 8; i++) {
    fprintf(f, "Palette%d", i);
    for (a = 0; a < 4; a++)
      fprintf(f, " %d %d %d", palette_colors_real[i * 3 * 4 + a * 3], palette_colors_real[i * 3 * 4 + a * 3 + 1], palette_colors_real[i * 3 * 4 + a * 3 + 2]);
    fprintf(f, "\n");
  }

  fclose(f);
}
