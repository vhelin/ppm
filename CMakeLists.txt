PROJECT(PPM C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

set(ppm_SOURCES
	button.c
	color.c
	draw.c
	edit.c
	export.c
	file.c
	import.c
	main.c
	map.c
	memory.c
	meta.c
	mouse.c
	png.c
	prefs.c
	project.c
	refresh.c	
)
# search for pkg-config
include (FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif ()

# check for libpng
pkg_check_modules (LIBPNG libpng16 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else ()
    INCLUDE_DIRECTORIES(${LIBPNG_INCLUDE_DIRS})
    LINK_DIRECTORIES (${LIBPNG_LIBRARY_DIRS})
    LINK_LIBRARIES (${LIBPNG_LIBRARIES})
endif ()

# check for gtk2
PKG_CHECK_MODULES(GTK2 REQUIRED gtk+-2.0)
IF(NOT GTK2_FOUND)
	message(FATAL_ERROR "You don't seem to have gtk+2.0 development libraries installed")
ELSE()
	INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
	LINK_DIRECTORIES(${GTK2_LIBRARY_DIRS})
	ADD_DEFINITIONS(${GTK2_CFLAGS_OTHER})
	LINK_LIBRARIES (${GTK2_LIBRARIES})
ENDIF()

ADD_EXECUTABLE (ppm
	"${ppm_SOURCES}"
)
