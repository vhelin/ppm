
/* tile properties byte description
   bit  function
   0-2  palette
    3   x-flip
    4   y-flip
    7   touched */

#define SUCCEEDED 0
#define FAILED 1

#define ON 0
#define OFF 1

#define GL_RGB 0
#define GL_RGBA 1

#define MEMORY_SIZE_4KB 64
#define MEMORY_SIZE_8KB 128
#define MEMORY_SIZE_12KB 192
#define MEMORY_SIZE_16KB 256

#define TILE_SOURCE_MEM 0
#define TILE_SOURCE_MET 1

#define EDIT_HEIGHT 289
#define EDIT_WIDTH 289

#define EDIT_MODE_8 0
#define EDIT_MODE_8x16 1
#define EDIT_MODE_16 2
#define EDIT_MODE_32 3

#define ZOOM_MODE_1 0
#define ZOOM_MODE_2 1
#define ZOOM_MODE_3 2

#define MAP_EXPORT_MODE_NORMAL 0
#define MAP_EXPORT_MODE_16x16_METATILE_F 1
#define MAP_EXPORT_MODE_16x16_METATILE_D 2
#define MAP_EXPORT_MODE_16x16_METATILE_L 3

#define MAP_DRAW_MEM 0
#define MAP_DRAW_META 1

#define PALETTE_MODE_PC 0
#define PALETTE_MODE_RUS 1
#define PALETTE_MODE_JED 2

#define PX 20
#define PY 20
#define PE 2
#define PM 1
#define PBX (PE*2 + PX*4 + PM*3)
#define PBY (PE*2 + PY)

#define EXPORT_SELECTION_DATA 0
#define EXPORT_SELECTION_PALETTE 1
#define EXPORT_SELECTION_TILE_PALETTE 2

#define MAP_EXPORT_SELECTION_MAP_DATA 0
#define MAP_EXPORT_SELECTION_MAP_PROPERTY_DATA 1

/* careful here, the following values are row numbers as well */

#define FORMAT_GBDK 0
#define FORMAT_RAW 1
#define FORMAT_RGBDS 2
#define FORMAT_TASM 3
#define FORMAT_WLA 4

#define SIZE_8BIT 0
#define SIZE_16BIT 1

#define PALETTE_16BIT 0
#define PALETTE_24BIT 1

#define EXPORT_FORMAT_2BIT_PLANAR_INTERLEAVED 0
#define EXPORT_FORMAT_2BIT_PLANAR_NONINTERLEAVED 1
#define EXPORT_FORMAT_8BIT_CHUNKY 2

struct metatile_16x16 {
  int t1, t2, t3, t4, p1, p2, p3, p4;
  int x, y;
  struct metatile_16x16 *next, *prev;
};
