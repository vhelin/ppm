
int draw_edit_area(void);
int draw_meta_area(void);
int draw_memory_area(void);
int draw_edit_pixel(int x, int y, int a, int b);
int draw_edit_pixel_8(int a, int b, int x, int y);
int draw_memory_pixel(int x, int y);
int draw_memory_pixel_8(int x, int y);
int draw_memory_pixel_plot(int x, int y, int r, int g, int b);
int draw_grid_rectangle(void);
int draw_tile_area(void);
int draw_tile_pixel(int x, int y);
int draw_and_refresh_all(void);
int draw_color_boxes(void);
int draw_color_box(int palette, int color);
int draw_button_color_indicator(unsigned int button, unsigned int palette, unsigned int color);

gchar *get_palette_buffer(int palette);

void memory_rectangle_draw(void);
void draw_meta_pixel_plot(int x, int y, int r, int g, int b);
void draw_meta_block(int a, int b, int x, int y);
