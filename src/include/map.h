
void draw_map_area(void);
void draw_map_block(unsigned int sx, unsigned int sy);
void draw_map_pixel_plot(int x, int y, int r, int g, int b);
void update_map(void);
void map_draw_rectangle(void);
void map_mouse_button_pressed(void);
void adjust_map_rectangle(int x, int y, unsigned int *a, unsigned int *b);
void map_store_undo_tile_data(void);

void map_zoom_1(GtkWidget *widget, gpointer data);
void map_zoom_2(GtkWidget *widget, gpointer data);
void map_zoom_3(GtkWidget *widget, gpointer data);
void map_copy_tile(GtkWidget *widget, gpointer data);
void map_cut_tile(GtkWidget *widget, gpointer data);
void map_clear_tile(GtkWidget *widget, gpointer data);
void map_paste_tile(GtkWidget *widget, gpointer data);
void map_undo_tile(GtkWidget *widget, gpointer data);
void map_new_tile_memory_map(GtkWidget *widget, gpointer data);

gint map_scrollbar_clicked(GtkWidget *widget, GdkEventButton *event);
gint map_button_press(GtkWidget *widget, GdkEventButton *event);
gint map_button_release(GtkWidget *widget, GdkEventButton *event);
gint map_motion_notify(GtkWidget *widget, GdkEventMotion *event);
gint map_key_press(GtkWidget *widget, GdkEventKey *event);
gint map_key_release(GtkWidget *widget, GdkEventKey *event);
gint map_check_press(GtkWidget *widget, GdkEventButton *event);

void clear_map(GtkWidget *widget, gpointer data);

int find_tile(unsigned int d, unsigned int a);
