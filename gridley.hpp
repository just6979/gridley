/* Various declarations for the Gridley project */

#ifndef GRIDLEY_HPP_INCLUDED
#define GRIDLEY_HPP_INCLUDED

/* constants */
#define MAX_POINTS 1000
#define MAX_LEN 80
const GLfloat PI = 3.14159265;
const GLint GRID = 1;
const GLint OVERLAY = 2;

/* types */

// 2d float point
struct Coord2f {
	GLfloat x;
	GLfloat y;
};

// 2D integer point
struct Coord2i {
	GLint x;
	GLint y;
};

// mouse state: 2D position and 5 buttons
#define MAX_MOUSE_BUTTONS 5
struct mouse_state {
	Coord2i pos;
	GLboolean button[MAX_MOUSE_BUTTONS];
};

// other globals, keep them in this struct to prevent strays
struct Globals {
	// framebuffer dimensions
	GLint width, height, bpp;
	// ranges of the origin centered ortho projection
	GLfloat x_range, y_range;
	// how many points to skip between gridlines
	GLint grid_scale;
	// how many pixels between each point
	GLfloat screen_scale;
	// array of chosen points
	Coord2f points[MAX_POINTS];
	// array denoting line connections or solo points
	GLboolean ends[MAX_POINTS];
	// index to last point, the one that will be picked next
	GLint last_point;
	// factor to perform each transform by
	GLfloat factor;
	// keep track of digits for entering factors
	GLint factor_index
	// how many points to display in status
	GLint display_points;
	// mouse state
	mouse_state mouse;
	// framebuffer (or backbuffer if doublebuffered) surface
	sf::RenderWindow window;
	// DejaVu Sans TrueType font
	sf::Font font;
	sf::Color font_color;
	GLint font_height;
	// flags
	GLboolean show_help;
	GLboolean show_cur_line;
	GLboolean snap_to_grid;
	GLboolean show_grid;
	GLboolean show_status;
	GLboolean fullscreen;
	GLboolean keep_factor;
	// is the event loop still going?
	GLboolean running;
} g;


/* functions */
int quit(int = 0);

#endif // GRIDLEY_HPP_INCLUDED
