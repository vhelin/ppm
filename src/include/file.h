
void file_open(GtkWidget *widget, gpointer data);
void file_open_ok(GtkWidget *widget, gpointer data);
void file_open_cancel(GtkWidget *widget, gpointer data);
void file_save(GtkWidget *widget, gpointer data);
void file_save_as(GtkWidget *widget, gpointer data);
void file_save_as_ok(GtkWidget *widget, gpointer data);
void file_save_as_cancel(GtkWidget *widget, gpointer data);
void file_save_as_png(GtkWidget *widget, gpointer data);
void file_save_as_png_ok(GtkWidget *widget, gpointer data);
void file_save_as_png_cancel(GtkWidget *widget, gpointer data);

void file_map_open(GtkWidget *widget, gpointer data);
void file_map_open_ok(GtkWidget *widget, gpointer data);
void file_map_open_cancel(GtkWidget *widget, gpointer data);
void file_map_save(GtkWidget *widget, gpointer data);
void file_map_save_as(GtkWidget *widget, gpointer data);
void file_map_save_as_ok(GtkWidget *widget, gpointer data);
void file_map_save_as_cancel(GtkWidget *widget, gpointer data);

int file_save_data(char *n);
int file_map_save_data(char *n);

int file_open_data(char *n);
int file_map_open_data(char *n);

int file_save_data_png(char *n);
