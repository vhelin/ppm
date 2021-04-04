
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
#include "include/import.h"
#include "include/map.h"
#include "include/refresh.h"


extern unsigned char palette_colors_real[3 * 4 * 8];
extern unsigned char memory_data[8 * 8 * 1024];
extern unsigned char palette_data[1024];
extern unsigned char *map_property_data;
extern unsigned int *map_data, edit_size_x, edit_size_y;
extern GtkWidget *file_selection_import_gbr, *file_selection_import_gbm;

char fn[256];



void import_gbr(GtkWidget *widget, gpointer data) {

  gtk_widget_show_all(file_selection_import_gbr);
}


void import_ok_gbr(GtkWidget *widget, gpointer data) {

  strcpy(fn, gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_import_gbr)));
  gtk_widget_hide_all(file_selection_import_gbr);
  import_gbr_file(fn);
}


void import_cancel_gbr(GtkWidget *widget, gpointer data) {

  gtk_widget_hide_all(file_selection_import_gbr);
}


void import_gbr_file(char *f) {

  unsigned char *d;
  unsigned int x, i, ot, oi, rl, w, h;
  FILE *a;


  a = fopen(f, "rb");
  if (a == NULL) {
    fprintf(stderr, "Could not open \"%s\".\n", f);
    return;
  }

  fseek(a, 0, SEEK_END);
  x = ftell(a);
  fseek(a, 0, SEEK_SET);

  if (x <= 4) {
    fclose(a);
    return;
  }

  d = malloc(x);

  if (d == NULL) {
    fprintf(stderr, "Could not allocare %d bytes.\n", x);
    fclose(a);
    return;
  }

  fread(d, 1, x, a);
  fclose(a);

  /* check header validity */
  if (strncmp(d, "GBO0", 4) != 0) {
    fprintf(stderr, "\"%s\" is not a real GBR file.\n", f);
    free(d);
    return;
  }

  i = 4;
  w = 0;
  h = 0;

  /* parse objects */
  while (i < x) {
    ot = d[i + 1] * 256 + d[i];
    i += 2;
    oi = d[i + 1] * 256 + d[i];
    i += 2;
    rl = d[i + 3] * 256 * 256 * 256 + d[i + 2] * 256 * 256 + d[i + 1] * 256 + d[i];
    i += 4;

    /* tile data */
    if (ot == 0x0002) {
      unsigned int a, b, c, t, u, x, y;


      u = i + 30;
      w = d[u + 1] * 256 + d[u];
      u += 2;
      h = d[u + 1] * 256 + d[u];
      u += 2;
      c = d[u + 1] * 256 + d[u];
      u += 2;
      u += 4;

      for (a = 0, b = 0, t = 0; t < c; t++) {
	for (y = b; y < b + h; y++)
	  for (x = a; x < a + w; x++)
	    memory_data[y * 256 + x] = d[u++];

	a += w;
	if (a >= 256) {
	  a -= 256;
	  b += h;
	}
      }
    }
    /* palettes */
    else if (ot == 0x000D) {
      unsigned int id, c, u, b;


      u = i;
      id = d[u + 1] * 256 + d[u];
      u += 2;
      c = d[u + 1] * 256 + d[u];
      u += 2;

      for (b = 0; b < c * 4; b++) {
	palette_colors_real[b * 3] = d[u++];
	palette_colors_real[b * 3 + 1] = d[u++];
	palette_colors_real[b * 3 + 2] = d[u++];
	u++;
      }
    }
    /* tile palette */
    else if (ot == 0x000E) {
      unsigned int id, c, u, b, x, y, a, g;


      u = i;
      id = d[u + 1] * 256 + d[u];
      u += 2;
      c = d[u + 1] * 256 + d[u];
      u += 2;

      for (x = 0, y = 0, b = 0; b < c * (w / 8) * (h / 8); b++) {
	for (a = y; a < y + (h / 8); a++)
	  for (g = x; g < x + (w / 8); g++)
	    palette_data[a * 32 + g] = d[u];

	x += w / 8;
	if (x >= 32) {
	  x -= 32;
	  y += h / 8;
	}

	u++;
      }
    }

    i += rl;
  }

  free(d);

  palette_mode_changed();
}


void import_gbm(GtkWidget *widget, gpointer data) {

  gtk_widget_show_all(file_selection_import_gbm);
}


void import_ok_gbm(GtkWidget *widget, gpointer data) {

  strcpy(fn, gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_import_gbm)));
  gtk_widget_hide_all(file_selection_import_gbm);
  import_gbm_file(fn);
}


void import_cancel_gbm(GtkWidget *widget, gpointer data) {

  gtk_widget_hide_all(file_selection_import_gbm);
}


void import_gbm_file(char *f) {

  unsigned char *d;
  unsigned int x, i, id, k, dx = 0, dy = 0, z, za, zb, p, t, r;
  FILE *a;


  a = fopen(f, "rb");
  if (a == NULL) {
    fprintf(stderr, "Could not open \"%s\".\n", f);
    return;
  }

  fseek(a, 0, SEEK_END);
  x = ftell(a);
  fseek(a, 0, SEEK_SET);

  if (x <= 4) {
    fclose(a);
    return;
  }

  d = malloc(x);

  if (d == NULL) {
    fprintf(stderr, "Could not allocare %d bytes.\n", x);
    fclose(a);
    return;
  }

  fread(d, 1, x, a);
  fclose(a);

  /* check header validity */
  if (strncmp(d, "GBO1", 4) != 0) {
    fprintf(stderr, "\"%s\" is not a real GBM file.\n", f);
    free(d);
    return;
  }

  i = 4;

  /* clear old map */
  memset(map_property_data, 0, 1024*1024);

  while (i < x) {
    i += 6;
    id = d[i + 1] * 256 + d[i];
    i += 10;
    k = d[i + 3] * 256 * 256 * 256 + d[i + 2] * 256 * 256 + d[i + 1] * 256 + d[i];
    i += 4;

    /* map properties */
    if (id == 0x002) {
      z = i + 128;
      dx = d[z + 3] * 256 * 256 * 256 + d[z + 2] * 256 * 256 + d[z + 1] * 256 + d[z];
      z += 4;
      dy = d[z + 3] * 256 * 256 * 256 + d[z + 2] * 256 * 256 + d[z + 1] * 256 + d[z];
    }
    /* map tile data */
    else if (id == 0x003) {
      z = i;
      /* 8x8 */
      if (edit_size_x == 8 && edit_size_y == 8) {
	for (za = 0; za < dy; za++)
	  for (zb = 0; zb < dx; zb++) {
	    map_data[(za<<10) + zb] = d[z + 2] + ((d[z + 1] & 3)<<8);
	    map_property_data[(za<<10) + zb] = 128 | ((d[z] & 64)>>3) | ((d[z] & 128)>>3);
	    p = (d[z + 1]>>2) & 7;
	    if (p != 0)
	      p--;
	    map_property_data[(za<<10) + zb] |= p;
	    z += 3;
	  }
      }
      /* 8x16 */
      else if (edit_size_x == 8 && edit_size_y == 16) {
	for (za = 0; za < dy; za++) {
	  for (zb = 0; zb < dx; zb++) {
	    t = d[z + 2] + ((d[z + 1] & 3)<<8);
	    t = (t / 32) * 64 + (t % 32);
	    r = 128 | ((d[z] & 64)>>3) | ((d[z] & 128)>>3);
	    p = (d[z + 1]>>2) & 7;
	    if (p != 0)
	      p--;
	    r |= p;
	    /* y-flip? */
	    if (r & 16) {
	      map_data[(za<<11) + zb] = t + 32;
	      map_data[(za<<11) + 1024 + zb] = t;
	    }
	    /* no y-flip */
	    else {
	      map_data[(za<<11) + zb] = t;
	      map_data[(za<<11) + 1024 + zb] = t + 32;
	    }

	    map_property_data[(za<<11) + zb] = r;
	    map_property_data[(za<<11) + 1024 + zb] = r;
	    z += 3;
	  }
	}
      }
      /* 16x16 */
      else if (edit_size_x == 16 && edit_size_y == 16) {
	for (za = 0; za < dy; za++) {
	  for (zb = 0; zb < dx; zb++) {
	    t = d[z + 2] + ((d[z + 1] & 3)<<8);
	    t = (t / 16) * 64 + (t % 16) * 2;
	    r = 128 | ((d[z] & 64)>>3) | ((d[z] & 128)>>3);
	    p = (d[z + 1]>>2) & 7;
	    if (p != 0)
	      p--;
	    r |= p;
	    /* y-flip? */
	    if ((r & 16) && (!(r & 8))) {
	      map_data[(za<<11) + (zb<<1)] = t + 32;
	      map_data[(za<<11) + 1024 + (zb<<1)] = t;
	      map_data[(za<<11) + (zb<<1) + 1] = t + 32 + 1;
	      map_data[(za<<11) + 1024 + (zb<<1) + 1] = t + 1;
	    }
	    /* x-flip? */
	    else if ((!(r & 16)) && (r & 8)) {
	      map_data[(za<<11) + (zb<<1)] = t + 1;
	      map_data[(za<<11) + 1024 + (zb<<1)] = t + 1 + 32;
	      map_data[(za<<11) + (zb<<1) + 1] = t;
	      map_data[(za<<11) + 1024 + (zb<<1) + 1] = t + 32;
	    }
	    /* x-flip and y-flip? */
	    else if ((r & 16) && (r & 8)) {
	      map_data[(za<<11) + (zb<<1)] = t + 32 + 1;
	      map_data[(za<<11) + 1024 + (zb<<1)] = t + 1;
	      map_data[(za<<11) + (zb<<1) + 1] = t + 32;
	      map_data[(za<<11) + 1024 + (zb<<1) + 1] = t;
	    }
	    /* no flips */
	    else {
	      map_data[(za<<11) + (zb<<1)] = t;
	      map_data[(za<<11) + 1024 + (zb<<1)] = t + 32;
	      map_data[(za<<11) + (zb<<1) + 1] = t + 1;
	      map_data[(za<<11) + 1024 + (zb<<1) + 1] = t + 32 + 1;
	    }

	    map_property_data[(za<<11) + (zb<<1)] = r;
	    map_property_data[(za<<11) + (zb<<1) + 1] = r;
	    map_property_data[(za<<11) + 1024 + (zb<<1)] = r;
	    map_property_data[(za<<11) + 1024 + (zb<<1) + 1] = r;
	    z += 3;
	  }
	}
      }
      /* 32x32 */
      else {
	for (za = 0; za < dy; za++) {
	  for (zb = 0; zb < dx; zb++) {
	    t = d[z + 2] + ((d[z + 1] & 3)<<8);
	    t = (t / 8) * 128 + (t % 8) * 4;
	    r = 128 | ((d[z] & 64)>>3) | ((d[z] & 128)>>3);
	    p = (d[z + 1]>>2) & 7;
	    if (p != 0)
	      p--;
	    r |= p;
	    /* y-flip? */
	    if ((r & 16) && (!(r & 8))) {
	      map_data[(za<<12) + (zb<<2)] = t + 32*3;
	      map_data[(za<<12) + 1024 + (zb<<2)] = t + 32*2;
	      map_data[(za<<12) + (zb<<2) + 1] = t + 32*3 + 1;
	      map_data[(za<<12) + 1024 + (zb<<2) + 1] = t + 32*2 + 1;
	      map_data[(za<<12) + (zb<<2) + 2] = t + 32*3 + 2;
	      map_data[(za<<12) + 1024 + (zb<<2) + 2] = t + 32*2 + 2;
	      map_data[(za<<12) + (zb<<2) + 3] = t + 32*3 + 3;
	      map_data[(za<<12) + 1024 + (zb<<2) + 3] = t + 32*2 + 3;
	      map_data[(za<<12) + 1024*2 + (zb<<2)] = t + 32*1;
	      map_data[(za<<12) + 1024*3 + (zb<<2)] = t;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 1] = t + 32*1 + 1;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 1] = t + 1;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 2] = t + 32*1 + 2;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 2] = t + 2;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 3] = t + 32*1 + 3;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 3] = t + 3;
	    }
	    /* x-flip? */
	    else if ((!(r & 16)) && (r & 8)) {
	      map_data[(za<<12) + (zb<<2)] = t + 3;
	      map_data[(za<<12) + 1024 + (zb<<2)] = t + 32*1 + 3;
	      map_data[(za<<12) + (zb<<2) + 1] = t + 2;
	      map_data[(za<<12) + 1024 + (zb<<2) + 1] = t + 32*1 + 2;
	      map_data[(za<<12) + (zb<<2) + 2] = t + 1;
	      map_data[(za<<12) + 1024 + (zb<<2) + 2] = t + 32*1 + 1;
	      map_data[(za<<12) + (zb<<2) + 3] = t;
	      map_data[(za<<12) + 1024 + (zb<<2) + 3] = t + 32*1;
	      map_data[(za<<12) + 1024*2 + (zb<<2)] = t + 32*2 + 3;
	      map_data[(za<<12) + 1024*3 + (zb<<2)] = t + 32*3 + 3;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 1] = t + 32*2 + 2;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 1] = t + 32*3 + 2;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 2] = t + 32*2 + 1;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 2] = t + 32*3 + 1;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 3] = t + 32*2;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 3] = t + 32*3;
	    }
	    /* x-flip and y-flip? */
	    else if ((r & 16) && (r & 8)) {
	      map_data[(za<<12) + (zb<<2)] = t + 32*3 + 3;
	      map_data[(za<<12) + 1024 + (zb<<2)] = t + 32*2 + 3;
	      map_data[(za<<12) + (zb<<2) + 1] = t + 32*3 + 2;
	      map_data[(za<<12) + 1024 + (zb<<2) + 1] = t + 32*2 + 2;
	      map_data[(za<<12) + (zb<<2) + 2] = t + 32*3 + 1;
	      map_data[(za<<12) + 1024 + (zb<<2) + 2] = t + 32*2 + 1;
	      map_data[(za<<12) + (zb<<2) + 3] = t + 32*3;
	      map_data[(za<<12) + 1024 + (zb<<2) + 3] = t + 32*2;
	      map_data[(za<<12) + 1024*2 + (zb<<2)] = t + 32*1 + 1;
	      map_data[(za<<12) + 1024*3 + (zb<<2)] = t + 1;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 1] = t + 32*1 + 2;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 1] = t + 2;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 2] = t + 32*1 + 1;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 2] = t + 1;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 3] = t + 32*1;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 3] = t;
	    }
	    /* no flips */
	    else {
	      map_data[(za<<12) + (zb<<2)] = t;
	      map_data[(za<<12) + 1024 + (zb<<2)] = t + 32*1;
	      map_data[(za<<12) + (zb<<2) + 1] = t + 1;
	      map_data[(za<<12) + 1024 + (zb<<2) + 1] = t + 32*1 + 1;
	      map_data[(za<<12) + (zb<<2) + 2] = t + 2;
	      map_data[(za<<12) + 1024 + (zb<<2) + 2] = t + 32*1 + 2;
	      map_data[(za<<12) + (zb<<2) + 3] = t + 3;
	      map_data[(za<<12) + 1024 + (zb<<2) + 3] = t + 32*1 + 3;
	      map_data[(za<<12) + 1024*2 + (zb<<2)] = t + 32*2;
	      map_data[(za<<12) + 1024*3 + (zb<<2)] = t + 32*3;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 1] = t + 32*2 + 1;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 1] = t + 32*3 + 1;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 2] = t + 32*2 + 2;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 2] = t + 32*3 + 2;
	      map_data[(za<<12) + 1024*2 + (zb<<2) + 3] = t + 32*2 + 3;
	      map_data[(za<<12) + 1024*3 + (zb<<2) + 3] = t + 32*3 + 3;
	    }

	    map_property_data[(za<<12) + (zb<<2)] = r;
	    map_property_data[(za<<12) + (zb<<2) + 1] = r;
	    map_property_data[(za<<12) + 1024 + (zb<<2)] = r;
	    map_property_data[(za<<12) + 1024 + (zb<<2) + 1] = r;
	    map_property_data[(za<<12) + (zb<<2) + 2] = r;
	    map_property_data[(za<<12) + (zb<<2) + 1 + 2] = r;
	    map_property_data[(za<<12) + 1024 + (zb<<2) + 2] = r;
	    map_property_data[(za<<12) + 1024 + (zb<<2) + 1 + 2] = r;
	    map_property_data[(za<<12) + 2048 + (zb<<2)] = r;
	    map_property_data[(za<<12) + 2048 + (zb<<2) + 1] = r;
	    map_property_data[(za<<12) + 2048 + 1024 + (zb<<2)] = r;
	    map_property_data[(za<<12) + 2048 + 1024 + (zb<<2) + 1] = r;
	    map_property_data[(za<<12) + 2048 + (zb<<2) + 2] = r;
	    map_property_data[(za<<12) + 2048 + (zb<<2) + 1 + 2] = r;
	    map_property_data[(za<<12) + 2048 + 1024 + (zb<<2) + 2] = r;
	    map_property_data[(za<<12) + 2048 + 1024 + (zb<<2) + 1 + 2] = r;
	    z += 3;
	  }
	}
      }

    }

    i += k;
  }

  free(d);

  draw_map_area();
  map_draw_rectangle();
  refresh_map_draw_area();
}
