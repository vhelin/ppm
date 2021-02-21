
int export_selected_data(void);
int export_tile_data(char *n);
int export_tile_palette_data(char *n);
int export_palette_data(char *n);
int export_8x8_tile(FILE *f, unsigned char *d, unsigned char e);
int export_write_byte(FILE *f, int c);
int export_write_word(FILE *f, int c);
int export_write_palette_byte(FILE *f, unsigned char c);
int init_export_data(unsigned int x);
int map_export_selected_data(void);

gint export_button_data_get(GtkWidget *widget, GdkEventButton *event);
gint export_button_palette_get(GtkWidget *widget, GdkEventButton *event);
gint export_button_tile_palette_get(GtkWidget *widget, GdkEventButton *event);
gint export_export_pressed(GtkWidget *widget, GdkEventButton *event);
gint export_cancel_pressed(GtkWidget *widget, GdkEventButton *event);
gint export_button_tile_size(GtkWidget *widget, GdkEvent *event);
gint map_export_button_map_data_get(GtkWidget *widget, GdkEventButton *event);
gint map_export_button_map_property_data_get(GtkWidget *widget, GdkEventButton *event);
gint map_export_export_pressed(GtkWidget *widget, GdkEventButton *event);
gint map_export_cancel_pressed(GtkWidget *widget, GdkEventButton *event);
gint map_export_button_tile_size(GtkWidget *widget, GdkEvent *event);

void export_format_tile_data_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);
void export_format_format_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);
void export_format_palette_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);
void export_adjust_end_spin_buttons(GtkWidget *widget);
void export(GtkWidget *widget, gpointer data);
void export_ok(GtkWidget *widget, gpointer data);
void export_cancel(GtkWidget *widget, gpointer data);
void map_export(GtkWidget *widget, gpointer data);
void map_export_ok(GtkWidget *widget, gpointer data);
void map_export_cancel(GtkWidget *widget, gpointer data);
void map_export_format_format_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);
void map_export_format_size_selected(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);

char *export_resolve_name(char *n);

int map_export_map_data(char *n);
int map_export_map_data_16x16_f(char *n);
int map_export_map_data_16x16_d(char *n);
int map_export_map_data_16x16_l(char *n);
int map_export_map_property_data(char *n);
int map_export_map_property_data_16x16_f(char *n);
int map_export_map_property_data_16x16_d(char *n); /* used also for library exporting */

int get_map_dimensions(void);
int ppm_to_cgb(int t, int p);
