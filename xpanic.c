/*
 *    xpanic:
 *            A simple X program to popup every now and
 *            again and relax you...
 *
 *   compile with:
 *          cc -O xpanic.c -lX11 -o xpanic
 *
 *   NOTE: you need an ansi C compiler. try gcc if cc fails.
 *
 * run with "-h" to see most options. entries in your ~/.Xdefaults
 * file of the form "xpanic.scale: 2.0" are also handled.
 *
 * Version 1.10
 *
 * 9-93,10-93   (c) Robin Humble  <rjh@pixel.maths.monash.edu.au>
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <math.h>	/* for random */
#ifndef usleep
#  ifdef SVR3		/* for usleep */
#    include <stropts.h>
#    include <poll.h>
#  else
#    include <sys/types.h>
#  endif
#endif
#include <sys/time.h>	/* for gettimeofday, usleep */
#include <stdlib.h>	/* for rand, atof, atoi, getenv */
#include <unistd.h>	/* for sleep, access */
#include <string.h>	/* for strncpy, strrchr, strcmp */


#define MAX_COLOURS 16
#define MIN(x, y) ((x < y)?(x):(y))
#define FG_POLY -1
#define BG_POLY -2
#define END_DATA -666


Display *display;
int screen_num;
Window win;
GC gc;
XColor col[MAX_COLOURS];
unsigned int width, height;
int screen_width, screen_height, bw;
unsigned long black, white;
XrmDatabase db;
char *prog;
XColor fg_colour, bg_colour;


/* prototypes */
void syntax(void);
double tim(void);
void get_GC(void);
void scale_item(XPoint *, float, int, int);
void scale_data(float, int, int);
char *grab_str(XrmValue);
void get_opts(int *, int *, float *, int *, int *, float *, int *, int *, int *, int *, float *);
void move_window(void);
void set_colours(Colormap, int *);
void show_window(int);
void draw_points(XPoint *);
void cycle_boo_colours(Colormap, int);
void draw_boo(int, int);
void cycle_chars_colours(Colormap, int, int);
void draw_chars(int);
int main(int, char **);



#define DATA_WIDTH  350
#define DATA_HEIGHT 250

/* define all the letters and symbols */
XPoint chars[] = {
  /* D */	{FG_POLY, 8}, {35, 20}, {75, 18}, {90, 30}, {87, 100}, {75, 116}, {44, 125}, {48, 36}, {34, 35},
  /* D hole */	{BG_POLY, 4}, {63, 37}, {72, 39}, {70, 92}, {60, 95}, 
  /* O */	{FG_POLY, 8}, {94, 30}, {105, 20}, {140, 20}, {150, 26}, {146, 105}, {134, 118}, {108, 122}, {95, 107}, 
  /* O hole */	{BG_POLY, 4}, {115, 40}, {125, 37}, {124, 93}, {113, 89}, 
  /* N */	{FG_POLY, 10}, {158, 12}, {170, 12}, {197, 67}, {200, 18}, {222, 20}, {222, 117}, {204, 117}, {177, 63}, {174, 118}, {150, 120}, 
  /* comma */	{FG_POLY, 6}, {227, 11}, {247, 11}, {242, 35}, {234, 42}, {233, 28}, {227, 28}, 
  /* T */	{FG_POLY, 8}, {256, 26}, {338, 18}, {332, 40}, {305, 39}, {320, 105}, {280, 111}, {287, 38}, {252, 37}, 
  /* P */	{FG_POLY, 10}, {5, 133}, {70, 130}, {78, 143}, {75, 165}, {64, 179}, {35, 186}, {30, 231}, {10, 234}, {21, 149}, {2, 150}, 
  /* P hole */	{BG_POLY, 6}, {42, 143}, {50, 141}, {52, 147}, {50, 160}, {46, 165}, {40, 166}, 
  /* A */	{FG_POLY, 8}, {91, 136}, {104, 131}, {132, 224}, {110, 235}, {100, 204}, {82, 200}, {70, 228}, {53, 230}, 
  /* A hole */	{BG_POLY, 3}, {95, 160}, {100, 177}, {90, 177}, 
  /* N */	{FG_POLY, 10}, {140, 130}, {157, 125}, {180, 190}, {177, 123}, {203, 128}, {195, 237}, {167, 239}, {153, 173}, {153, 235}, {135, 236}, 
  /* I */	{FG_POLY, 4}, {208, 152}, {233, 150}, {223, 231}, {208, 234}, 
  /* I hole */	{FG_POLY, 4}, {208, 128}, {234, 123}, {234, 140}, {208, 145}, 
  /* C */	{FG_POLY, 16}, {265, 125}, {300, 120}, {310, 140}, {308, 160}, {284, 167}, {284, 145}, {267, 149}, {260, 218}, {280, 218}, {282, 199}, {304, 196}, {302, 223}, {288, 235}, {253, 235}, {235, 222}, {248, 139}, 
		{END_DATA, 0}
};

XPoint arms[] = {
{FG_POLY, 21},
{80, 137}, {47, 142}, {36, 141}, {43, 118}, {45, 107}, {45, 92},
{41, 87}, {39, 82}, {42, 77}, {47, 74}, {56, 73}, {63, 74}, {65, 80},
{67, 83}, {78, 83}, {75, 91}, {60, 94}, {57, 96}, {52, 128}, {61, 128}, {73, 124},
{FG_POLY, /* 28 */ 29},
{266, 140}, {270, 135}, {302, 132}, {323, 128}, {325, 127}, {306, 104},
{297, 86}, {297, 82}, {302, 79}, {304, 71}, {301, 64}, {294, 61},
{286, 60}, /* old: {280, 64}, {275, 68}, {273, 73}, */
/* new: */
{279, 65}, {277, 69}, {275, 72}, {271, 72},
{260, 73}, {258, 75},
{261, 79}, {270, 80}, {274, 81}, {278, 85}, {285, 85}, {289, 94},
{305, 120}, {286, 121}, {276, 117}, {268, 112},
{END_DATA, 0}
};

XPoint short_tongue[] = {
{FG_POLY, 25}, {134, 189}, {137, 190}, {150, 196}, {161, 202}, {166, 204},
{169, 208},
{170, 213}, {176, 216}, {181, 217}, {188, 216}, {195, 215},
{203, 213}, {209, 209}, {211, 205}, {211, 202}, {207, 197}, {194, 177},
{185, 172}, {176, 170}, {170, 170}, {166, 175}, {162, 173}, {154, 173},
{144, 175}, {138, 181},
{END_DATA, 0}
};

XPoint rest_of_tongue[] = {
{FG_POLY, 19}, {169, 207}, {173, 226},
{178, 234}, {185, 238}, {200, 237}, {212, 234}, {217, 228}, {219, 220},
{214, 210},
{209, 199}, {211, 202}, {211, 205}, {209, 209}, {203, 213}, {195, 215},
{188, 216}, {181, 217}, {176, 216}, {170, 213},
{END_DATA, 0}
};

XPoint fingers[] = {
{FG_POLY, 28}, {42, 77}, {36, 68}, {31, 64}, {33, 59}, {40, 66}, {45, 73}, {42, 60},
{41, 52}, {45, 50}, {47, 63}, {51, 73}, {53, 73}, {51, 63}, {51, 50},
{54, 50}, {55, 63}, {57, 71}, {58, 71}, {60, 65}, {58, 53}, {63, 52},
{65, 64}, /* flat home bit: */ {63, 76}, {59, 75}, {55, 75}, {51, 75},
{47, 76}, {44, 76}, 
{FG_POLY, 25}, {279, 64}, {273, 42}, {276, 42}, {284, 61}, {287, 52}, {290, 43},
{293, 40}, {295, 43}, {291, 52}, {290, 60}, {294, 61}, {300, 49},
{306, 41}, {307, 44}, {301, 54}, {298, 62}, {300, 63}, {311, 49},
{314, 51}, /* flat home bit: */ {302, 66}, {296, 65}, {287, 65}, {285, 64},
{281, 63}, {280, 65},
{END_DATA, 0}
};

XPoint mouth[] = {
{BG_POLY, 29}, 
{90, 122}, {95, 122}, {100, 129}, {116, 140}, {136, 149}, {160, 154},
{178, 154}, {197, 150}, {214, 142}, {228, 132}, {241, 118}, {247, 113},
{252, 112}, {256, 114}, {257, 117}, {257, 124}, {253, 143}, {247, 157},
{235, 169}, {221, 182}, {203, 190}, {181, 192}, {158, 192}, {136, 190},
{119, 182}, {107, 171}, {96, 157}, {89, 139}, {88, 126},
{END_DATA, 0}
};

XPoint teeth[] = {
{FG_POLY, 4}, {98, 124}, {96, 127}, {99, 130}, {102, 128},
{FG_POLY, 4}, {104, 131}, {103, 134}, {104, 137}, {109, 135},
{FG_POLY, 4}, {109, 135}, {110, 140}, {112, 141}, {115, 139},
{FG_POLY, 5}, {117, 140}, {117, 146}, {121, 148}, {125, 147}, {126, 145},
{FG_POLY, 6}, {128, 145}, {128, 150}, {130, 152}, {135, 153}, {138, 151}, {138, 148},
{FG_POLY, 6}, {141, 150}, {141, 154}, {143, 157}, {150, 159}, {155, 157}, {155, 152},
{FG_POLY, 6}, {158, 154}, {158, 157}, {161, 161}, {167, 160}, {169, 157}, {169, 152},
{FG_POLY, 6}, {172, 153}, {173, 159}, {177, 160}, {182, 159}, {183, 155}, {182, 152},
{FG_POLY, 5}, {189, 151}, {189, 156}, {193, 156}, {198, 155}, {198, 149},
{FG_POLY, 5}, {200, 148}, {202, 153}, {208, 153}, {212, 149}, {208, 144},
{FG_POLY, 5}, {211, 143}, {213, 145}, {220, 144}, {222, 140}, {221, 136},
{FG_POLY, 5}, {224, 136}, {228, 138}, {232, 134}, {232, 129}, {231, 128},
{FG_POLY, 5}, {232, 127}, {234, 129}, {239, 126}, {240, 123}, {239, 121},
{FG_POLY, 4}, {239, 120}, {243, 122}, {246, 118}, {247, 113},
{END_DATA, 0}
};

int blob_x = 200, blob_y = 190;


/* app defaults */
char *defaults = "\
*geometry:  +100+100\n\
*scale:	    1.0\n\
*delay:	    150\n\
*sleep:	    300\n\
*uptime:    3\n\
*root:	    false\n\
*random:    false\n\
*boo:	    0.5\n\
";


/* possible options */
XrmOptionDescRec opts[] = {
  {"-display",	".display",	XrmoptionSepArg,	0},
  {"-geometry", ".geometry",	XrmoptionSepArg,	0},
  {"-geom",	".geometry",	XrmoptionSepArg,	0},
  {"-scale",	".scale",	XrmoptionSepArg,	0},
  {"-delay",	".delay",	XrmoptionSepArg,	0},
  {"-sleep",	".sleep",	XrmoptionSepArg,	0},
  {"-uptime",	".uptime",	XrmoptionSepArg,	0},
  {"-root",	".root",	XrmoptionNoArg,		"true"},
  {"-random",	".random",	XrmoptionNoArg,		"true"},
  {"-boo",	".boo",		XrmoptionSepArg,	0}
};
int num_opts = sizeof(opts)/sizeof(XrmOptionDescRec);


void syntax(void) {
    printf("usage: %s [-scale #.#] [-delay #] [-sleep #] [-uptime #.#]\n", prog);
    printf("              [-geometry +#+#] [-root] [-display display:screen]\n");
    printf("              [-random] [-boo #.#\n");
    printf("   where -scale     gives a scale factor for the window size\n");
    printf("         -delay     sets a milli second delay between flashes\n");
    printf("         -sleep     sets average seconds that the window goes away for\n");
    printf("         -uptime    sets number of seconds window is there for\n");
    printf("         -geometry  sets the location of the window\n");
    printf("         -root      runs on the screen background\n");
    printf("         -random    makes the window pop up at random locations\n");
    printf("         -boo       sets proportion that green smiley ball pops up\n");
    printf("defaults are:\n");
    printf("%s -scale 1.0 -delay 150 -sleep 300 -uptime 3.0 -geometry +100+100 -boo 0.5\n", prog);
}

#if 0
#ifndef usleep
void usleep(unsigned long);

/*
 * sleep for a number of micro-seconds
 */
void usleep(unsigned long usec) {
#ifdef SVR3
    poll((struct poll *)0, (size_t)0, usec/1000);   /* ms resolution */
#else
    struct timeval t;
    t.tv_usec = usec%(unsigned long)1000000;
    t.tv_sec = usec/(unsigned long)1000000;
    select(0, (void *)0, (void *)0, (void *)0, &t);
#endif
}
#endif
#endif


/*
 * returns a time in seconds with microsecond resolution
 */
double tim(void) {
  struct timeval tp;
  double t;

  gettimeofday(&tp, NULL);
  t = (float)(tp.tv_sec - 749174000) + 0.0000001*(float)(tp.tv_usec);

  return (t);
}


void get_GC(void) {
    unsigned long valuemask;
    XGCValues values;

    valuemask = 0;
    gc = XCreateGC(display, win, valuemask, &values);
}


void scale_item(XPoint *list, float scl, int offset_x, int offset_y) {
    int k = 0, i;

    while (list[k].x != END_DATA) {
        for (i = 0; i < list[k].y; i++) {
	    list[k+1+i].x *= scl;
	    list[k+1+i].y *= scl;
	    list[k+1+i].x += offset_x;
	    list[k+1+i].y += offset_y;
	}
        k += list[k].y + 1;
    }
}

void scale_data(float scl, int offset_x, int offset_y) {

    width *= scl;
    height *= scl;

    /* do chars */
    scale_item(chars, scl, offset_x, offset_y);

    /* do green ball picture thingy */
    blob_x *= scl;
    blob_y *= scl;

    scale_item(arms, scl, offset_x, offset_y);
    scale_item(short_tongue, scl, offset_x, offset_y);
    scale_item(rest_of_tongue, scl, offset_x, offset_y);
    scale_item(fingers, scl, offset_x, offset_y);
    scale_item(mouth, scl, offset_x, offset_y);
    scale_item(teeth, scl, offset_x, offset_y);
}


char *grab_str(XrmValue value) {
    char *str = (char *)malloc(value.size + 1);
    strncpy(str, (char *)value.addr, value.size);
    str[value.size] = 0;
    return (str);
}


void get_opts(int *x, int *y, float *scale, int *delay, int *sleepy_time,
 float *uptime, int *root, int *offset_x, int *offset_y,
 int *random_pos, float *boos) {

    XrmValue value;
    char *type, *geom, *tmp_str;

    /* get geom args and set defaults */
    *x = 100;
    *y = 100;
    if (XrmGetResource(db, "xpanic.geometry", "XPanic.Geometry", &type, &value)) {
	geom = grab_str(value);
	if (XParseGeometry(geom, x, y, &width, &height) != (XValue|YValue)) {
	    *x = 100;
	    *y = 100;
	}
    }

    /* get data scale arg, and scale the data up or down */
    if (XrmGetResource(db, "xpanic.scale", "XPanic.Scale", &type, &value)) 
	*scale = 0.5*atof(grab_str(value));
    if (*scale < 0) *scale = 0.5;

    /* get delay between flashes */
    /* entered in miliseconds, actually in micro... */
    if (XrmGetResource(db, "xpanic.delay", "XPanic.Delay", &type, &value)) 
	*delay = 1000*atoi(grab_str(value));
    if (*delay < 0) *delay = 150000;

    /* get average time between popups (default 300s ~ 5 mins) */
    if (XrmGetResource(db, "xpanic.sleep", "XPanic.Sleep", &type, &value)) 
	*sleepy_time = atoi(grab_str(value));
    if (*sleepy_time < 0) *sleepy_time = 300;

    /* get time window stays up for (default 3.0s) */
    if (XrmGetResource(db, "xpanic.uptime", "XPanic.Uptime", &type, &value)) 
	*uptime = atof(grab_str(value));
    if (*uptime < 0.0) *uptime = 3.0;

    /* see if we're running on a root window */
    *root = 0;
    *offset_x = *offset_y = 0;
    if (XrmGetResource(db, "xpanic.root", "XPanic.Root", &type, &value)) {
	tmp_str = grab_str(value);
	if (!strcmp(tmp_str, "true") || !strcmp(tmp_str, "True") || !strcmp(tmp_str, "TRUE")) {
	    *root = 1;
	    *scale = MIN((float)DisplayWidth(display, screen_num)/(float)DATA_WIDTH, (float)DisplayHeight(display, screen_num)/(float)DATA_HEIGHT);
	    *offset_x = (DisplayWidth(display, screen_num)  - (*scale)*DATA_WIDTH)/2;
	    *offset_y = (DisplayHeight(display, screen_num) - (*scale)*DATA_HEIGHT)/2;
	}
    }

    /* see if we are jumping to a random spot each time */
    *random_pos = 0;
    if (XrmGetResource(db, "xpanic.random", "XPanic.Random", &type, &value)) {
	tmp_str = grab_str(value);
	if (!strcmp(tmp_str, "true") || !strcmp(tmp_str, "True") || !strcmp(tmp_str, "TRUE"))
	    *random_pos = 1;
    }

    /* get proportion of boo's (default 0.5) */
    *boos = 0.5;
    if (XrmGetResource(db, "xpanic.boo", "XPanic.Boo", &type, &value))
	*boos = atof(grab_str(value));
    if (*boos < 0.0 || *boos > 1.0) *boos = 0.5;
}


void move_window(void) {
    int x, y;

    x = ((float)((random()) & 0xff)/255.0)*(screen_width  - width);
    y = ((float)((random()) & 0xff)/255.0)*(screen_height - height);

    XMoveWindow(display, win, x, y);
}


void set_colours(Colormap cmap, int *colours) {
    unsigned long plane_masks, pixels[2];

    bw = 0;

    /* choose a few colours */
    *colours = 5;
    XParseColor(display, cmap, "red", &col[0]);
    XAllocColor(display, cmap, &col[0]);
    XParseColor(display, cmap, "yellow", &col[1]);
    XAllocColor(display, cmap, &col[1]);
    XParseColor(display, cmap, "blue", &col[2]);
    XAllocColor(display, cmap, &col[2]);
    XParseColor(display, cmap, "green", &col[3]);
    XAllocColor(display, cmap, &col[3]);
    XParseColor(display, cmap, "purple", &col[4]);
    XAllocColor(display, cmap, &col[4]);

    if (XAllocColorCells(display, cmap, False, &plane_masks, 0, pixels, 2)) {
	fg_colour.pixel = pixels[0];
	bg_colour.pixel = pixels[1];

	fg_colour.red   = col[0].red;
	fg_colour.green = col[0].green;
	fg_colour.blue  = col[0].blue;
	bg_colour.red   = 0;
	bg_colour.green = 0;
	bg_colour.blue  = 0;

	fg_colour.flags = bg_colour.flags = DoRed | DoGreen | DoBlue;
	XStoreColor(display, cmap, &fg_colour);
	XStoreColor(display, cmap, &bg_colour);
    }
    else {
/*
fprintf (stderr, "%s: couldn't allocate two read-write color cells\n", prog);
fprintf (stderr, "running in b/w and redraw mode (no colormap cycling)\n");
*/
	bw = 1;
	fg_colour.pixel = BlackPixel(display, screen_num);
	bg_colour.pixel = WhitePixel(display, screen_num);
    }
}


void show_window(int root) {
    XEvent report;
    int expose = 0;

    XMapWindow(display, win);

    if (!root) {
        do {
	    XNextEvent(display, &report);
	    if (report.type == Expose) {
		while (XCheckTypedEvent(display, Expose, &report));
		expose = 1;
	    }
	} while (!expose);
    }
}


void draw_points(XPoint *list) {
    int k = 0;

    while (list[k].x != END_DATA) {
	XFillPolygon(display, win, gc, &list[k+1], list[k].y, Nonconvex, CoordModeOrigin);
        k += list[k].y + 1;
    }
}


#if 0
/* debug */
void draw_junk_points(XPoint *list) {
    int k = 0, i;

    while (list[k].x != END_DATA) {
	for (i = 0; i < list[k].y; i++) {
	    XDrawArc(display, win, gc, list[k+1+i].x, list[k+1+i].y, 2, 2, 0, 360*64);
	    XFlush(display);
	    usleep(1000000);
	}
        k += list[k].y + 1;
    }
}
#endif


void cycle_boo_colours(Colormap cmap, int bg_black) {

    bg_colour.red   = 0;
    bg_colour.green = 0;
    bg_colour.blue  = 0;
    fg_colour.red   = 0;
    fg_colour.green = 0;
    fg_colour.blue  = 0;

    if (!bg_black) {
	bg_colour.red   = 65535;
	fg_colour.green = 65535;
    }
    XStoreColor(display, cmap, &fg_colour);
    XStoreColor(display, cmap, &bg_colour);
    XFlush(display);
}


void draw_boo(int flicker, int first_call) {
    unsigned long green, red, green_flash, red_flash;
    int x, y;

    x = (int)((float)(width  - blob_x)/2.1);
    y = (int)((float)(height - blob_y)/3.0);

    if (bw) {
	green = white;
	red = white;
	green_flash = 0; /* unused */
	red_flash = 0;
    }
    else {
	green = col[3].pixel;
	red = col[0].pixel;
	green_flash = fg_colour.pixel;
	red_flash = bg_colour.pixel;
    }

    if (first_call) {
	/* clear the background to black */
	XSetForeground(display, gc, black);
	XFillRectangle(display, win, gc, 0, 0, width, height);

	/* draw green (white if b/w) circle */
	XSetForeground(display, gc, green);
	XFillArc(display, win, gc, x, y, blob_x, blob_y, 0, 360*64);

	/* draw green (white) arms */
	draw_points(arms);

	/* draw fingers in flashing green (white) */
	if (!bw) XSetForeground(display, gc, green_flash);
	draw_points(fingers);

	/* draw mouth in black */
	XSetForeground(display, gc, black);
	draw_points(mouth);

	/* draw short part of tongue in red (white) */
	XSetForeground(display, gc, red);
	draw_points(short_tongue);

	/* draw long part of tongue in flashing red (white) */
	if (!bw) XSetForeground(display, gc, red_flash);
	draw_points(rest_of_tongue);

	/* draw teeth in white */
	XSetForeground(display, gc, white);
	draw_points(teeth);
    }
    else {
	/*
	 * we only get here if it's b/w
	 */
	if (flicker) {
	    /* blank out fingers with black */
	    XSetForeground(display, gc, black);
	    draw_points(fingers);

	    /* blank out long part of tongue with black */
	    draw_points(rest_of_tongue);
	}
	else {
	    /* draw in fingers in green (white) */
	    XSetForeground(display, gc, green);
	    draw_points(fingers);

	    /* draw in long part of tongue in red (white) */
	    XSetForeground(display, gc, red);
	    draw_points(rest_of_tongue);
	}
    }

    XFlush(display);

#if 0
/* debug */
    XSetForeground(display, gc, red);
    draw_junk_points(fingers);
#endif
}


void cycle_chars_colours(Colormap cmap, int bg_black, int i) {

    if (bg_black) {
	bg_colour.red   = 0;
	bg_colour.green = 0;
	bg_colour.blue  = 0;
	fg_colour.red   = col[i].red;
	fg_colour.green = col[i].green;
	fg_colour.blue  = col[i].blue;
    }
    else {
	bg_colour.red   = col[i].red;
	bg_colour.green = col[i].green;
	bg_colour.blue  = col[i].blue;
	fg_colour.red   = 0;
	fg_colour.green = 0;
	fg_colour.blue  = 0;
    }
    XStoreColor(display, cmap, &fg_colour);
    XStoreColor(display, cmap, &bg_colour);
    XFlush(display);
}


void draw_chars(int bg_black) {
    int k;

    /* clear the background to the bg colour */
    if (!bg_black)
	XSetForeground(display, gc, bg_colour.pixel);
    else
	XSetForeground(display, gc, fg_colour.pixel);
    XFillRectangle(display, win, gc, 0, 0, width, height);

    /* draw the words! */
    k = 0;
    while (chars[k].x != END_DATA) {
        if ( (chars[k].x == FG_POLY && !bg_black) || (chars[k].x != FG_POLY && bg_black) )
	    XSetForeground(display, gc, fg_colour.pixel);
        else
            XSetForeground(display, gc, bg_colour.pixel);

	XFillPolygon(display, win, gc, &chars[k+1], chars[k].y, Nonconvex, CoordModeOrigin);
        k += chars[k].y + 1;
    }

    XFlush(display);
}


int main(int argc, char **argv) {

    Window rootwin;
    XSizeHints size_hints;
    XClassHint class_hints;
    char *display_name = NULL, *type;
    int x, y, offset_x, offset_y, delay, sleepy_time, snooze;
    int root, random_pos, first_call, boo;
    float scale, uptime, boos;
    double init_time;
    Colormap cmap;
    int i, bg_black, colours;
    XrmValue value;
    char fn[256];
    XEvent report;


    if ((prog = strrchr(argv[0], '/')) == NULL)
	prog = argv[0];
    else
	prog = &prog[1];

    /*
     * do some messy parsing...
     */
    /* built in defaults first */
    db = XrmGetStringDatabase(defaults);

    /* then ~/.Xdefaults */
    sprintf(fn, "%s/.Xdefaults", getenv("HOME"));
    if (access(fn, R_OK) == 0)
	XrmMergeDatabases(XrmGetFileDatabase(fn), &db);

    /* then command line */
    XrmParseCommand(&db, opts, num_opts, "xpanic", &argc, argv);

    /* wipeout with a helpful message if no comprende */
    if (argc != 1) {
	syntax();
	exit(0);
    }

    /* get display arg and check connection to server is ok */
    if (XrmGetResource(db, "xpanic.display", "XPanic.Display", &type, &value))
	display_name = grab_str(value);
    if ((display = XOpenDisplay(display_name)) == NULL) {
        fprintf(stderr, "%s: Cannot connect to X server", prog);
	if (getenv("DISPLAY") == NULL)
            fprintf(stderr, ", 'DISPLAY' environment variable not set.\n");
        else
            fprintf(stderr, " %s\n", XDisplayName(display_name));
        exit (-1);
    }

    screen_num = DefaultScreen(display);
    rootwin = RootWindow(display, screen_num);
    black = BlackPixel(display, screen_num);
    white = WhitePixel(display, screen_num);
    cmap = DefaultColormap(display, screen_num);
    screen_width = DisplayWidth(display, screen_num);
    screen_height = DisplayHeight(display, screen_num);

    get_opts(&x, &y, &scale, &delay, &sleepy_time, &uptime, &root,
             &offset_x, &offset_y, &random_pos, &boos);
    width = DATA_WIDTH;
    height = DATA_HEIGHT;



    /* scale the data to the chosen size */
    scale_data(scale, offset_x, offset_y);
    if (root) {
	width  = screen_width;
	height = screen_height;
    }

    set_colours(cmap, &colours);

    if (root)
        win = rootwin;
    else
	win = XCreateSimpleWindow(display, rootwin, x, y, width, height, 0, black, bg_colour.pixel);

    /* set size hints for window manager */
    size_hints.flags = USPosition;		/* size and position */
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    class_hints.res_name = prog;
    class_hints.res_class = "XPanic";

    /* set props for window manager */
    XSetStandardProperties(display, win, prog, prog, 0, argv, argc, &size_hints);
    XSetClassHint (display, win, &class_hints);

    XSelectInput(display, win, ExposureMask);

    get_GC();
    XSetForeground(display, gc, fg_colour.pixel);

    /*
     * I hate X - all this crap to go through to get one lousy window...
     * Finally display the window
     */
    if (random_pos && !root) move_window();
    show_window(root);

    /* init the colour indices and the random numbers */
    i = 0;
    srandom((int)(tim()*1000));

    /*
     * main loop - only way outa here is BLAT
     */
    while (1) {

	/* draw for the 1st time */
	first_call = 1;
	bg_black = 1;

	/* set whether we're doing a boo or a don't panic */
	boo = (int)(boos + (float)((random()) & 0xff)/255.0);

	if (!boo)
	    draw_chars(bg_black);
	else
	    draw_boo(bg_black, first_call);

	/* do our flashing thing for a few (default 3) seconds */
	first_call = 0;
        init_time = tim();
        while (tim() - init_time < uptime) {
	    if (bg_black++) {
		i++;
		i %= colours;
	    }
	    bg_black %= 2;

	    if (!boo) {
		/* chars stuff */
		if (!bw)
		    cycle_chars_colours(cmap, bg_black, i);
		else
		    /* darn, no colourmap cycling -> gotta redraw :-( */
		    draw_chars(bg_black);
	    }
	    else {
		/* boo stuff */
		if (!bw)
		    cycle_boo_colours(cmap, bg_black);
		else
		    /* gotta redraw some parts */
		    draw_boo(bg_black, first_call);
	    }

	    /* if exposed then redraw */
	    if (XCheckTypedEvent(display, Expose, &report))
		if (!boo)
		    draw_chars(bg_black);
		else {
		    first_call = 1;
		    draw_boo(bg_black, first_call);
		    first_call = 0;
		}

	    usleep(boo ? delay/2 : delay);  /* sleep for a bit between flashes */
	}

	/* hide from the world */
	if (root)
	    XClearWindow(display, win);
	else
	    XUnmapWindow(display, win);

	XFlush(display);

	/* sleep for a while... */
	snooze = sleepy_time*(1.0 + 0.5*(float)(((random()) & 0xff) - 127)/127.0);
	sleep(snooze);

	/* jump around the screen a bit */
	if (random_pos && !root) move_window();

	/*
	 * pop up now! especially that for the first time ever, the
	 * boss is there looking over your shoulder and thinking how
	 * in control and diligent you are...
	 */
	if (!root) show_window(root);
    }
}
