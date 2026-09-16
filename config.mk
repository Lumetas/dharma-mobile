# dharma version
VERSION = 1.0

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# --- feature-детект через pkg-config ---
PKG_CONFIG ?= pkg-config

# X11 — на glibc и musl обычно есть x11.pc
X11_CFLAGS := $(shell ${PKG_CONFIG} --cflags x11 2>/dev/null)
X11_LIBS   := $(shell ${PKG_CONFIG} --libs   x11 2>/dev/null)
ifeq (${X11_LIBS},)
X11_CFLAGS := -I/usr/include/X11 -I/usr/X11R6/include
X11_LIBS   := -L/usr/lib/X11 -lX11
endif

# Xinerama — опционально
XINERAMA_CFLAGS := $(shell ${PKG_CONFIG} --cflags xinerama 2>/dev/null)
XINERAMA_LIBS   := $(shell ${PKG_CONFIG} --libs   xinerama 2>/dev/null)
ifeq (${XINERAMA_LIBS},)
XINERAMA_CFLAGS :=
XINERAMA_LIBS   := -lXinerama
endif
XINERAMAFLAGS := -DXINERAMA

# freetype / Xft / fontconfig
FREETYPE_CFLAGS := $(shell ${PKG_CONFIG} --cflags xft fontconfig 2>/dev/null)
FREETYPE_LIBS   := $(shell ${PKG_CONFIG} --libs   xft fontconfig 2>/dev/null)
ifeq (${FREETYPE_LIBS},)
FREETYPE_CFLAGS := -I/usr/include/freetype2
FREETYPE_LIBS   := -lfontconfig -lXft
endif

# Xext / Xrender
XEXT_LIBS := $(shell ${PKG_CONFIG} --libs xext xrender 2>/dev/null)
ifeq (${XEXT_LIBS},)
XEXT_LIBS := -lXext -lXrender
endif

# cairo / cairo-xlib / libpng — для лаунчера
CAIRO_CFLAGS := $(shell ${PKG_CONFIG} --cflags cairo cairo-xlib libpng 2>/dev/null)
CAIRO_LIBS   := $(shell ${PKG_CONFIG} --libs   cairo cairo-xlib libpng 2>/dev/null)
ifeq (${CAIRO_LIBS},)
CAIRO_CFLAGS :=
CAIRO_LIBS   := -lcairo -lpng -lm
endif

# includes and libs
INCS = ${X11_CFLAGS} ${FREETYPE_CFLAGS}
LIBS = ${X11_LIBS} ${XINERAMA_LIBS} ${FREETYPE_LIBS} ${XEXT_LIBS}
LIBS_L = ${CAIRO_LIBS}

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC ?= cc
