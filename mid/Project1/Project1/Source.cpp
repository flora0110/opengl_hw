#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <GL/glut.h>
using namespace std;


#define WIDTH 1600.0f
#define HEIGHT 1024.0f
void init(void) {
    glClearColor(0.0,0.0,0.0,0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[] = { -1.0, 3.0, 2.0, 0.0 };
    GLfloat lmodel_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
    GLfloat local_view[] = { 0.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
}

void reshape(int w, int h)
{
    //glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    /*int x = 0;
    int y = 0;
    int ViewWidth = w;
    int ViewHeight = h;

    if (ViewWidth > ViewHeight)
    {
	   x = (int)((ViewWidth - ViewHeight) / 2);
	   ViewWidth = ViewHeight;

    }
    else
    {
	   y = (int)((ViewHeight - ViewWidth) / 2);
	   ViewHeight = ViewWidth;
    }
    glViewport(x, y, ViewWidth, ViewHeight);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);*/
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (GLfloat)w / (GLfloat)h, 1.0, 700.0);
    glMatrixMode(GL_MODELVIEW);
}
void DrawTree(float cx, float cy, float r, int num_segments) {
    
    glColor3f(94.0 / 255.0, 38.0 / 255.0, 18.0 / 255.0);
    glBegin(GL_POLYGON);
	   glVertex3f(0.0,cy+r*0.5,0.0001);
	   glVertex3f(cx-r*0.1, cy - r * 2,0.0001);
	   glVertex3f(cx + r * 0.1, cy - r * 2, 0.0001);
    glEnd();
    glBegin(GL_POLYGON);
	   glVertex3f(0.0, cy - r * 0.65, 0.0001);
	   glVertex3f(cx - r * 0.3, cy - r * 0.3, 0.0001);
	   glVertex3f(0.0, cy - r * 0.8, 0.0001);
    glEnd();
    glBegin(GL_POLYGON);
	   glVertex3f(0.0, cy - r * 0.67, 0.0001);
	   glVertex3f(cx + r * 0.5, cy, 0.0001);
	   glVertex3f(0.0, cy - r * 0.55, 0.0001);
    glEnd();
    glColor3f(34.0 / 255.0, 139.0 / 255.0, 34.0 / 255.0);
    glBegin(GL_POLYGON);
    for (int ii = 0; ii < num_segments; ii++) {
	   float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
	   float x = r * cosf(theta);//calculate the x component 
	   float y = r * sinf(theta);//calculate the y component 
	   glVertex3f(x + cx, y + cy,0.0);//output vertex 
    }
    glEnd();
}


static GLfloat meX = 0.0, meY = 0.0, meZ = 2.0;
static GLfloat seeX = 0.0, seeY = 0.0, seeZ = -1.0;
//static int year = 0, day = 0;
static int spin = 0.0;
static int spin2 = 0.0;
float rotateX = 0.0, rotateY = 0.0;
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//*****************************
	GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat body_color[] = { 245.0 / 255.0, 118.0 / 255.0, 39.0 / 255.0 };
	//GLfloat head_color[] = { 1.0,1.0,1.0 };
	GLfloat island_color[] = { 100.0 / 255.0, 227.0 / 255.0, 27.0 / 255.0 };

	GLfloat mat_diffuse_test[] = { 51.0 / 255.0, 52.0 / 255.0, 54.0 / 255.0 };
	GLfloat mat_diffuse_black[] = { 0.04,0.04,0.04 };
	GLfloat no_shininess[] = { 0.0 };
	GLfloat low_shininess[] = { 100.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_emission_test[] = { 192.0 / 255.0, 192.0 / 255.0, 192.0 / 255.0 };
	GLfloat mat_emission_black[] = { 0,0,0 };
	//*****************************
	glColor3f(0.0, 0.0, 0.0);
	glPushMatrix();//lookat
	glLoadIdentity();            //clear the matrix
	gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
	glRotatef(rotateX, 0.0, 1.0, 0.0);
	glRotatef(rotateY, 0.0, 0.0, 1.0);

	glPushMatrix();
	gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex2f(0.0, 0.0);
	glVertex2f(0.0, 10.0);
	glEnd();
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(10.0, 0.0, 0.0);
	glEnd();
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 10.0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	    glPushMatrix();//tree1
		  glTranslatef(0.5, 0.0, 0.0);
		  DrawTree(0, 0, 0.4, 100);
	    glPopMatrix();

	    glPushMatrix();//tree2
		   glTranslatef(-0.8, 0.0, 0.2);
		   DrawTree(0, 0, 0.4, 100);
	    glPopMatrix();

	   glTranslatef(meX, meY-0.8, meZ - 2);
	   glPushMatrix();//bb
		  glEnable(GL_LIGHTING);
		  glEnable(GL_LIGHT0);

		  glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
		  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		  glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
		  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_test);
		  glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_test);
		  //body
		  glPushMatrix();
			 glRotatef((GLfloat)spin, 0.0, 1.0, 0.0);
			 glutSolidSphere(0.3, 32, 32);
			 glMaterialfv(GL_FRONT, GL_DIFFUSE, body_color);
			 glMaterialfv(GL_FRONT, GL_EMISSION, body_color);
			 glTranslatef(0.0, 0.0, -0.12);
			 glutSolidSphere(0.2, 32, 32);
		  glPopMatrix();


		  //head
		  glPushMatrix();
			 
			 glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_black);
			 glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_test);
			 glTranslatef(0.0,0.2,0.1);
			 glRotatef((GLfloat)spin2, 0.0, 1.0, 0.0);
			 //glRotatef(30, 1.0, 0.0, 0.0);
			 GLdouble eqn[4] = { 0.0, 1.0, 0.0, 0.0 };
			 glClipPlane(GL_CLIP_PLANE0, eqn);
			 glEnable(GL_CLIP_PLANE0);
			 glutSolidSphere(0.2, 32, 32);
			 glDisable(GL_CLIP_PLANE0);

			 glTranslatef(0.0, 0.07, 0.01);
			 glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_black);
			 glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_black);
			 glTranslatef(0.0, 0.0, 0.12);
			 glutSolidSphere(0.06, 32, 32);
		  glPopMatrix();
		  /*/test
		  glPushMatrix();
			 glMaterialfv(GL_FRONT, GL_DIFFUSE, island_color);
			 glMaterialfv(GL_FRONT, GL_EMISSION, island_color);
			 glRotatef(30, 1.0, 0.0, 0.0);
			 glutSolidCube(1);
		  glPopMatrix();
		  //test*/
		  glDisable(GL_LIGHTING);
		  glDisable(GL_LIGHT0);
	   glPopMatrix();
	glPopMatrix();

	glutSwapBuffers();
}
static int can_spin = 0;
static int dir = 0;

void move(int m) {
    if (m == 1) {
	   meZ = meZ - 0.02;
	   seeZ = seeZ - 0.02;
	   dir = 1;
    }
    else if (m == 2) {
	   meZ = meZ + 0.02;
	   seeZ = seeZ + 0.02;
	   dir = 2;
    }
    else if (m == 3) {
	   meX = meX + 0.02;
	   seeX = seeX + 0.02;
	   dir = 3;
    }
    else if (m == 4) {
	   meX = meX - 0.02;
	   seeX = seeX - 0.02;
	   dir = 4;
    }
    glutPostRedisplay();
}
static int state = 0;
static int headstate = 0;
void jump(void) {
    if (headstate == 1) {
	   spin2 = (spin2 + 4) % 360;
	   glutPostRedisplay();
    }
    if (can_spin == 1) {
	   if ((dir == 1 && spin != 0) || (dir == 2 && spin != 180) || (dir == 4 && spin != 90) || (dir == 3 && spin != 270)) {
		  spin = (spin + 9) % 360;
	   }

	   if ((dir == 1 && spin == 0) || (dir == 2 && spin == 180) || (dir == 4 && spin == 90) || (dir == 3 && spin == 270)) {
		  can_spin = 0;
	   }
    }
    //cout << "j" << endl;
    if (state == 1) {
	   //cout << "up" << endl;
	   meY = meY + 0.01;
	   seeY = seeY + 0.01;
	   if (meY > 0.8) {
		  state = 2;//jump down
	   }
    }
    if (state == 2) {
	   //cout << "down" << endl;
	   meY = meY - 0.01;
	   seeY = seeY - 0.01;
	   if (meY < 0.01) {
		  state = 0;//not jumping
	   }
    }

    glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'w':
    case 'W':
	   can_spin = 1;
	   move(1);
	   
	   break;
    case 's':
    case 'S':
	   can_spin = 1;
	   move(2);
	  
	   break;
    case 'a':
    case 'A':
	   can_spin = 1;
	   move(4);
	   
	   break;
    case 'd':
    case 'D':
	   can_spin = 1;
	   move(3);
	   
	   break;
    case ' ':
	   state = 1;//jump up
	   break;
    case 27:
	   exit(0);
	   break;
    }
}
void catchKey(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT) {
	   seeX = seeX - 0.02;
	   rotateX--;
    }
    else if (key == GLUT_KEY_RIGHT) {
	   seeX = seeX + 0.02;
	   rotateX++;
    }
	  
    else if (key == GLUT_KEY_DOWN) {
	   seeY = seeY - 0.02;
	   rotateY--;
    }
	   
    else if (key == GLUT_KEY_UP) {
	   seeY = seeY + 0.02;
	   rotateY++;
    }
}


static int  oldmy = 0, oldmx = 0;
void mouse(int button, int state, int x, int y) {
    switch (button) {
    case GLUT_MIDDLE_BUTTON:
	   if (headstate==0) {
		  if (state == GLUT_DOWN)
			 headstate = 1;
	   }
	   else {
		  if (state == GLUT_DOWN)
			 headstate = 0;
	   }
	   break;
    case GLUT_LEFT_BUTTON:
    case GLUT_RIGHT_BUTTON:
	   if (state == GLUT_DOWN) //第一次滑鼠按下時,記錄滑鼠在視窗中的初始座標
		  oldmx = x, oldmy = y;
    default:
	   break;
    }

}


void onMouseMove(int x, int y) //滑鼠拖動
{
    if (x > oldmx) {
	   seeX = seeX + 0.01;
	   rotateX = rotateX+0.5;
	   oldmx = x;
    }
    else if (x < oldmx) {
	   seeX = seeX - 0.01;
	   rotateX = rotateX - 0.5;
	   oldmx = x;
    }
    if (y > oldmy) {
	   seeY = seeY - 0.01;
	   rotateY = rotateY - 0.5;
	   oldmy = y;
    }
    else if (y < oldmy) {
	   seeY = seeY + 0.01;
	   rotateY = rotateY + 0.5;
	   oldmy = y;
    }
    glutPostRedisplay();
}
int main(int argc, char* argv[]) {

	// Initialize GLUT
	glutInit(&argc, argv);
	// Set up some memory buffers for our display
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	// Set the window size
	glutInitWindowSize(WIDTH,HEIGHT);
	glutInitWindowPosition(100, 100);
	// Create the window with the title "Hello,GL"
	glutCreateWindow("modterm");

	init();
	// Bind the two functions (above) to respond when necessary
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(catchKey);
	//glutIdleFunc(jump);
	//glutIdleFunc(spinDisplay);
	glutMouseFunc(mouse);
	//glutIdleFunc(headspin);
	glutMotionFunc(onMouseMove);
	glutIdleFunc(jump);
	//glutMouseFunc(Mouse);
	// Very important!  This initializes the entry points in the OpenGL driver so we can 
	// call all the functions in the API.
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}


	glutMainLoop();
	return 0;
}