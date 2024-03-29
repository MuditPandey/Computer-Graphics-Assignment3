#include <windows.h> // for MS Windows
#include <iostream>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include<fstream>
#include<math.h>
#define PI 3.141592
#define WINDOW_X 640
#define WINDOW_Y 480
#define T_Interval 0.001
#define Intersect_d 1

using namespace std;

struct point2
{
	/* !defines a point in 2D space*/
	float x;
	float y;
};
struct point3
{
	/* !defines a point in 3D space*/
	float x;
	float y;
	float z;
};

struct face
{
	/* !defines a face represented by 4 vertices. The array stores the index of the vertices */
	int vertices[4];
};
struct polymesh
{
	/* !use to represent a polygon mesh having nver vertices and nface faces in 3D space. 
	*/
	int nver;
	int nface;
	point3 vertices[10000];
	face faces[10000];
};

point2 *points[1000];
int num_points = 0;
bool drag = false;
point2 *drag_point;
void handle_keyboard(unsigned char, int, int);
void handle_mouse(int, int);
void ReDraw();
void handle_click(int, int, int, int);
point2 *Lerp(float, point2 *, point2 *);
point2 *Findx(float, point2 **, int, bool del = true);
void draw_pixel(GLint, GLint, int color = 1);
void ClearScreen();
void draw_point(int, int);
int checkProximity(int, int, int);
void generateMesh();
void init();
void display();
void print_off(polymesh);

int main(int argc, char** argv) {
	glutInit(&argc, argv);            // Initialize GLUT
	init();                       // Our own OpenGL initialization
									//PlaySound(convertCharArrayToLPCWSTR("footstep.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	glutDisplayFunc(display);
	glutKeyboardFunc(handle_keyboard);// Register callback handler for window re-paint event
	glutMouseFunc(handle_click);
	glutPassiveMotionFunc(handle_mouse);
	glutWarpPointer(WINDOW_X / 2, WINDOW_Y / 2);
	ClearScreen();
	glutMainLoop();                 // Enter the infinite event-processing loop
	return 0;
}
void init() {
	/* !Function to initialize glut
	*/
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_X, WINDOW_Y);
	glutCreateWindow("Bezier Curve");
	gluOrtho2D(0, WINDOW_X, 0, WINDOW_Y);
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	ClearScreen();
}
void ClearScreen()
{
	/* !Function to clear screen i.e screen is painted only with background color
	*/
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
}
void display()
{

	/* !display function which displays the scene and runs in a loop
	*/
	ClearScreen();
}
point2 *Lerp(float t, point2 *a, point2 *b)
{
	/* !Function which returns a pointer to a point generated by Linear Interpolation of two points
	*/
	point2 *p = new point2();
	p->x = (1 - t) * a->x + t * b->x;
	p->y = (1 - t) * a->y + t * b->y;
	return p;
}

point2 *Findx(float t, point2 **p, int num, bool del)
{
	/* !Function which implements De Casteljau's algorithm for drawing a bezier curve
	*/
	point2 **q = new point2*[num - 1]; //memory allocation of array of pointers
	for (int i = 0; i < num - 1; ++i) //for all points in the array of pointers
	{
		q[i] = new point2(); //memory allocation of points
		q[i] = Lerp(t, p[i], p[i + 1]); //assigning the values of points using Lerp
	}
	if (del)
	{
		for (int i = 0; i < num; ++i) //memory deallocation of unused points
			delete[] p[i];
		delete[] p;
	}
	if (num == 2) //recursion break condition (if final x value is calculated)
		return q[0];
	return Findx(t, q, num - 1); //recursive call on the function
}

int checkProximity(int x, int y, int size)
{
	/* !Function to display a different cursor when the current mouse positon is near to an existing point
	*/
	for (int i = 0; i < num_points; ++i)
	{
		if (abs(points[i]->x - x) <= size && abs(points[i]->y - y) <= size)
			return i;
	}
	return -1;
}

void handle_mouse(int x, int y)
{
	/* !Event handler in case of passive mouse motion
	*/
	int prox = checkProximity(x, y, Intersect_d);
	if (prox >= 0) glutSetCursor(GLUT_CURSOR_INFO);
	else glutSetCursor(GLUT_CURSOR_CROSSHAIR);
}

void handle_keyboard(unsigned char key, int x, int y)
{
	/* !Event handler in case of a key press on the keyboard
	*/
	switch (key)
	{
	case 'z': cout << "UNDO last point" << endl;
		if (num_points > 0)
		{
			delete points[num_points];
			--num_points;
		}
		ReDraw();
		break;
	case 'f': cout << "Generating mesh..." << endl;
		generateMesh();
	}
}

void draw_pixel(GLint x, GLint y, int color)
{
	/* !Draws a point of varying color
	*/
	if (color == 1)
		glColor3f(1.0, 0.0, 0.0);
	else if (color == 2)
		glColor3f(0.0, 1.0, 0.0);
	else
		glColor3f(0.0, 0.0, 1.0);
	glPointSize(2.0);
	glBegin(GL_POINTS);
	glVertex2i(x, WINDOW_Y - y);
	glEnd();
}

void draw_point(int x, int y)
{
	/* !Draws a thick dot
	*/
	draw_pixel(x, y);
	draw_pixel(x - 1, y);
	draw_pixel(x + 1, y);
	draw_pixel(x, y + 1);
	draw_pixel(x, y - 1);
}

void handle_click(int button, int state, int x, int y)
{
	/* !Event handler in case of a mouse click
	*/
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		int prox = checkProximity(x, y, Intersect_d);
		if (prox >= 0)
		{
			drag_point = points[prox];
			drag = true;
			cout << "DRAGGING" << endl;
			return;
		}
		if (checkProximity(x, y, 2 * Intersect_d + 1) >= 0)
			return;
		points[num_points] = new point2();
		points[num_points]->x = x;
		points[num_points]->y = y;
		++num_points;
		cout << "Drawing pixel: " << "(" << x << ", " << y << ")" << endl;
		ReDraw();
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if (drag)
		{
			cout << "Dragging stopped" << endl;
			drag = false;
			glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			drag_point->x = x;
			drag_point->y = y;
			ReDraw();
		}

	}
	else if (button = GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		int prox = checkProximity(x, y, 2);
		if (prox == -1) return;
		delete points[prox];
		--num_points;
		for (int i = prox; i < num_points; ++i)
		{
			points[i] = points[i + 1];
		}
		ReDraw();

	}
}

void DrawCurve()
{
	/* !Draws a Bezier Curve for the given points using De Casteljau algorithm
	*/
	if (num_points < 2) return;
	point2 *x = new point2();
	for (float t = 0; t <= 1; t += T_Interval)
	{
		x = Findx(t, points, num_points, false);
		draw_pixel(x->x, x->y, 2);
	}
}

void ReDraw()
{
	/* !Clears the screen and redraws the bezier curve
	*/
	ClearScreen();
	for (int i = 0; i < num_points; ++i)
	{
		draw_point(points[i]->x, points[i]->y);
	}
	DrawCurve();
	glFlush();
}
void generateMesh()
{
	/* !Generates a surface of revolution of a curve about y-axis and stores the resulting surface in a polygon mesh
	*/
	//point vertices[10000];
	point3 finalvertices[11];
	//face faces[10000];
	polymesh p;
	int no_of_vertices = 11;
	int no_of_faces = 0;
	point2 *x = new point2();
	for (int i = 0; i < 11; i++)
	{
		x = Findx(i * 0.1f, points, num_points, false);
		p.vertices[i].x = x->x;
		p.vertices[i].y = WINDOW_Y - x->y;
		p.vertices[i].z = 0.0f;
	}
	for (int i = 0; i < 36; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			finalvertices[j].x = p.vertices[j].x*cos(10 * (i + 1)* PI / 180) - p.vertices[j].z*sin(10 * (i + 1)*PI / 180);
			finalvertices[j].y = p.vertices[j].y;
			finalvertices[j].z = -p.vertices[j].x*sin(10 * (i + 1)* PI / 180) - p.vertices[j].z*cos(10 * (i + 1)*PI / 180);
		}
		for (int i = 0; i < 11; i++)
		{
			p.vertices[no_of_vertices] = finalvertices[i];
			no_of_vertices++;
		}
		//MAKE FACES
		/*for (int j = 0; j < 10; j++)
		{
		p.faces[no_of_faces].vertices[0] = &p.vertices[no_of_vertices-2*11+j];
		p.faces[no_of_faces].vertices[1] = &p.vertices[no_of_vertices - 2 * 11+j + 1];
		p.faces[no_of_faces].vertices[2] = &p.vertices[no_of_vertices-11+j + 1];
		p.faces[no_of_faces].vertices[3] = &p.vertices[no_of_vertices - 11 + j ];
		no_of_faces++;
		}*/
		for (int j = 0; j < 10; j++)
		{
			p.faces[no_of_faces].vertices[0] = no_of_vertices - 2 * 11 + j;
			p.faces[no_of_faces].vertices[1] = no_of_vertices - 2 * 11 + j + 1;
			p.faces[no_of_faces].vertices[2] = no_of_vertices - 11 + j + 1;
			p.faces[no_of_faces].vertices[3] = no_of_vertices - 11 + j;
			no_of_faces++;
		}
	}
	p.nver = no_of_vertices;
	p.nface = no_of_faces;
	std::cout << "No. of vertices=" << p.nver << " and No. of faces=" << p.nface << endl;
	//std::cout << -p.vertices[0].x*sin(10 * (35 + 1)*PI / 180) - p.vertices[0].z*cos(10 * (35 + 1)*PI / 180);
	print_off(p);
}

void print_off(polymesh p)
{
	/* !Generates a OFF file to represent a polygon mesh
	*/
	ofstream f;
	f.open("test.off");
	f << "OFF" << endl;
	f << p.nver << " " << p.nface << " " << "0" << endl;
	for (int i = 0; i < p.nver; i++)
	{
		f << p.vertices[i].x << " " << p.vertices[i].y << " " << p.vertices[i].z << endl;
	}
	for (int i = 0; i < p.nface; i++)
	{
		f << "4 " << p.faces[i].vertices[0] << " " << p.faces[i].vertices[1] << " " << p.faces[i].vertices[2] << " " << p.faces[i].vertices[3] << endl;
	}
	f.close();
}