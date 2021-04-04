
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
#include "include/memory.h"
#include "include/button.h"


extern unsigned char memory_data[8 * 8 * 1024]; /* one byte / pixel */
extern unsigned char palette_data[1024]; /* one byte / palette number */
extern unsigned int color_one, palette_one;

unsigned int memory_control_value = 0;



void clear_memory(GtkWidget *widget, gpointer data) {

  unsigned int a, b;


  for (a = 0; a < 256; a++)
    for (b = 0; b < 256; b++)
      memory_data[a * 256 + b] = color_one;

  for (a = 0; a < 32; a++)
    for (b = 0; b < 32; b++)
      palette_data[a * 32 + b] = palette_one;

  draw_and_refresh_all();
}


gint memory_key_press(GtkWidget *widget, GdkEventKey *event) {

  if (event->keyval == GDK_Right)
    button_press_right(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Left)
    button_press_left(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Up)
    button_press_up(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Down)
    button_press_down(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_x)
    flip_x(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_y)
    flip_y(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_l)
    draw_area_shift_left(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_r)
    draw_area_shift_right(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_u)
    draw_area_shift_up(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_d)
    draw_area_shift_down(widget, (GdkEventButton *)event);
  else if (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R)
    memory_control_value = 1;

  return FALSE;
}


gint memory_key_release(GtkWidget *widget, GdkEventKey *event) {

  if (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R)
    memory_control_value = 0;

  return FALSE;
}
