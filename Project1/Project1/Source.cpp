/***************************
4108056029 王傳馨 第2II次作業10/16
***************************/

#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

#define WIDTH 1920.0f
#define HEIGHT 1080.0f
class NCHU {
protected:
    float startx, starty;
    void draw_ch(int(*data)[2], int len) {
	   glBegin(GL_POLYGON);
	   for (int i = 0; i < len; i++) {
		  glVertex2f(((data[i][0] - startx) / startx), ((starty - data[i][1]) / starty));
	   }
	   glEnd();
    }
public:
    NCHU(float width, float height) {
	   startx = width / 2;
	   starty = height / 2;
    }
};
class A:protected NCHU{
    private:
	   //A
	   int a_5[8][2] = {{73, 629},{86, 611},{120,607},{139,605},{171,606}, {170,617},{138,618},{119,619},};
	   int a_4[7][2] = { {94,601},{142,579},{143,567},{125,572},{103,580},{89,589},{87,597} ,};
	   int a_3[8][2] = { {129,541},{144,537},{153,554},{161,576},{173,591},{169,602},{155,587} ,{148,577} };
	   int a_three[2][3][2] = {
		  {{178,621},{178,645},{168,622},},
		  {{119,558},{125,572},{103,580}, },
	   };
	   int a_four[10][4][2] = {
		  {{73, 519},{92, 532},{86, 611},{73, 629},},
		  {{92,532},{83,566},{98,555},{99,542}},
		  {{98,555},{99,542},{128,534},{129,541}},
		  {{128,534},{129,541},{144,537},{140,529}},
		  {{144,537},{140,529},{149,527},{166,535}},
		  {{128,534},{140,529},{140,518},{127,522}},
		  {{112,559},{119,558,},{103,580},{99,575}},
		  {{119,558},{135,542},{139,554},{125,572}},
		  {{158,541},{167,553},{161,576},{153,554}},
		  {{148,577},{155,587},{138,605},{115,606}},
	   };
	   int a_five[4][5][2] = {
		  {{161,499},{180,498},{176,511},{92, 532},{73, 519},},
		  {{180,498},{176,511},{178,621},{178,645},{189,628},},
		  {{149,527},{166,535},{166,529},{158,512},{146,516}},
		  {{84,584},{109,550},{129,540},{112,559},{99,575}},
	   };

    public:
	   A(float width, float height):NCHU(width, height){
	   }
	   void display_ch() {
		  glColor3f(0.007, 0.4375, 0.589);
		  draw_ch(a_3, sizeof(a_3) / sizeof(a_3[0]));
		  draw_ch(a_4, sizeof(a_4) / sizeof(a_4[0]));
		  draw_ch(a_5, sizeof(a_5) / sizeof(a_5[0]));
		  for (int i = 0; i < 2; i++) {
			 draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
		  }
		  for (int i = 0; i < 10;i++) {
			 draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
		  }
		  for (int i = 0; i < 4; i++) {
			 draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
		  }
	   }
};



void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glShadeModel(GL_FLAT);
}

void display(void) {
    A a(WIDTH,HEIGHT);
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, WIDTH, HEIGHT);
    glOrtho(0.0f, WIDTH, 0.0f, HEIGHT, 0.0f, 0.0f);
    a.display_ch();
    glutSwapBuffers();
}

void reshape(int Width, int Height) {
    glViewport(0, 0, WIDTH, HEIGHT);
    glOrtho(0.0f, WIDTH, 0.0f, HEIGHT, 0.0f, 0.0f);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowPosition(100, 100);	//set the position of Window
    glutInitWindowSize(1920, 1080);		//set the size of Window
    glutCreateWindow("NCHU");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}





