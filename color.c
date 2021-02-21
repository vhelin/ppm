
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
#include "refresh.h"
#include "mouse.h"


extern unsigned char memory_data[8 * 8 * 1024]; /* one byte / pixel */
extern unsigned char undo_buffer[32 * 32];
extern unsigned char palette_colors[3 * 4 * 8]; /* eight palettes to choose from */
extern unsigned char palette_colors_real[3 * 4 * 8]; /* eight palettes to choose from */
extern unsigned int color_one, color_two, color_three;
extern unsigned int palette_one, palette_two, palette_three;
extern unsigned int latest_color, latest_palette;
extern unsigned int active_press, active_color, active_x, active_y;
extern unsigned int x, y;
extern unsigned int edit_mode, palette_mode;
extern unsigned int edit_grid;
extern unsigned int palette_mode_init;

extern GtkWidget *edit_draw_area, *memory_draw_area, *color_selection;
extern GtkWidget *palette_button_p0c, *palette_button_p0p, *palette_button_p1c, *palette_button_p1p;
extern GtkWidget *palette_button_p2c, *palette_button_p2p, *palette_button_p3c, *palette_button_p3p;
extern GtkWidget *palette_button_p4c, *palette_button_p4p, *palette_button_p5c, *palette_button_p5p;
extern GtkWidget *palette_button_p6c, *palette_button_p6p, *palette_button_p7c, *palette_button_p7p;
extern GtkWidget *palette_button_p0i, *palette_button_p1i, *palette_button_p2i, *palette_button_p3i;
extern GtkWidget *palette_button_p4i, *palette_button_p5i, *palette_button_p6i, *palette_button_p7i;
extern GtkWidget *palette_button_p0r, *palette_button_p1r, *palette_button_p2r, *palette_button_p3r;
extern GtkWidget *palette_button_p4r, *palette_button_p5r, *palette_button_p6r, *palette_button_p7r;

unsigned char palette_buffer_copy[3 * 4];
unsigned int palette_buffer_status = 0;
unsigned int rusty_filter[32768];



void color_selection_change(GtkWidget *widget, gpointer data) {

  gdouble color[3];


  gtk_color_selection_get_color(GTK_COLOR_SELECTION(color_selection), color);
  palette_colors_real[latest_palette * 4 * 3 + latest_color * 3] = color[0]*255;
  palette_colors_real[latest_palette * 4 * 3 + latest_color * 3 + 1] = color[1]*255;
  palette_colors_real[latest_palette * 4 * 3 + latest_color * 3 + 2] = color[2]*255;
  palette_color_remap(latest_palette, latest_color);

  draw_color_box(latest_palette, latest_color);

  if (palette_one == latest_palette && color_one == latest_color)
    draw_button_color_indicator(1, palette_one, color_one);
  if (palette_two == latest_palette && color_two == latest_color)
    draw_button_color_indicator(2, palette_two, color_two);
  if (palette_three == latest_palette && color_three == latest_color)
    draw_button_color_indicator(3, palette_three, color_three);

  if (latest_palette == 0)
    refresh_palette_1();
  else if (latest_palette == 1)
    refresh_palette_2();
  else if (latest_palette == 2)
    refresh_palette_3();
  else if (latest_palette == 3)
    refresh_palette_4();
  else if (latest_palette == 4)
    refresh_palette_5();
  else if (latest_palette == 5)
    refresh_palette_6();
  else if (latest_palette == 6)
    refresh_palette_7();
  else if (latest_palette == 7)
    refresh_palette_8();

  draw_and_refresh_all();
}


int refresh_palette_all(void) {

  refresh_palette_1();
  refresh_palette_2();
  refresh_palette_3();
  refresh_palette_4();
  refresh_palette_5();
  refresh_palette_6();
  refresh_palette_7();
  refresh_palette_8();

  return SUCCEEDED;
}


void palette_button_copy(GtkWidget *widget, gpointer data) {

  unsigned int c, a;


  c = palette_button_get_number(widget);
  for (a = 0; a < 3 * 4; a++)
    palette_buffer_copy[a] = palette_colors_real[3 * 4 * c + a];

  palette_buffer_status = 1;
}


void palette_button_paste(GtkWidget *widget, gpointer data) {

  unsigned int c, a;


  if (palette_buffer_status == 0)
    return;

  c = palette_button_get_number(widget);
  for (a = 0; a < 3 * 4; a++)
    palette_colors_real[3 * 4 * c + a] = palette_buffer_copy[a];

  palette_remap();
  draw_color_boxes();
  draw_button_color_indicator(1, palette_one, color_one);
  draw_button_color_indicator(2, palette_two, color_two);
  draw_button_color_indicator(3, palette_three, color_three);
  refresh_palette_all();
  draw_and_refresh_all();
}


unsigned int palette_button_get_number(GtkWidget *widget) {

  if (widget == palette_button_p0p || widget == palette_button_p0c)
    return 0;
  if (widget == palette_button_p1p || widget == palette_button_p1c)
    return 1;
  if (widget == palette_button_p2p || widget == palette_button_p2c)
    return 2;
  if (widget == palette_button_p3p || widget == palette_button_p3c)
    return 3;
  if (widget == palette_button_p4p || widget == palette_button_p4c)
    return 4;
  if (widget == palette_button_p5p || widget == palette_button_p5c)
    return 5;
  if (widget == palette_button_p6p || widget == palette_button_p6c)
    return 6;
  if (widget == palette_button_p7p || widget == palette_button_p7c)
    return 7;

  return 0;
}


void palette_button_reverse(GtkWidget *widget, gpointer data) {

  unsigned char r, g, b;
  unsigned int a;


  if (widget == palette_button_p0r)
    a = 0;
  else if (widget == palette_button_p1r)
    a = 1 * 3 * 4;
  else if (widget == palette_button_p2r)
    a = 2 * 3 * 4;
  else if (widget == palette_button_p3r)
    a = 3 * 3 * 4;
  else if (widget == palette_button_p4r)
    a = 4 * 3 * 4;
  else if (widget == palette_button_p5r)
    a = 5 * 3 * 4;
  else if (widget == palette_button_p6r)
    a = 6 * 3 * 4;
  else
    a = 7 * 3 * 4;

  /* swap 1 and 4 */
  r = palette_colors_real[a + 0];
  g = palette_colors_real[a + 1];
  b = palette_colors_real[a + 2];
  palette_colors_real[a + 0] = palette_colors_real[a + 3 * 3 + 0];
  palette_colors_real[a + 1] = palette_colors_real[a + 3 * 3 + 1];
  palette_colors_real[a + 2] = palette_colors_real[a + 3 * 3 + 2];
  palette_colors_real[a + 3 * 3 + 0] = r;
  palette_colors_real[a + 3 * 3 + 1] = g;
  palette_colors_real[a + 3 * 3 + 2] = b;

  /* swap 2 and 3 */
  r = palette_colors_real[a + 3];
  g = palette_colors_real[a + 4];
  b = palette_colors_real[a + 5];
  palette_colors_real[a + 3] = palette_colors_real[a + 3 * 2 + 0];
  palette_colors_real[a + 4] = palette_colors_real[a + 3 * 2 + 1];
  palette_colors_real[a + 5] = palette_colors_real[a + 3 * 2 + 2];
  palette_colors_real[a + 3 * 2 + 0] = r;
  palette_colors_real[a + 3 * 2 + 1] = g;
  palette_colors_real[a + 3 * 2 + 2] = b;

  palette_remap();
  palette_mode_changed();
}


void palette_button_interpolate(GtkWidget *widget, gpointer data) {

  unsigned int a;
  unsigned char nr, ng, nb;
  double ra, ga, ba, r, g, b;


  if (widget == palette_button_p0i)
    a = 0;
  else if (widget == palette_button_p1i)
    a = 1;
  else if (widget == palette_button_p2i)
    a = 2;
  else if (widget == palette_button_p3i)
    a = 3;
  else if (widget == palette_button_p4i)
    a = 4;
  else if (widget == palette_button_p5i)
    a = 5;
  else if (widget == palette_button_p6i)
    a = 6;
  else
    a = 7;

  ra = palette_colors_real[3 * 4 * a + 3 * 3 + 0] - palette_colors_real[3 * 4 * a + 0];
  ga = palette_colors_real[3 * 4 * a + 3 * 3 + 1] - palette_colors_real[3 * 4 * a + 1];
  ba = palette_colors_real[3 * 4 * a + 3 * 3 + 2] - palette_colors_real[3 * 4 * a + 2];
  ra = ra / 3;
  ga = ga / 3;
  ba = ba / 3;

  r = palette_colors_real[3 * 4 * a + 0] + ra;
  g = palette_colors_real[3 * 4 * a + 1] + ga;
  b = palette_colors_real[3 * 4 * a + 2] + ba;
  palette_colors_real[3 * 4 * a + 3 * 1 + 0] = r;
  palette_colors_real[3 * 4 * a + 3 * 1 + 1] = g;
  palette_colors_real[3 * 4 * a + 3 * 1 + 2] = b;
  color_map_to_gb_rgb((unsigned char)r, (unsigned char)g, (unsigned char)b, &nr, &ng, &nb);
  palette_colors[3 * 4 * a + 3 * 1 + 0] = nr;
  palette_colors[3 * 4 * a + 3 * 1 + 1] = ng;
  palette_colors[3 * 4 * a + 3 * 1 + 2] = nb;

  r += ra;
  g += ga;
  b += ba;
  palette_colors_real[3 * 4 * a + 3 * 2 + 0] = r;
  palette_colors_real[3 * 4 * a + 3 * 2 + 1] = g;
  palette_colors_real[3 * 4 * a + 3 * 2 + 2] = b;
  color_map_to_gb_rgb((unsigned char)r, (unsigned char)g, (unsigned char)b, &nr, &ng, &nb);
  palette_colors[3 * 4 * a + 3 * 2 + 0] = nr;
  palette_colors[3 * 4 * a + 3 * 2 + 1] = ng;
  palette_colors[3 * 4 * a + 3 * 2 + 2] = nb;

  draw_color_boxes();
  draw_button_color_indicator(1, palette_one, color_one);
  draw_button_color_indicator(2, palette_two, color_two);
  draw_button_color_indicator(3, palette_three, color_three);
  refresh_palette_all();
  draw_and_refresh_all();
}


unsigned int color_map_to_gb_rgb(unsigned char r, unsigned char g, unsigned char b, unsigned char *ar, unsigned char *ag, unsigned char *ab) {

  unsigned int nr, ng, nb, c;


  /* PC color map */
  if (palette_mode == PALETTE_MODE_PC) {
    *ar = r;
    *ag = g;
    *ab = b;
  }
  /* GBC rusty color map */
  else if (palette_mode == PALETTE_MODE_RUS) {
    nr = r >> 3;
    ng = g >> 3;
    nb = b >> 3;

    c = rusty_filter[(nb << 10) | (ng << 5) | nr];

    *ar = (c & 31) << 3;
    *ag = ((c >> 5) & 31) << 3;
    *ab = ((c >> 10) & 31) << 3;
  }
  /* GBC jedediah color map */
  else
    color_map_to_jedediah_rgb(r, g, b, ar, ag, ab);

  return SUCCEEDED;
}


void palette_pc(GtkWidget *widget, gpointer data) {

  if (palette_mode_init != 0) {
    palette_mode_init--;
    return;
  }

  palette_mode = PALETTE_MODE_PC;

  palette_mode_changed();
}


void palette_gbc_rusty(GtkWidget *widget, gpointer data) {

  if (palette_mode_init != 0) {
    palette_mode_init--;
    return;
  }

  palette_mode = PALETTE_MODE_RUS;

  palette_mode_changed();
}


void palette_gbc_jedediah(GtkWidget *widget, gpointer data) {

  if (palette_mode_init != 0) {
    palette_mode_init--;
    return;
  }

  palette_mode = PALETTE_MODE_JED;

  palette_mode_changed();
}


void palette_mode_changed(void) {

  palette_remap();
  draw_color_boxes();
  draw_button_color_indicator(1, palette_one, color_one);
  draw_button_color_indicator(2, palette_two, color_two);
  draw_button_color_indicator(3, palette_three, color_three);
  refresh_palette_all();
  draw_and_refresh_all();
  set_palette_window_title();
}


int gbc_get_value(int min, int max, int v) {

  return min+((double)(max-min))*(2.0*(v/31.0)-(v/31.0)*(v/31.0));
}


void gbc_gen_rusty_filter(void) {

  int r, g, b, nr, ng, nb;


  for (r = 0; r < 32; r++) {
    for (g = 0; g < 32; g++) {
      for (b = 0; b < 32; b++) {
	nr = gbc_get_value(gbc_get_value(4, 14, g),gbc_get_value(24, 29, g), r) - 4;
	ng = gbc_get_value(gbc_get_value(4 + gbc_get_value(0, 5, r), 14 + gbc_get_value(0, 3, r), b),
			   gbc_get_value(24 + gbc_get_value(0, 3, r), 29 + gbc_get_value(0, 1, r), b), g) - 4;
	nb = gbc_get_value(gbc_get_value(4 + gbc_get_value(0, 5, r), 14 + gbc_get_value(0, 3, r), g),
			   gbc_get_value(24 + gbc_get_value(0, 3, r), 29 + gbc_get_value(0, 1, r), g), b) - 4;
	rusty_filter[(b << 10) | (g << 5) | r] = (nb << 10) | (ng << 5) | nr;
      }
    }
  }
}


void palette_remap(void) {

  unsigned int p, i;


  for (p = 0; p < 8; p++) {
    for (i = 0; i < 4; i++) {
      palette_color_remap(p, i);
    }
  }
}


int palette_color_remap(unsigned int p, unsigned int c) {

  unsigned char r, g, b, nr, ng, nb;


  r = palette_colors_real[3 * 4 * p + c * 3 + 0];
  g = palette_colors_real[3 * 4 * p + c * 3 + 1];
  b = palette_colors_real[3 * 4 * p + c * 3 + 2];
  color_map_to_gb_rgb(r, g, b, &nr, &ng, &nb);
  palette_colors[3 * 4 * p + c * 3 + 0] = nr;
  palette_colors[3 * 4 * p + c * 3 + 1] = ng;
  palette_colors[3 * 4 * p + c * 3 + 2] = nb;

  return SUCCEEDED;
}


unsigned int color_map_to_jedediah_rgb(unsigned char nr, unsigned char ng, unsigned char nb, unsigned char *ar, unsigned char *ag, unsigned char *ab) {

  double gbr, gbg, gbb, r, g, b,
    rr = (double)204/204, rg = (double)61/193, rb = (double)61/177,
    gr = (double)90/204, gg = (double)153/193, gb = (double)90/177,
    br = (double)70/204, bg = (double)70/193, bb = (double)177/177;

  gbr = (double)nr/255;
  gbg = (double)ng/255;
  gbb = (double)nb/255;

  r = gbr*rr;
  r += (1-r)*gbg*gr;
  r += (1-r)*gbb*br;

  g = gbg*gg;
  g += (1-g)*gbr*rg;
  g += (1-g)*gbb*bg;
  g += (1-g)*((gbr+gbb)/2)*gbg;

  b = gbb*bb;
  b += (1-b)*gbg*gb;
  b += (1-b)*gbr*rb;

  *ar = r * 204;
  *ag = g * 193;
  *ab = b * 177;

  return SUCCEEDED;
}
