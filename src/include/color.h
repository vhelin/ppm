
void color_selection_change(GtkWidget *widget, gpointer data);
int refresh_palette_all(void);

void palette_button_copy(GtkWidget *widget, gpointer data);
void palette_button_paste(GtkWidget *widget, gpointer data);
void palette_button_reverse(GtkWidget *widget, gpointer data);
void palette_button_interpolate(GtkWidget *widget, gpointer data);
unsigned int palette_button_get_number(GtkWidget *widget);
unsigned int color_map_to_gb_rgb(unsigned char r, unsigned char g, unsigned char b, unsigned char *ar, unsigned char *ag, unsigned char *ab);

void palette_pc(GtkWidget *widget, gpointer data);
void palette_gbc_rusty(GtkWidget *widget, gpointer data);
void palette_gbc_jedediah(GtkWidget *widget, gpointer data);

void gbc_gen_rusty_filter(void);
void palette_remap(void);

int gbc_get_value(int min, int max, int v);
int palette_color_remap(unsigned int p, unsigned int c);

unsigned int color_map_to_jedediah_rgb(unsigned char nr, unsigned char ng, unsigned char nb, unsigned char *ar, unsigned char *ag, unsigned char *ab);

void palette_mode_changed(void);
