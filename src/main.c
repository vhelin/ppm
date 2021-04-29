
/****************************************************************************************
 ppm - yet another tile/map editor written by ville helin in 2000-2003 <vhelin@cc.hut.fi>
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef WIN32
  /*
  this needs to be before defines.h, since winerr.h defines
  SUCCEEDED and FAILED macros.
  */
  #include <windows.h>
  #undef SUCCEEDED
  #undef FAILED
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkhscrollbar.h>
#include <gtk/gtkvscrollbar.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gtypes.h>

#include "include/defines.h"
#include "include/main.h"
#include "include/prefs.h"
#include "include/draw.h"
#include "include/mouse.h"
#include "include/edit.h"
#include "include/memory.h"
#include "include/button.h"
#include "include/color.h"
#include "include/file.h"
#include "include/export.h"
#include "include/import.h"
#include "include/refresh.h"
#include "include/map.h"
#include "include/project.h"
#include "include/meta.h"


guchar rgb_buffer[EDIT_WIDTH * EDIT_HEIGHT * 3];
guchar memory_buffer[512 * 512 * 3];
guchar map_buffer[512 * 512 * 3];
guchar tile_buffer[96 * 2 * 96 * 2 * 3];
guchar meta_buffer[64 * 64 * 3];
guchar palette_buffer_1[PBX * PBY * 3], palette_buffer_2[PBX * PBY * 3], palette_buffer_3[PBX * PBY * 3], palette_buffer_4[PBX * PBY * 3];
guchar palette_buffer_5[PBX * PBY * 3], palette_buffer_6[PBX * PBY * 3], palette_buffer_7[PBX * PBY * 3], palette_buffer_8[PBX * PBY * 3];
unsigned char copy_data_buffer[32 * 32], copy_palette_buffer[4 * 4];
unsigned char memory_data[8 * 8 * 1024];
unsigned char palette_data[1024];
unsigned char palette_colors[3 * 4 * 8];
unsigned char palette_colors_real[3 * 4 * 8];
unsigned char *map_property_data = NULL;
unsigned int *map_data = NULL;

unsigned int memory_window_x, memory_window_y, edit_window_x, edit_window_y, tile_window_x, tile_window_y, palette_window_x, palette_window_y;
unsigned int map_window_x, map_window_y, metatile_window_x, metatile_window_y, metatile_window_loaded = OFF;
unsigned int memory_window_loaded = OFF, edit_window_loaded = OFF, tile_window_loaded = OFF, palette_window_loaded = OFF;
unsigned int edit_size_x, edit_size_y, edit_mode = EDIT_MODE_8;
unsigned int zoom_size = 2, zoom_mode = ZOOM_MODE_2;
unsigned int tile_zoom_size = 2, tile_zoom_mode = ZOOM_MODE_2;
unsigned int tile_window_status = OFF, palette_window_status = OFF, metatile_window_status = OFF;
unsigned int memory_check_status = ON;
unsigned int map_window_status = OFF, map_window_loaded = OFF;
unsigned int map_zoom_size = 2, map_zoom_mode = ZOOM_MODE_2;
unsigned int map_check_status = ON;

unsigned int color_one = 3, color_two = 2, color_three = 1;
unsigned int palette_one = 0, palette_two = 0, palette_three = 0;
unsigned int latest_color = 0, latest_palette = 0;
unsigned int active_press = OFF, active_color, active_palette, active_x, active_y;
unsigned int palette_mode = PALETTE_MODE_PC;
unsigned int export_format_format = FORMAT_RAW, export_format_palette = PALETTE_16BIT, export_format_tile_data = EXPORT_FORMAT_2BIT_PLANAR_INTERLEAVED;
unsigned int palette_mode_init = 0, palette_mode_loaded = OFF;
unsigned int memory_size = MEMORY_SIZE_4KB;
unsigned int map_export_format_format = FORMAT_RAW, map_export_format_size = SIZE_8BIT;

gchar savename[256], mapsavename[256], project_name[256], meta_name[256];

unsigned int x = 0, y = 0;
unsigned int edit_grid = ON, edit_grid_loaded = ON;
GtkWidget *edit_menubar, *memory_menubar, *tile_menubar, *palette_menubar, *map_menubar, *metatile_window;
GtkWidget *edit_vbox, *edit_hbox, *memory_vbox, *memory_hbox1, *memory_hbox2, *button_table, *button_hbox;
GtkWidget *edit_window, *edit_draw_area, *memory_window, *memory_draw_area, *map_window, *map_draw_area;
GtkItemFactory *edit_item_factory, *memory_item_factory, *tile_item_factory, *palette_item_factory, *map_item_factory, *metatile_item_factory;
GtkAccelGroup *edit_accel_group, *memory_accel_group, *tile_accel_group, *palette_accel_group, *map_accel_group, *metatile_accel_group;
GtkWidget *button_left, *button_right, *button_up, *button_down, *arrow_left, *arrow_right, *arrow_up, *arrow_down;
GtkWidget *shift_left, *shift_right, *shift_up, *shift_down, *sl1, *sr1, *sd1, *su1;
GtkWidget *tile_window, *tile_draw_area, *tile_vbox, *tile_hbox;
GtkWidget *button_flip_x, *button_flip_y, *button_rot_l, *button_rot_r;
GtkWidget *palette_window, *palette_vbox, *palette_hbox1, *palette_hbox2, *palette_table;
GtkWidget *color_selection, *memory_check_button, *map_check_button;
GtkWidget *palette_draw_area_1, *palette_draw_area_2, *palette_draw_area_3, *palette_draw_area_4;
GtkWidget *palette_draw_area_5, *palette_draw_area_6, *palette_draw_area_7, *palette_draw_area_8;
GtkWidget *file_selection_open, *file_selection_save_as, *file_selection_export, *file_selection_import_gbr, *file_selection_save_as_png;
GtkWidget *file_selection_map_open, *file_selection_map_save_as, *file_selection_map_export, *file_selection_import_gbm;
GtkWidget *file_selection_project_open, *file_selection_project_save_as;
GtkWidget *export_window, *export_vbox1, *export_vbox2, *export_vbox3, *export_vbox4, *export_vbox5;
GtkWidget *export_hbox1, *export_hbox2, *export_hbox3, *export_hbox4, *export_hbox5, *export_hbox6, *export_hbox7;
GtkWidget *export_entry_data, *export_entry_palette, *export_label_data, *export_label_palette, *export_table, *export_tile_table;
GtkWidget *export_label_tile_palette, *export_entry_tile_palette;
GtkWidget *export_button_data, *export_button_palette, *export_button_tile_palette;
GtkWidget *export_check_button_data, *export_check_button_palette, *export_check_button_tile_palette;
GtkWidget *export_radio_button_1, *export_radio_button_2, *export_radio_button_3, *export_radio_button_4;
GtkWidget *export_frame_file, *export_frame_tile, *export_frame_size, *export_frame_data_formats;
GtkWidget *export_label_end_x, *export_label_end_y;
GtkWidget *export_spin_end_x, *export_spin_end_y;
GtkWidget *export_size_table;
GtkWidget *export_button_export, *export_button_cancel;
GtkWidget *export_clist_format, *export_clist_palette, *export_clist_data, *map_export_clist_size;
GtkWidget *palette_button_p0c, *palette_button_p0p, *palette_button_p1c, *palette_button_p1p;
GtkWidget *palette_button_p2c, *palette_button_p2p, *palette_button_p3c, *palette_button_p3p;
GtkWidget *palette_button_p4c, *palette_button_p4p, *palette_button_p5c, *palette_button_p5p;
GtkWidget *palette_button_p6c, *palette_button_p6p, *palette_button_p7c, *palette_button_p7p;
GtkWidget *palette_button_p0i, *palette_button_p1i, *palette_button_p2i, *palette_button_p3i;
GtkWidget *palette_button_p4i, *palette_button_p5i, *palette_button_p6i, *palette_button_p7i;
GtkWidget *palette_button_p0r, *palette_button_p1r, *palette_button_p2r, *palette_button_p3r;
GtkWidget *palette_button_p4r, *palette_button_p5r, *palette_button_p6r, *palette_button_p7r;
GtkWidget *map_vbox1, *map_hbox1, *map_hbox2, *map_table, *map_scrollbar_h, *map_scrollbar_v;
GtkWidget *export_radio_button_tile_size, *map_export_clist_format;
GtkWidget *map_export_window, *map_export_vbox1, *map_export_vbox2, *map_export_hbox1, *map_export_hbox2, *map_export_hbox3;
GtkWidget *map_export_frame_file, *map_export_file_table, *map_export_entry_map_data, *map_export_check_button_map_data;
GtkWidget *map_export_button_map_data, *map_export_button_export, *map_export_button_cancel, *map_export_frame_mode;
GtkWidget *map_export_label_map_data, *map_export_frame_format, *map_export_hbox5;
GtkWidget *map_export_check_button_map_property_data, *map_export_button_map_property_data;
GtkWidget *map_export_entry_map_property_data, *map_export_label_map_property_data;
GtkWidget *map_export_frame_tile, *map_export_hbox4, *map_export_spin_offset, *map_export_table_offset, *map_export_label_offset;
GtkWidget *map_export_radio_button_1, *map_export_radio_button_2, *map_export_mode_table, *map_export_hbox6;
GtkWidget *map_export_radio_button_3, *meta_da;
GtkWidget *metatile_vbox1, *metatile_menubar, *meta_fr_img, *meta_fr_id, *meta_fr_pal, *metatile_hbox1, *metatile_hbox2;
GtkWidget *meta_right, *meta_left, *meta_arrow_right, *meta_arrow_left, *metatile_hbox3, *metatile_vbox2;
GtkWidget *map_export_vbox3, *meta_button_table, *meta_t_table, *meta_p_table, *meta_x_table, *meta_y_table;
GtkWidget *meta_spin_t1, *meta_spin_t2, *meta_spin_t3, *meta_spin_t4;
GtkWidget *meta_spin_p1, *meta_spin_p2, *meta_spin_p3, *meta_spin_p4;
GtkWidget *metatile_vbox3, *metatile_vbox4, *metatile_hbox4, *metatile_hbox5, *meta_fr_x, *meta_fr_y;
GtkWidget *metatile_vbox5, *metatile_vbox6, *metatile_hbox6, *metatile_hbox7;
GtkWidget *meta_cb_x1, *meta_cb_x2, *meta_cb_x3, *meta_cb_x4, *meta_cb_y1, *meta_cb_y2, *meta_cb_y3, *meta_cb_y4;
GtkWidget *file_selection_meta_open, *file_selection_meta_save_as, *map_export_radio_button_4;
GtkAdjustment *map_adj1, *map_adj2;

gchar *export_clist_title_format[] = { "Export" };
gchar *export_clist_data_format[5][1] = { { "GBDK .c" }, { "Raw binary .bin" }, { "RGBDS/ISAS .asm" }, { "TASM .asm" }, { "WLA .asm" } };
gchar *export_clist_title_palette[] = { "Palette format" };
gchar *export_clist_data_palette[2][1] = { { "16bit RGB" }, { "24bit RGB" } };
gchar *export_clist_title_data[] = { "Tile data format" };
gchar *export_clist_data_data[3][1] = { { "2bit planar interleaved" }, { "2bit planar non interleaved" }, { "8bit chunky" } };

gchar *map_export_clist_title_size[] = { "Tile index size" };
gchar *map_export_clist_data_size[2][1] = { { "8bit" }, { "16bit" } };

GSList *export_radio_button_group, *map_export_radio_button_group;

GtkTooltips *edit_tooltips, *palette_tooltips, *meta_tooltips;

GtkItemFactoryEntry edit_menu_items[] = {
  { "/_Edit",          NULL,         NULL, 0, "<Branch>" },
  { "/Edit/Undo",      "<control>Z", undo_tile, 0, NULL },
  { "/Edit/sep1",      NULL,         NULL, 0, "<Separator>" },
  { "/Edit/Cut",       "<control>X", cut_tile, 0, NULL },
  { "/Edit/_Copy",     "<control>C", copy_tile, 0, NULL },
  { "/Edit/Paste",     "<control>V", paste_tile, 0, NULL },
  { "/Edit/sep2",      NULL,         NULL, 0, "<Separator>" },
  { "/Edit/Clear",     NULL,         clear_tile, 0, NULL },
  { "/_Size",          NULL,         NULL, 0, "<Branch>" },
  { "/Size/8x8",       "<control>1", change_mode_8, 0, "<RadioItem>" },
  { "/Size/8x16",      "<control>2", change_mode_8x16, 0, "/Size/8x8" },
  { "/Size/16x16",     "<control>3", change_mode_16, 0, "/Size/8x8" },
  { "/Size/32x32",     "<control>4", change_mode_32, 0, "/Size/8x8" },
  { "/Size/sep3",      NULL,         NULL, 0, "<Separator>" },
  { "/Size/_Grid",     "<control>G", change_grid, 0, "<ToggleItem>" }
};
GtkItemFactoryEntry memory_menu_items[] = {
  { "/_Project",        NULL,         NULL, 0, "<Branch>" },
  { "/Project/Clear",   NULL,         clear_project, 0, NULL },
  { "/Project/sep1",    NULL,         NULL, 0, "<Separator>" },
  { "/Project/Open",    NULL,         project_open, 0, NULL },
  { "/Project/Save",    NULL,         project_save, 0, NULL },
  { "/Project/Save as", NULL,         project_save_as, 0, NULL },
  { "/Project/sep2",    NULL,         NULL, 0, "<Separator>" },
  { "/Project/_Quit",   "<control>Q", destroy, 0, NULL },
  { "/_File",           NULL,         NULL, 0, "<Branch>" },
  { "/File/Clear",      NULL,         clear_memory, 0, NULL },
  { "/File/sep3",       NULL,         NULL, 0, "<Separator>" },
  { "/File/_Open",      "<control>O", file_open, 0, NULL },
  { "/File/_Save",      "<control>S", file_save, 0, NULL },
  { "/File/Save As",    NULL,         file_save_as, 0, NULL },
  { "/File/Save As PNG",NULL,         file_save_as_png, 0, NULL },
  { "/File/sep4",       NULL,         NULL, 0, "<Separator>" },
  { "/File/_Export",    "<control>E", export, 0, NULL },
  { "/File/sep5",       NULL,         NULL, 0, "<Separator>" },
  { "/File/_Import GBR","<control>I", import_gbr, 0, NULL },
  { "/_Edit",           NULL,         NULL, 0, "<Branch>" },
  { "/Edit/Undo",       "<control>Z", undo_tile, 0, NULL },
  { "/Edit/sep6",       NULL,         NULL, 0, "<Separator>" },
  { "/Edit/Cut",        "<control>X", cut_tile, 0, NULL },
  { "/Edit/_Copy",      "<control>C", copy_tile, 0, NULL },
  { "/Edit/Paste",      "<control>V", paste_tile, 0, NULL },
  { "/Edit/sep7",       NULL,         NULL, 0, "<Separator>" },
  { "/Edit/Clear",      NULL,         clear_tile, 0, NULL },
  { "/_Zoom",           NULL,         NULL, 0, "<Branch>" },
  { "/Zoom/1:1 PC",     "<control>1", zoom_1, 0, "<RadioItem>" },
  { "/Zoom/2:1 PC",     "<control>2", zoom_2, 0, "/Zoom/1:1 PC" },
  { "/Zoom/2:1 CGB",    "<control>3", zoom_3, 0, "/Zoom/1:1 PC" },
  { "/_Memory",         NULL,         NULL, 0, "<Branch>" },
  { "/Memory/4KB",      "<alt>1",     memory_4kb, 0, "<RadioItem>" },
  { "/Memory/8KB",      "<alt>2",     memory_8kb, 0, "/Memory/4KB" },
  { "/Memory/12KB",     "<alt>3",     memory_12kb, 0, "/Memory/4KB" },
  { "/Memory/16KB",     "<alt>4",     memory_16kb, 0, "/Memory/4KB" },
  { "/_Windows",        NULL,         NULL, 0, "<Branch>" },
  { "/Windows/_Map",    "<control>M", map_window_toggle, 0, "<ToggleItem>" },
  { "/Windows/Metat_ile","<control>I", metatile_window_toggle, 0, "<ToggleItem>" },
  { "/Windows/_Palette","<control>P", palette_window_toggle, 0, "<ToggleItem>" },
  { "/Windows/_Tiled",  "<control>T", tile_window_toggle, 0, "<ToggleItem>" },
  { "/_Prefs",          NULL,         NULL, 0, "<Branch>" },
  { "/Prefs/S_ave",     "<control>A", prefs_save, 0, NULL }
};
GtkItemFactoryEntry metatile_menu_items[] = {
  { "/_File",           NULL,         NULL, 0, "<Branch>" },
  { "/File/Clear",      NULL,         metatile_lib_new, 0, NULL },
  { "/File/New from Map",NULL,        metatile_lib_new_from_map, 0, NULL },
  { "/File/Add from Map",NULL,        metatile_lib_add_from_map, 0, NULL },
  { "/File/sep1",       NULL,         NULL, 0, "<Separator>" },
  { "/File/_Open",      "<control>O", file_meta_open, 0, NULL },
  { "/File/_Save",      "<control>S", file_meta_save, 0, NULL },
  { "/File/Save As",    NULL,         file_meta_save_as, 0, NULL },
  { "/_Edit",           NULL,         NULL, 0, "<Branch>" },
  { "/Edit/Add",        NULL,         meta_add, 0, NULL },
  { "/Edit/Delete",     NULL,         meta_delete, 0, NULL }
};
GtkItemFactoryEntry tile_menu_items[] = {
  { "/_Zoom",          NULL,         NULL, 0, "<Branch>" },
  { "/Zoom/1:1 PC",    "<control>1", tile_zoom_1, 0, "<RadioItem>" },
  { "/Zoom/2:1 PC",    "<control>2", tile_zoom_2, 0, "/Zoom/1:1 PC" },
  { "/Zoom/2:1 CGB",   "<control>3", tile_zoom_3, 0, "/Zoom/1:1 PC" },
  { "/_Source",        NULL,         NULL, 0, "<Branch>" },
  { "/Source/_Memory",    "<control>M", tile_source_mem, 0, "<RadioItem>" },
  { "/Source/Me_tatile",  "<control>T", tile_source_met, 0, "/Source/Memory" }
};
GtkItemFactoryEntry palette_menu_items[] = {
  { "/_Mode",              NULL,         NULL, 0, "<Branch>" },
  { "/Mode/_PC",           "<control>P", palette_pc, 0, "<RadioItem>" },
  { "/Mode/CGB _Rusty",    "<control>R", palette_gbc_rusty, 0, "/Mode/PC" },
  { "/Mode/CGB _Jedediah", "<control>J", palette_gbc_jedediah, 0, "/Mode/PC" }
};
GtkItemFactoryEntry map_menu_items[] = {
  { "/_File",           NULL,          NULL, 0, "<Branch>" },
  { "/File/Clear",      NULL,          clear_map, 0, NULL },
  { "/File/New Tile Memory Map", NULL, map_new_tile_memory_map, 0, NULL },
  { "/File/sep1",       NULL,          NULL, 0, "<Separator>" },
  { "/File/_Open",      "<control>O",  file_map_open, 0, NULL },
  { "/File/_Save",      "<control>S",  file_map_save, 0, NULL },
  { "/File/Save As",    NULL,          file_map_save_as, 0, NULL },
  { "/File/sep2",       NULL,          NULL, 0, "<Separator>" },
  { "/File/_Export",    "<control>E",  map_export, 0, NULL },
  { "/File/sep3",       NULL,          NULL, 0, "<Separator>" },
  { "/File/_Import GBM","<control>I",  import_gbm, 0, NULL },
  { "/File/sep4",       NULL,          NULL, 0, "<Separator>" },
  { "/File/_Quit",      "<control>Q",  destroy, 0, NULL },
  { "/_Edit",           NULL,          NULL, 0, "<Branch>" },
  { "/Edit/Undo",       "<control>Z",  map_undo_tile, 0, NULL },
  { "/Edit/sep5",       NULL,          NULL, 0, "<Separator>" },
  { "/Edit/Cut",        "<control>X",  map_cut_tile, 0, NULL },
  { "/Edit/_Copy",      "<control>C",  map_copy_tile, 0, NULL },
  { "/Edit/Paste",      "<control>V",  map_paste_tile, 0, NULL },
  { "/Edit/sep6",       NULL,          NULL, 0, "<Separator>" },
  { "/Edit/Clear",      NULL,          map_clear_tile, 0, NULL },
  { "/_Draw",           NULL,          NULL, 0, "<Branch>" },
  { "/Draw/_Memory",   "<control>M",   draw_use_mem, 0, "<RadioItem>" },
  { "/Draw/Me_tatiles","<control>T",   draw_use_meta, 0, "/Draw/Memory" },
  { "/_Zoom",           NULL,          NULL, 0, "<Branch>" },
  { "/Zoom/1:1 PC",     NULL,          map_zoom_1, 0, "<RadioItem>" },
  { "/Zoom/2:1 PC",     NULL,          map_zoom_2, 0, "/Zoom/1:1 PC" },
  { "/Zoom/2:1 CGB",    NULL,          map_zoom_3, 0, "/Zoom/1:1 PC" }
};

extern unsigned char tile_data[96 * 96], tile_palette_data[12 * 12];
extern unsigned int map_x, map_y, map_rec_x, map_rec_y;
extern struct metatile_16x16 *mt_first, *mt_last, *m_first, *m_last;
extern int mt;

int metatile = 0, metatiles = 0, map_draw_mode = MAP_DRAW_MEM, tile_source = TILE_SOURCE_MEM;



#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif

  int i, v, a;


  /* alloc memory */
  map_data = (unsigned int *)malloc(1024 * 1024 * sizeof(unsigned int));
  if (map_data == NULL) {
    fprintf(stderr, "main[a]: out of memory.\n");
    return 1;
  }
  map_property_data = malloc(1024 * 1024);
  if (map_property_data == NULL) {
    fprintf(stderr, "main[b]: out of memory.\n");
    free(map_data);
    return 1;
  }

  gtk_set_locale();

#ifdef WIN32
  gtk_init(&__argc, &__argv);
#else
  gtk_init(&argc, &argv);
#endif

  /* init memory */
  memset(memory_data, 0, 8 * 8 * 1024);
  memset(palette_buffer_1, 0, PBX * PBY * 3);
  memset(palette_buffer_2, 0, PBX * PBY * 3);
  memset(palette_buffer_3, 0, PBX * PBY * 3);
  memset(palette_buffer_4, 0, PBX * PBY * 3);
  memset(palette_buffer_5, 0, PBX * PBY * 3);
  memset(palette_buffer_6, 0, PBX * PBY * 3);
  memset(palette_buffer_7, 0, PBX * PBY * 3);
  memset(palette_buffer_8, 0, PBX * PBY * 3);
  memset(map_data, 0, 1024 * 1024 * sizeof(unsigned int));
  memset(map_property_data, 0, 1024 * 1024);
  memset(meta_buffer, 0, 64 * 64 * 3);

  /* init palette colors */
  for (a = 0; a < 8; a++)
    for (i = 0, v = 255; i < 12; ) {
      palette_colors_real[a * 4 * 3 + i++] = v;
      palette_colors_real[a * 4 * 3 + i++] = v;
      palette_colors_real[a * 4 * 3 + i++] = v;
      v -= 85;
    }

  /* init palette data */
  memset(palette_data, 0, 1024);
  memset(tile_palette_data, 0, 12 * 12);

  /* generate pc->cgb color conversion table */
  gbc_gen_rusty_filter();

  /* load preferences */
  load_prefs();

  /* init save names */
  savename[0] = 0;
  mapsavename[0] = 0;
  project_name[0] = 0;

  /* init file selections */
  file_selection_open = gtk_file_selection_new("Open Tile Memory");
  file_selection_save_as = gtk_file_selection_new("Save Tile Memory as");
  file_selection_save_as_png = gtk_file_selection_new("Save Tile Memory as (PNG)");
  file_selection_map_open = gtk_file_selection_new("Open Map");
  file_selection_map_save_as = gtk_file_selection_new("Save Map as");
  file_selection_export = gtk_file_selection_new("Export Tile Memory");
  file_selection_import_gbr = gtk_file_selection_new("Import GBR");
  file_selection_import_gbm = gtk_file_selection_new("Import GBM");
  file_selection_map_export = gtk_file_selection_new("Export Map");
  file_selection_project_open = gtk_file_selection_new("Open Project");
  file_selection_project_save_as = gtk_file_selection_new("Save Project as");
  file_selection_meta_open = gtk_file_selection_new("Open Metatile library");
  file_selection_meta_save_as = gtk_file_selection_new("Save Metatile library as");
  gtk_signal_connect(GTK_OBJECT(file_selection_open), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_save_as), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_save_as_png), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_map_open), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_map_save_as), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_export), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_import_gbr), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_import_gbm), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_map_export), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_project_open), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_project_save_as), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_meta_open), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selection_meta_save_as), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_open)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_open_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_open)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_open_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_save_as)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_save_as_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_save_as)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_save_as_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_save_as_png)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_save_as_png_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_save_as_png)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_save_as_png_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_export)->ok_button), "clicked", GTK_SIGNAL_FUNC(export_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_export)->cancel_button), "clicked", GTK_SIGNAL_FUNC(export_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_import_gbr)->ok_button), "clicked", GTK_SIGNAL_FUNC(import_ok_gbr), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_import_gbr)->cancel_button), "clicked", GTK_SIGNAL_FUNC(import_cancel_gbr), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_import_gbm)->ok_button), "clicked", GTK_SIGNAL_FUNC(import_ok_gbm), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_import_gbm)->cancel_button), "clicked", GTK_SIGNAL_FUNC(import_cancel_gbm), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_map_open)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_map_open_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_map_open)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_map_open_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_map_save_as)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_map_save_as_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_map_save_as)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_map_save_as_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_map_export)->ok_button), "clicked", GTK_SIGNAL_FUNC(map_export_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_map_export)->cancel_button), "clicked", GTK_SIGNAL_FUNC(map_export_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_project_open)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_project_open_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_project_open)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_project_open_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_project_save_as)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_project_save_as_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_project_save_as)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_project_save_as_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_meta_open)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_meta_open_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_meta_open)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_meta_open_cancel), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_meta_save_as)->ok_button), "clicked", GTK_SIGNAL_FUNC(file_meta_save_as_ok), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selection_meta_save_as)->cancel_button), "clicked", GTK_SIGNAL_FUNC(file_meta_save_as_cancel), NULL);

  /* gdk rgd inits */
  gdk_rgb_init();
  gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
  gtk_widget_set_default_visual(gdk_rgb_get_visual());

  init_edit_window();
  init_memory_window();
  init_palette_window();
  init_tile_window();
  init_tile_export_window();
  init_map_window();
  init_map_export_window();
  init_metatile_window();

  /* important! size 8x8 by default, variable updating is done in radio button clicked callback */
  export_radio_button_tile_size = export_radio_button_1;

  palette_remap();
  draw_edit_area();
  draw_tile_area();
  draw_memory_area();
  draw_map_area();
  draw_color_boxes();
  set_new_button_color(palette_one, 1, color_one);
  draw_button_color_indicator(1, palette_one, color_one);
  draw_button_color_indicator(2, palette_two, color_two);
  draw_button_color_indicator(3, palette_three, color_three);

  /* display */
  gtk_widget_show_all(memory_window);
  gtk_widget_show_all(edit_window);

  /* toggle window buttons */
  if (tile_window_status == ON) {
    tile_window_status = OFF; /* transition from OFF to ON in callback */
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Tiled"), TRUE);
  }
  if (palette_window_status == ON) {
    palette_window_status = OFF; /* transition from OFF to ON in callback */
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Palette"), TRUE);
  }
  if (map_window_status == ON) {
    map_window_status = OFF; /* transition from OFF to ON in callback */
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Map"), TRUE);
  }
  if (metatile_window_status == ON) {
    metatile_window_status = OFF; /* transition from OFF to ON in callback */
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Metatile"), TRUE);
  }
  if (edit_grid_loaded == ON)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/Grid"), TRUE);


  if (edit_mode == EDIT_MODE_8)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/8x8"), TRUE);
  else if (edit_mode == EDIT_MODE_8x16)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/8x16"), TRUE);
  else if (edit_mode == EDIT_MODE_16)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/16x16"), TRUE);
  else
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/32x32"), TRUE);

  if (zoom_mode == ZOOM_MODE_1)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Zoom/1:1 PC"), TRUE);
  else if (zoom_mode == ZOOM_MODE_2)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Zoom/2:1 PC"), TRUE);
  else
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Zoom/2:1 CGB"), TRUE);

  if (memory_size == MEMORY_SIZE_4KB)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Memory/4KB"), TRUE);
  else if (memory_size == MEMORY_SIZE_8KB)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Memory/8KB"), TRUE);
  else if (memory_size == MEMORY_SIZE_12KB)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Memory/12KB"), TRUE);
  else
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Memory/16KB"), TRUE);

  if (tile_zoom_mode == ZOOM_MODE_1)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(tile_item_factory, "/Zoom/1:1 PC"), TRUE);
  else if (tile_zoom_mode == ZOOM_MODE_2)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(tile_item_factory, "/Zoom/2:1 PC"), TRUE);
  else
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(tile_item_factory, "/Zoom/2:1 CGB"), TRUE);

  if (map_zoom_mode == ZOOM_MODE_1)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(map_item_factory, "/Zoom/1:1 PC"), TRUE);
  else if (map_zoom_mode == ZOOM_MODE_2)
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(map_item_factory, "/Zoom/2:1 PC"), TRUE);
  else
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(map_item_factory, "/Zoom/2:1 CGB"), TRUE);

  if (palette_mode_loaded == ON) {
    palette_mode_init = 2;
    if (palette_mode == PALETTE_MODE_PC)
      gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(palette_item_factory, "/Mode/PC"), TRUE);
    else if (palette_mode == PALETTE_MODE_RUS)
      gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(palette_item_factory, "/Mode/CGB Rusty"), TRUE);
    else
      gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(palette_item_factory, "/Mode/CGB Jedediah"), TRUE);
  }

  gtk_main();

  /* free memory */
  free(map_data);
  free(map_property_data);

  return 0;
}


int init_map_window(void) {

  /* map window */
  map_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  set_map_window_title();

  /* menu */
  map_accel_group = gtk_accel_group_new();
  map_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", map_accel_group);
  gtk_item_factory_create_items(map_item_factory, sizeof(map_menu_items)/sizeof(map_menu_items[0]), map_menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(map_window), map_accel_group);
  map_menubar = gtk_item_factory_get_widget(map_item_factory, "<main>");

  /* adjustment check button */
  map_check_button = gtk_check_button_new_with_label("Adjust to tile size");
  if (map_check_status == ON)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(map_check_button), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(map_check_button), FALSE);

  GTK_WIDGET_UNSET_FLAGS(map_check_button, GTK_CAN_FOCUS);

  /* drawing area */
  map_draw_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(map_draw_area), map_zoom_size * 256, map_zoom_size * 256);
  gtk_widget_add_events(map_draw_area,
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

  /* boxes */
  map_vbox1 = gtk_vbox_new(FALSE, 0);
  map_hbox1 = gtk_hbox_new(FALSE, 0);
  map_hbox2 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(map_vbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(map_hbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(map_hbox2), 1);

  /* scrollbars */
  map_adj1 = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1024, 2, 32, 32));
  map_adj2 = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1024, 2, 32, 32));
  map_scrollbar_h = gtk_hscrollbar_new(map_adj1);
  map_scrollbar_v = gtk_vscrollbar_new(map_adj2);

  /* tables */
  map_table = gtk_table_new(2, 2, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(map_table), 1);
  gtk_table_set_col_spacings(GTK_TABLE(map_table), 1);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(map_window), map_vbox1);
  gtk_box_pack_start(GTK_BOX(map_vbox1), map_menubar, FALSE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(map_vbox1), map_hbox1);
  gtk_box_pack_start(GTK_BOX(map_hbox1), map_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_vbox1), map_hbox2, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_hbox2), map_check_button, TRUE, FALSE, 0);

  gtk_table_attach(GTK_TABLE(map_table), map_draw_area, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_table), map_scrollbar_v, 1, 2, 0, 1, GTK_SHRINK, GTK_FILL, 0, 0);
  gtk_table_attach(GTK_TABLE(map_table), map_scrollbar_h, 0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(map_draw_area), "expose_event", GTK_SIGNAL_FUNC(map_draw_area_expose), NULL);
  gtk_signal_connect(GTK_OBJECT(map_draw_area), "button_press_event", GTK_SIGNAL_FUNC(map_button_press), NULL);
  gtk_signal_connect(GTK_OBJECT(map_draw_area), "button_release_event", GTK_SIGNAL_FUNC(map_button_release), NULL);
  gtk_signal_connect(GTK_OBJECT(map_draw_area), "motion_notify_event", GTK_SIGNAL_FUNC(map_motion_notify), NULL);
  gtk_signal_connect(GTK_OBJECT(map_window), "key_press_event", GTK_SIGNAL_FUNC(map_key_press), NULL);
  gtk_signal_connect(GTK_OBJECT(map_window), "key_release_event", GTK_SIGNAL_FUNC(map_key_release), NULL);
  gtk_signal_connect(GTK_OBJECT(map_window), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(map_adj1), "value_changed", GTK_SIGNAL_FUNC(map_scrollbar_clicked), NULL);
  gtk_signal_connect(GTK_OBJECT(map_adj2), "value_changed", GTK_SIGNAL_FUNC(map_scrollbar_clicked), NULL);
  gtk_signal_connect(GTK_OBJECT(map_check_button), "button_press_event", GTK_SIGNAL_FUNC(map_check_press), NULL);

  gtk_window_set_policy(GTK_WINDOW(map_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_tile_export_window(void) {

  /* window */
  export_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(export_window), "PPM Tile Export");

  /* vertical boxes */
  export_vbox1 = gtk_vbox_new(FALSE, 0);
  export_vbox2 = gtk_vbox_new(FALSE, 0);
  export_vbox3 = gtk_vbox_new(FALSE, 0);
  export_vbox4 = gtk_vbox_new(FALSE, 0);
  export_vbox5 = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(export_vbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(export_vbox2), 1);
  gtk_container_border_width(GTK_CONTAINER(export_vbox3), 1);
  gtk_container_border_width(GTK_CONTAINER(export_vbox4), 1);
  gtk_container_border_width(GTK_CONTAINER(export_vbox5), 1);

  /* tables */
  export_table = gtk_table_new(3, 4, FALSE);
  export_tile_table = gtk_table_new(4, 1, FALSE);
  export_size_table = gtk_table_new(2, 2, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(export_table), 1);
  gtk_table_set_col_spacings(GTK_TABLE(export_table), 6);
  gtk_table_set_row_spacings(GTK_TABLE(export_tile_table), 1);
  gtk_table_set_col_spacings(GTK_TABLE(export_tile_table), 1);
  gtk_table_set_row_spacings(GTK_TABLE(export_size_table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(export_size_table), 6);

  /* horizontal boxes */
  export_hbox1 = gtk_hbox_new(FALSE, 0);
  export_hbox2 = gtk_vbox_new(FALSE, 0);
  export_hbox3 = gtk_vbox_new(FALSE, 0);
  export_hbox4 = gtk_hbox_new(FALSE, 0);
  export_hbox5 = gtk_hbox_new(FALSE, 0);
  export_hbox6 = gtk_hbox_new(FALSE, 0);
  export_hbox7 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(export_hbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(export_hbox2), 1);
  gtk_container_border_width(GTK_CONTAINER(export_hbox3), 1);
  gtk_container_border_width(GTK_CONTAINER(export_hbox4), 1);
  gtk_container_border_width(GTK_CONTAINER(export_hbox5), 1);
  gtk_container_border_width(GTK_CONTAINER(export_hbox6), 1);
  gtk_container_border_width(GTK_CONTAINER(export_hbox7), 1);

  /* frames */
  export_frame_file = gtk_frame_new("Files");
  export_frame_tile = gtk_frame_new("Tile order");
  export_frame_size = gtk_frame_new("Export tiles");
  export_frame_data_formats = gtk_frame_new("Data formats");
  gtk_container_border_width(GTK_CONTAINER(export_frame_file), 4);
  gtk_container_border_width(GTK_CONTAINER(export_frame_tile), 4);
  gtk_container_border_width(GTK_CONTAINER(export_frame_size), 4);
  gtk_container_border_width(GTK_CONTAINER(export_frame_data_formats), 4);

  /* clists */
  export_clist_data = gtk_clist_new_with_titles(1, export_clist_title_data);
  export_clist_format = gtk_clist_new_with_titles(1, export_clist_title_format);
  export_clist_palette = gtk_clist_new_with_titles(1, export_clist_title_palette);
  gtk_clist_set_selection_mode(GTK_CLIST(export_clist_data), GTK_SELECTION_SINGLE);
  gtk_clist_set_selection_mode(GTK_CLIST(export_clist_format), GTK_SELECTION_SINGLE);
  gtk_clist_set_selection_mode(GTK_CLIST(export_clist_palette), GTK_SELECTION_SINGLE);
  gtk_clist_set_column_width(GTK_CLIST(export_clist_data), 0, 150);
  gtk_clist_set_column_width(GTK_CLIST(export_clist_format), 0, 150);
  gtk_clist_set_column_width(GTK_CLIST(export_clist_palette), 0, 150);
  gtk_clist_append(GTK_CLIST(export_clist_data), export_clist_data_data[0]);
  gtk_clist_append(GTK_CLIST(export_clist_data), export_clist_data_data[1]);
  gtk_clist_append(GTK_CLIST(export_clist_data), export_clist_data_data[2]);
  gtk_clist_append(GTK_CLIST(export_clist_format), export_clist_data_format[0]);
  gtk_clist_append(GTK_CLIST(export_clist_format), export_clist_data_format[1]);
  gtk_clist_append(GTK_CLIST(export_clist_format), export_clist_data_format[2]);
  gtk_clist_append(GTK_CLIST(export_clist_format), export_clist_data_format[3]);
  gtk_clist_append(GTK_CLIST(export_clist_format), export_clist_data_format[4]);
  gtk_clist_append(GTK_CLIST(export_clist_palette), export_clist_data_palette[0]);
  gtk_clist_append(GTK_CLIST(export_clist_palette), export_clist_data_palette[1]);
  gtk_clist_select_row(GTK_CLIST(export_clist_data), export_format_tile_data, 0);
  gtk_clist_select_row(GTK_CLIST(export_clist_format), export_format_format, 0);
  gtk_clist_select_row(GTK_CLIST(export_clist_palette), export_format_palette, 0);

  /* buttons */
  export_button_data = gtk_button_new_with_label("Get");
  export_button_palette = gtk_button_new_with_label("Get");
  export_button_tile_palette = gtk_button_new_with_label("Get");
  export_button_export = gtk_button_new_with_label("Export");
  export_button_cancel = gtk_button_new_with_label("Cancel");

  /* spin buttons */
  export_spin_end_x = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(31, 0, 31, 1, 31, 0)), 0.2, 0);
  export_spin_end_y = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(31, 0, 31, 1, 31, 0)), 0.2, 0);

  /* text entries */
  export_entry_data = gtk_entry_new();
  export_entry_palette = gtk_entry_new();
  export_entry_tile_palette = gtk_entry_new();

  /* check buttons */
  export_check_button_data = gtk_check_button_new_with_label("Export");
  export_check_button_palette = gtk_check_button_new_with_label("Export");
  export_check_button_tile_palette = gtk_check_button_new_with_label("Export");

  /* labels */
  export_label_data = gtk_label_new("Tile data");
  export_label_tile_palette = gtk_label_new("Tile palette");
  export_label_palette = gtk_label_new("Palette");
  export_label_end_x = gtk_label_new("End X");
  export_label_end_y = gtk_label_new("End Y");
  gtk_label_set_justify(GTK_LABEL(export_label_data), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify(GTK_LABEL(export_label_palette), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify(GTK_LABEL(export_label_tile_palette), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify(GTK_LABEL(export_label_end_x), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify(GTK_LABEL(export_label_end_y), GTK_JUSTIFY_RIGHT);

  /* radio buttons */
  export_radio_button_1 = gtk_radio_button_new_with_label(NULL, "8x8");
  export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(export_radio_button_1));
  export_radio_button_2 = gtk_radio_button_new_with_label(export_radio_button_group, "8x16");
  export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(export_radio_button_2));
  export_radio_button_3 = gtk_radio_button_new_with_label(export_radio_button_group, "16x16 H");
  export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(export_radio_button_3));
  export_radio_button_4 = gtk_radio_button_new_with_label(export_radio_button_group, "16x16 V");
  export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(export_radio_button_4));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(export_radio_button_1), TRUE);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(export_window), export_vbox1);
  gtk_container_add(GTK_CONTAINER(export_vbox1), export_frame_file);
  gtk_container_add(GTK_CONTAINER(export_vbox1), export_hbox4);
  gtk_container_add(GTK_CONTAINER(export_vbox1), export_hbox5);
  gtk_container_add(GTK_CONTAINER(export_vbox1), export_vbox5);
  gtk_container_add(GTK_CONTAINER(export_hbox4), export_frame_tile);
  gtk_container_add(GTK_CONTAINER(export_hbox4), export_frame_size);
  gtk_container_add(GTK_CONTAINER(export_hbox5), export_frame_data_formats);
  gtk_container_add(GTK_CONTAINER(export_frame_file), export_hbox1);
  gtk_container_add(GTK_CONTAINER(export_frame_tile), export_hbox2);
  gtk_container_add(GTK_CONTAINER(export_frame_size), export_hbox3);
  gtk_container_add(GTK_CONTAINER(export_frame_data_formats), export_hbox6);
  gtk_box_pack_start(GTK_BOX(export_hbox6), export_clist_format, TRUE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(export_hbox6), export_vbox4);
  gtk_box_pack_start(GTK_BOX(export_vbox4), export_clist_palette, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(export_vbox4), export_clist_data, TRUE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(export_hbox1), export_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(export_hbox2), export_tile_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(export_hbox3), export_size_table, TRUE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(export_vbox5), export_hbox7, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(export_hbox7), export_button_export, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(export_hbox7), export_button_cancel, TRUE, TRUE, 20);

  gtk_table_attach(GTK_TABLE(export_table), export_label_data, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_entry_data, 1, 2, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_label_tile_palette, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_entry_tile_palette, 1, 2, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_label_palette, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_entry_palette, 1, 2, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_button_data, 2, 3, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_button_tile_palette, 2, 3, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_button_palette, 2, 3, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_check_button_data, 3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_check_button_tile_palette, 3, 4, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_table), export_check_button_palette, 3, 4, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_tile_table), export_radio_button_1, 0, 1, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_tile_table), export_radio_button_2, 0, 1, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_tile_table), export_radio_button_3, 0, 1, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_tile_table), export_radio_button_4, 0, 1, 3, 4, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_size_table), export_label_end_x, 0, 1, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_size_table), export_label_end_y, 0, 1, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_size_table), export_spin_end_x, 1, 2, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(export_size_table), export_spin_end_y, 1, 2, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(export_window), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(export_button_data), "clicked", GTK_SIGNAL_FUNC(export_button_data_get), NULL);
  gtk_signal_connect(GTK_OBJECT(export_button_palette), "clicked", GTK_SIGNAL_FUNC(export_button_palette_get), NULL);
  gtk_signal_connect(GTK_OBJECT(export_button_tile_palette), "clicked", GTK_SIGNAL_FUNC(export_button_tile_palette_get), NULL);
  gtk_signal_connect(GTK_OBJECT(export_button_export), "clicked", GTK_SIGNAL_FUNC(export_export_pressed), NULL);
  gtk_signal_connect(GTK_OBJECT(export_button_cancel), "clicked", GTK_SIGNAL_FUNC(export_cancel_pressed), NULL);
  gtk_signal_connect(GTK_OBJECT(export_radio_button_1), "clicked", GTK_SIGNAL_FUNC(export_button_tile_size), export_radio_button_1);
  gtk_signal_connect(GTK_OBJECT(export_radio_button_2), "clicked", GTK_SIGNAL_FUNC(export_button_tile_size), export_radio_button_2);
  gtk_signal_connect(GTK_OBJECT(export_radio_button_3), "clicked", GTK_SIGNAL_FUNC(export_button_tile_size), export_radio_button_3);
  gtk_signal_connect(GTK_OBJECT(export_radio_button_4), "clicked", GTK_SIGNAL_FUNC(export_button_tile_size), export_radio_button_4);
  gtk_signal_connect(GTK_OBJECT(export_clist_format), "select_row", GTK_SIGNAL_FUNC(export_format_format_selected), NULL);
  gtk_signal_connect(GTK_OBJECT(export_clist_palette), "select_row", GTK_SIGNAL_FUNC(export_format_palette_selected), NULL);
  gtk_signal_connect(GTK_OBJECT(export_clist_data), "select_row", GTK_SIGNAL_FUNC(export_format_tile_data_selected), NULL);

  gtk_window_set_policy(GTK_WINDOW(export_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_map_export_window(void) {

  /* window */
  map_export_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(map_export_window), "PPM Map Export");

  /* tables */
  map_export_file_table = gtk_table_new(2, 4, FALSE);
  map_export_table_offset = gtk_table_new(1, 2, TRUE);
  map_export_mode_table = gtk_table_new(4, 1, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(map_export_file_table), 1);
  gtk_table_set_col_spacings(GTK_TABLE(map_export_file_table), 6);
  gtk_table_set_row_spacings(GTK_TABLE(map_export_table_offset), 1);
  gtk_table_set_col_spacings(GTK_TABLE(map_export_table_offset), 6);
  gtk_table_set_row_spacings(GTK_TABLE(map_export_mode_table), 1);
  gtk_table_set_col_spacings(GTK_TABLE(map_export_mode_table), 1);

  /* frames */
  map_export_frame_file = gtk_frame_new("Files");
  map_export_frame_format = gtk_frame_new("Data formats");
  map_export_frame_tile = gtk_frame_new("Tile options");
  map_export_frame_mode = gtk_frame_new("Export mode");
  gtk_container_border_width(GTK_CONTAINER(map_export_frame_file), 4);
  gtk_container_border_width(GTK_CONTAINER(map_export_frame_format), 4);
  gtk_container_border_width(GTK_CONTAINER(map_export_frame_tile), 4);
  gtk_container_border_width(GTK_CONTAINER(map_export_frame_mode), 4);

  /* clists */
  map_export_clist_format = gtk_clist_new_with_titles(1, export_clist_title_format);
  map_export_clist_size = gtk_clist_new_with_titles(1, map_export_clist_title_size);
  gtk_clist_set_selection_mode(GTK_CLIST(map_export_clist_format), GTK_SELECTION_SINGLE);
  gtk_clist_set_selection_mode(GTK_CLIST(map_export_clist_size), GTK_SELECTION_SINGLE);
  gtk_clist_set_column_width(GTK_CLIST(map_export_clist_format), 0, 150);
  gtk_clist_set_column_width(GTK_CLIST(map_export_clist_size), 0, 150);
  gtk_clist_append(GTK_CLIST(map_export_clist_format), export_clist_data_format[0]);
  gtk_clist_append(GTK_CLIST(map_export_clist_format), export_clist_data_format[1]);
  gtk_clist_append(GTK_CLIST(map_export_clist_format), export_clist_data_format[2]);
  gtk_clist_append(GTK_CLIST(map_export_clist_format), export_clist_data_format[3]);
  gtk_clist_append(GTK_CLIST(map_export_clist_format), export_clist_data_format[4]);
  gtk_clist_append(GTK_CLIST(map_export_clist_size), map_export_clist_data_size[0]);
  gtk_clist_append(GTK_CLIST(map_export_clist_size), map_export_clist_data_size[1]);
  gtk_clist_select_row(GTK_CLIST(map_export_clist_format), map_export_format_format, 0);
  gtk_clist_select_row(GTK_CLIST(map_export_clist_size), map_export_format_size, 0);

  /* check buttons */
  map_export_check_button_map_data = gtk_check_button_new_with_label("Export");
  map_export_check_button_map_property_data = gtk_check_button_new_with_label("Export");

  /* radio buttons */
  map_export_radio_button_1 = gtk_radio_button_new_with_label(NULL, "Normal");
  map_export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(map_export_radio_button_1));
  map_export_radio_button_2 = gtk_radio_button_new_with_label(map_export_radio_button_group, "16x16 Metatiles F");
  map_export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(map_export_radio_button_2));
  map_export_radio_button_3 = gtk_radio_button_new_with_label(map_export_radio_button_group, "16x16 Metatiles D");
  map_export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(map_export_radio_button_3));
  map_export_radio_button_4 = gtk_radio_button_new_with_label(map_export_radio_button_group, "16x16 Metatiles L");
  map_export_radio_button_group = gtk_radio_button_group(GTK_RADIO_BUTTON(map_export_radio_button_4));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(map_export_radio_button_1), TRUE);

  /* buttons */
  map_export_button_map_data = gtk_button_new_with_label("Get");
  map_export_button_map_property_data = gtk_button_new_with_label("Get");
  map_export_button_export = gtk_button_new_with_label("Export");
  map_export_button_cancel = gtk_button_new_with_label("Cancel");

  /* text entries */
  map_export_entry_map_data = gtk_entry_new();
  map_export_entry_map_property_data = gtk_entry_new();

  /* labels */
  map_export_label_offset = gtk_label_new("Offset");
  map_export_label_map_data = gtk_label_new("Map data");
  map_export_label_map_property_data = gtk_label_new("Map property data");
  gtk_label_set_justify(GTK_LABEL(map_export_label_offset), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify(GTK_LABEL(map_export_label_map_data), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify(GTK_LABEL(map_export_label_map_property_data), GTK_JUSTIFY_RIGHT);

  /* vertical boxes */
  map_export_vbox1 = gtk_vbox_new(FALSE, 0);
  map_export_vbox2 = gtk_vbox_new(FALSE, 0);
  map_export_vbox3 = gtk_vbox_new(FALSE, 0);
  map_export_hbox1 = gtk_hbox_new(FALSE, 0);
  map_export_hbox2 = gtk_hbox_new(FALSE, 0);
  map_export_hbox3 = gtk_hbox_new(FALSE, 0);
  map_export_hbox4 = gtk_hbox_new(FALSE, 0);
  map_export_hbox5 = gtk_hbox_new(FALSE, 0);
  map_export_hbox6 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(map_export_vbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_vbox2), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_vbox3), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_hbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_hbox2), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_hbox3), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_hbox4), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_hbox5), 1);
  gtk_container_border_width(GTK_CONTAINER(map_export_hbox6), 1);

  /* spin buttons */
  map_export_spin_offset = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, -128, 127, 1, 32, 0)), 0.2, 0);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(map_export_window), map_export_vbox1);
  gtk_container_add(GTK_CONTAINER(map_export_vbox1), map_export_frame_file);
  gtk_container_add(GTK_CONTAINER(map_export_vbox1), map_export_hbox5);
  gtk_container_add(GTK_CONTAINER(map_export_vbox1), map_export_frame_format);
  gtk_container_add(GTK_CONTAINER(map_export_vbox1), map_export_vbox2);
  gtk_container_add(GTK_CONTAINER(map_export_frame_file), map_export_hbox1);
  gtk_container_add(GTK_CONTAINER(map_export_frame_tile), map_export_hbox4);
  gtk_container_add(GTK_CONTAINER(map_export_frame_mode), map_export_hbox6);
  gtk_container_add(GTK_CONTAINER(map_export_frame_format), map_export_hbox2);
  gtk_container_add(GTK_CONTAINER(map_export_hbox5), map_export_frame_tile);
  gtk_container_add(GTK_CONTAINER(map_export_hbox5), map_export_frame_mode);

  gtk_box_pack_start(GTK_BOX(map_export_vbox2), map_export_hbox3, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(map_export_hbox1), map_export_file_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_export_hbox3), map_export_button_export, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(map_export_hbox3), map_export_button_cancel, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(map_export_hbox2), map_export_clist_format, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_export_hbox2), map_export_clist_size, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_export_hbox4), map_export_vbox3, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_export_vbox3), map_export_table_offset, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(map_export_hbox6), map_export_mode_table, TRUE, FALSE, 0);

  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_label_map_data, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_entry_map_data, 1, 2, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_button_map_data, 2, 3, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_check_button_map_data, 3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_label_map_property_data, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_entry_map_property_data, 1, 2, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_button_map_property_data, 2, 3, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_file_table), map_export_check_button_map_property_data, 3, 4, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);

  gtk_table_attach(GTK_TABLE(map_export_table_offset), map_export_label_offset, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_table_offset), map_export_spin_offset, 1, 2, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);

  gtk_table_attach(GTK_TABLE(map_export_mode_table), map_export_radio_button_1, 0, 1, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_mode_table), map_export_radio_button_2, 0, 1, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_mode_table), map_export_radio_button_3, 0, 1, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(map_export_mode_table), map_export_radio_button_4, 0, 1, 3, 4, GTK_EXPAND, GTK_SHRINK, 0, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(map_export_window), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_button_map_data), "clicked", GTK_SIGNAL_FUNC(map_export_button_map_data_get), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_button_map_property_data), "clicked", GTK_SIGNAL_FUNC(map_export_button_map_property_data_get), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_button_export), "clicked", GTK_SIGNAL_FUNC(map_export_export_pressed), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_button_cancel), "clicked", GTK_SIGNAL_FUNC(map_export_cancel_pressed), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_clist_format), "select_row", GTK_SIGNAL_FUNC(map_export_format_format_selected), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_clist_size), "select_row", GTK_SIGNAL_FUNC(map_export_format_size_selected), NULL);
  gtk_signal_connect(GTK_OBJECT(map_export_radio_button_1), "clicked", GTK_SIGNAL_FUNC(map_export_button_tile_size), map_export_radio_button_1);
  gtk_signal_connect(GTK_OBJECT(map_export_radio_button_2), "clicked", GTK_SIGNAL_FUNC(map_export_button_tile_size), map_export_radio_button_2);
  gtk_signal_connect(GTK_OBJECT(map_export_radio_button_3), "clicked", GTK_SIGNAL_FUNC(map_export_button_tile_size), map_export_radio_button_3);
  gtk_signal_connect(GTK_OBJECT(map_export_radio_button_4), "clicked", GTK_SIGNAL_FUNC(map_export_button_tile_size), map_export_radio_button_4);

  gtk_window_set_policy(GTK_WINDOW(map_export_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_palette_window(void) {

  /* window */
  palette_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  set_palette_window_title();

  /* menu */
  palette_accel_group = gtk_accel_group_new();
  palette_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", palette_accel_group);
  gtk_item_factory_create_items(palette_item_factory, sizeof(palette_menu_items)/sizeof(palette_menu_items[0]), palette_menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(palette_window), palette_accel_group);
  palette_menubar = gtk_item_factory_get_widget(palette_item_factory, "<main>");

  /* drawing areas */
  palette_draw_area_1 = gtk_drawing_area_new();
  palette_draw_area_2 = gtk_drawing_area_new();
  palette_draw_area_3 = gtk_drawing_area_new();
  palette_draw_area_4 = gtk_drawing_area_new();
  palette_draw_area_5 = gtk_drawing_area_new();
  palette_draw_area_6 = gtk_drawing_area_new();
  palette_draw_area_7 = gtk_drawing_area_new();
  palette_draw_area_8 = gtk_drawing_area_new();
  gtk_widget_add_events(palette_draw_area_1, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_2, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_3, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_4, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_5, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_6, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_7, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(palette_draw_area_8, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_1), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_2), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_3), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_4), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_5), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_6), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_7), PBX, PBY);
  gtk_drawing_area_size(GTK_DRAWING_AREA(palette_draw_area_8), PBX, PBY);

  /* vertical boxes */
  palette_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(palette_vbox), 1);

  /* horizontal boxes */
  palette_hbox1 = gtk_hbox_new(FALSE, 0);
  palette_hbox2 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(palette_hbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(palette_hbox2), 1);

  /* button/palette table */
  palette_table = gtk_table_new(4, 10, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(palette_table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(palette_table), 2);

  /* palette copy/pase buttons */
  palette_button_p0c = gtk_button_new_with_label("C");
  palette_button_p0p = gtk_button_new_with_label("P");
  palette_button_p1c = gtk_button_new_with_label("C");
  palette_button_p1p = gtk_button_new_with_label("P");
  palette_button_p2c = gtk_button_new_with_label("C");
  palette_button_p2p = gtk_button_new_with_label("P");
  palette_button_p3c = gtk_button_new_with_label("C");
  palette_button_p3p = gtk_button_new_with_label("P");
  palette_button_p4c = gtk_button_new_with_label("C");
  palette_button_p4p = gtk_button_new_with_label("P");
  palette_button_p5c = gtk_button_new_with_label("C");
  palette_button_p5p = gtk_button_new_with_label("P");
  palette_button_p6c = gtk_button_new_with_label("C");
  palette_button_p6p = gtk_button_new_with_label("P");
  palette_button_p7c = gtk_button_new_with_label("C");
  palette_button_p7p = gtk_button_new_with_label("P");
  palette_button_p0i = gtk_button_new_with_label("IP");
  palette_button_p1i = gtk_button_new_with_label("IP");
  palette_button_p2i = gtk_button_new_with_label("IP");
  palette_button_p3i = gtk_button_new_with_label("IP");
  palette_button_p4i = gtk_button_new_with_label("IP");
  palette_button_p5i = gtk_button_new_with_label("IP");
  palette_button_p6i = gtk_button_new_with_label("IP");
  palette_button_p7i = gtk_button_new_with_label("IP");
  palette_button_p0r = gtk_button_new_with_label("R");
  palette_button_p1r = gtk_button_new_with_label("R");
  palette_button_p2r = gtk_button_new_with_label("R");
  palette_button_p3r = gtk_button_new_with_label("R");
  palette_button_p4r = gtk_button_new_with_label("R");
  palette_button_p5r = gtk_button_new_with_label("R");
  palette_button_p6r = gtk_button_new_with_label("R");
  palette_button_p7r = gtk_button_new_with_label("R");

  /* tooltips */
  palette_tooltips = gtk_tooltips_new();
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p0c, "Copy palette 0", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p0p, "Paste to palette 0", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p1c, "Copy palette 1", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p1p, "Paste to palette 1", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p2c, "Copy palette 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p2p, "Paste to palette 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p3c, "Copy palette 3", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p3p, "Paste to palette 3", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p4c, "Copy palette 4", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p4p, "Paste to palette 4", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p5c, "Copy palette 5", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p5p, "Paste to palette 5", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p6c, "Copy palette 6", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p6p, "Paste to palette 6", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p7c, "Copy palette 7", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p7p, "Paste to palette 7", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p0i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p1i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p2i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p3i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p4i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p5i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p6i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p7i, "Interpolate colors 1 and 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p0r, "Reverse palette 0", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p1r, "Reverse palette 1", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p2r, "Reverse palette 2", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p3r, "Reverse palette 3", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p4r, "Reverse palette 4", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p5r, "Reverse palette 5", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p6r, "Reverse palette 6", NULL);
  gtk_tooltips_set_tip(palette_tooltips, palette_button_p7r, "Reverse palette 7", NULL);

  /* color selection widget */
  color_selection = gtk_color_selection_new();

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(palette_window), palette_vbox);
  gtk_box_pack_start(GTK_BOX(palette_vbox), palette_menubar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(palette_vbox), palette_hbox1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(palette_vbox), palette_hbox2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(palette_hbox1), color_selection, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(palette_hbox2), palette_table, TRUE, FALSE, 0);

  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p0i, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p1i, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p2i, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p3i, 0, 1, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p4i, 6, 7, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p5i, 6, 7, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p6i, 6, 7, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p7i, 6, 7, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);

  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p0r, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p1r, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p2r, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p3r, 1, 2, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p4r, 7, 8, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p5r, 7, 8, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p6r, 7, 8, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p7r, 7, 8, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);

  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p0c, 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p1c, 2, 3, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p2c, 2, 3, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p3c, 2, 3, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p4c, 8, 9, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p5c, 8, 9, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p6c, 8, 9, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p7c, 8, 9, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);

  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p0p, 3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p1p, 3, 4, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p2p, 3, 4, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p3p, 3, 4, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p4p, 9, 10, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p5p, 9, 10, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p6p, 9, 10, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_button_p7p, 9, 10, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);

  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_1, 4, 5, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_2, 4, 5, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_3, 4, 5, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_4, 4, 5, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_5, 5, 6, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_6, 5, 6, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_7, 5, 6, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(palette_table), palette_draw_area_8, 5, 6, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(palette_window), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(color_selection), "color_changed", GTK_SIGNAL_FUNC(color_selection_change), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_1), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_1), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_2), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_2), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_3), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_3), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_4), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_4), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_5), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_5), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_6), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_6), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_7), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_7), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_8), "expose_event", GTK_SIGNAL_FUNC(palette_draw_area_expose_8), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_1), "button_press_event", GTK_SIGNAL_FUNC(palette_1_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_2), "button_press_event", GTK_SIGNAL_FUNC(palette_2_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_3), "button_press_event", GTK_SIGNAL_FUNC(palette_3_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_4), "button_press_event", GTK_SIGNAL_FUNC(palette_4_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_5), "button_press_event", GTK_SIGNAL_FUNC(palette_5_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_6), "button_press_event", GTK_SIGNAL_FUNC(palette_6_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_7), "button_press_event", GTK_SIGNAL_FUNC(palette_7_button), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_draw_area_8), "button_press_event", GTK_SIGNAL_FUNC(palette_8_button), NULL);

  gtk_signal_connect(GTK_OBJECT(palette_button_p0c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p0p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p1c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p1p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p2c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p2p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p3c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p3p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p4c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p4p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p5c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p5p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p6c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p6p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p7c), "clicked", GTK_SIGNAL_FUNC(palette_button_copy), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p7p), "clicked", GTK_SIGNAL_FUNC(palette_button_paste), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p0i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p1i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p2i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p3i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p4i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p5i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p6i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p7i), "clicked", GTK_SIGNAL_FUNC(palette_button_interpolate), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p0r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p1r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p2r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p3r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p4r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p5r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p6r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);
  gtk_signal_connect(GTK_OBJECT(palette_button_p7r), "clicked", GTK_SIGNAL_FUNC(palette_button_reverse), NULL);

  gtk_window_set_policy(GTK_WINDOW(palette_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_tile_window(void) {

  /* window */
  tile_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  set_tile_window_title();

  /* tile window menu */
  tile_accel_group = gtk_accel_group_new();
  tile_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", tile_accel_group);
  gtk_item_factory_create_items(tile_item_factory, sizeof(tile_menu_items)/sizeof(tile_menu_items[0]), tile_menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(tile_window), tile_accel_group);
  tile_menubar = gtk_item_factory_get_widget(tile_item_factory, "<main>");

  /* drawing area */
  tile_draw_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(tile_draw_area), tile_zoom_size * 96, tile_zoom_size * 96);

  /* vertical boxes */
  tile_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(tile_vbox), 1);

  /* horizontal boxes */
  tile_hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(tile_hbox), 1);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(tile_window), tile_vbox);
  gtk_box_pack_start(GTK_BOX(tile_vbox), tile_menubar, FALSE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(tile_vbox), tile_hbox);
  gtk_box_pack_start(GTK_BOX(tile_hbox), tile_draw_area, TRUE, FALSE, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(tile_window), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(tile_draw_area), "expose_event", GTK_SIGNAL_FUNC(tile_draw_area_expose), NULL);

  gtk_window_set_policy(GTK_WINDOW(tile_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_metatile_window(void) {

  /* window */
  metatile_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  set_metatile_window_title();

  /* menu */
  metatile_accel_group = gtk_accel_group_new();
  metatile_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", metatile_accel_group);
  gtk_item_factory_create_items(metatile_item_factory, sizeof(metatile_menu_items)/sizeof(metatile_menu_items[0]), metatile_menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(metatile_window), metatile_accel_group);
  metatile_menubar = gtk_item_factory_get_widget(metatile_item_factory, "<main>");

  /* boxes */
  metatile_vbox1 = gtk_vbox_new(FALSE, 0);
  metatile_vbox2 = gtk_vbox_new(FALSE, 0);
  metatile_vbox3 = gtk_vbox_new(FALSE, 0);
  metatile_vbox4 = gtk_vbox_new(FALSE, 0);
  metatile_vbox5 = gtk_vbox_new(FALSE, 0);
  metatile_vbox6 = gtk_vbox_new(FALSE, 0);
  metatile_hbox1 = gtk_hbox_new(FALSE, 0);
  metatile_hbox2 = gtk_hbox_new(FALSE, 0);
  metatile_hbox3 = gtk_hbox_new(FALSE, 0);
  metatile_hbox4 = gtk_hbox_new(FALSE, 0);
  metatile_hbox5 = gtk_hbox_new(FALSE, 0);
  metatile_hbox6 = gtk_hbox_new(FALSE, 0);
  metatile_hbox7 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(metatile_vbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_vbox2), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_vbox3), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_vbox4), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_vbox5), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_vbox6), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox2), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox3), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox4), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox5), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox6), 1);
  gtk_container_border_width(GTK_CONTAINER(metatile_hbox7), 1);

  /* buttons */
  meta_right = gtk_button_new();
  meta_left = gtk_button_new();
  meta_arrow_right = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
  meta_arrow_left = gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT);
  gtk_widget_set_usize(GTK_WIDGET(meta_arrow_right), 30, 30);
  gtk_widget_set_usize(GTK_WIDGET(meta_arrow_left), 30, 30);
  gtk_container_add(GTK_CONTAINER(meta_right), meta_arrow_right);
  gtk_container_add(GTK_CONTAINER(meta_left), meta_arrow_left);
  GTK_WIDGET_UNSET_FLAGS(meta_right, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_left, GTK_CAN_FOCUS);

  /* tooltips */
  meta_tooltips = gtk_tooltips_new();
  gtk_tooltips_set_tip(meta_tooltips, meta_right, "Next metatile", NULL);
  gtk_tooltips_set_tip(meta_tooltips, meta_left, "Previous metatile", NULL);

  /* check buttons */
  meta_cb_x1 = gtk_check_button_new();
  meta_cb_x2 = gtk_check_button_new();
  meta_cb_x3 = gtk_check_button_new();
  meta_cb_x4 = gtk_check_button_new();
  meta_cb_y1 = gtk_check_button_new();
  meta_cb_y2 = gtk_check_button_new();
  meta_cb_y3 = gtk_check_button_new();
  meta_cb_y4 = gtk_check_button_new();
  GTK_WIDGET_UNSET_FLAGS(meta_cb_x1, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_x2, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_x3, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_x4, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_y1, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_y2, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_y3, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(meta_cb_y4, GTK_CAN_FOCUS);

  /* spin buttons */
  meta_spin_t1 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1023, 1, 32, 0)), 0.2, 0);
  meta_spin_t2 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1023, 1, 32, 0)), 0.2, 0);
  meta_spin_t3 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1023, 1, 32, 0)), 0.2, 0);
  meta_spin_t4 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1023, 1, 32, 0)), 0.2, 0);
  meta_spin_p1 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 7, 1, 7, 0)), 0.2, 0);
  meta_spin_p2 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 7, 1, 7, 0)), 0.2, 0);
  meta_spin_p3 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 7, 1, 7, 0)), 0.2, 0);
  meta_spin_p4 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 7, 1, 7, 0)), 0.2, 0);

  /* frames */
  meta_fr_img = gtk_frame_new("Metatile");
  meta_fr_id = gtk_frame_new("Tile ID's");
  meta_fr_pal = gtk_frame_new("Palette");
  meta_fr_x = gtk_frame_new("X-flip");
  meta_fr_y = gtk_frame_new("Y-flip");
  gtk_container_border_width(GTK_CONTAINER(meta_fr_img), 4);
  gtk_container_border_width(GTK_CONTAINER(meta_fr_id), 4);
  gtk_container_border_width(GTK_CONTAINER(meta_fr_pal), 4);
  gtk_container_border_width(GTK_CONTAINER(meta_fr_x), 4);
  gtk_container_border_width(GTK_CONTAINER(meta_fr_y), 4);

  /* button table */
  meta_button_table = gtk_table_new(1, 2, TRUE);
  meta_t_table = gtk_table_new(2, 2, TRUE);
  meta_p_table = gtk_table_new(2, 2, TRUE);
  meta_x_table = gtk_table_new(2, 2, TRUE);
  meta_y_table = gtk_table_new(2, 2, TRUE);

  /* drawing area */
  meta_da = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(meta_da), 64, 64);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(metatile_window), metatile_vbox1);
  gtk_box_pack_start(GTK_BOX(metatile_vbox1), metatile_menubar, FALSE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(metatile_vbox1), metatile_hbox1);
  gtk_container_add(GTK_CONTAINER(metatile_vbox1), metatile_hbox2);
  gtk_container_add(GTK_CONTAINER(metatile_hbox1), meta_fr_img);
  gtk_container_add(GTK_CONTAINER(metatile_hbox1), meta_fr_id);
  gtk_container_add(GTK_CONTAINER(metatile_hbox1), meta_fr_pal);
  gtk_container_add(GTK_CONTAINER(metatile_hbox1), meta_fr_x);
  gtk_container_add(GTK_CONTAINER(metatile_hbox1), meta_fr_y);
  gtk_container_add(GTK_CONTAINER(meta_fr_img), metatile_hbox3);
  gtk_container_add(GTK_CONTAINER(meta_fr_id), metatile_hbox4);
  gtk_container_add(GTK_CONTAINER(meta_fr_pal), metatile_hbox5);
  gtk_container_add(GTK_CONTAINER(meta_fr_x), metatile_hbox6);
  gtk_container_add(GTK_CONTAINER(meta_fr_y), metatile_hbox7);
  gtk_box_pack_start(GTK_BOX(metatile_hbox2), meta_button_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_hbox3), metatile_vbox2, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_hbox4), metatile_vbox3, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_hbox5), metatile_vbox4, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_hbox6), metatile_vbox5, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_hbox7), metatile_vbox6, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_vbox2), meta_da, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_vbox3), meta_t_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_vbox4), meta_p_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_vbox5), meta_x_table, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(metatile_vbox6), meta_y_table, TRUE, FALSE, 0);

  /* place boxes and widgets */
  gtk_table_attach(GTK_TABLE(meta_button_table), meta_left, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_button_table), meta_right, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_t_table), meta_spin_t1, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_t_table), meta_spin_t2, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_t_table), meta_spin_t3, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_t_table), meta_spin_t4, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_p_table), meta_spin_p1, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_p_table), meta_spin_p2, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_p_table), meta_spin_p3, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_p_table), meta_spin_p4, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_x_table), meta_cb_x1, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_x_table), meta_cb_x2, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_x_table), meta_cb_x3, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_x_table), meta_cb_x4, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_y_table), meta_cb_y1, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_y_table), meta_cb_y2, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_y_table), meta_cb_y3, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(meta_y_table), meta_cb_y4, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(meta_da), "expose_event", GTK_SIGNAL_FUNC(meta_da_expose), NULL);
  gtk_signal_connect(GTK_OBJECT(metatile_window), "delete_event", GTK_SIGNAL_FUNC(delete_window_widget), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_right), "clicked", GTK_SIGNAL_FUNC(meta_press_right), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_left), "clicked", GTK_SIGNAL_FUNC(meta_press_left), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t1))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t2))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t3))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_t4))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p1))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p2))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p3))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(meta_spin_p4))), "value_changed", GTK_SIGNAL_FUNC(meta_tile_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_x1), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_x2), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_x3), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_x4), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_y1), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_y2), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_y3), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);
  gtk_signal_connect(GTK_OBJECT(meta_cb_y4), "button_press_event", GTK_SIGNAL_FUNC(meta_flip_press), NULL);

  gtk_window_set_policy(GTK_WINDOW(metatile_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_edit_window(void) {

  /* window */
  edit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  set_edit_window_title();
  gtk_widget_add_events(edit_window, GDK_KEY_RELEASE_MASK);

  /* menu */
  edit_accel_group = gtk_accel_group_new();
  edit_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", edit_accel_group);
  gtk_item_factory_create_items(edit_item_factory, sizeof(edit_menu_items)/sizeof(edit_menu_items[0]), edit_menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(edit_window), edit_accel_group);
  edit_menubar = gtk_item_factory_get_widget(edit_item_factory, "<main>");

  /* drawing area */
  edit_draw_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(edit_draw_area), EDIT_WIDTH, EDIT_HEIGHT);
  gtk_widget_add_events(edit_draw_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

  /* boxes */
  edit_vbox = gtk_vbox_new(FALSE, 0);
  edit_hbox = gtk_hbox_new(FALSE, 0);
  button_hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(edit_vbox), 1);
  gtk_container_border_width(GTK_CONTAINER(edit_hbox), 1);
  gtk_container_border_width(GTK_CONTAINER(button_hbox), 1);

  /* button table */
  button_table = gtk_table_new(3, 4, TRUE);

  /* tile buttons */
  button_right = gtk_button_new();
  button_left = gtk_button_new();
  button_up = gtk_button_new();
  button_down = gtk_button_new();
  arrow_right = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
  arrow_left = gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT);
  arrow_up = gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_OUT);
  arrow_down = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);
  gtk_widget_set_usize(GTK_WIDGET(arrow_right), 30, 30);
  gtk_widget_set_usize(GTK_WIDGET(arrow_left), 30, 30);
  gtk_widget_set_usize(GTK_WIDGET(arrow_up), 30, 30);
  gtk_widget_set_usize(GTK_WIDGET(arrow_down), 30, 30);
  gtk_container_add(GTK_CONTAINER(button_right), arrow_right);
  gtk_container_add(GTK_CONTAINER(button_left), arrow_left);
  gtk_container_add(GTK_CONTAINER(button_up), arrow_up);
  gtk_container_add(GTK_CONTAINER(button_down), arrow_down);

  GTK_WIDGET_UNSET_FLAGS(button_right, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(button_left, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(button_up, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(button_down, GTK_CAN_FOCUS);

  /* shift buttons */
  shift_right = gtk_button_new();
  shift_left = gtk_button_new();
  shift_up = gtk_button_new();
  shift_down = gtk_button_new();
  su1 = gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_OUT);
  sd1 = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);
  sl1 = gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT);
  sr1 = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
  gtk_widget_set_usize(GTK_WIDGET(su1), 20, 20);
  gtk_widget_set_usize(GTK_WIDGET(sd1), 20, 20);
  gtk_widget_set_usize(GTK_WIDGET(sl1), 20, 20);
  gtk_widget_set_usize(GTK_WIDGET(sr1), 20, 20);
  gtk_container_add(GTK_CONTAINER(shift_right), sr1);
  gtk_container_add(GTK_CONTAINER(shift_left), sl1);
  gtk_container_add(GTK_CONTAINER(shift_up), su1);
  gtk_container_add(GTK_CONTAINER(shift_down), sd1);

  GTK_WIDGET_UNSET_FLAGS(shift_right, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(shift_left, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(shift_up, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(shift_down, GTK_CAN_FOCUS);

  /* buttons */
  button_flip_x = gtk_button_new_with_label("Flip X");
  button_flip_y = gtk_button_new_with_label("Flip Y");
  button_rot_l = gtk_button_new_with_label("R90L");
  button_rot_r = gtk_button_new_with_label("R90R");
  GTK_WIDGET_UNSET_FLAGS(button_flip_x, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(button_flip_y, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(button_rot_l, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(button_rot_r, GTK_CAN_FOCUS);

  /* tooltips */
  edit_tooltips = gtk_tooltips_new();
  gtk_tooltips_set_tip(edit_tooltips, button_right, "Move right", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_left, "Move left", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_up, "Move up", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_down, "Move down", NULL);
  gtk_tooltips_set_tip(edit_tooltips, shift_right, "Shift right", NULL);
  gtk_tooltips_set_tip(edit_tooltips, shift_left, "Shift left", NULL);
  gtk_tooltips_set_tip(edit_tooltips, shift_up, "Shift up", NULL);
  gtk_tooltips_set_tip(edit_tooltips, shift_down, "Shift down", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_flip_x, "Flip X", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_flip_y, "Flip Y", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_rot_l, "Rotate 90 degrees left", NULL);
  gtk_tooltips_set_tip(edit_tooltips, button_rot_r, "Rotate 90 degrees right", NULL);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(edit_window), edit_vbox);
  gtk_box_pack_start(GTK_BOX(edit_vbox), edit_menubar, FALSE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(edit_vbox), edit_hbox);
  gtk_box_pack_start(GTK_BOX(edit_hbox), edit_draw_area, TRUE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(edit_vbox), button_hbox);
  gtk_box_pack_start(GTK_BOX(button_hbox), button_table, TRUE, FALSE, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_left, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_right, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_up, 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_down, 3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), shift_left, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), shift_right, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), shift_up, 2, 3, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), shift_down, 3, 4, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_rot_l, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_flip_x, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_flip_y, 2, 3, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_table_attach(GTK_TABLE(button_table), button_rot_r, 3, 4, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(edit_window), "destroy", GTK_SIGNAL_FUNC(destroy), NULL);
  gtk_signal_connect(GTK_OBJECT(edit_window), "key_press_event", GTK_SIGNAL_FUNC(key_press), NULL);
  gtk_signal_connect(GTK_OBJECT(edit_window), "key_release_event", GTK_SIGNAL_FUNC(key_release), NULL);
  gtk_signal_connect(GTK_OBJECT(edit_draw_area), "expose_event", GTK_SIGNAL_FUNC(draw_area_expose), NULL);
  gtk_signal_connect(GTK_OBJECT(edit_draw_area), "button_press_event", GTK_SIGNAL_FUNC(button_press), NULL);
  gtk_signal_connect(GTK_OBJECT(edit_draw_area), "button_release_event", GTK_SIGNAL_FUNC(button_release), NULL);
  gtk_signal_connect(GTK_OBJECT(edit_draw_area), "motion_notify_event", GTK_SIGNAL_FUNC(motion_notify), NULL);
  gtk_signal_connect(GTK_OBJECT(button_right), "clicked", GTK_SIGNAL_FUNC(button_press_right), NULL);
  gtk_signal_connect(GTK_OBJECT(button_left), "clicked", GTK_SIGNAL_FUNC(button_press_left), NULL);
  gtk_signal_connect(GTK_OBJECT(button_up), "clicked", GTK_SIGNAL_FUNC(button_press_up), NULL);
  gtk_signal_connect(GTK_OBJECT(button_down), "clicked", GTK_SIGNAL_FUNC(button_press_down), NULL);
  gtk_signal_connect(GTK_OBJECT(shift_right), "clicked", GTK_SIGNAL_FUNC(draw_area_shift_right), NULL);
  gtk_signal_connect(GTK_OBJECT(shift_left), "clicked", GTK_SIGNAL_FUNC(draw_area_shift_left), NULL);
  gtk_signal_connect(GTK_OBJECT(shift_up), "clicked", GTK_SIGNAL_FUNC(draw_area_shift_up), NULL);
  gtk_signal_connect(GTK_OBJECT(shift_down), "clicked", GTK_SIGNAL_FUNC(draw_area_shift_down), NULL);
  gtk_signal_connect(GTK_OBJECT(button_flip_x), "clicked", GTK_SIGNAL_FUNC(flip_x), NULL);
  gtk_signal_connect(GTK_OBJECT(button_flip_y), "clicked", GTK_SIGNAL_FUNC(flip_y), NULL);
  gtk_signal_connect(GTK_OBJECT(button_rot_l), "clicked", GTK_SIGNAL_FUNC(rotate_l), NULL);
  gtk_signal_connect(GTK_OBJECT(button_rot_r), "clicked", GTK_SIGNAL_FUNC(rotate_r), NULL);

  gtk_window_set_policy(GTK_WINDOW(edit_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


int init_memory_window(void) {

  /* memory window */
  memory_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  set_memory_window_title();

  /* memory window menu */
  memory_accel_group = gtk_accel_group_new();
  memory_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", memory_accel_group);
  gtk_item_factory_create_items(memory_item_factory, sizeof(memory_menu_items)/sizeof(memory_menu_items[0]), memory_menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(memory_window), memory_accel_group);
  memory_menubar = gtk_item_factory_get_widget(memory_item_factory, "<main>");

  /* drawing area */
  memory_draw_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(memory_draw_area), zoom_size * 256, zoom_size * 256);
  gtk_widget_add_events(memory_draw_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK);

  /* adjustment check button */
  memory_check_button = gtk_check_button_new_with_label("Adjust to tile size");
  if (memory_check_status == ON)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(memory_check_button), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(memory_check_button), FALSE);

  GTK_WIDGET_UNSET_FLAGS(memory_check_button, GTK_CAN_FOCUS);

  /* vertical boxes */
  memory_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(memory_vbox), 1);

  /* horizontal boxes */
  memory_hbox1 = gtk_hbox_new(FALSE, 0);
  memory_hbox2 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(memory_hbox1), 1);
  gtk_container_border_width(GTK_CONTAINER(memory_hbox2), 1);

  /* place boxes and widgets */
  gtk_container_add(GTK_CONTAINER(memory_window), memory_vbox);
  gtk_box_pack_start(GTK_BOX(memory_vbox), memory_menubar, FALSE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(memory_vbox), memory_hbox1);
  gtk_box_pack_start(GTK_BOX(memory_hbox1), memory_draw_area, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(memory_vbox), memory_hbox2, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(memory_hbox2), memory_check_button, TRUE, FALSE, 0);

  /* signals */
  gtk_signal_connect(GTK_OBJECT(memory_window), "destroy", GTK_SIGNAL_FUNC(destroy), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_draw_area), "expose_event", GTK_SIGNAL_FUNC(memory_draw_area_expose), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_draw_area), "button_press_event", GTK_SIGNAL_FUNC(memory_button_press), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_draw_area), "button_release_event", GTK_SIGNAL_FUNC(memory_button_release), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_draw_area), "motion_notify_event", GTK_SIGNAL_FUNC(memory_motion_notify), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_check_button), "button_press_event", GTK_SIGNAL_FUNC(memory_check_press), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_window), "key_press_event", GTK_SIGNAL_FUNC(memory_key_press), NULL);
  gtk_signal_connect(GTK_OBJECT(memory_window), "key_release_event", GTK_SIGNAL_FUNC(memory_key_release), NULL);

  memory_mode_changed();
  gtk_window_set_policy(GTK_WINDOW(memory_window), FALSE, FALSE, TRUE);

  return SUCCEEDED;
}


void destroy(GtkWidget *widget, gpointer data) {

  gtk_main_quit();
}


gint delete_window_widget(GtkWidget *widget, gpointer data) {

  if (widget == map_window) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Map"), FALSE);
    return TRUE;
  }
  if (widget == palette_window) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Palette"), FALSE);
    return TRUE;
  }
  if (widget == tile_window) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Tiled"), FALSE);
    return TRUE;
  }
  if (widget == metatile_window) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(memory_item_factory, "/Windows/Metatile"), FALSE);
    return TRUE;
  }

  gtk_widget_hide(widget);
  return TRUE;
}


void change_mode_8(GtkWidget *widget, gpointer data) {

  if (map_draw_mode == MAP_DRAW_META) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/16x16"), TRUE);
    return;
  }

  edit_mode = EDIT_MODE_8;
  change_mode_rest();
}


void change_mode_8x16(GtkWidget *widget, gpointer data) {

  if (map_draw_mode == MAP_DRAW_META) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/16x16"), TRUE);
    return;
  }

  edit_mode = EDIT_MODE_8x16;
  change_mode_rest();
}


void change_mode_16(GtkWidget *widget, gpointer data) {

  edit_mode = EDIT_MODE_16;
  change_mode_rest();
}


void change_mode_32(GtkWidget *widget, gpointer data) {

  if (map_draw_mode == MAP_DRAW_META) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/16x16"), TRUE);
    return;
  }

  edit_mode = EDIT_MODE_32;
  change_mode_rest();
}


void change_mode_rest(void) {

  if (edit_mode == EDIT_MODE_8) {
    map_adj1->step_increment = 1;
    map_adj2->step_increment = 1;
  }
  else if (edit_mode == EDIT_MODE_8x16) {
    map_adj1->step_increment = 1;
    map_adj2->step_increment = 2;    
    map_rec_y = (map_rec_y>>4)<<4;
  }
  else if (edit_mode == EDIT_MODE_16) {
    map_adj1->step_increment = 2;
    map_adj2->step_increment = 2;
    map_rec_x = (map_rec_x>>4)<<4;
    map_rec_y = (map_rec_y>>4)<<4;
  }
  else {
    map_adj1->step_increment = 4;
    map_adj2->step_increment = 4;
    map_rec_x = (map_rec_x>>5)<<5;
    map_rec_y = (map_rec_y>>5)<<5;
  }

  gtk_adjustment_changed(map_adj1);
  gtk_adjustment_changed(map_adj2);

  change_edit_draw_area_size();
  set_edit_window_title();
  set_memory_window_title();
  set_map_window_title();
  draw_and_refresh_all();
}


void change_grid(GtkWidget *widget, gpointer data) {

  if (edit_grid_loaded == ON) {
    edit_grid_loaded = OFF;
    draw_edit_area();
    return;
  }
  else {
    if (edit_grid == ON)
      edit_grid = OFF;
    else
      edit_grid = ON;
  }

  change_edit_draw_area_size();
  draw_edit_area();
  refresh_edit_draw_area();
}


void change_edit_draw_area_size(void) {

  if (edit_mode == EDIT_MODE_8x16) {
    if (edit_grid == ON)
      gtk_drawing_area_size(GTK_DRAWING_AREA(edit_draw_area), EDIT_WIDTH/2 + 1, EDIT_HEIGHT);
    else
      gtk_drawing_area_size(GTK_DRAWING_AREA(edit_draw_area), EDIT_WIDTH/2 - 1, EDIT_HEIGHT - 1);
  }
  else {
    if (edit_grid == ON)
      gtk_drawing_area_size(GTK_DRAWING_AREA(edit_draw_area), EDIT_WIDTH, EDIT_HEIGHT);
    else
      gtk_drawing_area_size(GTK_DRAWING_AREA(edit_draw_area), EDIT_WIDTH - 1, EDIT_HEIGHT - 1);
  }
}


void set_edit_window_title(void) {

  char t[256];


  sprintf(t, "PPM Tile %d ", get_current_tile_number());

  if (edit_mode == EDIT_MODE_8) {
    strcat(t, "(8x8)");
    edit_size_x = 8;
    edit_size_y = 8;
  }
  else if (edit_mode == EDIT_MODE_8x16) {
    strcat(t, "(8x16)");
    edit_size_x = 8;
    edit_size_y = 16;
  }
  else if (edit_mode == EDIT_MODE_16) {
    strcat(t, "(16x16)");
    edit_size_x = 16;
    edit_size_y = 16;
  }
  else {
    strcat(t, "(32x32)");
    edit_size_x = 32;
    edit_size_y = 32;
  }

  gtk_window_set_title(GTK_WINDOW(edit_window), t);
}


int get_current_tile_number(void) {

  if (edit_mode == EDIT_MODE_8)
    return (y / 8 * 32) + (x>>3);
  else if (edit_mode == EDIT_MODE_8x16)
    return (y / 16 * 32) + (x>>3);
  else if (edit_mode == EDIT_MODE_16)
    return (y / 16 * 16) + (x>>4);
  else
    return (y / 32 * 8) + (x>>5);
}


void set_memory_window_title(void) {

  char t[256];


  sprintf(t, "PPM Memory (%d, %d) ", x / edit_size_x, y / edit_size_y);

  if (zoom_mode == ZOOM_MODE_1) {
    strcat(t, "(1:1 PC)");
    zoom_size = 1;
  }
  else if (zoom_mode == ZOOM_MODE_2) {
    strcat(t, "(2:1 PC)");
    zoom_size = 2;
  }
  else {
    strcat(t, "(2:1 CGB)");
    zoom_size = 2;
  }

  gtk_window_set_title(GTK_WINDOW(memory_window), t);
}


void zoom_1(GtkWidget *widget, gpointer data) {

  zoom_mode = ZOOM_MODE_1;
  set_memory_window_title();
  draw_memory_area();
  memory_mode_changed();
}


void zoom_2(GtkWidget *widget, gpointer data) {

  zoom_mode = ZOOM_MODE_2;
  set_memory_window_title();
  draw_memory_area();
  memory_mode_changed();
}


void zoom_3(GtkWidget *widget, gpointer data) {

  zoom_mode = ZOOM_MODE_3;
  set_memory_window_title();
  draw_memory_area();
  memory_mode_changed();
}


void tile_window_toggle(GtkWidget *widget, gpointer data) {

  if (tile_window_status == ON) {
    tile_window_status = OFF;
    gdk_window_get_position(tile_window->window, &tile_window_x, &tile_window_y);
    gtk_widget_hide_all(tile_window);
  }
  else {
    tile_window_status = ON;
    draw_tile_area();
    gtk_widget_show_all(tile_window);
    tile_window_loaded = ON;
  }
}


void palette_window_toggle(GtkWidget *widget, gpointer data) {

  if (palette_window_status == ON) {
    palette_window_status = OFF;
    gdk_window_get_position(palette_window->window, &palette_window_x, &palette_window_y);
    gtk_widget_hide_all(palette_window);
  }
  else {
    palette_window_status = ON;
    gtk_widget_show_all(palette_window);
    palette_window_loaded = ON;
  }
}


void map_window_toggle(GtkWidget *widget, gpointer data) {

  if (map_window_status == ON) {
    map_window_status = OFF;
    gdk_window_get_position(map_window->window, &map_window_x, &map_window_y);
    gtk_widget_hide_all(map_window);
  }
  else {
    map_window_status = ON;
    draw_map_area();
    gtk_widget_show_all(map_window);
    map_window_loaded = ON;
  }
}


void metatile_window_toggle(GtkWidget *widget, gpointer data) {

  if (metatile_window_status == ON) {
    metatile_window_status = OFF;
    gdk_window_get_position(metatile_window->window, &metatile_window_x, &metatile_window_y);
    gtk_widget_hide_all(metatile_window);
  }
  else {
    metatile_window_status = ON;
    draw_meta_area();
    gtk_widget_show_all(metatile_window);
    metatile_window_loaded = ON;
  }
}


void tile_zoom_1(GtkWidget *widget, gpointer data) {

  tile_zoom_mode = ZOOM_MODE_1;
  draw_tile_area();
  gtk_drawing_area_size(GTK_DRAWING_AREA(tile_draw_area), 96, 96);
  set_tile_window_title();
}


void tile_zoom_2(GtkWidget *widget, gpointer data) {

  tile_zoom_mode = ZOOM_MODE_2;
  draw_tile_area();
  gtk_drawing_area_size(GTK_DRAWING_AREA(tile_draw_area), 96 * 2, 96 * 2);
  set_tile_window_title();
}


void tile_zoom_3(GtkWidget *widget, gpointer data) {

  tile_zoom_mode = ZOOM_MODE_3;
  draw_tile_area();
  gtk_drawing_area_size(GTK_DRAWING_AREA(tile_draw_area), 96 * 2, 96 * 2);
  set_tile_window_title();
}


void set_tile_window_title(void) {

  if (tile_zoom_mode == ZOOM_MODE_1) {
    gtk_window_set_title(GTK_WINDOW(tile_window), "PPM Tiled (1:1 PC)");
    tile_zoom_size = 1;
  }
  else if (tile_zoom_mode == ZOOM_MODE_2) {
    gtk_window_set_title(GTK_WINDOW(tile_window), "PPM Tiled (2:1 PC)");
    tile_zoom_size = 2;
  }
  else {
    gtk_window_set_title(GTK_WINDOW(tile_window), "PPM Tiled (2:1 CGB)");
    tile_zoom_size = 2;
  }
}


void set_metatile_window_title(void) {

  struct metatile_16x16 *m;
  char t[256];
  int i;


  if (metatiles > 0) {
    m = mt_first;
    for (i = 0; i < metatile - 1; i++)
      m = m->next;
    sprintf(t, "PPM Metatile %d of %d (%d, %d)", metatile, metatiles, m->x, m->y);
  }
  else
    sprintf(t, "PPM Metatile");
  gtk_window_set_title(GTK_WINDOW(metatile_window), t);
}


void set_palette_window_title(void) {

  char t[256];


  strcpy(t, "PPM Palette ");

  if (palette_mode == PALETTE_MODE_PC)
    strcat(t, "(PC)");
  else if (palette_mode == PALETTE_MODE_RUS)
    strcat(t, "(CGB Rusty)");
  else
    strcat(t, "(CGB Jedediah)");

  gtk_window_set_title(GTK_WINDOW(palette_window), t);
}


void set_map_window_title(void) {

  char t[256];


  sprintf(t, "PPM Map (%d, %d) ", (map_rec_x>>3) + map_x, (map_rec_y>>3) + map_y);

  if (map_zoom_mode == ZOOM_MODE_1) {
    strcat(t, "(1:1 PC)");
    map_zoom_size = 1;
  }
  else if (map_zoom_mode == ZOOM_MODE_2) {
    strcat(t, "(2:1 PC)");
    map_zoom_size = 2;
  }
  else {
    strcat(t, "(2:1 CGB)");
    map_zoom_size = 2;
  }

  gtk_window_set_title(GTK_WINDOW(map_window), t);
}


void memory_4kb(GtkWidget *widget, gpointer data) {

  memory_size = MEMORY_SIZE_4KB;
  draw_memory_area();
  memory_mode_changed();
}


void memory_8kb(GtkWidget *widget, gpointer data) {

  memory_size = MEMORY_SIZE_8KB;
  draw_memory_area();
  memory_mode_changed();
}


void memory_12kb(GtkWidget *widget, gpointer data) {

  memory_size = MEMORY_SIZE_12KB;
  draw_memory_area();
  memory_mode_changed();
}


void memory_16kb(GtkWidget *widget, gpointer data) {

  memory_size = MEMORY_SIZE_16KB;
  draw_memory_area();
  memory_mode_changed();
}


void memory_mode_changed(void) {

  /* adjust the end x and y values */
  export_adjust_end_spin_buttons(export_radio_button_tile_size);
  gtk_drawing_area_size(GTK_DRAWING_AREA(memory_draw_area), zoom_size<<8, memory_size * zoom_size);
}


void draw_use_meta(GtkWidget *widget, gpointer data) {

  if (metatiles == 0) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(map_item_factory, "/Draw/Memory"), TRUE);
    return;
  }

  map_draw_mode = MAP_DRAW_META;
  gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(edit_item_factory, "/Size/16x16"), TRUE);
}


void draw_use_mem(GtkWidget *widget, gpointer data) {

  map_draw_mode = MAP_DRAW_MEM;
}


void tile_source_met(GtkWidget *widget, gpointer data) {

  if (metatiles == 0) {
    gtk_check_menu_item_set_active((GtkCheckMenuItem *)gtk_item_factory_get_widget(tile_item_factory, "/Source/Memory"), TRUE);
    return;
  }

  tile_source = TILE_SOURCE_MET;

  draw_tile_area();
  refresh_tile_draw_area();
}


void tile_source_mem(GtkWidget *widget, gpointer data) {

  tile_source = TILE_SOURCE_MEM;

  draw_tile_area();
  refresh_tile_draw_area();
}
