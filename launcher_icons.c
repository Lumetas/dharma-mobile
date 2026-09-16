#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cairo/cairo.h>

#include "launcher_icons.h"

/* --- утилиты --- */

static char *
trim(char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    char *e = s + strlen(s);
    while (e > s && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' ' || e[-1] == '\t'))
        *--e = '\0';
    return s;
}

static void
sanitize_exec(char *exec)
{
    char *p;
    while ((p = strstr(exec, " %"))) {
        char c = p[2];
        if (strchr("UuFfiIck", c)) {
            memmove(p, p + 3, strlen(p + 3) + 1);
        } else {
            memmove(p, p + 1, strlen(p + 1) + 1);
            p++;
        }
    }
}

/* --- парсинг .desktop --- */

static int
parse_desktop(const char *path, LauncherApp *app)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    memset(app, 0, sizeof(*app));

    char line[2048];
    int in_entry = 0;
    int ok = 0;
    int first = 1;

    while (fgets(line, sizeof(line), f)) {
        char *p = line;

        if (first) {
            first = 0;
            if ((unsigned char)p[0] == 0xEF &&
                (unsigned char)p[1] == 0xBB &&
                (unsigned char)p[2] == 0xBF)
                p += 3;
        }

        if (p[0] == '[') {
            in_entry = (strncmp(p, "[Desktop Entry]", 15) == 0);
            continue;
        }
        if (!in_entry) continue;

        if (strncmp(p, "Name=", 5) == 0) {
            snprintf(app->name, sizeof(app->name), "%s", trim(p + 5));
        } else if (strncmp(p, "Exec=", 5) == 0) {
            snprintf(app->exec, sizeof(app->exec), "%s", trim(p + 5));
            sanitize_exec(app->exec);
        } else if (strncmp(p, "Icon=", 5) == 0) {
            snprintf(app->icon, sizeof(app->icon), "%s", trim(p + 5));
        } else if (strncmp(p, "Comment=", 8) == 0) {
            snprintf(app->comment, sizeof(app->comment), "%s", trim(p + 8));
        } else if (strncmp(p, "NoDisplay=true", 14) == 0) {
            app->nodisplay = 1;
        } else if (strncmp(p, "Type=Application", 16) == 0) {
            ok = 1;
        }
    }
    fclose(f);

    if (!ok || !app->name[0] || !app->exec[0] || app->nodisplay)
        return 0;
    return 1;
}

static int
cmp_apps(const void *a, const void *b)
{
    const LauncherApp *x = a, *y = b;
    return strcasecmp(x->name, y->name);
}

LauncherApp *
launcher_load_apps(int *out_n)
{
    const char *home = getenv("HOME");

    const char *dirs[8];
    int ndirs = 0;
    dirs[ndirs++] = "/usr/share/applications";
    dirs[ndirs++] = "/usr/local/share/applications";
    static char userdir[512];
    if (home) {
        snprintf(userdir, sizeof(userdir), "%s/.local/share/applications", home);
        dirs[ndirs++] = userdir;
    }

    int cap = 128, n = 0;
    LauncherApp *apps = calloc(cap, sizeof(*apps));
    if (!apps) return NULL;

    for (int d = 0; d < ndirs; d++) {
        fprintf(stderr, "launcher: scanning '%s'\n", dirs[d]);
        DIR *dp = opendir(dirs[d]);
        if (!dp) {
            fprintf(stderr, "launcher:   opendir failed: %s\n", strerror(errno));
            continue;
        }

        int found = 0, parsed = 0, dup = 0;
        struct dirent *de;
        while ((de = readdir(dp))) {
            if (de->d_name[0] == '.') continue;
            size_t len = strlen(de->d_name);
            if (len < 8 || strcmp(de->d_name + len - 8, ".desktop") != 0) continue;
            found++;

            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirs[d], de->d_name);

            LauncherApp app;
            if (!parse_desktop(path, &app)) {
                fprintf(stderr, "launcher:   skip %s\n", de->d_name);
                continue;
            }
            parsed++;

            int is_dup = 0;
            for (int i = 0; i < n; i++) {
                if (strcmp(apps[i].name, app.name) == 0) { is_dup = 1; break; }
            }
            if (is_dup) { dup++; continue; }

            if (n == cap) {
                cap *= 2;
                LauncherApp *tmp = realloc(apps, cap * sizeof(*apps));
                if (!tmp) { free(apps); return NULL; }
                apps = tmp;
            }
            apps[n++] = app;
        }
        closedir(dp);
        fprintf(stderr, "launcher:   found=%d parsed=%d dup=%d\n", found, parsed, dup);
    }

    qsort(apps, n, sizeof(*apps), cmp_apps);
    fprintf(stderr, "launcher: total apps=%d\n", n);
    *out_n = n;
    return apps;
}

/* --- поиск иконок --- */

static char *
find_in_dir(const char *dir, const char *name, int size)
{
    DIR *dp = opendir(dir);
    if (!dp) return NULL;

    char want[256];
    snprintf(want, sizeof(want), "%dx%d", size, size);

    char *best = NULL;
    int best_score = -1;

    struct dirent *de;
    while ((de = readdir(dp))) {
        if (de->d_type == DT_DIR && de->d_name[0] != '.') {
            char sub[1024];
            snprintf(sub, sizeof(sub), "%s/%s", dir, de->d_name);
            char *r = find_in_dir(sub, name, size);
            if (r) {
                int score = strstr(sub, want) ? 1000 : 1;
                if (score > best_score) {
                    free(best);
                    best = r;
                    best_score = score;
                } else {
                    free(r);
                }
            }
            continue;
        }
        char base[256];
        const char *ext = strrchr(de->d_name, '.');
        if (!ext) continue;
        size_t blen = ext - de->d_name;
        if (blen >= sizeof(base)) continue;
        memcpy(base, de->d_name, blen);
        base[blen] = '\0';
        if (strcmp(base, name) != 0) continue;

        int score = 1;
        if (strstr(dir, want)) score = 1000;
        else if (strstr(dir, "48x48")) score = 500;
        else if (strstr(dir, "64x64")) score = 600;
        else if (strstr(dir, "128x128")) score = 700;
        else if (strstr(dir, "scalable")) score = 800;

        if (strcmp(ext, ".png") == 0) score += 10;
        if (strcmp(ext, ".svg") == 0) score += 5;

        if (score > best_score) {
            free(best);
            char full[1024];
            snprintf(full, sizeof(full), "%s/%s", dir, de->d_name);
            best = strdup(full);
            best_score = score;
        }
    }
    closedir(dp);
    return best;
}

char *
launcher_find_icon(const char *icon_name, int size)
{
    if (!icon_name || !icon_name[0]) return NULL;

    if (icon_name[0] == '/') {
        struct stat st;
        if (stat(icon_name, &st) == 0 && S_ISREG(st.st_mode))
            return strdup(icon_name);
        return NULL;
    }

    const char *home = getenv("HOME");
    const char *xdg_data = getenv("XDG_DATA_DIRS");
    if (!xdg_data || !*xdg_data) xdg_data = "/usr/share:/usr/local/share";

    const char *themes[] = {
        "hicolor", "Adwaita", "Papirus", "Papirus-Dark",
        "Arc", "Numix", "gnome", "breeze", NULL
    };

    char *result = NULL;

    for (int t = 0; themes[t] && !result; t++) {
        char base[512];
        if (home) {
            snprintf(base, sizeof(base), "%s/.local/share/icons/%s", home, themes[t]);
            result = find_in_dir(base, icon_name, size);
            if (result) break;
        }
        char *copy = strdup(xdg_data);
        char *save = NULL;
        for (char *p = strtok_r(copy, ":", &save); p && !result; p = strtok_r(NULL, ":", &save)) {
            snprintf(base, sizeof(base), "%s/icons/%s", p, themes[t]);
            result = find_in_dir(base, icon_name, size);
        }
        free(copy);
    }

    return result;
}

/* --- загрузка PNG в cairo --- */

cairo_surface_t *
launcher_load_icon_surface(const char *path, int size)
{
    if (!path) return NULL;

    /* SVG пропускаем — нужна librsvg, не тянем */
    if (strstr(path, ".svg")) return NULL;

    cairo_surface_t *img = cairo_image_surface_create_from_png(path);
    if (cairo_surface_status(img) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(img);
        return NULL;
    }

    int w = cairo_image_surface_get_width(img);
    int h = cairo_image_surface_get_height(img);
    if (w <= 0 || h <= 0) {
        cairo_surface_destroy(img);
        return NULL;
    }

    cairo_surface_t *scaled = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
    cairo_t *cr = cairo_create(scaled);

    /* фон прозрачный */
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_scale(cr, (double)size / w, (double)size / h);
    cairo_set_source_surface(cr, img, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(img);
    return scaled;
}
