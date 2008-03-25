/*
Justin White
CS 436
2004-09-17

I wrote this program for CS-436, Computer Graphics, at Bridgewater State College.
I've heard the professor used it at lease the next few semesters as an example.
Cool!
*/

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/glut.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/* constants */
const float PI = 3.14159265;
#define MAX_POINTS 100
#define LINES 26
#define MAX_LEN 80

/* types */
// store world coords and line endpoint status
typedef struct point {
	GLfloat x, y, z;
	GLboolean end;
} Point;

// 3 by 3 matrix
typedef GLfloat Matrix3x3[3][3];

// flags, all boolean globals
typedef struct flags {
	GLboolean show_help;
	GLboolean show_cur_line;
	GLboolean snap_to_grid;
	GLboolean show_grid;
	GLboolean show_status;
	GLboolean fullscreen;
	GLboolean keep_factor;
} Flags;

// other globals, keep them in this struct to prevent strays
typedef struct globals {
	// size of the window
	GLint view_width, view_height;
	GLfloat x_range, y_range;
	// how many points to skip between gridlines
	GLfloat grid_scale;
	// how many pixels per point
	GLfloat screen_scale;
	// array of chosen points
	Point points[MAX_POINTS];
	// index to last point, the one that will be picked next
	GLint last_point;
	// factor to perform each transform by
	GLfloat factor;
	GLint factor_index;
	// how many points to display in status
	GLint display_points;
	// mouse position
	GLint mouse_x;
	GLint mouse_y;
} Globals;

/* globals */
Globals g;
Flags f;

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

/* translates a screen location to a grid location */
Point screen_to_world(GLfloat x,  GLfloat y, GLfloat z, GLboolean end) {
	Point tmp;

	tmp.x = (x - g.view_width / 2) / g.screen_scale;
	tmp.y = -(y - g.view_height / 2) / g.screen_scale;
	tmp.z = z;
	tmp.end = end;

	return tmp;
}

/*
Point screen_to_world(Point screen) {
	return screen_to_world(screen.x, screen.y, screen.z, screen.end);
}
*/

/* remember current position */
void update_position(GLint x, GLint y) {
	// in homogeneous world coords
	g.points[g.last_point] = screen_to_world(x, y, 1.0, true);
	// in screen coords
	g.mouse_x = x;
	g.mouse_y = y;
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
/* floating point round, one Point at a time */
Point round_Point(Point p) {
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

void undo(void) {
	if (g.last_point > 0) g.last_point--;
}

/* drawing functions */
void draw_grid(void) {
	GLfloat i;

	glBegin(GL_LINES);
		// grid, gray
		glColor3f(0.9, 0.9, 0.9);
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
	glBegin(GL_LINES);
	// lines through origin, red
		glColor3f(1.0, 0.0, 0.0);
		glVertex2f(-g.x_range, 0.0);
		glVertex2f(g.x_range, 0.0);
		glVertex2f(0.0, -g.y_range);
		glVertex2f(0.0, g.y_range);
	glEnd();
}

void draw_points(void) {
	GLint i;

	// all points, black
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	for (i = 0; i < g.last_point - 1; i++) {
		if (g.points[i].x != 0) {
			glVertex2f(g.points[i].x, g.points[i].y);
		}
	}
	// last point, green
	if (g.last_point > 0) {
		glColor3f(0.0, 1.0, 0.0);
		glVertex2f(g.points[g.last_point - 1].x, g.points[g.last_point - 1].y);
	}
	// current point, yellow
	glColor4f(1.0, 1.0, 0.0, 0.5);
	glVertex2f(g.points[g.last_point].x, g.points[g.last_point].y);

	glEnd();
}

void draw_lines(void) {
	GLint i;

	// lines, black
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < g.last_point; i++) {
		if (g.points[i].end == false) {
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
	if (g.last_point > 0) {
		// current line, green to yellow
		glColor3f(0.0, 1.0, 0.0);
		glLineStipple(1, 0xF0F0);
		glEnable(GL_LINE_STIPPLE);
			glBegin(GL_LINES);
			glVertex2f(g.points[g.last_point - 1].x, g.points[g.last_point - 1].y);
// 			glColor3f(1.0, 1.0, 0.0);
			glVertex2f(g.points[g.last_point].x, g.points[g.last_point].y);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}
}

void draw_string(char* msg, GLint max_len) {
	GLint j;
	for (j = 0; j < max_len; j++) {
		if (msg[j] == '\0') { break; }
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, msg[j]);
	}
}

void draw_help(void) {
	GLint i;

	char* help_msg[LINES] = {
		"Gridley Help",
		"F1 = Toggle help display",
		"q = Quit",
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
		"ESC = Reset transform factor, if retained.",
		"Numbers in the form /[-]\\d*[.]\\d*/ will",
		"    alter the transform factor",
	};

	// help message, blue
	glColor3f(0.0, 0.0, 1.0);
	// save current transformation matrix
	glPushMatrix();
	for (i = 0; i < LINES; i++) {
		glRasterPos2f(-g.x_range + 0.25, g.y_range - 0.5 - 0.6 * i);
		draw_string(help_msg[i], MAX_LEN);
	}
	// remember last transformation matrix
	glPopMatrix();
}

void draw_status(void) {
	int i;
	char msg[MAX_LEN];
	// point status, blue
	glColor3f(0.0, 0.0, 1.0);
	// save current transformation matrix
	glPushMatrix();

	// screen coords
	// move to bottom left
	glRasterPos2f(-g.x_range + 0.25, -g.y_range + 0.25);
 snprintf(msg, MAX_LEN, "Mouse: [%d,%d]", g.mouse_x, g.mouse_y);
	draw_string(msg, MAX_LEN);

	// current point
	// move to bottom left, a little higher
	glRasterPos2f(-g.x_range + 0.25, -g.y_range + 1.0);
	snprintf(msg, MAX_LEN, "#%d: [%.2f,%.2f]",
		g.last_point,
		g.points[g.last_point].x,
		g.points[g.last_point].y
	);
	draw_string(msg, MAX_LEN);

	// current point, snapped to grid
	// move to bottom left, a little right
	glRasterPos2f(-g.x_range + 5, -g.y_range + 1.0);
	if (f.snap_to_grid) {
		snprintf(msg, MAX_LEN, "  Snapped : [%.2f,%.2f]",
			round_float(g.points[g.last_point].x),
			round_float(g.points[g.last_point].y)
		);
		draw_string(msg,MAX_LEN);
	}

	// previous points, quantity is choosable
	for (i = 1; i <= g.display_points && i <= g.last_point; i++) {
		// move to lower left corner, a little higher each time
		glRasterPos2f(-g.x_range + 0.25, -g.y_range + 1.0 + 0.6 * i);
		// to distinguish from line ending points. JW 2007-01-09
		/*if (g.points[g.last_point -i].end == true) {
			snprintf(msg, MAX_LEN, "#%d: [%.2f,%.2f)",
				g.last_point - i,
				g.points[g.last_point - i].x,
				g.points[g.last_point - i].y
			);
		} else {*/
			snprintf(msg, MAX_LEN, "#%d: [%.2f,%.2f]",
				g.last_point - i,
				g.points[g.last_point - i].x,
				g.points[g.last_point - i].y
			);
// }
	// display message
		draw_string(msg, MAX_LEN);
	}

	// show transformation factor
	glRasterPos2f(0.25, -g.y_range + 0.25);
	if (g.factor != 0) {
		if (f.keep_factor) {
			snprintf(msg, MAX_LEN, "Transform Factor: %4f; Retained", g.factor);
		} else {
			snprintf(msg, MAX_LEN, "Transform Factor: %4f", g.factor);
		}
	} else {
		if (f.keep_factor) {
			snprintf(msg, MAX_LEN, "Transform Factor: Default; Retained");
		} else {
			snprintf(msg, MAX_LEN, "Transform Factor: Default");
		}
	}
	draw_string(msg, MAX_LEN);

	// remember last transformation matrix
	glPopMatrix();
}


/* GLUT callbacks */
void display (void)
{
    glClear(GL_COLOR_BUFFER_BIT);

	if (f.show_grid) {
		draw_grid();
		draw_origin();
	}
	if (f.show_help) draw_help();
	if (f.show_status) draw_status();
	draw_points();
	draw_lines();
	if (f.show_cur_line) draw_cur_line();

	glutSwapBuffers();
}


void reshape(GLint w, GLint h)
{
	g.view_width = w;
	g.view_height = h;
	// range is half the height or width
	g.x_range = g.view_width / 2 / g.screen_scale;
	g.y_range = g.view_height / 2 / g.screen_scale;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// set view transform to put 0,0 in the center of the window
	glOrtho(-g.x_range, g.x_range, -g.y_range, g.y_range, -1, 1);
	glViewport(0, 0, g.view_width, g.view_height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutPostRedisplay();
}

void keyboard(unsigned char key, GLint x, GLint y) {
	GLint i;
	GLfloat scale_factor = 2;
	GLfloat shear_factor = 2;
	GLfloat rotate_factor = 30;
	// temp storage for rotations
	GLfloat rads;
	GLfloat tmp_x;

	// read digits, make float from them
	if (is_digit(key) || (key == '.') || (key == '-')) {
		g.factor = float_from_chars(key);
		glutPostRedisplay();
		return;
	}


	// if user entered a factor, use it, otherwise keep defaults
	if (g.factor != 0) {
		scale_factor = g.factor;
		rotate_factor = g.factor;
		shear_factor = g.factor;
	}

	switch (key) {
	// quit
	case 'q':
		exit(0);
		break;
	// toggle fullscreen
	case 'f':
		if (f.fullscreen) {
			glutReshapeWindow(500, 500);
			glutPositionWindow(100, 50);
		} else {
			glutFullScreen();
		}
		f.fullscreen = !f.fullscreen;
		break;
	// toggle current line display
	case 'l':
		f.show_cur_line = !f.show_cur_line;
		break;
	// toggle grid
	case 'g':
		f.show_grid = !f.show_grid;
		break;
	// toggle snap to grid
	case 'p':
		f.snap_to_grid = !f.snap_to_grid;
		break;
	// clear points
	case 'c':
		g.last_point = 0;
		break;
	// undo last point
	case 'u':
		undo();
		break;
	// reflect over x or y axis
	case 'x':
		for (i = 0; i < g.last_point; i++) {
			g.points[i].x = -g.points[i].x;
		}
		break;
	case 'y':
		for (i = 0; i < g.last_point; i++) {
			g.points[i].y = -g.points[i].y;
		}
		break;
	// rotate by n degrees, clockwise or counterclockwise
	case 'r':
		rads = degrees_to_radians(rotate_factor);
		for (i = 0; i < g.last_point; i++) {
			tmp_x = g.points[i].x;
			g.points[i].x = g.points[i].x * cos(rads) + g.points[i].y * -sin(rads);
			g.points[i].y = tmp_x * sin(rads) + g.points[i].y * cos(rads);
		}
		break;
	case 'R':
		rads = degrees_to_radians(rotate_factor);
		rads *= -1;
		for (i = 0; i < g.last_point; i++) {
			tmp_x = g.points[i].x;
			g.points[i].x = g.points[i].x * cos(rads) + g.points[i].y * -sin(rads);
			g.points[i].y = tmp_x * sin(rads) + g.points[i].y * cos(rads);
		}
		break;
	// scale by one; up, down, y up, y down, x up, x down
	case 's':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].x *= scale_factor;
			g.points[i].y *= scale_factor;
		}
		break;
	case 'S':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].x /= scale_factor;
			g.points[i].y /= scale_factor;
		}
		break;
	case 't':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].y *= scale_factor;
		}
		break;
	case 'T':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].y /= scale_factor;
		}
		break;
	case 'w':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].x *= scale_factor;
		}
		break;
	case 'W':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].x /= scale_factor;
		}
		break;
	// shear x by 1
	case 'h':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].x += g.points[i].y;
		}
		break;
	case 'H':
		for(i = 0; i < g.last_point; i++ ) {
			g.points[i].x -= g.points[i].y;
		}
		break;
	case 27:
	// ESCAPE
		g.factor_index = 0;
		g.factor = 0;
		break;
	case '}': case ']':
		if (g.display_points < MAX_POINTS) g.display_points++;
		break;
	case '{': case '[':
		if (g.display_points > 0) g.display_points--;
		break;
	default:
		break;
	}

	if (!f.keep_factor) {
		g.factor_index = 0;
		g.factor = 0;
	}

	glutPostRedisplay();
}

void special(GLint key, GLint x, GLint y) {
	GLfloat translate_factor = 1.0;
	GLint i;
	// if user entered a factor, use it, otherwise keep defaults
	if (g.factor != 0.0) {
		translate_factor = g.factor;
	}

	switch (key) {
	// translate by 1: +y, -y, -x, +x
	case GLUT_KEY_UP:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].y += translate_factor;
		}
		break;
	case GLUT_KEY_DOWN:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].y -= translate_factor;
		}
		break;
	case GLUT_KEY_LEFT:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].x -= translate_factor;
		}
		break;
	case GLUT_KEY_RIGHT:
		for (i = 0; i < g.last_point; i++) {
			g.points[i].x += translate_factor;
		}
		break;
// toggle help display
	case GLUT_KEY_F1:
		f.show_help = !f.show_help;
		break;
	case GLUT_KEY_F2:
		f.show_status = !f.show_status;
		break;
	case GLUT_KEY_F3:
		f.keep_factor = !f.keep_factor;
		break;
	default:
		break;
	}

	if (!f.keep_factor) {
		g.factor_index = 0;
		g.factor = 0;
	}

	glutPostRedisplay();
}

void mouse(GLint button, GLint state, GLint x, GLint y) {
	// too many points?
	if (g.last_point >= MAX_POINTS) {
	// don't do anything
		printf("Maximum points limit hit: %d.\n", MAX_POINTS);
		return;
	}
	// left button, connect previous point
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) {
			g.points[g.last_point].end = true;
			if (f.snap_to_grid) {
				g.points[g.last_point] = round_Point(g.points[g.last_point]);
			} else {
				g.points[g.last_point] = g.points[g.last_point];
			}
			g.last_point++;
		}
	}
	// right button, do not connect previous point
	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_UP) {
			g.points[g.last_point].end = false;
			if (f.snap_to_grid) {
				g.points[g.last_point] = round_Point(g.points[g.last_point]);
			} else {
				g.points[g.last_point] = g.points[g.last_point];
			}
			g.last_point++;
		}
	}

	glutPostRedisplay();
}

void motion(GLint x, GLint y) {
	// update the position whenever the mouse moves while any buttons are down
	update_position(x, y);
	glutPostRedisplay();
}

void passive_motion(GLint x, GLint y) {
	// update the position whenever the mouse moves while all button are up
	update_position(x, y);
	glutPostRedisplay();
}

void entry(GLint state) {
}

void visibility(GLint state) {
}

void idle(void) {
}

/* setup functions */
void gl_setup (void)
{
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_SMOOTH);
	glLineWidth(1.0);
	glPointSize(3.0);
	// clear to white
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void glut_setup(void) {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	// set up window
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(g.view_width, g.view_height);
	glutCreateWindow("Gridley");
	// register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutEntryFunc(entry);
	glutVisibilityFunc(visibility);
	glutIdleFunc(idle);
}

/* entry point */
int main(GLint argc, char** argv)
{
	g.view_width = g.view_height = 500;
	g.grid_scale = 1;
	g. screen_scale = 20;
	g.display_points = 5;

	f.fullscreen = false;
	f.show_grid = true;
	f.show_cur_line = true;
	f.show_help = false;
	f.show_status = true;
	f.snap_to_grid = true;
	// enable this is we want the factor to reset to default after any operation
	// false is more like Lorenzen's
	f.keep_factor = false;

	glutInit(&argc, argv);
	glut_setup();
	gl_setup ();
	printf("Running...\n");
	glutMainLoop();
	return 0;
}
