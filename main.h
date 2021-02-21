
void destroy(GtkWidget *widget, gpointer data);
void change_mode_8(GtkWidget *widget, gpointer data);
void change_mode_8x16(GtkWidget *widget, gpointer data);
void change_mode_16(GtkWidget *widget, gpointer data);
void change_mode_32(GtkWidget *widget, gpointer data);
void change_grid(GtkWidget *widget, gpointer data);
void zoom_1(GtkWidget *widget, gpointer data);
void zoom_2(GtkWidget *widget, gpointer data);
void zoom_3(GtkWidget *widget, gpointer data);
void tile_zoom_1(GtkWidget *widget, gpointer data);
void tile_zoom_2(GtkWidget *widget, gpointer data);
void tile_zoom_3(GtkWidget *widget, gpointer data);
void tile_source_mem(GtkWidget *widget, gpointer data);
void tile_source_met(GtkWidget *widget, gpointer data);

void tile_window_toggle(GtkWidget *widget, gpointer data);
void palette_window_toggle(GtkWidget *widget, gpointer data);
void map_window_toggle(GtkWidget *widget, gpointer data);
void metatile_window_toggle(GtkWidget *widget, gpointer data);
void draw_use_meta(GtkWidget *widget, gpointer data);
void draw_use_mem(GtkWidget *widget, gpointer data);

void set_edit_window_title(void);
void set_memory_window_title(void);
void set_tile_window_title(void);
void set_palette_window_title(void);
void set_map_window_title(void);
void set_metatile_window_title(void);
void change_edit_draw_area_size(void);

void memory_4kb(GtkWidget *widget, gpointer data);
void memory_8kb(GtkWidget *widget, gpointer data);
void memory_12kb(GtkWidget *widget, gpointer data);
void memory_16kb(GtkWidget *widget, gpointer data);

void memory_mode_changed(void);
void change_mode_rest(void);

int init_edit_window(void);
int init_memory_window(void);
int init_tile_window(void);
int init_palette_window(void);
int init_tile_export_window(void);
int init_map_window(void);
int init_map_export_window(void);
int init_metatile_window(void);

int get_current_tile_number(void);

gint delete_window_widget(GtkWidget *widget, gpointer data);
