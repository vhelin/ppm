
void metatile_lib_new(GtkWidget *widget, gpointer data);
void metatile_lib_new_from_map(GtkWidget *widget, gpointer data);
void metatile_lib_add_from_map(GtkWidget *widget, gpointer data);
gint meta_press_left(GtkWidget *widget, GdkEventButton *event);
gint meta_press_right(GtkWidget *widget, GdkEventButton *event);
gint meta_tile_changed(GtkWidget *widget, GdkEventButton *event);
gint meta_flip_press(GtkWidget *widget, GdkEventButton *event);
void meta_add(GtkWidget *widget, gpointer data);
void meta_delete(GtkWidget *widget, gpointer data);

int build_metatile_map(void);
int free_metatile_map(void);
int update_meta_spin_buttons(void);
int new_metatiles_from_map(int mark);

void file_meta_open_cancel(GtkWidget *widget, gpointer data);
void file_meta_save_as_cancel(GtkWidget *widget, gpointer data);
void file_meta_open_ok(GtkWidget *widget, gpointer data);
void file_meta_save_as_ok(GtkWidget *widget, gpointer data);
void file_meta_open(GtkWidget *widget, gpointer data);
void file_meta_save(GtkWidget *widget, gpointer data);
void file_meta_save_as(GtkWidget *widget, gpointer data);

int file_meta_open_data(char *n);
int file_meta_save_data(char *n);
