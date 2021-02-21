
void clear_project(GtkWidget *widget, gpointer data);
void file_project_save_as_cancel(GtkWidget *widget, gpointer data);
void file_project_save_as_ok(GtkWidget *widget, gpointer data);
void file_project_open_cancel(GtkWidget *widget, gpointer data);
void file_project_open_ok(GtkWidget *widget, gpointer data);
void project_save_as(GtkWidget *widget, gpointer data);
void project_save(GtkWidget *widget, gpointer data);
void project_open(GtkWidget *widget, gpointer data);
int project_save_data(char *n);
int copy_base_name(char *d, char *s);
