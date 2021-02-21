
gint button_press(GtkWidget *widget, GdkEventButton *event);
gint button_release(GtkWidget *widget, GdkEventButton *event);
gint motion_notify(GtkWidget *widget, GdkEventMotion *event);
gint memory_button_press(GtkWidget *widget, GdkEventButton *event);
gint memory_button_release(GtkWidget *widget, GdkEventButton *event);
gint memory_motion_notify(GtkWidget *widget, GdkEventMotion *event);
gint memory_check_press(GtkWidget *widget, GdkEventButton *event);
gint palette_1_button(GtkWidget *widget, GdkEventButton *event);
gint palette_2_button(GtkWidget *widget, GdkEventButton *event);
gint palette_3_button(GtkWidget *widget, GdkEventButton *event);
gint palette_4_button(GtkWidget *widget, GdkEventButton *event);
gint palette_5_button(GtkWidget *widget, GdkEventButton *event);
gint palette_6_button(GtkWidget *widget, GdkEventButton *event);
gint palette_7_button(GtkWidget *widget, GdkEventButton *event);
gint palette_8_button(GtkWidget *widget, GdkEventButton *event);
gint palette_x_button(GtkWidget *widget, GdkEventButton *event, unsigned int palette);

int set_new_button_color(unsigned int palette, unsigned int button, unsigned int color);
int adjust_memory_rectangle_position(int nx, int ny);

unsigned int flood_fill(unsigned int ax, unsigned int ay, unsigned char old_c, unsigned char old_p, unsigned char c, unsigned char p);
