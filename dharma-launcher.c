#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include "launcher_icons.h"

/* --- базовые размеры (при UI_SCALE = 1.0) --- */
#define BASE_PAD            24
#define BASE_TILE_GAP       20
#define BASE_ICON_SIZE      96
#define BASE_LABEL_H        48
#define BASE_TOP_BAR_H      60

#define BASE_FONT_LABEL     16
#define BASE_FONT_FALLBACK  48
#define BASE_FONT_EMPTY     20

#define BASE_KNOB_W         80
#define BASE_KNOB_H         6
#define BASE_KNOB_Y         16

#define BASE_RADIUS_TILE    16
#define BASE_RADIUS_KNOB    3

#define FONT_FAMILY         "sans"

/* скорость анимации */
#define ANIM_SPEED          0.35
#define SCROLL_VEL_DECAY    0.92

/* цвета */
#define COL_BG_R 0.118
#define COL_BG_G 0.118
#define COL_BG_B 0.180
#define COL_FG_R 0.804
#define COL_FG_G 0.839
#define COL_FG_B 0.957
#define COL_SEL_R 0.192
#define COL_SEL_G 0.196
#define COL_SEL_B 0.267

/* --- масштаб (заполняется в main) --- */
static double ui_scale = 0.1;

static int   s_pad;
static int   s_tile_gap;
static int   s_icon_size;
static int   s_label_h;
static int   s_top_bar_h;
static int   s_font_label;
static int   s_font_fallback;
static int   s_font_empty;
static int   s_knob_w;
static int   s_knob_h;
static int   s_knob_y;
static int   s_radius_tile;
static int   s_radius_knob;

static void
apply_scale(void)
{
    s_pad          = (int)(BASE_PAD        * ui_scale);
    s_tile_gap     = (int)(BASE_TILE_GAP   * ui_scale);
    s_icon_size    = (int)(BASE_ICON_SIZE  * ui_scale);
    s_label_h      = (int)(BASE_LABEL_H    * ui_scale);
    s_top_bar_h    = (int)(BASE_TOP_BAR_H  * ui_scale);
    s_font_label   = (int)(BASE_FONT_LABEL * ui_scale);
    s_font_fallback= (int)(BASE_FONT_FALLBACK * ui_scale);
    s_font_empty   = (int)(BASE_FONT_EMPTY * ui_scale);
    s_knob_w       = (int)(BASE_KNOB_W     * ui_scale);
    s_knob_h       = (int)(BASE_KNOB_H     * ui_scale);
    s_knob_y       = (int)(BASE_KNOB_Y     * ui_scale);
    s_radius_tile  = (int)(BASE_RADIUS_TILE* ui_scale);
    s_radius_knob  = (int)(BASE_RADIUS_KNOB* ui_scale);
    if (s_radius_knob < 1) s_radius_knob = 1;
    if (s_radius_tile < 1) s_radius_tile = 1;
}

/* --- состояние --- */
typedef struct {
    LauncherApp app;
    cairo_surface_t *icon;
    int x, y, w, h;
} Tile;

static Display *dpy;
static int screen;
static Window root, win;
static int sw, sh;
static Visual *visual;
static Colormap cmap;
static int depth;
static cairo_surface_t *surface;
static cairo_t *cr;

static Tile *tiles = NULL;
static int ntiles = 0;
static int cols = 4;

static double scroll_y = 0;
static double scroll_target = 0;
static double scroll_vel = 0;
static double content_h = 0;

static int dragging = 0;
static int drag_start_y = 0;
static double drag_start_scroll = 0;
static int drag_moved = 0;

static int selected = -1;

static int closing = 0;
static double close_anim = 0;

/* --- утилиты --- */

static void
die(const char *msg)
{
    fprintf(stderr, "dharma-launcher: %s\n", msg);
    exit(1);
}

static void
spawn_cmd(const char *exec)
{
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", exec, (char *)NULL);
        _exit(1);
    }
}

/* --- DPI из Xresources --- */

static double
get_xft_dpi(void)
{
    char *rms = XResourceManagerString(dpy);
    if (!rms) return 96.0;

    XrmInitialize();
    XrmDatabase db = XrmGetStringDatabase(rms);
    if (!db) return 96.0;

    char *type = NULL;
    XrmValue val;
    double dpi = 96.0;

    if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &val)) {
        if (type && strcmp(type, "String") == 0 && val.addr)
            dpi = atof(val.addr);
    }
    XrmDestroyDatabase(db);
    return dpi > 0 ? dpi : 96.0;
}

/* --- рендер --- */

static void
draw_rounded_rect(cairo_t *c, double x, double y, double w, double h, double r)
{
    if (r < 0.5) r = 0.5;
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    cairo_new_sub_path(c);
    cairo_arc(c, x + r,     y + r,     r, M_PI, 1.5 * M_PI);
    cairo_arc(c, x + w - r, y + r,     r, 1.5 * M_PI, 2 * M_PI);
    cairo_arc(c, x + w - r, y + h - r, r, 0, 0.5 * M_PI);
    cairo_arc(c, x + r,     y + h - r, r, 0.5 * M_PI, M_PI);
    cairo_close_path(c);
}

static void
render(void)
{
    if (!cr) return;

    /* фон */
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, COL_BG_R, COL_BG_G, COL_BG_B);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    /* ручка сверху */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.12);
    draw_rounded_rect(cr,
        sw / 2.0 - s_knob_w / 2.0, s_knob_y,
        s_knob_w, s_knob_h, s_radius_knob);
    cairo_fill(cr);

    cairo_save(cr);
    cairo_translate(cr, 0, -scroll_y);

    for (int i = 0; i < ntiles; i++) {
        Tile *t = &tiles[i];

        if (t->y + t->h < scroll_y - 50) continue;
        if (t->y > scroll_y + sh + 50) continue;

        int x = t->x;
        int y = t->y;

        if (i == selected) {
            cairo_set_source_rgb(cr, COL_SEL_R, COL_SEL_G, COL_SEL_B);
            draw_rounded_rect(cr, x, y, t->w, t->h, s_radius_tile);
            cairo_fill(cr);
        }

        if (t->icon) {
            int ix = x + (t->w - s_icon_size) / 2;
            int iy = y + (int)(16 * ui_scale);
            cairo_set_source_surface(cr, t->icon, ix, iy);
            cairo_paint(cr);
        } else {
            int ix = x + (t->w - s_icon_size) / 2;
            int iy = y + (int)(16 * ui_scale);

            cairo_set_source_rgb(cr, 0.4, 0.45, 0.65);
            draw_rounded_rect(cr, ix, iy, s_icon_size, s_icon_size, s_radius_tile);
            cairo_fill(cr);

            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_select_font_face(cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, s_font_fallback);
            char letter[2] = { t->app.name[0], 0 };
            cairo_text_extents_t ext;
            cairo_text_extents(cr, letter, &ext);
            cairo_move_to(cr,
                ix + (s_icon_size - ext.width) / 2 - ext.x_bearing,
                iy + (s_icon_size - ext.height) / 2 - ext.y_bearing);
            cairo_show_text(cr, letter);
        }

        /* подпись */
        cairo_set_source_rgb(cr, COL_FG_R, COL_FG_G, COL_FG_B);
        cairo_select_font_face(cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, s_font_label);

        char label[128];
        snprintf(label, sizeof(label), "%s", t->app.name);

        cairo_text_extents_t ext;
        cairo_text_extents(cr, label, &ext);
        while (ext.width > t->w - 12 && strlen(label) > 4) {
            label[strlen(label) - 4] = '\0';
            strcat(label, "...");
            cairo_text_extents(cr, label, &ext);
        }

        cairo_move_to(cr,
            x + (t->w - ext.width) / 2 - ext.x_bearing,
            y + (int)(16 * ui_scale) + s_icon_size + (int)(12 * ui_scale) - ext.y_bearing);
        cairo_show_text(cr, label);
    }

    cairo_restore(cr);

    if (ntiles == 0) {
        cairo_set_source_rgb(cr, 0.6, 0.6, 0.7);
        cairo_select_font_face(cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, s_font_empty);
        const char *msg = "No applications found";
        cairo_text_extents_t ext;
        cairo_text_extents(cr, msg, &ext);
        cairo_move_to(cr, (sw - ext.width) / 2, sh / 2);
        cairo_show_text(cr, msg);
    }

    if (closing && close_anim > 0) {
        cairo_set_source_rgba(cr, 0, 0, 0, 1.0 - close_anim);
        cairo_paint(cr);
    }

    cairo_surface_flush(surface);
    XFlush(dpy);
}

/* --- layout --- */

static void
layout_tiles(void)
{
    int min_tile_w = (int)(160 * ui_scale);
    cols = sw / min_tile_w;
    if (cols < 3) cols = 3;
    if (cols > 8) cols = 8;

    int tile_w = (sw - 2 * s_pad - (cols - 1) * s_tile_gap) / cols;
    int tile_h = s_icon_size + s_label_h + (int)(32 * ui_scale);

    for (int i = 0; i < ntiles; i++) {
        int row = i / cols;
        int col = i % cols;
        tiles[i].x = s_pad + col * (tile_w + s_tile_gap);
        tiles[i].y = s_top_bar_h + row * (tile_h + s_tile_gap);
        tiles[i].w = tile_w;
        tiles[i].h = tile_h;
    }

    int rows = (ntiles + cols - 1) / cols;
    content_h = s_top_bar_h + rows * (tile_h + s_tile_gap) + s_pad;
}

/* --- загрузка --- */

static void
load_tiles(void)
{
    int n = 0;
    LauncherApp *apps = launcher_load_apps(&n);
    fprintf(stderr, "dharma-launcher: loaded %d apps\n", n);

    if (!apps || n == 0) {
        if (apps) free(apps);
        return;
    }

    tiles = calloc(n, sizeof(Tile));
    ntiles = n;

    int icons_ok = 0;
    for (int i = 0; i < n; i++) {
        tiles[i].app = apps[i];
        char *icon_path = launcher_find_icon(apps[i].icon, s_icon_size);
        if (icon_path) {
            tiles[i].icon = launcher_load_icon_surface(icon_path, s_icon_size);
            if (tiles[i].icon) icons_ok++;
            free(icon_path);
        }
    }
    fprintf(stderr, "dharma-launcher: %d/%d icons loaded\n", icons_ok, n);

    free(apps);
    layout_tiles();
}

/* --- события --- */

static void
handle_button_press(XButtonEvent *e)
{
    if (e->button != Button1) return;

    drag_start_y = e->y;
    drag_start_scroll = scroll_target;
    dragging = 1;
    drag_moved = 0;
    scroll_vel = 0;

    if (e->y < s_top_bar_h && scroll_y < 5) {
        dragging = 2;
    }
}

static void
handle_motion(XMotionEvent *e)
{
    if (!dragging) return;

    if (dragging == 2) {
        int dy = e->y - drag_start_y;
        if (dy > 50) {
            closing = 1;
            close_anim = 1.0 - (dy - 50) / 200.0;
            if (close_anim < 0) close_anim = 0;
        }
        render();
        return;
    }

    int dy = e->y - drag_start_y;
    if (abs(dy) > 8) drag_moved = 1;
    scroll_target = drag_start_scroll - dy;
    scroll_y = scroll_target;
    render();
}

static void
handle_button_release(XButtonEvent *e)
{
    if (e->button != Button1) return;

    if (dragging == 2) {
        int dy = e->y - drag_start_y;
        if (dy > 80) {
            exit(0);
        }
        dragging = 0;
        closing = 0;
        render();
        return;
    }

    if (!drag_moved) {
        int tx = e->x;
        int ty = e->y + (int)scroll_y;
        for (int i = 0; i < ntiles; i++) {
            Tile *t = &tiles[i];
            if (tx >= t->x && tx < t->x + t->w &&
                ty >= t->y && ty < t->y + t->h) {
                spawn_cmd(t->app.exec);
                exit(0);
            }
        }
        exit(0);
    } else {
        scroll_vel = -(e->y - drag_start_y) * 0.15;
    }

    dragging = 0;
}

static void
handle_key(XKeyEvent *e)
{
    KeySym ks = XLookupKeysym(e, 0);
    if (ks == XK_Escape || ks == XK_BackSpace || ks == XK_q) {
        exit(0);
    }
}

/* --- анимация --- */

static void
tick_animation(void)
{
    int need_render = 0;

    if (fabs(scroll_vel) > 0.5) {
        scroll_target += scroll_vel;
        scroll_vel *= SCROLL_VEL_DECAY;
        need_render = 1;
    } else {
        scroll_vel = 0;
    }

    if (fabs(scroll_target - scroll_y) > 0.5) {
        scroll_y += (scroll_target - scroll_y) * ANIM_SPEED;
        need_render = 1;
    } else {
        scroll_y = scroll_target;
    }

    double max_scroll = content_h - sh;
    if (max_scroll < 0) max_scroll = 0;
    if (scroll_target < 0) { scroll_target = 0; scroll_vel = 0; }
    if (scroll_target > max_scroll) { scroll_target = max_scroll; scroll_vel = 0; }

    if (need_render) render();
}

/* --- main --- */

int
main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    signal(SIGCHLD, SIG_IGN);

    dpy = XOpenDisplay(NULL);
    if (!dpy) die("cannot open display");

    /* масштаб из Xft.dpi (по умолчанию 96 = 1.0) */
    double dpi = get_xft_dpi();
    ui_scale = dpi / 96.0;
    if (ui_scale < 0.5) ui_scale = 0.5;
    if (ui_scale > 4.0) ui_scale = 4.0;
    apply_scale();
    fprintf(stderr, "dharma-launcher: Xft.dpi=%.1f scale=%.2f\n", dpi, ui_scale);

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    visual = DefaultVisual(dpy, screen);
    cmap = DefaultColormap(dpy, screen);
    depth = DefaultDepth(dpy, screen);

    XSetWindowAttributes wa = {
        .override_redirect = True,
        .background_pixel = 0x1e1e2e,
        .border_pixel = 0,
        .colormap = cmap,
        .event_mask = ButtonPressMask | ButtonReleaseMask |
                      PointerMotionMask | ExposureMask | KeyPressMask |
                      StructureNotifyMask,
    };
    win = XCreateWindow(dpy, root, 0, 0, sw, sh, 0,
        depth, InputOutput, visual,
        CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
        &wa);

    XStoreName(dpy, win, "dharma-launcher");

    surface = cairo_xlib_surface_create(dpy, win, visual, sw, sh);
    cairo_status_t st = cairo_surface_status(surface);
    if (st != CAIRO_STATUS_SUCCESS)
        die(cairo_status_to_string(st));

    cr = cairo_create(surface);
    st = cairo_status(cr);
    if (st != CAIRO_STATUS_SUCCESS)
        die(cairo_status_to_string(st));

    load_tiles();

    XMapRaised(dpy, win);
    XSetInputFocus(dpy, win, RevertToPointerRoot, CurrentTime);
    XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

    render();

    int xfd = ConnectionNumber(dpy);
    while (1) {
        while (XPending(dpy)) {
            XEvent ev;
            XNextEvent(dpy, &ev);
            switch (ev.type) {
            case ButtonPress:   handle_button_press(&ev.xbutton); break;
            case ButtonRelease: handle_button_release(&ev.xbutton); break;
            case MotionNotify:  handle_motion(&ev.xmotion); break;
            case KeyPress:      handle_key(&ev.xkey); break;
            case Expose:
                if (ev.xexpose.count == 0) render();
                break;
            case ConfigureNotify:
                if (ev.xconfigure.width != sw || ev.xconfigure.height != sh) {
                    sw = ev.xconfigure.width;
                    sh = ev.xconfigure.height;
                    cairo_xlib_surface_set_size(surface, sw, sh);
                    layout_tiles();
                    render();
                }
                break;
            }
        }
        tick_animation();

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(xfd, &fds);
        struct timeval tv = { .tv_sec = 0, .tv_usec = 16000 };
        select(xfd + 1, &fds, NULL, NULL, &tv);
    }

    return 0;
}
