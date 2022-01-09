//texture使用的index: 
//人物0~14號、31號，小女孩15號~30號，主選單49號，死亡選單48號，勝利選單50號，教學頁面47號
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Windows.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <string.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "string"
#include <irrKlang.h>
#include <assert.h>
#include "glm.h"
#include <cstdlib> /* 亂數相關函數 */
#include <ctime>   /* 時間相關函數 */
GLMmodel* model;
GLMmodel* star_model = NULL;

/*
     glm.c
     Nate Robins, 1997
     ndr@pobox.com, http://www.pobox.com/~ndr/
     Wavefront OBJ model file format reader/writer/manipulator.
     Includes routines for generating smooth normals with
     preservation of edges, welding redundant vertices & texture
     coordinate generation (spheremap and planar projections) + more.
 */
 //#include "glm.h"
#pragma warning(disable:4996)
#define T(x) (model->triangles[(x)])

 /* _GLMnode: general purpose node
  */
typedef struct _GLMnode {
    GLuint           index;
    GLboolean        averaged;
    struct _GLMnode* next;
} GLMnode;

/* glmMax: returns the maximum of two floats */
static GLfloat
glmMax(GLfloat a, GLfloat b)
{
    if (b > a)
        return b;
    return a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat
glmAbs(GLfloat f)
{
    if (f < 0)
        return -f;
    return f;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLfloat
glmDot(GLfloat* u, GLfloat* v)
{
    assert(u); assert(v);

    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
static GLvoid
glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
    assert(u); assert(v); assert(n);

    n[0] = u[1] * v[2] - u[2] * v[1];
    n[1] = u[2] * v[0] - u[0] * v[2];
    n[2] = u[0] * v[1] - u[1] * v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static GLvoid
glmNormalize(GLfloat* v)
{
    GLfloat l;

    assert(v);

    l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

/* glmEqual: compares two vectors and returns GL_TRUE if they are
 * equal (within a certain threshold) or GL_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLboolean
glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
    if (glmAbs(u[0] - v[0]) < epsilon &&
        glmAbs(u[1] - v[1]) < epsilon &&
        glmAbs(u[2] - v[2]) < epsilon)
    {
        return GL_TRUE;
    }
    return GL_FALSE;
}

/* glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors    - array of GLfloat[3]'s to be welded
 * numvectors - number of GLfloat[3]'s in vectors
 * epsilon    - maximum difference between vectors
 *
 */
GLfloat*
glmWeldVectors(GLfloat* vectors, GLuint* numvectors, GLfloat epsilon)
{
    GLfloat* copies;
    GLuint   copied;
    GLuint   i, j;

    copies = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (*numvectors + 1));
    memcpy(copies, vectors, (sizeof(GLfloat) * 3 * (*numvectors + 1)));

    copied = 1;
    for (i = 1; i <= *numvectors; i++) {
        for (j = 1; j <= copied; j++) {
            if (glmEqual(&vectors[3 * i], &copies[3 * j], epsilon)) {
                goto duplicate;
            }
        }

        /* must not be any duplicates -- add to the copies array */
        copies[3 * copied + 0] = vectors[3 * i + 0];
        copies[3 * copied + 1] = vectors[3 * i + 1];
        copies[3 * copied + 2] = vectors[3 * i + 2];
        j = copied;				/* pass this along for below */
        copied++;

    duplicate:
        /* set the first component of this vector to point at the correct
          index into the new copies array */
        vectors[3 * i + 0] = (GLfloat)j;
    }

    *numvectors = copied - 1;
    return copies;
}

/* glmFindGroup: Find a group in the model
 */
GLMgroup*
glmFindGroup(GLMmodel* model, char* name)
{
    GLMgroup* group;

    assert(model);

    group = model->groups;
    while (group) {
        if (!strcmp(name, group->name))
            break;
        group = group->next;
    }

    return group;
}

/* glmAddGroup: Add a group to the model
 */
GLMgroup*
glmAddGroup(GLMmodel* model, char* name)
{
    GLMgroup* group;

    group = glmFindGroup(model, name);
    if (!group) {
        group = (GLMgroup*)malloc(sizeof(GLMgroup));
        group->name = strdup(name);
        group->material = 0;
        group->numtriangles = 0;
        group->triangles = NULL;
        group->next = model->groups;
        model->groups = group;
        model->numgroups++;
    }

    return group;
}

/* glmFindGroup: Find a material in the model
 */
GLuint
glmFindMaterial(GLMmodel* model, char* name)
{
    GLuint i;

    /* XXX doing a linear search on a string key'd list is pretty lame,
      but it works and is fast enough for now. */
    for (i = 0; i < model->nummaterials; i++) {
        if (!strcmp(model->materials[i].name, name))
            goto found;
    }

    /* didn't find the name, so print a warning and return the default
      material (0). */
    printf("glmFindMaterial():  can't find material \"%s\".\n", name);
    i = 0;

found:
    return i;
}


/* glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * NOTE: the return value should be free'd.
 */
static char*
glmDirName(char* path)
{
    char* dir;
    char* s;

    dir = strdup(path);

    s = strrchr(dir, '/');
    if (s)
        s[1] = '\0';
    else
        dir[0] = '\0';

    return dir;
}


/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
static GLvoid
glmReadMTL(GLMmodel* model, char* name)
{
    FILE* file;
    char* dir;
    char* filename;
    char  buf[128];
    GLuint nummaterials, i;

    dir = glmDirName(model->pathname);
    filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
    strcpy(filename, dir);
    strcat(filename, name);
    free(dir);

    file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "glmReadMTL() failed: can't open material file \"%s\".\n",
            filename);
        exit(1);
    }
    free(filename);

    /* count the number of materials in the file */
    nummaterials = 1;
    while (fscanf(file, "%s", buf) != EOF) {
        switch (buf[0]) {
        case '#':				/* comment */
         /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'n':				/* newmtl */
            fgets(buf, sizeof(buf), file);
            nummaterials++;
            sscanf(buf, "%s %s", buf, buf);
            break;
        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }

    rewind(file);

    model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
    model->nummaterials = nummaterials;

    /* set the default material */
    for (i = 0; i < nummaterials; i++) {
        model->materials[i].name = NULL;
        model->materials[i].shininess = 65.0;
        model->materials[i].diffuse[0] = 0.8;
        model->materials[i].diffuse[1] = 0.8;
        model->materials[i].diffuse[2] = 0.8;
        model->materials[i].diffuse[3] = 1.0;
        model->materials[i].ambient[0] = 0.2;
        model->materials[i].ambient[1] = 0.2;
        model->materials[i].ambient[2] = 0.2;
        model->materials[i].ambient[3] = 1.0;
        model->materials[i].specular[0] = 0.0;
        model->materials[i].specular[1] = 0.0;
        model->materials[i].specular[2] = 0.0;
        model->materials[i].specular[3] = 1.0;
    }
    model->materials[0].name = strdup("default");

    /* now, read in the data */
    nummaterials = 0;
    while (fscanf(file, "%s", buf) != EOF) {
        switch (buf[0]) {
        case '#':				/* comment */
         /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'n':				/* newmtl */
            fgets(buf, sizeof(buf), file);
            sscanf(buf, "%s %s", buf, buf);
            nummaterials++;
            model->materials[nummaterials].name = strdup(buf);
            break;
        case 'N':
            fscanf(file, "%f", &model->materials[nummaterials].shininess);
            /* wavefront shininess is from [0, 1000], so scale for OpenGL */
            model->materials[nummaterials].shininess /= 1000.0;
            model->materials[nummaterials].shininess *= 128.0;
            break;
        case 'K':
            switch (buf[1]) {
            case 'd':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].diffuse[0],
                    &model->materials[nummaterials].diffuse[1],
                    &model->materials[nummaterials].diffuse[2]);
                break;
            case 's':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].specular[0],
                    &model->materials[nummaterials].specular[1],
                    &model->materials[nummaterials].specular[2]);
                break;
            case 'a':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].ambient[0],
                    &model->materials[nummaterials].ambient[1],
                    &model->materials[nummaterials].ambient[2]);
                break;
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            }
            break;
        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }
}

/* glmWriteMTL: write a wavefront material library file
 *
 * model      - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
static GLvoid
glmWriteMTL(GLMmodel* model, char* modelpath, char* mtllibname)
{
    FILE* file;
    char* dir;
    char* filename;
    GLMmaterial* material;
    GLuint i;

    dir = glmDirName(modelpath);
    filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(mtllibname)));
    strcpy(filename, dir);
    strcat(filename, mtllibname);
    free(dir);

    /* open the file */
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "glmWriteMTL() failed: can't open file \"%s\".\n",
            filename);
        exit(1);
    }
    free(filename);

    /* spit out a header */
    fprintf(file, "#  \n");
    fprintf(file, "#  Wavefront MTL generated by GLM library\n");
    fprintf(file, "#  \n");
    fprintf(file, "#  GLM library\n");
    fprintf(file, "#  Nate Robins\n");
    fprintf(file, "#  ndr@pobox.com\n");
    fprintf(file, "#  http://www.pobox.com/~ndr\n");
    fprintf(file, "#  \n\n");

    for (i = 0; i < model->nummaterials; i++) {
        material = &model->materials[i];
        fprintf(file, "newmtl %s\n", material->name);
        fprintf(file, "Ka %f %f %f\n",
            material->ambient[0], material->ambient[1], material->ambient[2]);
        fprintf(file, "Kd %f %f %f\n",
            material->diffuse[0], material->diffuse[1], material->diffuse[2]);
        fprintf(file, "Ks %f %f %f\n",
            material->specular[0], material->specular[1], material->specular[2]);
        fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);
        fprintf(file, "\n");
    }
}

/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid
glmFirstPass(GLMmodel* model, FILE* file)
{
    GLuint    numvertices;		/* number of vertices in model */
    GLuint    numnormals;			/* number of normals in model */
    GLuint    numtexcoords;		/* number of texcoords in model */
    GLuint    numtriangles;		/* number of triangles in model */
    GLMgroup* group;			/* current group */
    unsigned  v, n, t;
    char      buf[128];

    /* make a default group */
    char def[] = "default";
    group = glmAddGroup(model, def);

    numvertices = numnormals = numtexcoords = numtriangles = 0;
    while (fscanf(file, "%s", buf) != EOF) {
        switch (buf[0]) {
        case '#':				/* comment */
         /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':				/* v, vn, vt */
            switch (buf[1]) {
            case '\0':			/* vertex */
           /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numvertices++;
                break;
            case 'n':				/* normal */
           /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numnormals++;
                break;
            case 't':				/* texcoord */
           /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numtexcoords++;
                break;
            default:
                printf("glmFirstPass(): Unknown token \"%s\".\n", buf);
                exit(1);
                break;
            }
            break;
        case 'm':
            fgets(buf, sizeof(buf), file);
            sscanf(buf, "%s %s", buf, buf);
            model->mtllibname = strdup(buf);
            glmReadMTL(model, buf);
            break;
        case 'u':
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'g':				/* group */
         /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
            sscanf(buf, "%s", buf);
#else
            buf[strlen(buf) - 1] = '\0';	/* nuke '\n' */
#endif
            group = glmAddGroup(model, buf);
            break;
        case 'f':				/* face */
            v = n = t = 0;
            fscanf(file, "%s", buf);
            /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
            if (strstr(buf, "//")) {
                /* v//n */
                sscanf(buf, "%d//%d", &v, &n);
                fscanf(file, "%d//%d", &v, &n);
                fscanf(file, "%d//%d", &v, &n);
                numtriangles++;
                group->numtriangles++;
                while (fscanf(file, "%d//%d", &v, &n) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            }
            else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                /* v/t/n */
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                numtriangles++;
                group->numtriangles++;
                while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            }
            else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                /* v/t */
                fscanf(file, "%d/%d", &v, &t);
                fscanf(file, "%d/%d", &v, &t);
                numtriangles++;
                group->numtriangles++;
                while (fscanf(file, "%d/%d", &v, &t) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            }
            else {
                /* v */
                fscanf(file, "%d", &v);
                fscanf(file, "%d", &v);
                numtriangles++;
                group->numtriangles++;
                while (fscanf(file, "%d", &v) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            }
            break;

        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }

    /* set the stats in the model structure */
    model->numvertices = numvertices;
    model->numnormals = numnormals;
    model->numtexcoords = numtexcoords;
    model->numtriangles = numtriangles;

    /* allocate memory for the triangles in each group */
    group = model->groups;
    while (group) {
        group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
        group->numtriangles = 0;
        group = group->next;
    }
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid
glmSecondPass(GLMmodel* model, FILE* file)
{
    GLuint    numvertices;		/* number of vertices in model */
    GLuint    numnormals;			/* number of normals in model */
    GLuint    numtexcoords;		/* number of texcoords in model */
    GLuint    numtriangles;		/* number of triangles in model */
    GLfloat* vertices;			/* array of vertices  */
    GLfloat* normals;			/* array of normals */
    GLfloat* texcoords;			/* array of texture coordinates */
    GLMgroup* group;			/* current group pointer */
    GLuint    material;			/* current material */
    GLuint    v, n, t;
    char      buf[128];

    /* set the pointer shortcuts */
    vertices = model->vertices;
    normals = model->normals;
    texcoords = model->texcoords;
    group = model->groups;

    /* on the second pass through the file, read all the data into the
      allocated arrays */
    numvertices = numnormals = numtexcoords = 1;
    numtriangles = 0;
    material = 0;
    while (fscanf(file, "%s", buf) != EOF) {
        switch (buf[0]) {
        case '#':				/* comment */
         /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':				/* v, vn, vt */
            switch (buf[1]) {
            case '\0':			/* vertex */
                fscanf(file, "%f %f %f",
                    &vertices[3 * numvertices + 0],
                    &vertices[3 * numvertices + 1],
                    &vertices[3 * numvertices + 2]);
                numvertices++;
                break;
            case 'n':				/* normal */
                fscanf(file, "%f %f %f",
                    &normals[3 * numnormals + 0],
                    &normals[3 * numnormals + 1],
                    &normals[3 * numnormals + 2]);
                numnormals++;
                break;
            case 't':				/* texcoord */
                fscanf(file, "%f %f",
                    &texcoords[2 * numtexcoords + 0],
                    &texcoords[2 * numtexcoords + 1]);
                numtexcoords++;
                break;
            }
            break;
        case 'u':
            fgets(buf, sizeof(buf), file);
            sscanf(buf, "%s %s", buf, buf);
            group->material = material = glmFindMaterial(model, buf);
            break;
        case 'g':				/* group */
         /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
            sscanf(buf, "%s", buf);
#else
            buf[strlen(buf) - 1] = '\0';	/* nuke '\n' */
#endif
            group = glmFindGroup(model, buf);
            group->material = material;
            break;
        case 'f':				/* face */
            v = n = t = 0;
            fscanf(file, "%s", buf);
            /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
            if (strstr(buf, "//")) {
                /* v//n */
                sscanf(buf, "%d//%d", &v, &n);
                T(numtriangles).vindices[0] = v;
                T(numtriangles).nindices[0] = n;
                fscanf(file, "%d//%d", &v, &n);
                T(numtriangles).vindices[1] = v;
                T(numtriangles).nindices[1] = n;
                fscanf(file, "%d//%d", &v, &n);
                T(numtriangles).vindices[2] = v;
                T(numtriangles).nindices[2] = n;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while (fscanf(file, "%d//%d", &v, &n) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
                    T(numtriangles).nindices[0] = T(numtriangles - 1).nindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
                    T(numtriangles).nindices[1] = T(numtriangles - 1).nindices[2];
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            }
            else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                /* v/t/n */
                T(numtriangles).vindices[0] = v;
                T(numtriangles).tindices[0] = t;
                T(numtriangles).nindices[0] = n;
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                T(numtriangles).vindices[1] = v;
                T(numtriangles).tindices[1] = t;
                T(numtriangles).nindices[1] = n;
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                T(numtriangles).vindices[2] = v;
                T(numtriangles).tindices[2] = t;
                T(numtriangles).nindices[2] = n;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
                    T(numtriangles).tindices[0] = T(numtriangles - 1).tindices[0];
                    T(numtriangles).nindices[0] = T(numtriangles - 1).nindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
                    T(numtriangles).tindices[1] = T(numtriangles - 1).tindices[2];
                    T(numtriangles).nindices[1] = T(numtriangles - 1).nindices[2];
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            }
            else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                /* v/t */
                T(numtriangles).vindices[0] = v;
                T(numtriangles).tindices[0] = t;
                fscanf(file, "%d/%d", &v, &t);
                T(numtriangles).vindices[1] = v;
                T(numtriangles).tindices[1] = t;
                fscanf(file, "%d/%d", &v, &t);
                T(numtriangles).vindices[2] = v;
                T(numtriangles).tindices[2] = t;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while (fscanf(file, "%d/%d", &v, &t) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
                    T(numtriangles).tindices[0] = T(numtriangles - 1).tindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
                    T(numtriangles).tindices[1] = T(numtriangles - 1).tindices[2];
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            }
            else {
                /* v */
                sscanf(buf, "%d", &v);
                T(numtriangles).vindices[0] = v;
                fscanf(file, "%d", &v);
                T(numtriangles).vindices[1] = v;
                fscanf(file, "%d", &v);
                T(numtriangles).vindices[2] = v;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while (fscanf(file, "%d", &v) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
                    T(numtriangles).vindices[2] = v;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            }
            break;

        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }

#if 0
    /* announce the memory requirements */
    printf(" Memory: %d bytes\n",
        numvertices * 3 * sizeof(GLfloat) +
        numnormals * 3 * sizeof(GLfloat) * (numnormals ? 1 : 0) +
        numtexcoords * 3 * sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
        numtriangles * sizeof(GLMtriangle));
#endif
}

/* public functions */
/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat
glmUnitize(GLMmodel* model)
{
    GLuint  i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    GLfloat cx, cy, cz, w, h, d;
    GLfloat scale;

    assert(model);
    assert(model->vertices);

    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];

        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];

        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }

    /* calculate model width, height, and depth */
    w = glmAbs(maxx) + glmAbs(minx);
    h = glmAbs(maxy) + glmAbs(miny);
    d = glmAbs(maxz) + glmAbs(minz);

    /* calculate center of the model */
    cx = (maxx + minx) / 2.0;
    cy = (maxy + miny) / 2.0;
    cz = (maxz + minz) / 2.0;

    /* calculate unitizing scale factor */
    scale = 2.0 / glmMax(glmMax(w, h), d);

    /* translate around center then scale */
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] -= cx;
        model->vertices[3 * i + 1] -= cy;
        model->vertices[3 * i + 2] -= cz;
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }

    return scale;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;

    assert(model);
    assert(model->vertices);
    assert(dimensions);

    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];

        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];

        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }

    /* calculate model width, height, and depth */
    dimensions[0] = glmAbs(maxx) + glmAbs(minx);
    dimensions[1] = glmAbs(maxy) + glmAbs(miny);
    dimensions[2] = glmAbs(maxz) + glmAbs(minz);
}

/* glmScale: Scales a model by a given amount.
 *
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale)
{
    GLuint i;

    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 *
 * model - properly initialized GLMmodel structure
 */
GLvoid
glmReverseWinding(GLMmodel* model)
{
    GLuint i, swap;

    assert(model);

    for (i = 0; i < model->numtriangles; i++) {
        swap = T(i).vindices[0];
        T(i).vindices[0] = T(i).vindices[2];
        T(i).vindices[2] = swap;

        if (model->numnormals) {
            swap = T(i).nindices[0];
            T(i).nindices[0] = T(i).nindices[2];
            T(i).nindices[2] = swap;
        }

        if (model->numtexcoords) {
            swap = T(i).tindices[0];
            T(i).tindices[0] = T(i).tindices[2];
            T(i).tindices[2] = swap;
        }
    }

    /* reverse facet normals */
    for (i = 1; i <= model->numfacetnorms; i++) {
        model->facetnorms[3 * i + 0] = -model->facetnorms[3 * i + 0];
        model->facetnorms[3 * i + 1] = -model->facetnorms[3 * i + 1];
        model->facetnorms[3 * i + 2] = -model->facetnorms[3 * i + 2];
    }

    /* reverse vertex normals */
    for (i = 1; i <= model->numnormals; i++) {
        model->normals[3 * i + 0] = -model->normals[3 * i + 0];
        model->normals[3 * i + 1] = -model->normals[3 * i + 1];
        model->normals[3 * i + 2] = -model->normals[3 * i + 2];
    }
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmFacetNormals(GLMmodel* model)
{
    GLuint  i;
    GLfloat u[3];
    GLfloat v[3];

    assert(model);
    assert(model->vertices);

    /* clobber any old facetnormals */
    if (model->facetnorms)
        free(model->facetnorms);

    /* allocate memory for the new facet normals */
    model->numfacetnorms = model->numtriangles;
    model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) *
        3 * (model->numfacetnorms + 1));

    for (i = 0; i < model->numtriangles; i++) {
        model->triangles[i].findex = i + 1;

        u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
            model->vertices[3 * T(i).vindices[0] + 0];
        u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
            model->vertices[3 * T(i).vindices[0] + 1];
        u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
            model->vertices[3 * T(i).vindices[0] + 2];

        v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
            model->vertices[3 * T(i).vindices[0] + 0];
        v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
            model->vertices[3 * T(i).vindices[0] + 1];
        v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
            model->vertices[3 * T(i).vindices[0] + 2];

        glmCross(u, v, &model->facetnorms[3 * (i + 1)]);
        glmNormalize(&model->facetnorms[3 * (i + 1)]);
    }
}

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid
glmVertexNormals(GLMmodel* model, GLfloat angle)
{
    GLMnode* node;
    GLMnode* tail;
    GLMnode** members;
    GLfloat* normals;
    GLuint    numnormals;
    GLfloat   average[3];
    GLfloat   dot, cos_angle;
    GLuint    i, avg;

    assert(model);
    assert(model->facetnorms);

    /* calculate the cosine of the angle (in degrees) */
    cos_angle = cos(angle * M_PI / 180.0);

    /* nuke any previous normals */
    if (model->normals)
        free(model->normals);

    /* allocate space for new normals */
    model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
    model->normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numnormals + 1));

    /* allocate a structure that will hold a linked list of triangle
      indices for each vertex */
    members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
    for (i = 1; i <= model->numvertices; i++)
        members[i] = NULL;

    /* for every triangle, create a node for each vertex in it */
    for (i = 0; i < model->numtriangles; i++) {
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next = members[T(i).vindices[0]];
        members[T(i).vindices[0]] = node;

        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next = members[T(i).vindices[1]];
        members[T(i).vindices[1]] = node;

        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next = members[T(i).vindices[2]];
        members[T(i).vindices[2]] = node;
    }

    /* calculate the average normal for each vertex */
    numnormals = 1;
    for (i = 1; i <= model->numvertices; i++) {
        /* calculate an average normal for this vertex by averaging the
          facet normal of every triangle this vertex is in */
        node = members[i];
        if (!node)
            fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
        average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
        avg = 0;
        while (node) {
            /* only average if the dot product of the angle between the two
              facet normals is greater than the cosine of the threshold
              angle -- or, said another way, the angle between the two
              facet normals is less than (or equal to) the threshold angle */
            dot = glmDot(&model->facetnorms[3 * T(node->index).findex],
                &model->facetnorms[3 * T(members[i]->index).findex]);
            if (dot > cos_angle) {
                node->averaged = GL_TRUE;
                average[0] += model->facetnorms[3 * T(node->index).findex + 0];
                average[1] += model->facetnorms[3 * T(node->index).findex + 1];
                average[2] += model->facetnorms[3 * T(node->index).findex + 2];
                avg = 1;			/* we averaged at least one normal! */
            }
            else {
                node->averaged = GL_FALSE;
            }
            node = node->next;
        }

        if (avg) {
            /* normalize the averaged normal */
            glmNormalize(average);

            /* add the normal to the vertex normals list */
            model->normals[3 * numnormals + 0] = average[0];
            model->normals[3 * numnormals + 1] = average[1];
            model->normals[3 * numnormals + 2] = average[2];
            avg = numnormals;
            numnormals++;
        }

        /* set the normal of this vertex in each triangle it is in */
        node = members[i];
        while (node) {
            if (node->averaged) {
                /* if this node was averaged, use the average normal */
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = avg;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = avg;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = avg;
            }
            else {
                /* if this node wasn't averaged, use the facet normal */
                model->normals[3 * numnormals + 0] =
                    model->facetnorms[3 * T(node->index).findex + 0];
                model->normals[3 * numnormals + 1] =
                    model->facetnorms[3 * T(node->index).findex + 1];
                model->normals[3 * numnormals + 2] =
                    model->facetnorms[3 * T(node->index).findex + 2];
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = numnormals;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = numnormals;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = numnormals;
                numnormals++;
            }
            node = node->next;
        }
    }

    model->numnormals = numnormals - 1;

    /* free the member information */
    for (i = 1; i <= model->numvertices; i++) {
        node = members[i];
        while (node) {
            tail = node;
            node = node->next;
            free(tail);
        }
    }
    free(members);

    /* pack the normals array (we previously allocated the maximum
      number of normals that could possibly be created (numtriangles *
      3), so get rid of some of them (usually alot unless none of the
      facet normals were averaged)) */
    normals = model->normals;
    model->normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numnormals + 1));
    for (i = 1; i <= model->numnormals; i++) {
        model->normals[3 * i + 0] = normals[3 * i + 0];
        model->normals[3 * i + 1] = normals[3 * i + 1];
        model->normals[3 * i + 2] = normals[3 * i + 2];
    }
    free(normals);
}


/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmLinearTexture(GLMmodel* model)
{
    GLMgroup* group;
    GLfloat dimensions[3];
    GLfloat x, y, scalefactor;
    GLuint i;

    assert(model);

    if (model->texcoords)
        free(model->texcoords);
    model->numtexcoords = model->numvertices;
    model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (model->numtexcoords + 1));

    glmDimensions(model, dimensions);
    scalefactor = 2.0 /
        glmAbs(glmMax(glmMax(dimensions[0], dimensions[1]), dimensions[2]));

    /* do the calculations */
    for (i = 1; i <= model->numvertices; i++) {
        x = model->vertices[3 * i + 0] * scalefactor;
        y = model->vertices[3 * i + 2] * scalefactor;
        model->texcoords[2 * i + 0] = (x + 1.0) / 2.0;
        model->texcoords[2 * i + 1] = (y + 1.0) / 2.0;
    }

    /* go through and put texture coordinate indices in all the triangles */
    group = model->groups;
    while (group) {
        for (i = 0; i < group->numtriangles; i++) {
            T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
            T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
            T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
        }
        group = group->next;
    }

#if 0
    printf("glmLinearTexture(): generated %d linear texture coordinates\n",
        model->numtexcoords);
#endif
}

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmSpheremapTexture(GLMmodel* model)
{
    GLMgroup* group;
    GLfloat theta, phi, rho, x, y, z, r;
    GLuint i;

    assert(model);
    assert(model->normals);

    if (model->texcoords)
        free(model->texcoords);
    model->numtexcoords = model->numnormals;
    model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (model->numtexcoords + 1));

    for (i = 1; i <= model->numnormals; i++) {
        z = model->normals[3 * i + 0];	/* re-arrange for pole distortion */
        y = model->normals[3 * i + 1];
        x = model->normals[3 * i + 2];
        r = sqrt((x * x) + (y * y));
        rho = sqrt((r * r) + (z * z));

        if (r == 0.0) {
            theta = 0.0;
            phi = 0.0;
        }
        else {
            if (z == 0.0)
                phi = 3.14159265 / 2.0;
            else
                phi = acos(z / rho);

            if (y == 0.0)
                theta = 3.141592365 / 2.0;
            else
                theta = asin(y / r) + (3.14159265 / 2.0);
        }

        model->texcoords[2 * i + 0] = theta / 3.14159265;
        model->texcoords[2 * i + 1] = phi / 3.14159265;
    }

    /* go through and put texcoord indices in all the triangles */
    group = model->groups;
    while (group) {
        for (i = 0; i < group->numtriangles; i++) {
            T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
            T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
            T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
        }
        group = group->next;
    }
}

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model)
{
    GLMgroup* group;
    GLuint i;

    assert(model);

    if (model->pathname)   free(model->pathname);
    if (model->mtllibname) free(model->mtllibname);
    if (model->vertices)   free(model->vertices);
    if (model->normals)    free(model->normals);
    if (model->texcoords)  free(model->texcoords);
    if (model->facetnorms) free(model->facetnorms);
    if (model->triangles)  free(model->triangles);
    if (model->materials) {
        for (i = 0; i < model->nummaterials; i++)
            free(model->materials[i].name);
    }
    free(model->materials);
    while (model->groups) {
        group = model->groups;
        model->groups = model->groups->next;
        free(group->name);
        free(group->triangles);
        free(group);
    }

    free(model);
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel*
glmReadOBJ(char* filename)
{
    GLMmodel* model;
    FILE* file;

    /* open the file */
    file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "glmReadOBJ() failed: can't open data file \"%s\".\n",
            filename);
        exit(1);
    }

    /* allocate a new model */
    model = (GLMmodel*)malloc(sizeof(GLMmodel));
    model->pathname = strdup(filename);
    model->mtllibname = NULL;
    model->numvertices = 0;
    model->vertices = NULL;
    model->numnormals = 0;
    model->normals = NULL;
    model->numtexcoords = 0;
    model->texcoords = NULL;
    model->numfacetnorms = 0;
    model->facetnorms = NULL;
    model->numtriangles = 0;
    model->triangles = NULL;
    model->nummaterials = 0;
    model->materials = NULL;
    model->numgroups = 0;
    model->groups = NULL;
    model->position[0] = 0.0;
    model->position[1] = 0.0;
    model->position[2] = 0.0;

    /* make a first pass through the file to get a count of the number
      of vertices, normals, texcoords & triangles */
    glmFirstPass(model, file);

    /* allocate memory */
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
        3 * (model->numvertices + 1));
    model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
        model->numtriangles);
    if (model->numnormals) {
        model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
            3 * (model->numnormals + 1));
    }
    if (model->numtexcoords) {
        model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
            2 * (model->numtexcoords + 1));
    }

    /* rewind to beginning of file and read in the data this pass */
    rewind(file);

    glmSecondPass(model, file);

    /* close the file */
    fclose(file);

    return model;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode)
{
    GLuint    i;
    FILE* file;
    GLMgroup* group;

    assert(model);

    /* do a bit of warning */
    if (mode & GLM_FLAT && !model->facetnorms) {
        printf("glmWriteOBJ() warning: flat normal output requested "
            "with no facet normals defined.\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_SMOOTH && !model->normals) {
        printf("glmWriteOBJ() warning: smooth normal output requested "
            "with no normals defined.\n");
        mode &= ~GLM_SMOOTH;
    }
    if (mode & GLM_TEXTURE && !model->texcoords) {
        printf("glmWriteOBJ() warning: texture coordinate output requested "
            "with no texture coordinates defined.\n");
        mode &= ~GLM_TEXTURE;
    }
    if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
        printf("glmWriteOBJ() warning: flat normal output requested "
            "and smooth normal output requested (using smooth).\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_COLOR && !model->materials) {
        printf("glmWriteOBJ() warning: color output requested "
            "with no colors (materials) defined.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_MATERIAL && !model->materials) {
        printf("glmWriteOBJ() warning: material output requested "
            "with no materials defined.\n");
        mode &= ~GLM_MATERIAL;
    }
    if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
        printf("glmWriteOBJ() warning: color and material output requested "
            "outputting only materials.\n");
        mode &= ~GLM_COLOR;
    }


    /* open the file */
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "glmWriteOBJ() failed: can't open file \"%s\" to write.\n",
            filename);
        exit(1);
    }

    /* spit out a header */
    fprintf(file, "#  \n");
    fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
    fprintf(file, "#  \n");
    fprintf(file, "#  GLM library\n");
    fprintf(file, "#  Nate Robins\n");
    fprintf(file, "#  ndr@pobox.com\n");
    fprintf(file, "#  http://www.pobox.com/~ndr\n");
    fprintf(file, "#  \n");

    if (mode & GLM_MATERIAL && model->mtllibname) {
        fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
        glmWriteMTL(model, filename, model->mtllibname);
    }

    /* spit out the vertices */
    fprintf(file, "\n");
    fprintf(file, "# %d vertices\n", model->numvertices);
    for (i = 1; i <= model->numvertices; i++) {
        fprintf(file, "v %f %f %f\n",
            model->vertices[3 * i + 0],
            model->vertices[3 * i + 1],
            model->vertices[3 * i + 2]);
    }

    /* spit out the smooth/flat normals */
    if (mode & GLM_SMOOTH) {
        fprintf(file, "\n");
        fprintf(file, "# %d normals\n", model->numnormals);
        for (i = 1; i <= model->numnormals; i++) {
            fprintf(file, "vn %f %f %f\n",
                model->normals[3 * i + 0],
                model->normals[3 * i + 1],
                model->normals[3 * i + 2]);
        }
    }
    else if (mode & GLM_FLAT) {
        fprintf(file, "\n");
        fprintf(file, "# %d normals\n", model->numfacetnorms);
        for (i = 1; i <= model->numnormals; i++) {
            fprintf(file, "vn %f %f %f\n",
                model->facetnorms[3 * i + 0],
                model->facetnorms[3 * i + 1],
                model->facetnorms[3 * i + 2]);
        }
    }

    /* spit out the texture coordinates */
    if (mode & GLM_TEXTURE) {
        fprintf(file, "\n");
        fprintf(file, "# %d texcoords\n", model->texcoords);
        for (i = 1; i <= model->numtexcoords; i++) {
            fprintf(file, "vt %f %f\n",
                model->texcoords[2 * i + 0],
                model->texcoords[2 * i + 1]);
        }
    }

    fprintf(file, "\n");
    fprintf(file, "# %d groups\n", model->numgroups);
    fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
    fprintf(file, "\n");

    group = model->groups;
    while (group) {
        fprintf(file, "g %s\n", group->name);
        if (mode & GLM_MATERIAL)
            fprintf(file, "usemtl %s\n", model->materials[group->material].name);
        for (i = 0; i < group->numtriangles; i++) {
            if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).nindices[0],
                    T(group->triangles[i]).tindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).nindices[1],
                    T(group->triangles[i]).tindices[1],
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).nindices[2],
                    T(group->triangles[i]).tindices[2]);
            }
            else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).findex);
            }
            else if (mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).tindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).tindices[1],
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).tindices[2]);
            }
            else if (mode & GLM_SMOOTH) {
                fprintf(file, "f %d//%d %d//%d %d//%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).nindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).nindices[1],
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).nindices[2]);
            }
            else if (mode & GLM_FLAT) {
                fprintf(file, "f %d//%d %d//%d %d//%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).findex);
            }
            else {
                fprintf(file, "f %d %d %d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).vindices[2]);
            }
        }
        fprintf(file, "\n");
        group = group->next;
    }

    fclose(file);
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmDraw(GLMmodel* model, GLuint mode)
{
    static GLuint i;
    static GLMgroup* group;
    static GLMtriangle* triangle;
    static GLMmaterial* material;

    assert(model);
    assert(model->vertices);

    /* do a bit of warning */
    if (mode & GLM_FLAT && !model->facetnorms) {
        printf("glmDraw() warning: flat render mode requested "
            "with no facet normals defined.\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_SMOOTH && !model->normals) {
        printf("glmDraw() warning: smooth render mode requested "
            "with no normals defined.\n");
        mode &= ~GLM_SMOOTH;
    }
    if (mode & GLM_TEXTURE && !model->texcoords) {
        printf("glmDraw() warning: texture render mode requested "
            "with no texture coordinates defined.\n");
        mode &= ~GLM_TEXTURE;
    }
    if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
        printf("glmDraw() warning: flat render mode requested "
            "and smooth render mode requested (using smooth).\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_COLOR && !model->materials) {
        printf("glmDraw() warning: color render mode requested "
            "with no materials defined.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_MATERIAL && !model->materials) {
        printf("glmDraw() warning: material render mode requested "
            "with no materials defined.\n");
        mode &= ~GLM_MATERIAL;
    }
    if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
        printf("glmDraw() warning: color and material render mode requested "
            "using only material mode.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_COLOR)
        glEnable(GL_COLOR_MATERIAL);
    else if (mode & GLM_MATERIAL)
        glDisable(GL_COLOR_MATERIAL);

    /* perhaps this loop should be unrolled into material, color, flat,
      smooth, etc. loops?  since most cpu's have good branch prediction
      schemes (and these branches will always go one way), probably
      wouldn't gain too much?  */

    group = model->groups;
    while (group) {
        if (mode & GLM_MATERIAL) {
            material = &model->materials[group->material];
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
        }

        if (mode & GLM_COLOR) {
            glColor3fv(material->diffuse);
        }

        glBegin(GL_TRIANGLES);
        for (i = 0; i < group->numtriangles; i++) {
            triangle = &T(group->triangles[i]);

            if (mode & GLM_FLAT)
                glNormal3fv(&model->facetnorms[3 * triangle->findex]);

            if (mode & GLM_SMOOTH)
                glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
            if (mode & GLM_TEXTURE)
                glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
            glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);

            if (mode & GLM_SMOOTH)
                glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
            if (mode & GLM_TEXTURE)
                glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
            glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);

            if (mode & GLM_SMOOTH)
                glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
            if (mode & GLM_TEXTURE)
                glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
            glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);

        }
        glEnd();

        group = group->next;
    }
}

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 * GLM_FLAT and GLM_SMOOTH should not both be specified.  */
GLuint
glmList(GLMmodel* model, GLuint mode)
{
    GLuint list;

    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glmDraw(model, mode);
    glEndList();

    return list;
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid
glmWeld(GLMmodel* model, GLfloat epsilon)
{
    GLfloat* vectors;
    GLfloat* copies;
    GLuint   numvectors;
    GLuint   i;

    /* vertices */
    numvectors = model->numvertices;
    vectors = model->vertices;
    copies = glmWeldVectors(vectors, &numvectors, epsilon);

#if 0
    printf("glmWeld(): %d redundant vertices.\n",
        model->numvertices - numvectors - 1);
#endif

    for (i = 0; i < model->numtriangles; i++) {
        T(i).vindices[0] = (GLuint)vectors[3 * T(i).vindices[0] + 0];
        T(i).vindices[1] = (GLuint)vectors[3 * T(i).vindices[1] + 0];
        T(i).vindices[2] = (GLuint)vectors[3 * T(i).vindices[2] + 0];
    }

    /* free space for old vertices */
    free(vectors);

    /* allocate space for the new vertices */
    model->numvertices = numvectors;
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
        3 * (model->numvertices + 1));

    /* copy the optimized vertices into the actual vertex list */
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] = copies[3 * i + 0];
        model->vertices[3 * i + 1] = copies[3 * i + 1];
        model->vertices[3 * i + 2] = copies[3 * i + 2];
    }

    free(copies);
}


#if 0
/* normals */
if (model->numnormals) {
    numvectors = model->numnormals;
    vectors = model->normals;
    copies = glmOptimizeVectors(vectors, &numvectors);

    printf("glmOptimize(): %d redundant normals.\n",
        model->numnormals - numvectors);

    for (i = 0; i < model->numtriangles; i++) {
        T(i).nindices[0] = (GLuint)vectors[3 * T(i).nindices[0] + 0];
        T(i).nindices[1] = (GLuint)vectors[3 * T(i).nindices[1] + 0];
        T(i).nindices[2] = (GLuint)vectors[3 * T(i).nindices[2] + 0];
    }

    /* free space for old normals */
    free(vectors);

    /* allocate space for the new normals */
    model->numnormals = numvectors;
    model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
        3 * (model->numnormals + 1));

    /* copy the optimized vertices into the actual vertex list */
    for (i = 1; i <= model->numnormals; i++) {
        model->normals[3 * i + 0] = copies[3 * i + 0];
        model->normals[3 * i + 1] = copies[3 * i + 1];
        model->normals[3 * i + 2] = copies[3 * i + 2];
    }

    free(copies);
}

/* texcoords */
if (model->numtexcoords) {
    numvectors = model->numtexcoords;
    vectors = model->texcoords;
    copies = glmOptimizeVectors(vectors, &numvectors);

    printf("glmOptimize(): %d redundant texcoords.\n",
        model->numtexcoords - numvectors);

    for (i = 0; i < model->numtriangles; i++) {
        for (j = 0; j < 3; j++) {
            T(i).tindices[j] = (GLuint)vectors[3 * T(i).tindices[j] + 0];
        }
    }

    /* free space for old texcoords */
    free(vectors);

    /* allocate space for the new texcoords */
    model->numtexcoords = numvectors;
    model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
        2 * (model->numtexcoords + 1));

    /* copy the optimized vertices into the actual vertex list */
    for (i = 1; i <= model->numtexcoords; i++) {
        model->texcoords[2 * i + 0] = copies[2 * i + 0];
        model->texcoords[2 * i + 1] = copies[2 * i + 1];
    }

    free(copies);
}
#endif

#if 0
/* look for unused vertices */
/* look for unused normals */
/* look for unused texcoords */
for (i = 1; i <= model->numvertices; i++) {
    for (j = 0; j < model->numtriangles; i++) {
        if (T(j).vindices[0] == i ||
            T(j).vindices[1] == i ||
            T(j).vindices[1] == i)
            break;
    }
}
#endif

using namespace std;
#define WIDTH 1600.0f
#define HEIGHT 1024.0f 
#define REDISPLAYTIMERID 1
#define MUSIC_ROTATE 2
#define GIRL_WAIT_TIME 20 //小女孩轉動間格(秒)
#define GAME_START_TIME 30 //遊戲時間(秒)
#define SHIELD_TIME 5 //防護罩秒數
#define PICK_LEN 20
#define STAR_TIME 5//吃到星星 增加遊戲時間的秒數
#define BUTTON_NUM 3

// sound
irrklang::ISoundEngine* engine_start = irrklang::createIrrKlangDevice();
irrklang::ISoundEngine* engine_game = irrklang::createIrrKlangDevice();
irrklang::ISoundEngine* engine_gun = irrklang::createIrrKlangDevice();

bool start_menu = true, die_menu = false, guide=false; int button_select = 0;
//start_menu: 正在顯示選單而非遊戲畫面
bool walk = false, swing_down = false, first_part = true, die = false, unbeat = false, pick = false, pick_star=false;
//swing_down: 擺動時是否在下墜, first_part: 擺動時前半部分, die: 死掉動畫
int swing = 0, die_angle = 0, face_angle = 180, girl_face_angle = 180;
//swing: 走路時，手腳擺動的角度, die_angle: 死掉時，人物倒下的角度
int pass = 0;
int time_count = GAME_START_TIME;
static int kill = 0, kill_time = 0, shoot = 0;//0 不射 1 要射 2 射完
bool turn_to_kill = true;
int time_x, time_y, time_z;
static int length = 100, width = 50, sky_height = 70, floor_height = -2;
static GLfloat meX = 0.0, meY = 3.0, meZ = length * 2 - 3;
static GLfloat seeX = 0.0, seeY = 0.0, seeZ = -1.0;
static GLfloat shield_1X, shield_1Y, shield_1Z, shield_2X, shield_2Y, shield_2Z, shield_3X, shield_3Y, shield_3Z;
static GLfloat star_1X, star_1Y, star_1Z, star_2X, star_2Y, star_2Z, star_3X, star_3Y, star_3Z;
bool shield1 = 0, shield2 = 0, shield3 = 0, star1=true, star2=true, star3=true, star_add_time=false;
int shield1_t = 0, shield2_t = 0, shield3_t = 0, star_t=0;
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
GLuint texture_shield[5];

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
    unsigned long size; // size of the image in bytes.loadTexture(file_name, image1
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
    //printf("%s index %d\n",file_name,tex_index);
    glBindTexture(GL_TEXTURE_2D, texture[tex_index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

/* 畫選單背景 */
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

/* 畫死掉後的選單logo */
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

/* 畫贏了後的選單logo */
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
        case 2:
            glBindTexture(GL_TEXTURE_2D, texture[31]); //back leg
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

/* 畫盾牌 */
static void DrawShield(GLfloat size, GLenum type)
{
    //char* file_name = (char*)malloc(100 * sizeof(char));
    //Image* image1 = (Image*)malloc(sizeof(Image));
    //Image* image2 = (Image*)malloc(sizeof(Image));
    //Image* image3 = (Image*)malloc(sizeof(Image));
    //Image* image4 = (Image*)malloc(sizeof(Image));

    //strcpy(file_name, "hair_new.bmp");
    //loadTexture(file_name, image1, 51); //textureNo 
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(type);
    //glNormal3fv(&n[i][0]);
    glTexCoord2d(0.0, 0.0); glVertex3f(0.8, 3.6, -10);
    glTexCoord2d(0.0, 1.0); glVertex3f(0.8, 2, -10);
    glTexCoord2d(1.0, 1.0); glVertex3f(-0.8, 2, -10);
    glTexCoord2d(1.0, 0.0); glVertex3f(-0.8, 3.6, -10);
    glEnd();

    //strcpy(file_name, "back_body_gray2.bmp");
    //loadTexture(file_name, image2, 52); //textureNo 0
    glBindTexture(GL_TEXTURE_2D, texture[51]);
    glBegin(type);
    //glNormal3fv(&n[i][0]);
    glTexCoord2d(0.0, 0.0); glVertex3f(+1, 2, -10);
    glTexCoord2d(0.0, 1.0); glVertex3f(+1, 0, -10);
    glTexCoord2d(1.0, 1.0); glVertex3f(-1, 0, -10);
    glTexCoord2d(1.0, 0.0);  glVertex3f(-1, 2, -10);
    glEnd();

    //strcpy(file_name, "front_hand_gray2.bmp");
    //loadTexture(file_name, image3, 53); //textureNo 0
    glBindTexture(GL_TEXTURE_2D, texture[52]);
    glBegin(type);
    //glNormal3fv(&n[i][0]);
    glTexCoord2d(0.0, 1.0); glVertex3f(+1, 2, -10);
    glTexCoord2d(0.0, 0.0); glVertex3f(+1, 0.2, -10);
    glTexCoord2d(1.0, 0.0); glVertex3f(+1.4, 0.2, -10);
    glTexCoord2d(1.0, 1.0); glVertex3f(+1.4, 2, -10);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, texture[52]);
    glBegin(type);
    //glNormal3fv(&n[i][0]);
    glTexCoord2d(0.0, 1.0); glVertex3f(-1, 2, -10);
    glTexCoord2d(0.0, 0.0); glVertex3f(-1, 0.2, -10);
    glTexCoord2d(1.0, 0.0); glVertex3f(-1.4, 0.2, -10);
    glTexCoord2d(1.0, 1.0); glVertex3f(-1.4, 2, -10);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(type);
    //glNormal3fv(&n[i][0]);
    glTexCoord2d(0.0, 0.0); glVertex3f(0.2, -2, -10);
    glTexCoord2d(0.0, 1.0); glVertex3f(0.2, 0, -10);
    glTexCoord2d(1.0, 1.0); glVertex3f(-0.2, 0, -10);
    glTexCoord2d(1.0, 0.0);  glVertex3f(-0.2, -2, -10);
    glEnd();




    /*
    glBindTexture(GL_TEXTURE_2D, texture[54]);
    glBegin(type);
    //glNormal3fv(&n[i][0]);
    glTexCoord2d(0.0, 0.0); glVertex3fv(&v[faces[i][0]][0]);
    glTexCoord2d(0.0, 1.0); glVertex3fv(&v[faces[i][1]][0]);
    glTexCoord2d(1.0, 1.0); glVertex3fv(&v[faces[i][2]][0]);
    glTexCoord2d(1.0, 0.0); glVertex3fv(&v[faces[i][3]][0]);
    glEnd();*/
}
void setShield(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));


    strcpy(file_name, "back_body_gray2.bmp");
    loadTexture(file_name, image1, 51);
    strcpy(file_name, "front_hand_gray2.bmp");
    loadTexture(file_name, image1, 52);
}

/* 指定bmp，載入選單texture */
void setMenuBackground(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "how_to_play.bmp");
    loadTexture(file_name, image4, 47); //textureNo 47
    strcpy(file_name, "dead_menu.bmp");
    loadTexture(file_name, image3, 48); //textureNo 48
    strcpy(file_name, "menu_background.bmp");
    loadTexture(file_name, image1, 49); //textureNo 49
    strcpy(file_name, "win_page2.bmp");
    loadTexture(file_name, image2, 50); //textureNo 50
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
    Image* image5 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "front_leg_new.bmp");
    loadTexture(file_name, image1, 11); //textureNo 11

    strcpy(file_name, "back_leg_new.bmp");
    loadTexture(file_name, image2, 12); //textureNo 12

    strcpy(file_name, "side_leg_new.bmp");
    loadTexture(file_name, image3, 13); //textureNo 13

    strcpy(file_name, "feet_new.bmp");
    loadTexture(file_name, image4, 14); //textureNo 14

    strcpy(file_name, "side_leg2_new.bmp");
    loadTexture(file_name, image5, 31); //textureNo 31
}
void setLeg_girl(void) {
    char* file_name = (char*)malloc(100 * sizeof(char));
    Image* image1 = (Image*)malloc(sizeof(Image));
    Image* image2 = (Image*)malloc(sizeof(Image));
    Image* image3 = (Image*)malloc(sizeof(Image));
    Image* image4 = (Image*)malloc(sizeof(Image));

    strcpy(file_name, "g_front_leg_new.bmp");
    loadTexture(file_name, image1, 26); 

    strcpy(file_name, "g_back_leg_new.bmp");
    loadTexture(file_name, image2, 27); 

    strcpy(file_name, "g_side_leg_new.bmp");
    loadTexture(file_name, image3, 28); 

    strcpy(file_name, "g_side_leg2_new.bmp");
    loadTexture(file_name, image4, 29); 
}

/* 選單的選擇按鈕 */
void draw_button(void) {
    glColor3f(0.39215, 0.584313, 0.92941);
    glTranslatef(-7.0, 2.0, 1.0);
    glLineWidth(8);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-313, -80, 0); // XYZ left, top
    glVertex3f(-50, -80, 0); // XYZ right, top
    glVertex3f(-50, -125, 0); // XYZ right, bottom
    glVertex3f(-313, -125, 0); // XYZ left, bottom
    glEnd();
}

void myinit(void)
{
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Create Texture
    glGenTextures(70, texture); //total 70 textures
    setHead();

    setBody();
    setArm();
    setLeg();
    setHead_girl();
    setBody_girl();
    setArm_girl();
    setLeg_girl();

    setMenuBackground();
    setShield();

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

    /***************************************************************/

    /*image*****************************************/
    /****************************************************/


    //cout << "off" << endl;
   

}


void timerFunc(int nTimerID)
{
    //printf("start_menu %b\n",start_menu);
   //if (start_menu) printf("a\n");
    //else printf("b\n");
    switch (nTimerID)
    {
    case REDISPLAYTIMERID:
        // printf("time count %d\n",time_count);
         //printf("1 : %d %b\n",shield1_t,shield1);
         //printf("2 : %d %b\n", shield2_t, shield2);
         //printf("3 : %d %b\n", shield3_t, shield3);
        if (time_count > 0 && !pass && !start_menu) {
            time_count--;
            glutPostRedisplay();
            /* shield time countdown */
            if (shield1_t > 0) {
                shield1_t--;
                if (shield1_t == 0) {
                    shield1 = false;
                    shield1_t = -1;
                }
            }
            if (shield2_t > 0) {
                shield2_t--;
                if (shield2_t == 0) {
                    shield2 = false;
                    shield2_t = -1;
                }
            }
            if (shield3_t > 0) {
                shield3_t--;
                if (shield3_t == 0) {
                    shield3 = false;
                    shield3_t = -1;
                }
            }
        }
        glutTimerFunc(1000, timerFunc, REDISPLAYTIMERID);
        break;
    case MUSIC_ROTATE:
        //小女孩轉動
        //printf("killtime %d\n",kill_time);
        if (!start_menu && button_select==0) {
            if (kill_time <= GIRL_WAIT_TIME) {
                //printf("killtim %d\n", kill_time);
                kill_time++;
            }
            else {
                //printf("wait %d\n",kill_time);
                //printf("origin girl_face_angle %d\n", girl_face_angle);
                girl_face_angle = (girl_face_angle + 30);
                if (kill == 1) {
                    turn_to_kill = false;
                    girl_face_angle = (girl_face_angle + 30);
                    kill = 0;
                }
                
                //printf("after girl_face_angle %d\n", girl_face_angle);
                if (girl_face_angle >= 359 && turn_to_kill) { kill = 1; kill_time = 0; }
                else if (girl_face_angle >= 170 && !turn_to_kill) {
                    kill = 0; kill_time = 0; engine_game->play2D("audio/girl_song.mp3", false);
                    turn_to_kill = true;
                }
                girl_face_angle = (girl_face_angle) % 360;
                //printf("girl_face_angle %d\n", girl_face_angle);
            }
        }
        //printf("time %d\n",test_time++);
        //printf("b\n");
        glutPostRedisplay();
        //printf("bbb\n");
        glutTimerFunc(100, timerFunc, MUSIC_ROTATE);
        //printf("bbbbbb\n");
        break;
    }
}
/*void DrawString(string str)
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
}*/

void DrawString2(string str)
{
    //char note[100] = "";
    //strcpy(note, str);
    //glRasterPos2f(-3.0, 3.9); //字體位置	
    //glColor3f(0.0, 0.0, 0.0);
    glPushMatrix();
    for (int i = 0; i < str.size(); i++) { //loop to display character by character
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
    }
    glPopMatrix();
}
void auto_rotate(void) {
    Sleep(80); //每80ms更新一次此function
    
    if (time_count <= 0 && !unbeat) {
        die = true;
        //printf("tiem_count %d\n", time_count);
        //printf("3 die %d \n", pass, die_menu);
    }
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
    //人物死掉
    if (die && shoot == 0) {
        shoot = 1;
    }
    if (shoot==1) {
        engine_gun->play2D("audio/shoot2.mp3", false);
        shoot = 2;
    }
    //向後倒地
    else if (shoot == 2) {
        if (die_angle > -90) die_angle = die_angle - 10;
        else {
            die_menu = true;
            walk = false;
            //printf("4 die %d\n", die_menu);
        }
    }

    /* shield collision detection */
    if (pick) {
        //printf("pick\n");
        //printf("meZ : %f\n", meZ);
        if (shield1_t == 0) {
            //printf("1 : %f %f\n", abs(shield_1X - meX), shield_1Z);
            if (abs(shield_1X - meX) < 3 && shield_1Z > (meZ - PICK_LEN) && meZ > shield_1Z) {
                //printf("1 ok\n");
                shield1_t = SHIELD_TIME;
                shield1 = true;
            }
        }
        if (shield2_t == 0) {
            //printf("2 : %f %f\n", abs(shield_2X - meX), shield_2Z);
            if (abs(shield_2X - meX) < 3 && shield_2Z > (meZ - PICK_LEN) && meZ > shield_2Z) {
                //printf("2 ok\n");
                shield2_t = SHIELD_TIME;
                shield2 = true;
            }
        }
        if (shield3_t == 0) {
            //printf("3 : %f %f\n", abs(shield_3X - meX), shield_3Z);
            if (abs(shield_3X - meX) < 3 && shield_3Z > (meZ - PICK_LEN) && meZ > shield_3Z) {
                //printf("3 ok\n");
                shield3_t = SHIELD_TIME;
                shield3 = true;
            }
        }
    }

    /* star collision detection */
    if (pick_star) {
        if (abs(star_1X - meX) < 3 && star_1Z > (meZ - PICK_LEN) && meZ > star_1Z) {
            //printf("star 1 ok\n");
            if (star1) {
                star1 = false;
                time_count += STAR_TIME;
                star_add_time = true;
            }
        }

        if (abs(star_2X - meX) < 3 && star_2Z > (meZ - PICK_LEN) && meZ > star_2Z) {
            //printf("star 2 ok\n");
            if (star2) {
                star2 = false;
                time_count += STAR_TIME;
                star_add_time = true;
            }
        }

        if (abs(star_3X - meX) < 3 && star_3Z > (meZ - PICK_LEN) && meZ > star_3Z) {
            //printf("star 3 ok\n");
            if (star3) {
                star3 = false;
                time_count += STAR_TIME;
                star_add_time = true;
            }
        }
    }

    star_t = (star_t+1) % 17;
    if (star_t == 16) {
        star_add_time = false;
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
    /*line*/
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
        glTranslatef(0.0, 65.0, 0.0);
        draw_button();
        glPopMatrix();
        break;
    case 1:
        glPushMatrix(); //how to play的框框
        glTranslatef(0.0, 10.0, 0.0);
        draw_button();
        glPopMatrix();
        break;
    case 2:
        glPushMatrix(); //exit的框框
        glTranslatef(0.0, -45.0, 0.0);
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

void display_guide_menu(void) {
    if (guide) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();            //clear the matrix

        gluLookAt(0.0, 3.0, length * 2 - 3, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glPushMatrix(); //畫盾牌logo
            glEnable(GL_TEXTURE_2D);
            glTranslatef(-6, -2, 190);
            //glScalef(0.4, 0.4, 0.4);
            DrawShield(100.0, GL_QUADS);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix(); //畫背景
            glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texture[47]);
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
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix(); //畫星星logo
            glEnable(GL_CULL_FACE);
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);

            if (!star_model) {
                char filename[] = "Gold_Star.obj";
                star_model = glmReadOBJ(filename);
                if (!star_model) exit(0);
                glmUnitize(star_model);
                glmFacetNormals(star_model);
                glmVertexNormals(star_model, 90.0);
            }
 
            glPushMatrix();
                glTranslatef(-120, 30, 0);
                glColor3f(1.0, 0.84313, 0);
                glScalef(50, 50, 50);
                glRotatef(90, 0.0, 1.0, 0.0);
                glmDraw(star_model, GLM_SMOOTH);
            glPopMatrix();

            glEnable(GL_TEXTURE_2D);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHT0);
            glDisable(GL_NORMALIZE);
            glDisable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
        glPopMatrix();

        glutSwapBuffers();
    }
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
    //printf("pass %d die %d \n",pass, die_menu);
    if (die_menu == true || pass == 1) {
        glPushMatrix();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        if (pass == 1)  glColor4f(0.0f, 0.53725f, 0.63922f, 0.5);
        else glColor4f(0.54509f, 0.0f, 0.0f, 0.5);
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
        if (!pass) {
            strcpy(note, "Press R to recover");
            glRasterPos2f(-3.0, 3.9); //字體位置	
            for (int i = 0; i < (int)strlen(note); i++) { //loop to display character by character
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, note[i]);
            }
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
    else if (guide) {
        display_guide_menu();
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
        glLoadIdentity();            //clear the matrix
        gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
        glColor3f(0, 0, 0);
        //計時***************************************
        glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

            if (star_add_time) {
                glColor3f(1, 0, 0); //成功吃到星星 文字顏色變紅色做提醒
            }
            else {
                glColor3f(0, 0, 0);
            }

            glTranslatef(0.0, 0.0, 5.0);
          
            glTranslatef(meX, meY, meZ); //設文字位置
            glTranslatef(0.0, 0.9 - meY, -10); 
            glTranslatef(0, -1.5, 1);
           
            glRasterPos2f(-1.0, 7.0);

            string str(std::to_string(time_count));
            string str1 = "remain time : ";
            string str2 = " s";
            str = str1 + str + str2;
            DrawString2(str);
           
            
            glColor3f(0, 0, 0);
            if (shield3) {
                string str(std::to_string(shield3_t));
                string str1 = "unbeatable time : ";
                string str2 = " s";
                str = str1 + str + str2;
                glRasterPos2f(-1.15, 6.5);
                DrawString2(str);
            }
            else if (shield2) {
                string str(std::to_string(shield2_t));
                string str1 = "unbeatable time : ";
                string str2 = " s";
                str = str1 + str + str2;
                DrawString2(str);
            }
            else if (shield1) {
                string str(std::to_string(shield1_t));
                string str1 = "unbeatable time : ";
                string str2 = " s";
                str = str1 + str + str2;
                DrawString2(str);
            }
            
            
            glEnable(GL_TEXTURE_2D);
            
            
        glPopMatrix();
        //********************************************
        //draw_view
        glEnable(GL_TEXTURE_2D);
        draw_view(length, width, sky_height, floor_height);

        //xyz-------------------------
        /*glPushMatrix();
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
        glPopMatrix();*/
        // ------------------------xyz*/

        /* 畫盾牌 */
        if (unbeat || shield1 || shield2 || shield3) {
            glPushMatrix();
            glTranslatef(meX, 0, meZ);
            DrawShield(20.0, GL_QUADS);
            glPopMatrix();
        }
        if (shield1_t == 0) {
            glPushMatrix();
            glTranslatef(shield_1X, shield_1Y, shield_1Z);
            // glTranslatef(0,0,180);
            DrawShield(20.0, GL_QUADS);
            glPopMatrix();
        }
        if (shield2_t == 0) {
            glPushMatrix();
            glTranslatef(shield_2X, shield_2Y, shield_2Z);
            //glTranslatef(0,0,90);
            DrawShield(20.0, GL_QUADS);
            glPopMatrix();
        }
        if (shield3_t == 0) {
            glPushMatrix();
            glTranslatef(shield_3X, shield_3Y, shield_3Z);
            //glTranslatef(0,0,195);
            DrawShield(20.0, GL_QUADS);
            glPopMatrix();
        }
 
        /* 畫星星 */
        glPushMatrix();
            if (star1 || star2 || star3) {
                glEnable(GL_CULL_FACE);
                glEnable(GL_LIGHT0);
                glEnable(GL_NORMALIZE);
                glEnable(GL_COLOR_MATERIAL);
                glEnable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);

                if (!star_model) {
                    char filename[] = "Gold_Star.obj";
                    star_model = glmReadOBJ(filename);
                    if (!star_model) exit(0);
                    glmUnitize(star_model);
                    glmFacetNormals(star_model);
                    glmVertexNormals(star_model, 90.0);
                }

                if (star1) {
                    glPushMatrix();
                    glColor3f(1.0, 0.84313, 0);
                    glTranslatef(star_1X, star_1Y, star_1Z);
                    glScalef(2, 2, 2);
                    glRotatef(90, 0.0, 1.0, 0.0);
                    glmDraw(star_model, GLM_SMOOTH);
                    glPopMatrix();
                }

                if(star2){
                    glPushMatrix();
                    glColor3f(1.0, 0.84313, 0);
                    glTranslatef(star_2X, star_2Y, star_2Z);
                    glScalef(2, 2, 2);
                    glRotatef(90, 0.0, 1.0, 0.0);
                    glmDraw(star_model, GLM_SMOOTH);
                    glPopMatrix();
                }
                
                if (star3) {
                    glPushMatrix();
                    glColor3f(1.0, 0.84313, 0);
                    glTranslatef(star_3X, star_3Y, star_3Z);
                    glScalef(2, 2, 2);
                    glRotatef(90, 0.0, 1.0, 0.0);
                    glmDraw(star_model, GLM_SMOOTH);
                    glPopMatrix();
                }

                glEnable(GL_TEXTURE_2D);
                glDisable(GL_CULL_FACE);
                glDisable(GL_LIGHT0);
                glDisable(GL_NORMALIZE);
                glDisable(GL_COLOR_MATERIAL);
                glDisable(GL_LIGHTING);
            }
        glPopMatrix();
 
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

        //if (pass) printf("pass=%d\n", pass);
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
    if (kill == 1 && die == false && !(unbeat || shield1 || shield2 || shield3) && !pass) { die = true; 
    }
    if (unbeat && time_count == 0) { //雖然有拿無敵盾牌 但timeout，結束遊戲
        die = true;
        //printf("1 die %d\n", die_menu);
    }

    if (die == true) return;
    //cout <<"seeX: " << seeX << endl;
   // cout <<"meX: " << meX << endl;
    if (m == 1) {
        if (meZ > 2) {
            //printf("at %d %f\n",length, meZ);
            meZ = meZ - 1;
            time_z -= 1;
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
            time_z += 1;
            face_angle = 180;
        }
    }
    else if (m == 3) {
        if (meX < width - 2) {
            meX = meX + 1;
            //seeZ = meZ;
            //seeX = meX+ 0.1;
            seeX = seeX + 1;
            time_x += 1;
            face_angle = 90;
        }
    }
    else if (m == 4) {
        if (meX > -width + 2) {
            meX = meX - 1;
            time_x -= 1;
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
    case 'F':
    case 'f': //pick
        pick = false;
        pick_star = false;
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
        if (die == false) walk = true;
        move(1);
        break;
    case 's':
    case 'S':
        
        if (die == false) walk = true;
        move(2);
        break;
    case 'a':
    case 'A':
        
        if (die == false) walk = true;
        move(4);
        break;
    case 'd':
    case 'D':
        
        if (die == false) walk = true;
        move(3);
        break;
    case 'R':
    case 'r': //死掉後重生
        if (die) {
            die = false;
            die_menu = false;
            die_angle = 0;
            time_count = GAME_START_TIME;
            engine_gun->stopAllSounds();
            engine_game->play2D("audio/girl_song.mp3", false);
            //shield1 = false; shield2 = false; shield3 = false;
            //shield1_t = 0; shield2_t = 0; shield3_t = 0;
            star1 = true; star2 = true; star3 = true;
            meX = 0.0, meY = 3.0, meZ = length * 2 - 3; //restart game
            seeX = 0.0, seeY = 0.0, seeZ = -1.0;
            face_angle = 180;
            girl_face_angle = 180;
            /* shield position *////////////////////////////////////////////////////////////
            shield_1X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            shield_1Z = ((40) * rand() / (RAND_MAX + 1.0)) + 100;

            shield_2X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            shield_2Z = ((40) * rand() / (RAND_MAX + 1.0)) + 130;

            shield_3X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;

            shield_3Z = ((35) * rand() / (RAND_MAX + 1.0)) + 152;

            /* star position *////////////////////////////////////////////////////////////
            int ltime = time(NULL);
            int stime = (unsigned)ltime / 2;
            srand(stime);
            star_1X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            star_1Z = ((40) * rand() / (RAND_MAX + 1.0)) + 100;

            star_2X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            star_2Z = ((40) * rand() / (RAND_MAX + 1.0)) + 130;

            star_3X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            star_3Z = ((35) * rand() / (RAND_MAX + 1.0)) + 152;

            shield1 = false;
            shield2 = false;
            shield3 = false;
            shield1_t = 0;
            shield2_t = 0;
            shield3_t = 0;
            star1 = true;
            star2 = true;
            star3 = true;
            kill_time = 0;
            kill = 0;
            shoot = 0;
            turn_to_kill = true;
            die = false;
            die_menu = false;
        }
        break;
    case 'P':
    case 'p': //無敵模式
        unbeat = true;
        break;
    case 'F':
    case 'f': //pick
        pick = true;
        pick_star = true;

        
        break;
    case 13: // “enter” on keyboard
        //printf("button_select=%d\n", button_select);
        if (start_menu && button_select == (BUTTON_NUM-1)) { //目前在選單 且選到exit 按下enter
            exit(0);
        }
        else if (start_menu && button_select == 1) { //介紹頁面
        //////////////////////////////////////////////////////////////////////////////////////////          
            guide = true; start_menu = false;
        }
        else if (start_menu && button_select==0) {
            //printf("start menu\n");
            time_count = GAME_START_TIME;
            start_menu = false;
            //engine_start->drop();
            engine_start->stopAllSounds();
            engine_gun->stopAllSounds();
            //engine_game = irrklang::createIrrKlangDevice();
            //printf("play girl \n");
            engine_game->play2D("audio/girl_song.mp3", false);
            
            

            face_angle = 180;
            girl_face_angle = 180;
            pass = 0;
            srand(time(NULL));
            /* shield position *////////////////////////////////////////////////////////////
            shield_1X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            shield_1Z = ((40) * rand() / (RAND_MAX + 1.0)) + 100;

            shield_2X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            shield_2Z = ((40) * rand() / (RAND_MAX + 1.0)) + 130;

            shield_3X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            
            shield_3Z = ((35) * rand() / (RAND_MAX + 1.0)) + 152;
            
            /* star position *////////////////////////////////////////////////////////////
            int ltime = time(NULL);
            int stime = (unsigned)ltime / 2;
            srand(stime);
            star_1X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            star_1Z = ((40) * rand() / (RAND_MAX + 1.0)) + 100;

            star_2X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            star_2Z = ((40) * rand() / (RAND_MAX + 1.0)) + 130;

            star_3X = (width * 1.9) * rand() / (RAND_MAX + 1.0) - width * 0.95;
            star_3Z = ((35) * rand() / (RAND_MAX + 1.0)) + 152;
           
            shield1 = 0;
            shield2 = 0;
            shield3 = 0;
            shield1_t = 0;
            shield2_t = 0;
            shield3_t = 0;
            star1 = true;
            star2 = true;
            star3 = true;
          
            kill = 0;
            shoot = 0;
            kill_time = 0;
            turn_to_kill = true;
            die = false;
            die_menu = false;
        }
        break;
    case 'b':
        guide = false; start_menu = true;
        break;
    case '+':
    case '=':
        time_count += 1;
        break;
    case '-':
        time_count -= 1;
        break;
    case 27: // “esc” on keyboard
        start_menu = true; //回到主選單
        die = false;
        die_menu = false;

        //printf("esc\n");
        //printf("pass %d die %d \n", pass, die_menu);
        //engine_game->drop();
        engine_game->stopAllSounds();
        //engine_start = irrklang::createIrrKlangDevice();
        engine_start->play2D("audio/Main_Theme_cut1.mp3", true);


        meX = 0.0, meY = 3.0, meZ = length * 2 - 3; //restart game
        seeX = 0.0, seeY = 0.0, seeZ = -1.0;
        face_angle = 180;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}
void keyboardSpecial(GLint key, GLint x, GLint y) {
    if (key == GLUT_KEY_UP) {
        if (button_select == 0) button_select = (BUTTON_NUM - 1);
        else button_select--;
    }
    else if (key == GLUT_KEY_DOWN) {
        if (button_select == (BUTTON_NUM-1)) button_select = 0;
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
    //time_x = width / 2-2;
    time_x = -15;
    time_y = sky_height / 2;
    time_z = length * 1.5;
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
    //printf("main\n");
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