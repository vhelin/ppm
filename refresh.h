
gboolean draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean memory_draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean tile_draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean map_draw_area_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_1(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_2(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_3(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_4(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_5(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_6(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_7(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean palette_draw_area_expose_8(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean meta_da_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

void refresh_draw_data(void);
void refresh_edit_draw_area(void);
void refresh_memory_draw_area(void);
void refresh_tile_draw_area(void);
void refresh_map_draw_area(void);
void refresh_meta_draw_area(void);

void refresh_palette_1(void);
void refresh_palette_2(void);
void refresh_palette_3(void);
void refresh_palette_4(void);
void refresh_palette_5(void);
void refresh_palette_6(void);
void refresh_palette_7(void);
void refresh_palette_8(void);
