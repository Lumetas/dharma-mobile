/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int border_radius = 0;
static const unsigned int gappih    = 13;
static const unsigned int monocle_bar_height = 0;
static const unsigned int bar_height = 0;
static const unsigned int line_size  = 2;       /* underline thickness for active/urgent tags */
static const unsigned int tag_gap    = 8;       /* gap between workspace tags in bar */
static const unsigned int gappiv    = 13;
static const unsigned int gappoh    = 13;
static const unsigned int gappov    = 13;
static const unsigned int gapp_top = 13;
static const int smartgaps          = 0;
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 1 means bottom bar */
static const char *fonts[]          = { "monospace:Bold:size=16", "FontAwesome:Bold:size=16" }; /* добавлен FontAwesome для иконок */
static const char dmenufont[]       = "monospace:size=14";
static const char col_dark0[]      = "#051b35";   /* Глубокий синий */
static const char col_dark1[]      = "#0e4466";   
static const char col_dark2[]      = "#1d5e72";   
static const char col_dark3[]      = "#246f8b";   
static const char col_dark4[]      = "#308fad";   
static const char col_light0[]     = "#d6b8a1";   
static const char col_light1[]     = "#bb794a";   
static const char col_light2[]     = "#a34c65";   
static const char col_light3[]     = "#644862";   
static const char col_light4[]     = "#53b0bc";   

static const char col_accent_blue[] = "#308fad";
static const char col_accent_green[] = "#53b0bc";
static const char col_accent_yellow[] = "#d6b8a1";
static const char col_accent_red[] = "#a34c65";

static const char col_text_normal[] = "#d6b8a1";
static const char col_text_selected[] = "#ffffff";
static const char col_text_dim[] = "#246f8b";

void EnvConfig() { 
	setenv("MOZ_USE_XINPUT2", "1", 1);
	setenv("TOUCHSCREEN", "generic ft5x06 (51)", 1);
	setenv("GDK_SCALE", "3", 1);
	setenv("GDK_DPI_SCALE", "1.0", 1);
	setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0", 1);
	setenv("QT_SCREEN_SCALE_FACTORS", "3", 1);
	setenv("QT_FONT_DPI", "200", 1);
	setenv("FORCE_SCALE_FACTOR", "3", 1);
	setenv("QT_QPA_PLATFORM", "xcb", 1);
}


#define WMNAME "Dharma"
#define WMNAME_LEN 6

static const char *colors[][3]      = {
	/*               fg (текст)        bg (фон)        border   */
	[SchemeNorm] = { "#aaaaaa",         col_dark0,      col_light3 },	// inactive occupied tag
	[SchemeSel]  = { "#ffffff",         "#00000000",      col_light4 }, // active tag
	[SchemeApp]  = { "#ffffff",         col_dark0,      col_light0 }, // appname
	[SchemeUrg]  = { col_accent_red,    col_dark0,      col_accent_red }, // urgent tag
};

/* Дополнительные схемы для bar (если используются в config.def.h) */
static const char col_bar_bg[]      = "#0C0B15";   /* Фон бара */
static const char col_bar_fg[]      = "#D18AFF";   /* Текст бара */
// static const char col_bar_active[]  = "#E0B0FF";   /* Активный тег */
static const char col_bar_active[]  = "#5A5775";   /* Активный тег */
static const char col_bar_inactive[]= "#5A5775";   /* Неактивный тег */
// static const char col_bar_urgent[]  = "#FF6B8B";   /* Срочный тег */
static const char col_bar_urgent[]  = "#5A5775";   /* Срочный тег */


/* Кнопка слева (на месте имени приложения). Команда + что рисовать. */
static const char *barcmd[]   = { "/bin/sh", "-c", "~/dharma-mobile/dharma-launcher > ~/log 2>&1", NULL };
/* ==== action-кнопка (третья справа/по центру) ==== */
static const char *actionbtn_label = "A ";        /* что видно на кнопке */
static const char *actionbtn_cmd[] = { "/bin/sh", "-c",
	"namaste", NULL };              /* твоя произвольная команда */

/* ==== свайп от левого края → Escape ==== */
static const int   swipe_width     = 20;    /* ширина сенсорной зоны, px (0 = выкл) */
static const int   swipe_threshold = 100;   /* минимальная длина свайпа вправо, px */

static const int   swipe_right_width     = 20;
static const int   swipe_right_threshold = 100;

static const char *top_gesture_1[] = { "/bin/sh", "-c", "~/dharma-mobile/hot-panel open", NULL};
static const char *top_gesture_2[] = { "/bin/sh", "-c", "~/dharma-mobile/hot-panel close", NULL};
static const char **top_gestures[] = {
	top_gesture_1,
	// top_gesture_2,
};


/* ==== свайп сверху → toggle fullscreen текущего окна ==== */
static const int   swipe_top_height    = 40;   /* высота зоны сверху, px (0 = выкл) */
static const int   swipe_top_threshold = 80;   /* мин. длина свайпа вниз, px */

static const char *swipe_cmd[]     = { "/bin/sh", "-c",
	"xdotool key Escape", NULL };              /* что выполнить при свайпе */
static const char *barlabel   = " M ";        /* что видно на кнопке */
static const int   tag_longpress_ms = 500;  /* длительность удержания, мс */
/* Правые кнопки */
static const char *closebtn_label = " X ";     /* крест */
static const char *kbdbtn_label   = " K";     /* клавиатура */

/* ==== on-screen keyboard ==== */
static const char *keyboardcmd[]        = { "/bin/sh", "-c", "~/svkbd/svkbd-mobile-intl -d", NULL };
static const char *keyboard_classname   = "svkbd";  /* совпадение по WM_CLASS (instance или class) */
static const float keyboard_factor      = 0.35f;    /* высота клавиатуры = 35% от m->wh */

static void updatestatus_c(void)
{
	char bat_str[8], tm_str[16];
	int cap = 0;

	/* battery capacity: сначала energy_now/energy_full, потом charge, потом capacity */
	FILE *f;
	long now = 0, full = 0;

	// f = fopen("/sys/class/power_supply/BAT0/energy_now", "r");
	// if (f && fscanf(f, "%ld", &now) == 1) {
	// 	fclose(f);
	// 	f = fopen("/sys/class/power_supply/BAT0/energy_full", "r");
	// 	if (f && fscanf(f, "%ld", &full) == 1 && full > 0)
	// 		cap = (int)((now * 100) / full);
	// 	if (f) fclose(f);
	// } else if (f) fclose(f);

	if (cap <= 0) {
		f = fopen("/sys/class/power_supply/qcom-battery/capacity", "r");
		if (f) {
			if (fscanf(f, "%d", &cap) != 1) cap = 0;
			fclose(f);
		}
	}

	if (cap < 0) cap = 0;
	if (cap > 100) cap = 100;

	snprintf(bat_str, sizeof(bat_str), "%d%%", cap);

	/* время */
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	strftime(tm_str, sizeof(tm_str), "%H:%M", lt);

	setstatus("%s %s", bat_str, tm_str);
}

/* tagging */
static const char *tags[] = { "A", "B", "C", "D" };
#define TAG(n) (1 << ((n) - 1))
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	// { NULL, "Navigator", NULL, TAG(1), 0, -1 },
};

/* layout(s) */
static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "T",      tile },    /* first entry is default */
	{ "F",      NULL },    /* no layout function means floating behavior */
	{ "M",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dharma-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

static const int restartdharma = 1; /* включить автоперезапуск */

/* Медиаклавиши */
#define XF86XK_AudioLowerVolume 0x1008ff11
#define XF86XK_AudioRaiseVolume 0x1008ff13
#define XF86XK_AudioMute        0x1008ff12
#define XF86XK_MonBrightnessUp  0x1008ff02
#define XF86XK_MonBrightnessDown 0x1008ff03
#define XF86XK_AudioPlay        0x1008ff14
#define XF86XK_AudioPrev        0x1008ff16
#define XF86XK_PowerOff			0x1008ff2a
#define XF86XK_AudioNext        0x1008ff17
#define XF86XK_KbLightUp		0x1008ff05
#define XF86XK_KbLightDown		0x1008ff06
#define DOUBLE_TAP_MS 400

/* Команды */
static const char *power_single_cmd[]  = { "sh", "-c", "~/dharma-mobile/power_menu", NULL };
static const char *power_double_cmd[]  = { "sh", "-c", "~/dharma-mobile/screen", NULL };

static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */

/* Команда для увеличения яркости клавиатуры с ограничением до 255 */
static const char *kbd_backlight_up[] = { 
    "/bin/bash", "-c", 
    "brightness=$(cat /sys/class/leds/smc::kbd_backlight/brightness); "
    "new_brightness=$((brightness + 50)); "
    "if [ $new_brightness -gt 255 ]; then new_brightness=255; fi; "
    "echo $new_brightness > /sys/class/leds/smc::kbd_backlight/brightness", 
    NULL 
};

/* Команда для уменьшения яркости клавиатуры с ограничением до 0 */
static const char *kbd_backlight_down[] = { 
    "/bin/bash", "-c", 
    "brightness=$(cat /sys/class/leds/smc::kbd_backlight/brightness); "
    "new_brightness=$((brightness - 50)); "
    "if [ $new_brightness -lt 0 ]; then new_brightness=0; fi; "
    "echo $new_brightness > /sys/class/leds/smc::kbd_backlight/brightness", 
    NULL 
};

static const char *roficmd[] = { "dmenu_apps", NULL };
static const char *termcmd[]  = { "namaste", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = roficmd } },
	// { MODKEY|ShiftMask,             XK_d,      spawn,          {.v = kando } },
	{ MODKEY|ControlMask|ShiftMask, XK_r,      quit,           {1} }, /* перезапуск dharma с сохранением окон */
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_u,      spawn,          {.v = (const char*[]){"/bin/sh", "-c", "~/dharma/aliases BROWSER", NULL}}},
	{ MODKEY,                       XK_p,      spawn,          {.v = (const char*[]){"/bin/sh", "-c", "flameshot gui", NULL}}},
	{ MODKEY,                       XK_s,      spawn,          {.v = (const char*[]){"/bin/sh", "-c", "~/dharma/screen -a", NULL}}},
	{ MODKEY,                       XK_m,      spawn,          {.v = (const char*[]){"/bin/sh", "-c", "~/dharma/lock", NULL}}},
	{ MODKEY,                       XK_w,      spawn,          {.v = (const char*[]){"/bin/sh", "-c", "dmenu_windows", NULL}}},
	{ MODKEY,                       XK_n,      spawn,          {.v = (const char*[]){"/bin/sh", "-c", "nvide", NULL}}},
	{ MODKEY|ShiftMask,				XK_i,	   spawn,		   {.v = (const char*[]){"/bin/sh", "-c", "[ -n \"$(warpd --hint2 --oneshot)\" ] && warpd --click 1", NULL}}},
	{ MODKEY|ControlMask|ShiftMask,				XK_i,	   spawn,		   {.v = (const char*[]){"/bin/sh", "-c", "[ -n \"$(warpd --hint2 --oneshot)\" ] && warpd --click 3", NULL}}},

	{ MODKEY|ShiftMask,				XK_o,	   spawn,		   {.v = (const char*[]){"/bin/sh", "-c", "[ -n \"$(warpd --hint --oneshot)\" ] && warpd --click 1", NULL}}},
	{ MODKEY|ControlMask|ShiftMask,				XK_o,	   spawn,		   {.v = (const char*[]){"/bin/sh", "-c", "[ -n \"$(warpd --hint --oneshot)\" ] && warpd --click 3", NULL}}},

	
	/* Медиаклавиши */
	{ 0,                            XF86XK_AudioPlay,        spawn, {.v = (const char*[]){"playerctl", "play-pause", NULL}} },
	{ 0,                            XF86XK_AudioPrev,        spawn, {.v = (const char*[]){"playerctl", "previous", NULL}} },
	{ 0,                            XF86XK_AudioNext,        spawn, {.v = (const char*[]){"playerctl", "next", NULL}} },



	// { 0,                            XF86XK_MonBrightnessUp,  spawn, {.v = (const char*[]){"brightnessctl", "set", "+5%", NULL}} },
	// { 0,                            XF86XK_MonBrightnessDown,spawn, {.v = (const char*[]){"brightnessctl", "set", "5%-", NULL}} },
	// { 0,                            XF86XK_AudioLowerVolume, spawn, {.v = (const char*[]){"pactl", "set-sink-volume", "@DEFAULT_SINK@", "-5%", NULL}} },
	// { 0,                            XF86XK_AudioRaiseVolume, spawn, {.v = (const char*[]){"pactl", "set-sink-volume", "@DEFAULT_SINK@", "+5%", NULL}} },
	// { 0,                            XF86XK_AudioMute,        spawn, {.v = (const char*[]){"pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL}} },
	//    { 0, XF86XK_KbLightUp,   spawn, {.v = kbd_backlight_up} },
	//    { 0, XF86XK_KbLightDown, spawn, {.v = kbd_backlight_down} },
	// { 0, XF86XK_PowerOff, spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/power_menu", NULL}} },
	{ 0, XF86XK_PowerOff, spawn_power_action, {0} },
	{ 0, XF86XK_AudioLowerVolume, spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls volume -", NULL}} },
	{ 0, XF86XK_AudioRaiseVolume, spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls volume +", NULL}} },
	{ 0, XF86XK_AudioMute,        spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls volume mute", NULL}} },
	{ 0, XF86XK_MonBrightnessUp,   spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls screen +", NULL}} },
	{ 0, XF86XK_MonBrightnessDown, spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls screen -", NULL}} },
	{ 0, XF86XK_KbLightUp,         spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls kbd +", NULL}} },
	{ 0, XF86XK_KbLightDown,       spawn, {.v = (const char*[]){"sh", "-c", "~/dharma-mobile/keyboard_controls kbd -", NULL}} },
	
	
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_o,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_y,      zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	// { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[2]} },
    // { MODKEY,                       XK_f,      fullscreen,     {0} },
	// { MODKEY|ShiftMask,             XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefloating,      {0} },
	// { MODKEY,                       XK_space,  togglefloating, {0} },
	// { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	// { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	TAGKEYS(                        XK_0,                      9)
	{ MODKEY|ShiftMask,             XK_c,      quit,           {0} },
	{ MODKEY|ShiftMask,				XK_r,      quit,           {1} }, 
    // gaps
    { MODKEY|ShiftMask,             XK_g,      togglegaps,     {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	// { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkAppBtn,            0,              Button1,        spawn,          {.v = barcmd } },
	{ ClkCloseBtn,          0,              Button1,        killclient,     {0} },
	{ ClkKbdBtn,            0,              Button1,        togglekeyboard, {0} },
	{ ClkActionBtn,         0,              Button1,        spawn,          {.v = actionbtn_cmd} },
};

