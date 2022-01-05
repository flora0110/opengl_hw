//texture使用的index: 
//人物0~14號，小女孩15號~30號，主選單49號，死亡選單48號
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Windows.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>

//******************************************
#include <string.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "string"
#include <irrKlang.h>

using namespace std;
#define WIDTH 1600.0f
#define HEIGHT 1024.0f 
#define REDISPLAYTIMERID 1
#define MUSIC_ROTATE 2
//********************************************

// sound
irrklang::ISoundEngine* engine_start = irrklang::createIrrKlangDevice();
irrklang::ISoundEngine* engine_game = irrklang::createIrrKlangDevice();

bool start_menu = true, die_menu = false; int button_select = 0;
//start_menu: 正在顯示選單而非遊戲畫面
bool walk = false, swing_down = false, first_part = true, die = false,unbeat=false;
//swing_down: 擺動時是否在下墜, first_part: 擺動時前半部分, die: 死掉動畫
int swing = 0, die_angle = 0, face_angle = 180, girl_face_angle = 180;
//swing: 走路時，手腳擺動的角度, die_angle: 死掉時，人物倒下的角度
int pass = 0;
int time_count = 100;
static int kill = 0, kill_time = 0;

GLuint texture[70];

/* 貼圖時使用的一些matrix */
static GLfloat n[6][3] = //normal
{
  {-1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {0.0, 0.0, 1.0},
  {0.0, 0.0, -1.0}
};
static GLint faces[6][4] =
{
  {0, 1, 2, 3},
  {3, 2, 6, 7},
  {7, 6, 5, 4},
  {4, 5, 1, 0},
  {5, 6, 2, 1},
  {7, 4, 0, 3}
};

struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char* data;
};
typedef struct Image Image;

/*image***********************************************/
GLuint texture_backgrond[2];

#define checkImageWidth 64
#define checkImageHeight 64

GLubyte checkImage[checkImageWidth][checkImageHeight][3];
void makeCheckImage(void) {
    int i, j, c;
    for (i = 0; i < checkImageWidth; i++) {
        for (j = 0; j < checkImageHeight; j++) {
            c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;
        }
    }
}

int ImageLoad_backgrond(char* filename, Image* image) {
    FILE* file;
    unsigned long size; // size of the image in bytes.
    unsigned long i; // standard counter.
    unsigned short int planes; // number of planes in image (must be 1)
    unsigned short int bpp; // number of bits per pixel (must be 24)
    char temp; // temporary color storage for bgr-rgb conversion.
    // make sure the file is there.
    if ((file = fopen(filename, "rb")) == NULL) {
        printf("File Not Found : %s\n", filename);
        return 0;
    }
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);
    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
        printf("Error reading width from %s.\n", filename);
        return 0;
    }
    //printf("Width of %s: %lu\n", filename, image->sizeX);
    // read the height
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
        printf("Error reading height from %s.\n", filename);
        return 0;
    }
    //printf("Height of %s: %lu\n", filename, image->sizeY);
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;
    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
        printf("Error reading planes from %s.\n", filename);
        return 0;
    }
    if (planes != 1) {
        printf("Planes from %s is not 1: %u\n", filename, planes);
        return 0;
    }
    // read the bitsperpixel
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
        printf("Error reading bpp from %s.\n", filename);
        return 0;
    }
    if (bpp != 24) {
        printf("Bpp from %s is not 24: %u\n", filename, bpp);
        return 0;
    }
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);
    // read the data.
    image->data = (char*)malloc(size);
    if (image->data == NULL) {
        printf("Error allocating memory for color-corrected image data");
        return 0;
    }
    if ((i = fread(image->data, size, 1, file)) != 1) {
        printf("Error reading image data from %s.\n", filename);
        return 0;
    }
    for (i = 0; i < size; i += 3) { // reverse all of the colors. (bgr -> rgb)
        temp = image->data[i];
        image->data[i] = image->data[i + 2];
        image->data[i + 2] = temp;
    }
    // we're done.
    return 1;
}

Image* loadTexture_backgrond() {
    Image* image1;
    // allocate space for texture
    image1 = (Image*)malloc(sizeof(Image));
    if (image1 == NULL) {
        printf("Error allocating space for image");
        exit(0);
    }
    char filename[] = "wallpaper5.bmp";
    if (!ImageLoad_backgrond(filename, image1)) {
        exit(1);
    }
    return image1;
}
/*image****************************************************************/

/* 從texture資料夾 載入bmp圖片 */
int ImageLoad(char* pic, Image* image) {
    FILE* file;
    unsigned long size; // size of the image in bytes.
    unsigned long i; // standard counter.
    unsigned short int planes; // number of planes in image (must be 1)
    unsigned short int bpp; // number of bits per pixel (must be 24)
    char temp; // temporary color storage for bgr-rgb conversion.
    // make sure the file is there.

    char filename[100];
    sprintf(filename, "texture/%s", pic); //加入資料夾的相對路徑

    if ((file = fopen(filename, "rb")) == NULL) {
        printf("File Not Found : %s\n", filename);
        return 0;
    }
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);
    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
        printf("Error reading width from %s.\n", filename);
        return 0;
    }
    //printf("Width of %s: %lu\n", filename, image->sizeX);
    // read the height
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
        printf("Error reading height from %s.\n", filename);
        return 0;
    }
    //printf("Height of %s: %lu\n", filename, image->sizeY);
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;
    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
        printf("Error reading planes from %s.\n", filename);
        return 0;
    }
    if (planes != 1) {
        printf("Planes from %s is not 1: %u\n", filename, planes);
        return 0;
    }
    // read the bitsperpixel
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
        printf("Error reading bpp from %s.\n", filename);
        return 0;
    }
    if (bpp != 24) {
        printf("Bpp from %s is not 24: %u\n", filename, bpp);
        return 0;
    }
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);
    // read the data.
    image->data = (char*)malloc(size);
    if (image->data == NULL) {
        printf("Error allocating memory for color-corrected image data");
        return 0;
    }
    if ((i = fread(image->data, size, 1, file)) != 1) {
        printf("Error reading image data from %s.\n", filename);
        return 0;
    }
    for (i = 0; i < size; i += 3) { // reverse all of the colors. (bgr -> rgb)
        temp = image->data[i];
        image->data[i] = image->data[i + 2];
        image->data[i + 2] = temp;
    }
    // we're done.
    return 1;
}

/* 設定texture的屬性 */
void loadTexture(char* file_name, Image* image, int tex_index) {
    if (!ImageLoad(file_name, image)) {
        exit(1);
    }

    glBindTexture(GL_TEXTURE_2D, texture[tex_index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

/* 畫選單背景 *//////////////////////////////////////////////////////////////////////////////////////////////
static void drawMenuBackground() {
    glBindTexture(GL_TEXTURE_2D, texture[49]);
    glPushMatrix();
        glTranslatef(-380, -220, 0.0);
        glScalef(4, 4, 4);
        glBegin(GL_QUADS);
            glTexCoord2d(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
            glTexCoord2d(0.0, 1.0); glVertex3f(0.0, 108.0, 0.0);
            glTexCoord2d(1.0, 1.0); glVertex3f(192.0, 108.0, 0.0);
            glTexCoord2d(1.0, 0.0); glVertex3f(192.0, 0.0, 0.0);
        glEnd();
        glScalef(0.25, 0.25, 0.25);
    glPopMatrix();
}

/* 畫死掉後的選單logo *//////////////////////////////////////////////////////////////////////////////////////////////////////
static void drawDeadBackground() {
    glBindTexture(GL_TEXTURE_2D, texture[48]);
    glPushMatrix();
        glBegin(GL_QUADS);
            glTexCoord2d(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
            glTexCoord2d(0.0, 1.0); glVertex3f(0.0, 108.0, 0.0);
            glTexCoord2d(1.0, 1.0); glVertex3f(192.0, 108.0, 0.0);
            glTexCoord2d(1.0, 0.0); glVertex3f(192.0, 0.0, 0.0);
        glEnd();
        glScalef(0.25, 0.25, 0.25);
    glPopMatrix();
}

/* 畫贏了後的選單logo *//////////////////////////////////////////////////////////////////////////////////////////////////////
static void drawWinBackground() {
    glBindTexture(GL_TEXTURE_2D, texture[50]);
    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2d(0.0, 1.0); glVertex3f(0.0, 108.0, 0.0);
    glTexCoord2d(1.0, 1.0); glVertex3f(192.0, 108.0, 0.0);
    glTexCoord2d(1.0, 0.0); glVertex3f(192.0, 0.0, 0.0);
    glEnd();
    glScalef(0.25, 0.25, 0.25);
    glPopMatrix();
}

/* 畫頭 */
static void drawHead(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, texture[2]); //right face (model的視角)
            break;
        case 2:
            glBindTexture(GL_TEXTURE_2D, texture[3]); //left face
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[0]); //front face
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, texture[1]); //hair
            break;
        }
        glBegin(type);
        glNormal3fv(&n[i][0]);
        glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}
static void drawHead_girl(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, texture[18]); //right face (model的視角)
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, texture[16]); //right face (model的視角)
            break;
        case 2:
            glBindTexture(GL_TEXTURE_2D, texture[17]); //left face
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[15]); //front face
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, texture[19]); //bottom hair
            break;
        }
        glBegin(type);
        glNormal3fv(&n[i][0]);
        glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}

/* 畫身體 */
static void drawBody(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2; //left width
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2; //right width
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 1.3; //bottom height
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2; //upper height
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 4; //back thickness
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 4; //front thickness

    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[4]); //front body
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, texture[5]); //back body
            break;
        }
        glBegin(type);
        glNormal3fv(&n[i][0]);
        glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}
static void drawBody_girl(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2; //left width
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2; //right width
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 1.3; //bottom height
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2; //upper height
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 4; //back thickness
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 4; //front thickness

    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[20]); //front body
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, texture[30]); //腳底
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, texture[21]); //back body
            break;
        }
        glBegin(type);
        glNormal3fv(&n[i][0]);
        glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}
/* 畫手臂 */
static void drawArm(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 6; //left width
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 6; //right width
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 1.3; //bottom height
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2; //upper height
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 4; //back thickness
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 4; //front thickness

    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, texture[6]); //side arm
            break;
        case 1:
            glBindTexture(GL_TEXTURE_2D, texture[7]); //shoulder
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, texture[10]); //手掌
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[8]); //front arm
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, texture[9]); //back arm
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, texture[5]); //others
            break;
        }
        glBegin(type);
            glNormal3fv(&n[i][0]);
            glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
            glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
            glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
            glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}
static void drawArm_girl(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 6; //left width
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 6; //right width
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 1.3; //bottom height
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2; //upper height
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 4; //back thickness
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 4; //front thickness
 
    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, texture[23]); //side arm
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, texture[25]); //手掌
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[22]); //front arm
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, texture[24]); //back arm
            break;

        default:
            glBindTexture(GL_TEXTURE_2D, texture[23]); //others
            break;
        }
        glBegin(type);
            glNormal3fv(&n[i][0]);
            glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
            glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
            glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
            glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}

/* 畫腿 */
static void drawLeg(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 4; //left width
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 4; //right width
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 1.3; //bottom height
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2; //upper height
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 4; //back thickness
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 4; //front thickness

    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, texture[13]); //side leg
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, texture[14]); //腳底
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[11]); //front leg
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, texture[12]); //back leg
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, texture[5]); //others
            break;
        }
        glBegin(type);
            glNormal3fv(&n[i][0]);
            glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
            glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
            glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
            glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}
static void drawLeg_girl(GLfloat size, GLenum type)
{
    GLfloat v[8][3];
    GLint i;
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 6; //left width
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 6; //right width
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 1.3; //bottom height
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2; //upper height
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 4; //back thickness
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 4; //front thickness
    
    for (i = 5; i >= 0; i--) {
        switch (i) {
        case 2:
            glBindTexture(GL_TEXTURE_2D, texture[28]); //side arm
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, texture[26]); //front leg
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, texture[27]); //back leg
            break;

        default:
            glBindTexture(GL_TEXTURE_2D, texture[29]); //others
            break;
        }

        glBegin(type);
            glNormal3fv(&n[i][0]);
            glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
            glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
            glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
            glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}

/* 指定bmp，載入選單texture */
void setMenuBackground(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "menu_background.bmp");
    loadTexture(file_name, image1, 49); //textureNo 49

    strcpy(file_name, "dead_menu.bmp");
    loadTexture(file_name, image1, 48); //textureNo 49
    strcpy(file_name, "win_page2.bmp");
    loadTexture(file_name, image1, 50); //textureNo 50
}

/* 指定bmp，載入頭部texture */
void setHead(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "face_dead_new.bmp");
    loadTexture(file_name, image1, 0); //textureNo 0

    strcpy(file_name, "hair_new.bmp");
    loadTexture(file_name, image2, 1); //textureNo 1

    strcpy(file_name, "side_face_right_new.bmp");
    loadTexture(file_name, image3, 2); //textureNo 2

    strcpy(file_name, "side_face_left_new.bmp");
    loadTexture(file_name, image4, 3); //textureNo 3
}
void setHead_girl(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));
    Image* image5 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "g_face_new.bmp");
    loadTexture(file_name, image1, 15); 
  
    strcpy(file_name, "g_back_hair_new.bmp");
    loadTexture(file_name, image2, 16);

    strcpy(file_name, "g_side_hair_left_new.bmp");
    loadTexture(file_name, image3, 17); 

    strcpy(file_name, "g_side_hair_right_new.bmp");
    loadTexture(file_name, image4, 18); //textureNo 3

    strcpy(file_name, "g_bottom_hair_new.bmp");
    loadTexture(file_name, image5, 19); //textureNo 3
}

/* 指定bmp，載入身體texture */
void setBody(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "front_body_dead_new.bmp");
    loadTexture(file_name, image1, 4); //textureNo 4

    strcpy(file_name, "back_body_new.bmp");
    loadTexture(file_name, image2, 5); //textureNo 5  
}
void setBody_girl(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "g_front_body_new.bmp");
    loadTexture(file_name, image1, 20); 

    strcpy(file_name, "g_back_body_new.bmp");
    loadTexture(file_name, image2, 21); 

    strcpy(file_name, "g_bottom_body_new.bmp");
    loadTexture(file_name, image3, 30); 
}

/* 指定bmp，載入手臂texture */
void setArm(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));
    Image* image5 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "side_hand_new.bmp");
    loadTexture(file_name, image1, 6); //textureNo 6

    strcpy(file_name, "shoulder_new.bmp");
    loadTexture(file_name, image2, 7); //textureNo 7

    strcpy(file_name, "front_hand_new.bmp");
    loadTexture(file_name, image3, 8); //textureNo 8

    strcpy(file_name, "back_hand_new.bmp");
    loadTexture(file_name, image4, 9); //textureNo 9

    strcpy(file_name, "hand_new.bmp");
    loadTexture(file_name, image5, 10); //textureNo 10
}
/* 指定bmp，載入手臂texture */
void setArm_girl(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));
    Image* image5 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "g_front_hand_new.bmp"); //front and back hand
    loadTexture(file_name, image1, 22); 

    strcpy(file_name, "g_side_hand_new.bmp");
    loadTexture(file_name, image3, 23); //textureNo 8

    strcpy(file_name, "g_back_hand_new.bmp");
    loadTexture(file_name, image4, 24); //textureNo 9

    strcpy(file_name, "g_bottom_hand_new.bmp");
   loadTexture(file_name, image5, 25); //textureNo 10
}

/* 指定bmp，載入腿texture */
void setLeg(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "front_leg_new.bmp");
    loadTexture(file_name, image1, 11); //textureNo 11

    strcpy(file_name, "back_leg_new.bmp");
    loadTexture(file_name, image2, 12); //textureNo 12

    strcpy(file_name, "side_leg_new.bmp");
    loadTexture(file_name, image3, 13); //textureNo 13

    strcpy(file_name, "feet_new.bmp");
    loadTexture(file_name, image4, 14); //textureNo 14
}
void setLeg_girl(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "g_front_leg_new.bmp");
    loadTexture(file_name, image1, 26); //textureNo 11
    
    strcpy(file_name, "g_back_leg_new.bmp");
    loadTexture(file_name, image2, 27); //textureNo 12

    strcpy(file_name, "g_side_leg_new.bmp");
    loadTexture(file_name, image3, 28); //textureNo 13

    strcpy(file_name, "g_side_leg2_new.bmp");
    loadTexture(file_name, image4, 29); //textureNo 14
}

/* 選單的選擇按鈕 */
void draw_button(void) {
    glColor3f(1.0, 0.0, 0.0);
    glTranslatef(-7.0, 2.0, 1.0);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
        glVertex3f(-300, -70, 0); // XYZ left, top
        glVertex3f(-60, -70, 0); // XYZ right, top
        glVertex3f(-60, -120, 0); // XYZ right, bottom
        glVertex3f(-300, -120, 0); // XYZ left, bottom
    glEnd();
}

void myinit(void)
{
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Create Texture
    glGenTextures(50, texture); //total 50 textures
    setHead();

    setBody();
    setArm();
    setLeg();
    setHead_girl();
    setBody_girl();
    setArm_girl();
    setLeg_girl();

    setMenuBackground();

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE); //不要讓glScalef()改到我的法向量

    /*****************************************************/
    glClearColor(0.0, 0.0, 0.0, 0.0);
    //
    GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    //GLfloat diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat position[] = { 0.0, 3.0, 2.0, 0.0 };
    GLfloat lmodel_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
    GLfloat local_view[] = { 0.0 };

    //GL_AMBIENT: 設置環境光
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    //GL_DIFFUSE: 設置漫射光
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    //GL_POSITION: 設置光線位置
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

    /*image******************************************/
    glDepthFunc(GL_LESS);
    Image* image_backgrond = loadTexture_backgrond();
    if (image_backgrond == NULL) {
        printf("Image was not returned from loadTexture\n");
        exit(0);
    }
    makeCheckImage();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Create Texture
    glGenTextures(2, texture_backgrond);
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    //GL_TEXTURE_MIN_FILTER表示設定材質影像需要放大時該如何處理
    //GL_TEXTURE_MAG_FILTER表示設定材質影像需要縮小時該如何處理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image_backgrond->sizeX, image_backgrond->sizeY, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image_backgrond->data);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glEnable(GL_TEXTURE_2D);
    //glShadeModel(GL_FLAT);
    /*image*****************************************/
    /****************************************************/
    
 
    
    
}
int test_time = 0;
void timerFunc(int nTimerID)
{
    switch (nTimerID)
    {
    case REDISPLAYTIMERID:
        if (time_count > 0) {
            time_count--;
            glutPostRedisplay();
            glutTimerFunc(1000, timerFunc, REDISPLAYTIMERID);
        }
        break;
    case MUSIC_ROTATE:
        //小女孩轉動
        printf("killtime %d\n",kill_time);
        if (kill_time <= 30) {
            kill_time++;
        }
        else {
            girl_face_angle = (girl_face_angle + 30);
            if (girl_face_angle >= 359 && kill == 0) { kill = 1; kill_time = 0; }
            else if (girl_face_angle >= 170 && kill == 1) { kill = 0; kill_time = 0; engine_game->play2D("audio/girl_song.mp3", false);
            }
            girl_face_angle = (girl_face_angle) % 360;
            printf("girl_face_angle %d\n", girl_face_angle);
        }
        printf("time %d\n",test_time++);
        glutTimerFunc(100, timerFunc, MUSIC_ROTATE);
        break;
    }
}
void DrawString(string str)
{
    static int isFirstCall = 1;
    static GLuint lists;

    if (isFirstCall)
    {//第一次调用时 为每个ASCII字符生成一个显示列表
        isFirstCall = 0;
        //申请MAX__CHAR个连续的显示列表编号
        lists = glGenLists(128);
        //把每个字符的绘制命令装到对应的显示列表中
        wglUseFontBitmaps(wglGetCurrentDC(), 0, 128, lists);
    }

    //调用每个字符对应的显示列表，绘制每个字符
    for (int i = 0; i < str.length(); i++)
        glCallList(lists + str.at(i));
}

void auto_rotate(void) {
    Sleep(80); //每80ms更新一次此function
    

    //手腳擺動
    if (swing_down) {
        swing = (swing - 10) % 40;
        if (swing == 0) {
            if (first_part == true) first_part = false;
            else first_part = true;

            swing_down = false;
        }
    }
    else {
        swing = (swing + 10) % 40;
        if (swing == 30) {
            swing_down = true;
        }
    }

    //人物死掉向後倒地
    if (die) {
        if (die_angle >= -90) die_angle = die_angle - 15;
        else {
            die_menu = true;
        }
    }

    glutPostRedisplay();
}
void draw_view(int length, int width, int sky_height, int floor_height) {
    /*image************************************/
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    //right wall back
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(width, floor_height, 0);

    glTexCoord2f(0.67, 0.0);
    glVertex3f(width, floor_height, length);

    glTexCoord2f(0.67, 1.0);
    glVertex3f(width, sky_height, length);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(width, sky_height, 0);
    glEnd();
    glPopMatrix();

    //right wall front
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(width, floor_height, length * 2);

    glTexCoord2f(0.67, 0.0);
    glVertex3f(width, floor_height, length);

    glTexCoord2f(0.67, 1.0);
    glVertex3f(width, sky_height, length);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(width, sky_height, length * 2);

    glEnd();
    glPopMatrix();

    //back wall
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-width, floor_height, 0);

    glTexCoord2f(0.6, 0.0);
    glVertex3f(width, floor_height, 0);

    glTexCoord2f(0.6, 1.0);
    glVertex3f(width, sky_height, 0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-width, sky_height, 0);
    glEnd();
    glPopMatrix();

    //left wall back
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-width, floor_height, length);

    glTexCoord2f(0.67, 0.0);
    glVertex3f(-width, floor_height, 0);

    glTexCoord2f(0.67, 1.0);
    glVertex3f(-width, sky_height, 0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-width, sky_height, length);

    glEnd();
    glPopMatrix();

    //left wall front
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-width, floor_height, length);

    glTexCoord2f(0.67, 0.0);
    glVertex3f(-width, floor_height, length * 2);

    glTexCoord2f(0.67, 1.0);
    glVertex3f(-width, sky_height, length * 2);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-width, sky_height, length);

    glEnd();
    glPopMatrix();

    //floor back
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-width, floor_height, 0);

    glTexCoord2f(0.67, 0.0);
    glVertex3f(-width, floor_height, length);

    glTexCoord2f(0.67, 0.3);
    glVertex3f(width, floor_height, length);

    glTexCoord2f(0.0, 0.3);
    glVertex3f(width, floor_height, 0);
    glEnd();
    glPopMatrix();

    //floor front
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_backgrond[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(width, floor_height, length * 2);

    glTexCoord2f(0.67, 0.0);
    glVertex3f(width, floor_height, length);

    glTexCoord2f(0.67, 0.3);
    glVertex3f(-width, floor_height, length);

    glTexCoord2f(0.0, 0.3);
    glVertex3f(-width, floor_height, length * 2);

    glEnd();

    glPopMatrix();
    /*****************************************/
    /* sky */
    glDisable(GL_TEXTURE_2D);
    glColor3f((float)208 / 255, (float)222 / 255, (float)223 / 255);
    //glColor3f(1.0f, 1.0f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-width, sky_height, 0);
    glVertex3f(-width, sky_height, length * 2);
    glVertex3f(width, sky_height, length * 2);
    glVertex3f(width, sky_height, 0);
    glEnd();
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(width, floor_height + 0.001, length - 20);
    glVertex3f(width, floor_height + 0.001, length - 25);
    glVertex3f(-width, floor_height + 0.001, length - 25);
    glVertex3f(-width, floor_height + 0.001, length - 20);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

static int length = 100, width = 50, sky_height = 70, floor_height = -2;
static GLfloat meX = 0.0, meY = 3.0, meZ = length * 2 - 3;
static GLfloat seeX = 0.0, seeY = 0.0, seeZ = -1.0;


void display_start_menu(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();            //clear the matrix

    gluLookAt(0.0, 3.0, length * 2 - 3, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    switch (button_select) {
        case 0:
            glPushMatrix(); //start的框框
                glTranslatef(0.0, 10.0, 0.0);
                draw_button();
            glPopMatrix();
            break;
        case 1:
            glPushMatrix(); //exit的框框
                glTranslatef(0.0, -65.0, 0.0);
                draw_button();
            glPopMatrix();
            break;
    }

    glPushMatrix();
        glEnable(GL_TEXTURE_2D);
            drawMenuBackground();
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glutSwapBuffers();
}

void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i; float PI = 3.1415;
    int triangleAmount = 20; //# of triangles used to draw circle

    //GLfloat radius = 0.8f; //radius
    GLfloat twicePi = 2.0f * PI;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // center of circle
    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(
            x + (radius * cos(i * twicePi / triangleAmount)),
            y + (radius * sin(i * twicePi / triangleAmount))
        );
    }
    glEnd();
}

void display_die_menu(void) {
    if (die_menu == true || pass==1) {
        glPushMatrix();
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            
            if (pass == 1)  glColor4f(0.0f, 0.53725f, 0.63922f, 0.5);
            else glColor4f(1.0f, 0.53725f, 0.63922f, 0.5);
            glTranslatef(meX, meY, meZ);
            glTranslatef(0.0, 0.9 - meY, -10); //初始位置(可改)

            drawFilledCircle(0.0, 0.0, 80.0);

            glDisable(GL_BLEND);
        glPopMatrix();

        glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glTranslatef(meX, meY, meZ);
            glTranslatef(0.0, 0.9 - meY, -10); //初始位置(可改)
            glTranslatef(-6, 4, 1);
            glScalef(0.06, 0.06, 0.06);
            if (pass == 1)  drawWinBackground();
            else drawDeadBackground();

            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
         glPopMatrix();

         glPushMatrix(); 
            char* temp1 = (char*)malloc(5 * sizeof(char));
            char note[100] = "";
            if (!temp1) {
                printf("malloc error!\n"); return;
            }
            
            glTranslatef(meX, meY, meZ);
            glTranslatef(0.0, 0.9 - meY, -10); //初始位置(可改)
            glTranslatef(0, -1.5, 1);
            glColor3f(1.0, 1.0, 1.0);
            strcpy(note, "Press R to recover");
            glRasterPos2f(-3.0, 3.9); //字體位置	
            for (int i = 0; i < (int)strlen(note); i++) { //loop to display character by character
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, note[i]);
            }
            strcpy(note, "Press ESC to main menu");
            glRasterPos2f(-4.0, 3.0); //字體位置	
            for (int i = 0; i < (int)strlen(note); i++) { //loop to display character by character
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, note[i]);
            }
        glPopMatrix();
    }
}

void display(void) {
    if (start_menu) {
        display_start_menu();
    }
    else {
        
        GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
        GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
        GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
        GLfloat mat_diffuse[] = { 0.1, 0.5, 0.8, 1.0 };
        GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat no_shininess[] = { 0.0 };
        GLfloat low_shininess[] = { 5.0 };
        GLfloat high_shininess[] = { 100.0 };
        GLfloat mat_emission[] = { 0.3, 0.2, 0.2, 0.0 };
        //*****************************
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor3f(0, 0, 0);//设置绘图颜色

        //計時***************************************
        glTranslatef(meX, meY - 2, meZ - (length * 2 - 3));
        glRasterPos3f(width / 2, sky_height / 2, length * 1.5);//设置文字位置
        string str(std::to_string(time_count));
        string str1 = "remain time : ";
        string str2 = " s";
        str = str1 + str + str2;
        DrawString(str);
        //********************************************
        glLoadIdentity();            //clear the matrix
        gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);

        //xyz-------------------------
        glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            glColor3f(1.0, 0.0, 0.0);
            glBegin(GL_LINES);
            glVertex2f(0.0, 0.0);
            glVertex2f(0.0, 100.0);
            glEnd();
            glColor3f(0.0, 1.0, 0.0);
            glBegin(GL_LINES);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(100.0, 0.0, 0.0);
            glEnd();
            glColor3f(0.0, 0.0, 1.0);
            glBegin(GL_LINES);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, 0.0, 100.0);
            glEnd();
            glEnable(GL_TEXTURE_2D);
        glPopMatrix();
        // ------------------------xyz*/
        //draw_view
        draw_view(length, width, sky_height, floor_height);
        /***********************************************************/
        glPushMatrix();
            glEnable(GL_TEXTURE_2D);

            glPushMatrix();
                glTranslatef(0, 8.5, 20);
                glRotatef(girl_face_angle, 0.0, 1.0, 0.0);
                //head
                glPushMatrix();
                    glTranslatef(0, 50, 0);
                    drawHead_girl(20.0, GL_QUADS);
                glPopMatrix();
                //body
                glPushMatrix();
                    glTranslatef(0, 30, 0);
                    drawBody_girl(20.0, GL_QUADS);
                glPopMatrix();
                //arm
                glPushMatrix();
                    glTranslatef(-12, 30.0, 0.0);
                    glRotatef(180, 0.0, 1.0, 0.0);
                    drawArm_girl(20.0, GL_QUADS);
                glPopMatrix();
                //arm
                glPushMatrix();
                    glTranslatef(12, 30.0, 0.0);
                    drawArm_girl(20.0, GL_QUADS);
                glPopMatrix();
                //left leg
                glPushMatrix();
                    glTranslatef(-6.0, 4.0, 0.0);
                    glRotatef(180, 0.0, 1.0, 0.0);
                    drawLeg_girl(20.0, GL_QUADS);
                glPopMatrix();
                glPushMatrix();
                    glTranslatef(6.0, 4.0, 0.0);
                    drawLeg_girl(20.0, GL_QUADS);
                glPopMatrix();
            glPopMatrix();

            glPushMatrix();
                glTranslatef(meX, meY, meZ);
                glTranslatef(0.0, 0.9 - meY, -10); //初始位置(可改)
                glRotatef(face_angle, 0.0, 1.0, 0.0);

                if (die) { //死掉了
                    glTranslatef(0.0, -2.0, 0.0);
                    glRotatef(die_angle, 1.0, 0.0, 0.0);
                    glTranslatef(0.0, 3.0, 0.0);
                }

                //head
                glPushMatrix();
                    drawHead(1.0, GL_QUADS);
                glPopMatrix();
                //body
                glPushMatrix();
                    glTranslatef(0.0, -1.0, 0.0);
                    drawBody(1.0, GL_QUADS);
                glPopMatrix();
                //left arm
                glPushMatrix();
                    glTranslatef(-0.65, -1.0, 0.0);
                    if (walk == true) {
                        if (first_part) glRotatef(-swing, 1.0, 0.0, 0.0); //擺動前半部分 往前擺
                        else glRotatef(swing, 1.0, 0.0, 0.0); //擺動後半部分 往後擺
                    }
                    drawArm(1.0, GL_QUADS);
                glPopMatrix();
                //right arm
                glPushMatrix();
                    glTranslatef(0.65, -1.0, 0.0);
                    if (walk == true) {
                        if (first_part) glRotatef(swing, 1.0, 0.0, 0.0); //擺動前半部分 往前擺
                        else glRotatef(-swing, 1.0, 0.0, 0.0); //擺動後半部分 往後擺
                    }
                    glRotatef(180, 0.0, 1.0, 0.0);
                    drawArm(1.0, GL_QUADS);
                glPopMatrix();
                //left leg
                glPushMatrix();
                    glTranslatef(-0.25, -2.25, 0.0);
                    if (walk == true) {
                        glTranslatef(0.0, 1.0, 0.0);
                        if (first_part) glRotatef(swing, 1.0, 0.0, 0.0); //擺動前半部分 往前擺
                        else glRotatef(-swing, 1.0, 0.0, 0.0); //擺動後半部分 往後擺
                        glTranslatef(0.0, -1.0, 0.0);
                    }
                    drawLeg(1.0, GL_QUADS);
                glPopMatrix();
                //right leg
                glPushMatrix();
                    glTranslatef(0.25, -2.25, 0.0);
                    if (walk == true) {
                        glTranslatef(0.0, 1.0, 0.0);
                        if (first_part) glRotatef(-swing, 1.0, 0.0, 0.0); //擺動前半部分 往前擺
                        else glRotatef(swing, 1.0, 0.0, 0.0); //擺動後半部分 往後擺
                        glTranslatef(0.0, -1.0, 0.0);
                    }
                    glRotatef(180, 0.0, 1.0, 0.0);
                    drawLeg(1.0, GL_QUADS);
                glPopMatrix();
            glPopMatrix(); //finish drawing model

            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        if(pass) printf("pass=%d\n", pass);
        display_die_menu(); //若死掉，使用死掉畫面

        glutSwapBuffers();
    }
}

void myReshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (GLfloat)w / (GLfloat)h, 1.0, 700.0);
    glMatrixMode(GL_MODELVIEW);
}

void move(int m) {
    if (kill==1 && die == false && !unbeat) die = true;
    if (time_count <= 0 && !unbeat) die = true;
    if (die == true) return;
    //cout <<"seeX: " << seeX << endl;
   // cout <<"meX: " << meX << endl;
    if (m == 1) {
        if (meZ > 2) {
            printf("at %d %f\n",length, meZ);
            meZ = meZ - 1;
            if (meZ < (length - 10)) pass = 1;
            //seeX = meX;
            //seeZ =meZ - 0.1;
            seeZ = seeZ - 1;
            face_angle = 180;
        }

    }
    else if (m == 2) {
       
        if (meZ < length * 2 - 2) {
            meZ = meZ + 1;
            
            //seeX = meX;
            //seeZ = meZ + 0.1;
            seeZ = seeZ + 1;
            face_angle = 180;
        }
    }
    else if (m == 3) {
        if (meX < width - 2) {
            meX = meX + 1;
            //seeZ = meZ;
            //seeX = meX+ 0.1;
            seeX = seeX + 1;
            face_angle = 90;
        }
    }
    else if (m == 4) {
        if (meX > -width + 2) {
            meX = meX - 1;
            //seeZ = meZ;
            //seeX = meX - 0.1;
            seeX = seeX - 1;
            face_angle = 270;
        }
    }
    glutPostRedisplay();
}

/* 釋放鍵盤時會call此function */
void keyboardUp(unsigned char key, int x, int y) {
    glutIgnoreKeyRepeat(1); //ignore key repeat
    switch (key) {
    case 'w':
    case 'a':
    case 's':
    case 'd':
        //cout << "B" << endl;
        walk = false; //釋放鍵盤時，停止走路
        break;
    case 'P':
    case 'p': //無敵模式
        unbeat = false;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y) {
    glutIgnoreKeyRepeat(0); //do NOT ignore key repeat
    switch (key) {
    case 'w':
    case 'W':
        move(1);
        if (die == false) walk = true;
        break;
    case 's':
    case 'S':
        move(2);
        if (die == false) walk = true;
        break;
    case 'a':
    case 'A':
        move(4);
        if (die == false) walk = true;
        break;
    case 'd':
    case 'D':
        move(3);
        if (die == false) walk = true;
        break;
    case 'R':
    case 'r': //死掉後重生
        if (die) {
            die = false;
            die_menu = false;
            die_angle = 0;
            time_count = 100;
        }
        break;
    case 'P':
    case 'p': //無敵模式
        unbeat = true;
        break;
    case 13: // “enter” on keyboard
        if (start_menu && button_select == 1) { //目前在選單 且選到exit 按下enter
            exit(0);
        }

        if (start_menu) {
            printf("start menu\n");
            time_count = 100;
            start_menu = false;
            engine_start->drop();
            engine_game = irrklang::createIrrKlangDevice();
            engine_game->play2D("audio/girl_song.mp3", false);
            kill_time = 0;
            test_time = 0;
            kill = 0;
        }
        break;
    case 27: // “esc” on keyboard
        start_menu = true; //回到主選單
        die = false;
        die_menu = false;
        printf("esc\n");
        engine_game->drop();
        engine_start = irrklang::createIrrKlangDevice();
        engine_start->play2D("audio/Main_Theme_cut1.mp3", true);
        meX = 0.0, meY = 3.0, meZ = length * 2 - 3; //restart game
        seeX = 0.0, seeY = 0.0, seeZ = -1.0;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void keyboardSpecial(GLint key, GLint x, GLint y) {
    if (key == GLUT_KEY_UP) {
        if (button_select == 0) button_select = 1;
        else button_select--;
    }
    else if (key == GLUT_KEY_DOWN) {
        if (button_select == 1) button_select = 0;
        else button_select++;
    }
    glutPostRedisplay();
}
void mouse(int button, int state, int x, int y)
{
    switch (button)
    {
        /*case GLUT_LEFT_BUTTON: //滑鼠左鍵: 中槍死掉 (暫時的)
            if (state == GLUT_DOWN) {
                if (die == false) die = true;
            }
            break;
        */
    default:
        break;
    }
}

int main(int argc, char** argv)
{
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1200, 650);
    glutCreateWindow("final project");

    glewExperimental = GL_TRUE;
    if (glewInit()) {
        exit(EXIT_FAILURE);
    }

    myinit();
    engine_start->play2D("audio/Main_Theme_cut1.mp3", true);
    printf("main\n");
    glutDisplayFunc(display);
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(keyboardSpecial);
    //glutMouseFunc(mouse);
    glutIdleFunc(auto_rotate);
    glutTimerFunc(1000, timerFunc, REDISPLAYTIMERID);
    glutTimerFunc(100, timerFunc, MUSIC_ROTATE);
    glutMainLoop();

    return 0;
}