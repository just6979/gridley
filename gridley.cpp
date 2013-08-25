/*
Copyright (C) 2004-2009 Justin White
For licensing information see:
	"LICENSE" file distributed with the program
	"http://www.gnu.org/licenses/agpl-3.0.txt"

I wrote this for CS-436 Computer Graphics at Bridgewater State College.
I've heard the professor used it at least the next few semesters as an example.
Cool!
*/

/*
2004-09-17 - GLUT version
2013-08 - SFML update

This new version has been ported to use SFML instead of GLUT.
It is still useful for education, maybe better since GLUT is limited and old,
while SFML is more capable and currently (2013-08) actively developed.

The old version is still available from revision control (Mercurial).
Recent revisions include Code::Blocks IDE project files as well.

SFML Update Progress:
2013-08-25: 1 hour

*/

/* standard libraries */
#include <cstdio>
#include <cstdlib>
#include <cmath>

/* other libraries */
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

/* various declarations */
#include "gridley.hpp"

/* utility functions */
GLfloat float_from_chars(char key) {
	// store digits to be turned into float
	static char factor_str[11] = {0};
	// stop char for strtod
	char* end = '\0';
	if (g.factor_index >= 10) g.factor_index = 0;
	factor_str[g.factor_index++] = key;
	factor_str[g.factor_index] = '\0';
	return (float)strtod(factor_str, &end);
}

GLboolean is_digit(char c) {
	if ((c >= '0') && (c <= '9')) return true;
	else return false;
}

// convert interger mouse coords to floating point grid coords
Coord2f screen2grid(Coord2i screen) {
	Coord2f tmp;

	// move the points to fit the centered origin
	GLfloat x = GLfloat(screen.x - (g.width / 2));
	GLfloat y = GLfloat(-screen.y + (g.height / 2));

	// adjust for scaling
	tmp.x = x / g.screen_scale;
	tmp.y = y / g.screen_scale;

	return tmp;
}

/* round a floating point number to nearest whole */
GLfloat round_float(GLfloat f) {
	GLfloat r, a;

	r = fabs(fmod(f, 1.0f));
	if (f >= 0.0) {
		if (r >= 0.5) {
			a = ceil(f);
		} else {
			a = floor(f);
		}
	} else {
		if (r < 0.5) {
			a = ceil(f);
		} else {
			a = floor(f);
		}
	}
	return a;
}
/* floating point round a whole 2d floating point coord*/
Coord2f round_Coord2f(Coord2f p) {
	p.x = round_float(p.x);
	p.y = round_float(p.y);
	return p;
}

GLboolean is_uppercase(char key) {
	if ((key >= 'A') && (key <= 'Z')) {
		return true;
	} else {
		return false;
	}
}

GLfloat degrees_to_radians(GLfloat degrees) {
	return (PI / 180.0) * degrees;
}

/* drawing functions */
void draw_grid(void) {
	GLfloat i;

	// grid, gray
	glColor3f(0.9, 0.9, 0.9);
	glBegin(GL_LINES);
		// vertical lines in -x
		for (i = 0; i > -g.x_range; i -= g.grid_scale) {
			glVertex2f(i, -g.y_range);
			glVertex2f(i, g.y_range);
		}
		// vertical lines in x
		for (i = 0; i < g.x_range; i += g.grid_scale) {
			glVertex2f(i, -g.y_range);
			glVertex2f(i, g.y_range);
		}
		// horizontal lines in -y
		for (i = 0; i > -g.y_range; i -= g.grid_scale) {
			glVertex2f(-g.x_range, i);
			glVertex2f(g.x_range, i);
		}
		// horizontal lines in y
		for (i = 0; i < g.y_range; i += g.grid_scale) {
			glVertex2f(-g.x_range, i);
			glVertex2f(g.x_range, i);
		}
	glEnd();
}

void draw_origin(void) {
	// lines through origin
	glBegin(GL_LINES);
		// x red
		glColor3f(1.0, 0.0, 0.0);
		glVertex2f(-g.x_range, 0.0);
		glVertex2f(g.x_range, 0.0);
		// y green
		glColor3f(0.0, 1.0, 0.0);
		glVertex2f(0.0, -g.y_range);
		glVertex2f(0.0, g.y_range);
	glEnd();
}

void draw_points(void) {
	GLint i;

	// existing points, black
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	for (i = 0; i < g.last_point; i++) {
		if (g.points[i].x != 0) {
			glVertex2f(g.points[i].x, g.points[i].y);
		}
	}
	// current point, green
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f(g.points[g.last_point].x, g.points[g.last_point].y);

	glEnd();
}

void draw_lines(void) {
	GLint i;

	// lines, black
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < g.last_point; i++) {
		if (g.ends[i] == true) {
			glEnd();
			glBegin(GL_LINE_STRIP);
			glVertex2f(g.points[i].x, g.points[i].y);
		} else {
			glVertex2f(g.points[i].x, g.points[i].y);
		}
	}
	glEnd();
}

void draw_cur_line(void) {
	Coord2f last;

	if (g.snap_to_grid) {
		last = round_Coord2f(g.points[g.last_point]);
	} else {
		last.x = g.points[g.last_point].x;
		last.y = g.points[g.last_point].y;
	}
	if (g.last_point >= 0) {
		// current line, black to blue
		glColor3f(0.0, 0.0, 0.0);
		glLineStipple(1, 0xF0F0);
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINES);
			if (g.last_point == 0) {
				glVertex2f(0, 0);
			} else {
				glVertex2f(g.points[g.last_point - 1].x, g.points[g.last_point - 1].y);
			}
			glColor3f(0.0, 0.0, 1.0);
			glVertex2f(last.x, last.y);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}
}

/* Quick utility function for texture creation */
static int power_of_two(int input)
{
	int value = 1;

	while ( value < input ) {
		value <<= 1;
	}
	return value;
}

//GLuint SurfToTex(SDL_Surface *surface, GLfloat *texcoord)
//{
//	GLuint texture;
//	int w, h;
//	SDL_Surface *image;
//	SDL_Rect area;
////	Uint32 saved_flags;
////	Uint8  saved_alpha;
//
//	/* Use the surface width and height expanded to powers of 2 */
//	w = power_of_two(surface->w);
//	h = power_of_two(surface->h);
//	texcoord[0] = 0.0f;			/* Min X */
//	texcoord[1] = 0.0f;			/* Min Y */
//	texcoord[2] = (GLfloat)surface->w / w;	/* Max X */
//	texcoord[3] = (GLfloat)surface->h / h;	/* Max Y */
//
//	image = SDL_CreateRGBSurface(
//		SDL_SWSURFACE,
//		w, h,
//		32,
//		/* OpenGL RGBA masks */
//		#if SDL_BYTEORDER == SDL_LIL_ENDIAN
//			0x000000FF,
//			0x0000FF00,
//			0x00FF0000,
//			0xFF000000
//		#else
//			0xFF000000,
//			0x00FF0000,
//			0x0000FF00,
//			0x000000FF
//		#endif
//	);
//	if ( image == NULL ) {
//		return 0;
//	}
//
//	/* Save the alpha blending attributes */
////	saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
////	saved_alpha = surface->format->alpha;
////	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
////		SDL_SetAlpha(surface, 0, 0);
////	}
//
//	/* Copy the surface into the GL texture image */
//	area.x = 0;
//	area.y = 0;
//	area.w = surface->w;
//	area.h = surface->h;
//	SDL_BlitSurface(surface, &area, image, &area);
//
//	/* Restore the alpha blending attributes */
////	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
////		SDL_SetAlpha(surface, saved_flags, saved_alpha);
////	}
//
//	/* Create an OpenGL texture for the image */
//	glGenTextures(1, &texture);
//	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D,
//		     0,
//		     GL_RGBA,
//		     w, h,
//		     0,
//		     GL_RGBA,
//		     GL_UNSIGNED_BYTE,
//		     image->pixels);
//
//	/* No longer needed */
//	SDL_FreeSurface(image);
//
//	return texture;
//}

void draw_string(const char* msg, GLint x, GLint y) {
	sf::Text text(msg, g.font, 12);
	text.setColor(g.font_color);
	text.setPosition(x, y);
	g.window.draw(text);
}

void draw_help(void) {
	#define LINES 25
	GLint i;

	const char* help_msg[LINES] = {
		"Gridley Help",
		"Escape, q = Quit",
		"F1 = Toggle help display",
		"F2 = Toggle status display",
		"F3 = Toggle factor retention",
		"[, ] = Quantity of points in status: -, +",
		"f = Toggle fullscreen",
		"g = Toggle grid",
		"l = Toggle current line display",
		"p = Toggle Snap to Grid",
		"c = Clear all points",
		"u = Undo last point",
		"x = Reflect across X axis",
		"y = Reflect across Y axis",
		"r, R = Rotate: CW, CCW. Default: 30 deg",
		"s, S = Scale: up, down. Default: 2",
		"t, T = Scale Y only: up, down. Default: 2",
		"w, W = Scale X only: up, down. Default: 2",
		"h, H = Shear X only: +1, -1. Default: 1",
		"Up = Translate up. Default: 1",
		"Down = Translate down. Default: 1",
		"Left = Translate left. Default: 1",
		"Right = Translate right. Default: 1",
		"/[-]\\d*[.]\\d*/ = Alter transform factor",
		"Backspace = Reset transform factor, if retained.",
	};

	for (i = 0; i < LINES; i++) {
		draw_string(help_msg[i], 5, g.font_height * i);
	}
}

void draw_status(void) {
	int i;
	char msg[MAX_LEN];
	// point status, blue
	glColor3f(0.0, 0.0, 1.0);

	// show transformation factor, bottom left
	if (g.factor != 0) {
		if (g.keep_factor) {
			snprintf(msg, MAX_LEN, "Transform Factor: %4f; Retained", g.factor);
		} else {
			snprintf(msg, MAX_LEN, "Transform Factor: %4f", g.factor);
		}
	} else {
		if (g.keep_factor) {
			snprintf(msg, MAX_LEN, "Transform Factor: Default; Retained");
		} else {
			snprintf(msg, MAX_LEN, "Transform Factor: Default");
		}
	}
	draw_string(msg, 0, g.height - g.font_height * 1);

	// current point: bottom left
	if (g.snap_to_grid) {
		snprintf(msg, MAX_LEN, "#%d: (%.2f, %.2f)  [%.2f, %.2f]",
			g.last_point,
			round_float(g.points[g.last_point].x),
			round_float(g.points[g.last_point].y),
			g.points[g.last_point].x,
			g.points[g.last_point].y
		);
	} else {
		snprintf(msg, MAX_LEN, "#%d: (%.2f, %.2f)",
			g.last_point,
			g.points[g.last_point].x,
			g.points[g.last_point].y
		);
	}
	draw_string(msg, 0, g.height - g.font_height * 3);

	// previous points, quantity is choosable. up left side
	for (i = 1; i <= g.display_points && i <= g.last_point; i++) {
		// move to lower left corner, a little higher each time
		snprintf(msg, MAX_LEN, "#%d: (%.2f,%.2f)",
			g.last_point - i,
			g.points[g.last_point - i].x,
			g.points[g.last_point - i].y
		);
		// display message
		draw_string(msg, 0, g.height - (g.font_height * 3) - g.font_height * i);
	}

	// screen coords: bottom middle
	snprintf(msg, MAX_LEN, "Mouse: [%d,%d]", g.mouse.pos.x, g.mouse.pos.y);
	draw_string(msg, g.width / 2 + 10, g.height - g.font_height * 1);

}

void key_down(sf::Event::KeyEvent key) {
	GLint i;

	GLfloat scale_factor = 2;
	GLfloat shear_factor = 2;
	GLfloat rotate_factor = 30;
	GLfloat translate_factor = 1.0;

	// temp storage for rotations
	GLfloat rads;
	GLfloat tmp_x;

	// read digits, make float from them
//	if (is_digit(key) || (key == '.') || (key == '-')) {
//		g.factor = float_from_chars(key);
//		return;
//	}

	// if user entered a factor, use it, otherwise keep defaults
	if (g.factor != 0) {
		scale_factor = g.factor;
		rotate_factor = g.factor;
		shear_factor = g.factor;
		translate_factor = g.factor;
	}

	switch (key.code) {
	// quit
	case sf::Keyboard::Q:
	case sf::Keyboard::Escape:
		g.window.close();
		break;
	// toggle fullscreen
	case sf::Keyboard::F:
		if (g.fullscreen) {
		} else {
		}
		g.fullscreen = !g.fullscreen;
		break;
	// toggle current line display
	case sf::Keyboard::L:
		g.show_cur_line = !g.show_cur_line;
		break;
	// toggle grid
	case sf::Keyboard::G:
		g.show_grid = !g.show_grid;
		break;
	// toggle snap to grid
	case sf::Keyboard::P:
		g.snap_to_grid = !g.snap_to_grid;
		break;
	// clear points
	case sf::Keyboard::C:
		g.last_point = 0;
		break;
	// undo last point
	case sf::Keyboard::U:
		if (g.last_point > 0) g.last_point--;
		break;
	// reflect over x or y axis
	case sf::Keyboard::X:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].x = -g.points[i].x;
		}
		break;
	case sf::Keyboard::Y:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].y = -g.points[i].y;
		}
		break;
	// rotate by n degrees, clockwise or counterclockwise (SHIFT)
	case sf::Keyboard::R:
		rads = degrees_to_radians(rotate_factor);
		if (key.shift) rads *= -1;
		for (i = 0; i < g.last_point; i++) {
			tmp_x = g.points[i].x;
			g.points[i].x = g.points[i].x * cos(rads) + g.points[i].y * -sin(rads);
			g.points[i].y = tmp_x * sin(rads) + g.points[i].y * cos(rads);
		}
		break;
	// scale by one; up, down; y up, y down; x up, x down
	case sf::Keyboard::S:
		for(i = 0; i < g.last_point; i++ ) {
			if (key.shift) {
				g.points[i].x /= scale_factor;
				g.points[i].y /= scale_factor;
			} else {
				g.points[i].x *= scale_factor;
				g.points[i].y *= scale_factor;
			}
		}
		break;
	case sf::Keyboard::T:
		for(i = 0; i < g.last_point; i++ ) {
			if (key.shift) {
				g.points[i].y /= scale_factor;
			} else {
				g.points[i].y *= scale_factor;
			}
		}
		break;
	case sf::Keyboard::W:
		for(i = 0; i < g.last_point; i++ ) {
			if (key.shift) {
				g.points[i].x /= scale_factor;
			} else {
				g.points[i].x *= scale_factor;
			}
		}
		break;
	// shear x by 1
	case sf::Keyboard::H:
		for(i = 0; i < g.last_point; i++ ) {
			if (key.shift) {
				g.points[i].x -= g.points[i].y;
			} else {
				g.points[i].x += g.points[i].y;
			}
		}
		break;
	// translate by 1: +y, -y, -x, +x
	case sf::Keyboard::Up:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].y += translate_factor;
		}
		break;
	case sf::Keyboard::Down:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].y -= translate_factor;
		}
		break;
	case sf::Keyboard::Left:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].x -= translate_factor;
		}
		break;
	case sf::Keyboard::Right:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].x += translate_factor;
		}
		break;
	// reset the factor
	case sf::Keyboard::BackSpace:
		g.factor_index = 0;
		g.factor = 0;
		break;
	// increase number of points in the displayed list
	case sf::Keyboard::RBracket:
		if (g.display_points < MAX_POINTS) g.display_points++;
		break;
	// decrease number of point in the list
	case sf::Keyboard::LBracket:
		if (g.display_points > 0) g.display_points--;
		break;
	// toggle help, status, or factor display
	case sf::Keyboard::F1:
		g.show_help = !g.show_help;
		break;
	case sf::Keyboard::F2:
		g.show_status = !g.show_status;
		break;
	case sf::Keyboard::F3:
		g.keep_factor = !g.keep_factor;
		break;
	default:
		break;
	}

	if (!g.keep_factor) {
		// reset the factor
		g.factor_index = 0;
		g.factor = 0;
	}
}

sf::Color make_color(int r, int g, int b) {
	sf::Color tmp;
	tmp.r = r;
	tmp.g = g;
	tmp.b = b;
	return tmp;
}

void make_line(GLboolean end = false) {
	// too many points?
	if (g.last_point >= MAX_POINTS) {
		printf("Maximum points limit hit: %d.\n", MAX_POINTS);
		return;
	}

	g.ends[g.last_point] = end;

	if (g.snap_to_grid) {
		g.points[g.last_point] = round_Coord2f(g.points[g.last_point]);
	} else {
		g.points[g.last_point] = g.points[g.last_point];
	}
	g.last_point++;
}

void setup_gl_view(GLint type) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0);
	glPointSize(4.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (type == GRID) {
		// set view transform to put 0,0 in center, range scaled
		g.x_range = GLfloat(g.width / 2) / g.screen_scale;
		g.y_range = GLfloat(g.height / 2) / g.screen_scale;
		glOrtho(-g.x_range, g.x_range, -g.y_range, g.y_range, -1, 1);
		glDisable(GL_TEXTURE_2D);
	}
	if (type == OVERLAY) {
		// set view transform to put 0,0 in bottom left, full size
		glOrtho(0, g.width, g.height, 0, -1, 1);
		glEnable(GL_TEXTURE_2D);
	}
	glViewport(0, 0, g.width, g.height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void setup_video(GLint w, GLint h) {
	int flags;

	printf("Initializing video\n");
	flags = sf::Style::Titlebar|sf::Style::Close;
	if (g.fullscreen) {
		flags |= sf::Style::Fullscreen;
	} else {
		flags |= sf::Style::Resize;
	}

	g.window.create(sf::VideoMode(g.width, g.height), "Gridley", flags);
	//window.setVerticalSyncEnabled(true);
	g.window.setFramerateLimit(60);

	g.width = w;
	g.height = h;

	sf::ContextSettings settings = g.window.getSettings();
	printf("Utilizing OpenGL %i.%i\n", settings.majorVersion, settings.minorVersion);

	setup_gl_view(GRID);

	return;
}

/* entry point */
int main(GLint argc, char** argv) {
	// allocate storage for events popped off the queue
	sf::Event event;
	// keep track of how many milliseconds since the last update
	sf::Time theTime;
	int cycle_time = 1000;
	int last_time = 0;
	int this_time = 0;

	printf("Starting up\n");
	g.width = g.height = 800;
	g.grid_scale = 1;
	g. screen_scale = 20.0;
	g.display_points = 10;

	// set some flags
	g.fullscreen = false;
	g.show_grid = true;
	g.show_cur_line = true;
	g.show_help = false;
	g.show_status = true;
	g.snap_to_grid = true;

	// enable this if we want the factor to reset after any operation
	// false is more like Lorenzen's
	g.keep_factor = true;

	printf("Compiled with SFML %i.%i\n", SFML_VERSION_MAJOR, SFML_VERSION_MINOR);

	setup_video(g.width, g.height);

	const char* font_filename = "DejaVuSans.ttf";
	printf("Loading font: %s\n", font_filename);
	g.font.loadFromFile(font_filename);
	g.font_height = 12;
	g.font_color = make_color(0, 0, 0);

//	g.running = true;
//	printf("Running...\n");
	while(g.window.isOpen()) {
		// give up CPU just long enough to maintain ~60 Hz
		this_time = theTime.asMilliseconds();
		cycle_time = this_time - last_time;
		/* update state of the simulation */
		// find the mouse and point to it with the next line
		g.points[g.last_point] = screen2grid(g.mouse.pos);

		/* draw everything and flip buffers */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw scaled things
		setup_gl_view(GRID);
		if (g.show_grid) {
			draw_grid();
			draw_origin();
		}
		draw_points();
		draw_lines();
		if (g.show_cur_line) draw_cur_line();

		// draw overlays
		setup_gl_view(OVERLAY);
		if (g.show_help) draw_help();
		if (g.show_status) draw_status();

		g.window.display();
		// get all the events on the queue
		while(g.window.pollEvent(event)){
			switch(event.type){
				case sf::Event::Closed:
					g.window.close();
					break;
				case sf::Event::Resized:
					setup_video(event.size.width, event.size.height);
					break;
				case sf::Event::KeyPressed:
					key_down(event.key);
					break;
				case sf::Event::MouseMoved:
					g.mouse.pos.x = event.mouseMove.x;
					g.mouse.pos.y = event.mouseMove.y;
					break;
				case sf::Event::MouseButtonPressed:
					g.mouse.button[1] = true;
					break;
				case sf::Event::MouseButtonReleased:
					g.mouse.button[2] = false;
					if (event.mouseButton.button == sf::Mouse::Left) {
						make_line();
					}
					if (event.mouseButton.button == sf::Mouse::Right) {
						make_line(true);
					}
					break;
				default:
					break;
			}
		}
	}
	exit(0);
}
