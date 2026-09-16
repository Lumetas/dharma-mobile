#ifndef LAUNCHER_ICONS_H
#define LAUNCHER_ICONS_H

#include <cairo/cairo.h>

typedef struct {
    char name[128];
    char exec[512];
    char icon[256];
    char comment[256];
    int  nodisplay;
} LauncherApp;

LauncherApp *launcher_load_apps(int *out_n);
char *launcher_find_icon(const char *icon_name, int size);
cairo_surface_t *launcher_load_icon_surface(const char *path, int size);

#endif
