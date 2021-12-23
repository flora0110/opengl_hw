//Jeff Chastine
#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

using namespace std;

#include <GL\glut.h>
#include "glm.h"


#include <cstdlib> /* 亂數相關函數 */
#include <ctime>   /* 時間相關函數 */
GLMmodel* model;

/*
	 glm.c
	 Nate Robins, 1997
	 ndr@pobox.com, http://www.pobox.com/~ndr/

	 Wavefront OBJ model file format reader/writer/manipulator.

	 Includes routines for generating smooth normals with
	 preservation of edges, welding redundant vertices & texture
	 coordinate generation (spheremap and planar projections) + more.

 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

#define WIDTH 1600.0f
#define HEIGHT 1024.0f
GLMmodel* pmodel = NULL;
GLMmodel* pmodel2 = NULL;
GLMmodel* pmodel3 = NULL;
const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };
//*****************************
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_diffuse_test[] = { 51.0 / 255.0, 52.0 / 255.0, 54.0 / 255.0 };
GLfloat mat_diffuse_black[] = { 0.04,0.04,0.04 };
GLfloat no_shininess[] = { 0.0 };
GLfloat mat_emission_test[] = { 51.0 / 255.0, 52.0 / 255.0, 54.0 / 255.0 };
GLfloat mat_emission_black[] = { 0,0,0 };
//*****************************
float sX, sY, mX, mY;
static GLfloat meX = 0.0, meY = 0.0, meZ = 3.0, fly = 0;
static GLfloat seeX = 0.0, seeY = 0.0, seeZ = -1.0;
static GLfloat myHP = 2.0,M1_HP=0.5,M2_HP=0.5,M3_HP=0.5;
static GLfloat M1_X = 0.0, M1_Y = 0.0, M1_Z = 0.0;
static GLfloat M2_X = 1.0, M2_Y = 0.0, M2_Z = 0.0;
static GLfloat M3_X = -1.0, M3_Y = 0.0, M3_Z = 0.0;
static GLfloat bullet_X = 0.0, bullet_Y = 0.0, bullet_Z = 3.5,zoom=0.0;
int shootstate = 0, M1_die = 0,M2_die=0,M3_die=0,fog_open=0;
float hurt_a = 0.0,hurt_b=0.0,hurt_c=0.0;
class NCHU {
protected:
    float startx, starty, middlex, middley;
    int check(int(*data)[2], int len) {
	   int cross;
	   int Ax, Ay;
	   int Bx, By;
	   for (int i = 2; i < len; i++) {
		  Ax = data[i - 1][0] - data[i - 2][0];
		  Ay = data[i - 1][1] - data[i - 2][1];
		  Bx = data[i][0] - data[i - 1][0];
		  By = data[i][1] - data[i - 1][1];
		  cross = Ax * By - Ay * Bx;
		  if (cross > 0) {
			 return 1;
		  }
		  else if (cross < 0) {
			 return -1;
		  }
	   }
	   return 0;
    }
    void draw_ch(int(*data)[2], int len) {
	   //cout <<" arrive_logo " << arrive_logo << endl;
	   int cross = check(data, len);
	   if (cross == 1) {
		  glBegin(GL_POLYGON);
		  for (int i = 0; i < len; i++) {
			 glVertex3f(((data[i][0] - middlex) / startx), ((middley - data[i][1]) / starty), 0.0);
		  }
		  glEnd();
	   }
	   else if (cross == -1) {
		  int reverse_data[20][2];
		  int j = 0;
		  for (int i = len - 1; i >= 0; i--) {
			 reverse_data[j][0] = data[i][0];
			 reverse_data[j++][1] = data[i][1];
		  }
		  glBegin(GL_POLYGON);
		  for (int i = 0; i < len; i++) {
			 glVertex3f(((data[i][0] - middlex) / startx), ((middley - data[i][1]) / starty), 0.0);
		  }
		  glEnd();
	   }
    }

public:
    NCHU(float width, float height) {
	   middlex = (width / 4) + 200;
	   middley = (height * 3 / 4) - 100;
	   startx = (width / 2);
	   starty = height / 2;
	   mX = (width / 4) + 200;
	   mY = (height * 3 / 4) - 100;
	   sX = (width / 2);
	   sY = height / 2;
    }
};
class A :protected NCHU {
private:
    //A
    int a_5[8][2] = { {73, 629},{86, 611},{120,607},{139,605},{171,606}, {170,617},{138,618},{119,619}, };
    int a_4[7][2] = { {94,601},{142,579},{143,567},{125,572},{103,580},{89,589},{87,597} , };
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
    A(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   draw_ch(a_3, sizeof(a_3) / sizeof(a_3[0]));
	   draw_ch(a_4, sizeof(a_4) / sizeof(a_4[0]));
	   draw_ch(a_5, sizeof(a_5) / sizeof(a_5[0]));
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 10; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 4; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
    }
};
class B :protected NCHU {
private:
    int a_four[2][4][2] = {
	   {{269,588},{287,565},{314,553},{313,559}},
	   {{314,553},{313,559},{318,560},{327,554}},
    };
    int a_five[2][5][2] = {
	   {{275,520},{275,508},{308,522},{308,529},{287,533}},
	   {{318,560},{327,554},{331,561},{295,604},{277,605}},
    };
    int a_six[1][6][2] = {
	   {{292,546},{315,541},{287,565},{269,588},{260,586},{260,573}},
    };
    int a_seven[2][7][2] = {
	   {{308,529},{287,533},{244,542},{240,558}, {247,562},{329,541},{324,530}},
	   {{295,604},{277,605},{238,608},{249,622},{314,613},{352,612},{346,602}},
    };

public:
    B(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }

    }
};
class C :protected NCHU {
private:
    int a_four[4][4][2] = {
	   {{442,501},{452,499},{450,671},{438,680}},
	   {{377,529},{388,535},{396,533},{393,529}},
	   {{439,527} ,{436,519},{480,517},{471,524}},
	   {{480,517},{471,524},{489,529},{489,522}},
    };
    int a_five[1][5][2] = {
	   {{471,524},{489,529},{447,563},{414,575},{415,536}},
    };
    int a_six[2][6][2] = {
	   {{471,524},{489,529},{447,563},{414,575},{415,566},{447,551}},
	   {{439,527} ,{436,519} ,{395,525},{393,529},{396,533},{406,542}},
    };
    int a_seven[2][7][2] = {
	   {{377,529},{388,535},{391,535},{398,580},{389,578},{378,561},{374,534}},
	   {{414,575},{415,566},{447,557},{460,556},{469,558},{465,568},{444,575}},
    };

public:
    C(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 4; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }
    }
};
class D :protected NCHU {
private:
    int a_three[2][3][2] = {
	   {{608,547},{615,562},{613,565}},
	   {{577,604},{581,598},{597,595}},
    };
    int a_four[13][4][2] = {
	   {{540,516},{551,513},{554,520},{544,522}},
	   {{554,520},{544,522},{544,582},{553,579}},
	   {{554,512},{562,510},{569,518},{560,518}},
	   {{572,511},{599,499},{608,498},{612,504}},
	   {{596,536},{584,538},{586,541},{591,545}},
	   {{598,549},{598,553},{587,555},{587,550}},
	   {{605,517},{606,509},{621,507},{623,517}},
	   {{623,517},{631,519},{626,538},{620,534}},
	   {{601,543},{615,543},{618,548},{601,546}},
	   {{611,558},{614,556},{622,565},{614,568}},
	   {{651,556},{657,555},{660,570},{651,565} },
	   {{526,603},{529,619},{541,627},{541,609}},
	   {{654,632},{659,623},{659,612},{623,589}},
    };
    int a_five[6][5][2] = {
	   {{560,518},{569,518},{569,574},{563,585},{559,574}},
	   {{599,499},{608,498},{612,504},{601,578},{597,580}},
	   {{577,511},{583,510},{583,572},{577,580},{576,572}},
	   {{586,541},{591,545},{588,565},{581,564},{581,544}},
	   {{621,507},{623,517},{631,519},{637,501},{628,501}},
	   {{541,627},{541,609},{555,604},{574,601},{571,610}},
    };
    int a_six[2][6][2] = {
	   {{626,538},{620,534},{617,559},{613,568},{615,576},{624,565}},
	   {{570,579},{584,568},{620,562},{660,559},{660,570},{662,570}},
    };
    int a_seven[2][7][2] = {
	   {{574,541},{574,533},{588,524},{596,522},{608,527},{596,536},{584,538}},
	   {{626,538},{620,534},{607,527},{602,530},{622,552},{627,551},{631,542}},
    };
    int a_nine[9][2] = { {547,577},{506,586},{495,586},{493,601},{499,607},{504,607},{523,595},{570,579},{584,568} };

public:
    D(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 13; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 6; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }
	   draw_ch(a_nine, sizeof(a_nine) / sizeof(a_nine[0]));
    }
};
class E :protected NCHU {
private:
    int a_three[1][3][2] = {
	   {{769,503},{769,525},{762,525}},
    };
    int a_four[10][4][2] = {
	   {{723,553},{729,564},{760,551},{759,540}},
	   {{760,551},{759,540},{768,538},{768,549}},
	   {{768,549},{768,538},{785,535},{788,548}},
	   {{766,507},{771,508},{770,539},{768,540}},
	   {{762,525},{767,522},{764,542},{757,544}},
	   {{755,546},{761,549},{759,565},{751,563}},
	   {{759,565},{751,563},{744,577},{755,577}},
	   {{722,599},{722,602},{712,605},{716,600}},
	   {{770,577},{799,586},{809,593},{789,593}},
	   {{789,593},{790,604},{774,604},{782,599}},
    };
    int a_five[1][5][2] = {
	   {{750,590},{736,586},{721,596},{720,604},{735,604}},
    };
    int a_six[2][6][2] = {
	   {{744,577},{755,577},{750,589},{744,597},{750,590},{736,586}},
	   {{809,593},{789,593},{790,604},{801,611},{805,610},{809,606}},
    };
    int a_seven[2][7][2] = {
    };
    int a_eight[8][2] = { {728,552},{729,564} ,{725,570},{717,574},{706,575},{700,568},{702,557},{713,553} };

public:
    E(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 10; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   draw_ch(a_eight, sizeof(a_eight) / sizeof(a_eight[0]));
    }
};
class F :protected NCHU {
private:
    int a_three[7][3][2] = {
	   {{871,521},{874,515},{865,513}},
	   {{909,513},{911,507},{904,508}},
	   {{955,563},{940,565},{940,556}},
	   {{940,565},{940,556},{924,571}},
	   {{914,598},{901,598},{905,608}},
	   {{911,644},{896,638},{904,623}},
	   {{843,674},{848,669},{826,665}},
    };
    int a_four[22][4][2] = {
	   {{860,507},{869,493},{875,502},{867,512}},
	   {{876,540},{869,537},{883,525},{890,526}},
	   {{854,554},{863,554},{863,567},{854,570}},
	   { {887, 548}, { 891,539 }, { 882,533 }, {877,537}},
	   {{880,558},{895,554},{900,545},{890,535}},
	   {{900,545},{890,535},{898,525},{901,529}},
	   {{898,525}, {883,525},{876,516},{907,513}},
	   {{885,516},{896,513},{904,504},{896,515}},
	   {{909,513},{911,507},{920,503},{921,513}},
	   {{925,531},{922,517},{921,513},{931,512}},
	   {{910,544},{926,541},{921,550},{908,553}},
	   {{840,593},{846,585},{843,568},{826,590}},
	   {{846,585},{857,576},{859,564},{841,576}},
	   {{887,565},{935,553},{942,545},{895,554}},
	   {{899,583},{896,572},{884,577},{894,583}},
	   {{914,589},{913,598},{923,595},{934,588}},
	   {{923,595},{934,588},{940,594},{937,598}},
	   {{901,598},{895,592},{872,599},{876,604}},
	   {{911,644},{914,598},{905,608},{904,623}},
	   {{911,644},{896,638},{873,658},{893,660}},
	   {{873,658},{893,660},{871,672},{848,669}},
	   {{873,658},{871,672},{843,674},{848,669}},
    };
    int a_five[4][5][2] = {
	   {{909,521},{907,530},{902,525},{901,522},{904,515}},
	   {{832,607},{840,602},{840,593},{826,590},{826,599}},
	   {{857,576},{887,565},{895,554},{880,557},{859,565}},
	   {{872,599},{876,604},{871,605},{867,603},{867,598}},
    };
    int a_six[2][6][2] = {
	   {{921,513},{931,512},{935,506},{935,499},{930,497},{920,503}},
	   {{910,544},{926,541},{925,531},{922,517},{909,521},{907,530}},
    };
    int a_seven[5][7][2] = {
	   {{860,507},{867,512},{873,524},{865,536},{853,536},{848,524},{852,504}},
	   {{876,540},{869,537},{866,536},{853,536},{854,554},{863,554},{875,544}},
	   {{935,553},{942,545},{950,545},{958,551},{959,557},{955,563},{940,556}},
	   {{899,583},{907,583},{910,580},{912,573},{911,564},{905,564},{896,572}},
	   {{899,583},{907,583},{914,589},{913,598},{901,598},{895,592},{893,588}},
    };
    int a_eight[8][2] = { };

public:
    F(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 7; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 22; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 4; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 5; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }
    }
};
static GLfloat color[3] = { 0.007, 0.4375, 0.589 };
class Logo :protected NCHU {
private:
    int a_three[5][3][2] = {
	   {{1198,517},{1212,482},{1159,490}},
	   //left
	   {{1175,633},{1162,630},{1169,615}},

	   {{1034,649},{1058,638},{1044,622}},
	   {{1074,509},{1056,515},{1053,499}},

	   {{1127,586},{1057,569},{1107,538}},
    };
    int a_four[18][4][2] = {
	   {{1224,652},{1206,607},{1175,613},{1179,634}},
	   {{1206,607},{1175,613},{1162,581},{1199,575}},
	   {{1162,581},{1199,575},{1197,550},{1148,541}},
	   {{1197,550},{1148,541},{1117,523},{1198,517}},

	   {{1117,523},{1198,517},{1212,482},{1159,490}},
	   {{1159,490},{1116,490},{1101,509},{1117,523}},
	   {{1101,509},{1085,500},{1092,488},{1116,490}},
	   {{1117,523},{1159,490},{1116,490},{1148,541}},

	   {{1085,500},{1092,488},{1069,483},{1066,492}},
	   {{1069,483},{1066,492},{1049,487},{1045,476}},
	   //left
	   {{1162,630},{1169,615},{1163,601},{1141,625}},

	   {{1058,638},{1044,622},{1051,597},{1076,632}},
	   {{1051,597},{1076,632},{1099,627},{1057,569} },
	   {{1099,627},{1057,569},{1127,587},{1122,625} },

	   {{1057,569},{1129,586},{1105,528},{1059,545}},
	   {{1105,528},{1059,545},{1058,531},{1093,519}},
	   {{1058,531},{1093,519},{1074,509},{1056,515}},
	   {{1127,586},{1105,528},{1066,528},{1064,578}},

    };
    int a_five[1][5][2] = {
	   //left
	   {{1163,601},{1141,625},{1122,625},{1127,587},{1156,589}},
    };
    //red
    int red_three[1][3][2] = {
	   {{1161,579},{1166,569},{1156,570}},
    };
    int red_four[5][4][2] = {
	   {{1166,569},{1156,570},{1150,561},{1168,557}},
	   {{1150,561},{1168,557},{1164,543},{1142,550}},
	   {{1164,543},{1142,550},{1134,541},{1156,531}},
	   {{1134,541},{1156,531},{1143,525},{1125,532}},
	   {{1143,525},{1125,532},{1117,523},{1126,521}},
    };
    int ball[15][2] = { {1155,589},{1150,578},{1142,566},{1133,554},{1124,545},{1114,536},{1105,528},
	   {1098,537},{1092,546},{1093,571},{1098,581},{1107,590},{1118,595},{1131,596},{1142,594} };

    int ball_2[15][2] = { {1141,576},{1132,580},{1121,581},{1112,580},{1102,576},{1093,571},{1089,558},{1092,546},{1097,537},
	 {1105,528},{1114,536},{1124,545},{1133,554}, {1142,566},{1146,572},
    };
    int r_ball_2[2][6][2] = {
	   {{1152,563},{1156,552},{1156,543},{1154,533},{1144,527},{1142,550}},
	   {{1144,527},{1142,550},{1134,540},{1125,530},{1118,525},{1132,523}},
    };

    int ball_3[9][2] = {
	   {1106,529}, {1114,536},{1124,545},{1133,554},{1136,559},{1125,561},{1114,560},{1105,552},{1101,541}
    };
    int r_ball_3[7][2] = {
	   {1142,550},{1134,540},{1125,530},{1118,525},{1132,523},{1142,530},{1144,540},
    };

public:

    Logo(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   //cout << "dis" << endl;
	   //cout << color[1] << endl;
	   //glColor3f(0.007, 0.4375, 0.589);
	   glColor3f(color[0], color[1], color[2]);
	   for (int i = 0; i < 5; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 18; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }/*
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   draw_ch(a_eight, sizeof(a_eight) / sizeof(a_eight[0]));*/
	   glColor3f(0.6, 0.0, 0.0);
	   for (int i = 0; i < 1; i++) {
		  draw_ch(red_three[i], sizeof(red_three[i]) / sizeof(red_three[i][0]));
	   }
	   for (int i = 0; i < 5; i++) {
		  draw_ch(red_four[i], sizeof(red_four[i]) / sizeof(red_four[i][0]));
	   }
	   draw_ch(ball, sizeof(ball) / sizeof(ball[0]));
	   glColor3f(0.8, 0.0, 0.0);
	   draw_ch(ball_2, sizeof(ball_2) / sizeof(ball_2[0]));
	   for (int i = 0; i < 2; i++) {
		  draw_ch(r_ball_2[i], sizeof(r_ball_2[i]) / sizeof(r_ball_2[i][0]));
	   }
	   glColor3f(1.0, 0.0, 0.0);
	   draw_ch(ball_3, sizeof(ball_3) / sizeof(ball_3[0]));
	   draw_ch(r_ball_3, sizeof(r_ball_3) / sizeof(r_ball_3[0]));
    }
};
class NCHU_en {
protected:
    float startx, starty, s, middlex, middley;

    int check(int(*data)[2], int len) {
	   int cross;
	   int Ax, Ay;
	   int Bx, By;
	   for (int i = 2; i < len; i++) {
		  Ax = data[i - 1][0] - data[i - 2][0];
		  Ay = data[i - 1][1] - data[i - 2][1];
		  Bx = data[i][0] - data[i - 1][0];
		  By = data[i][1] - data[i - 1][1];
		  cross = Ax * By - Ay * Bx;
		  if (cross > 0) {
			 return 1;
		  }
		  else if (cross < 0) {
			 return -1;
		  }
	   }
	   return 0;
    }
    void draw_en(int(*data)[2], int move) {
	   int cross = check(data, 3);
	   if (cross == 1) {
		  glBegin(GL_TRIANGLES);
		  for (int i = 0; i < 3; i++) {
			 glVertex2f(((data[i][0] - s + move) / startx), ((middley - data[i][1]) / starty));
		  }
		  glEnd();
	   }
	   else if (cross == -1) {
		  int reverse_data[20][2];
		  int j = 0;
		  for (int i = 2; i >= 0; i--) {
			 reverse_data[j][0] = data[i][0];
			 reverse_data[j++][1] = data[i][1];
		  }
		  glBegin(GL_TRIANGLES);
		  for (int i = 0; i < 3; i++) {
			 glVertex2f(((data[i][0] - s + move) / startx), ((middley - data[i][1]) / starty));
		  }
		  glEnd();
	   }
    }
public:
    NCHU_en(float width, float height) {
	   startx = width / 2;
	   starty = height / 2;
	   middlex = width / 4 + 200;
	   middley = (height * 3 / 4) - 100;
	   s = middlex + 40.0;
    }
};
class National :protected NCHU_en {
private:
    int N[7][3][2] = {
	   {{123,790},{129,790},{123,743}},
	   {{129,790},{123,743},{129,756}},

	   {{123,743},{129,756},{129,743}},

	   {{129,756},{129,743},{155,790}},
	   {{129,743},{155,790},{163,790}},
	   {{155,790},{163,790},{163,743}},
	   {{163,743},{157,743},{157,790}},
    };
    int a[26][3][2] = {
	   {{207,791},{206,763},{201,791}},
	   {{206,763},{201,791},{200,770}},
	   {{201,787},{201,779},{191,792}},
	   {{201,779},{191,792},{189,788}},
	   {{191,792},{189,788},{181,791}},
	   {{189,788},{181,791},{183,786}},
	   {{181,791},{183,786},{175,784}},
	   {{183,786},{175,784},{181,780}},
	   {{175,784},{181,780},{179,774}},
	   {{181,780},{179,774},{185,776}},
	   {{179,774},{185,776},{191,771}},
	   {{185,776},{191,771},{200,774}},
	   {{191,771},{200,774},{200,770}},

	   {{200,770},{200,764},{206,763}},
	   {{200,764},{206,763},{202,758}},
	   {{206,763},{202,758},{196,760}},
	   {{200,764},{196,760},{202,758}},
	   {{202,758},{196,760},{196,756}},
	   {{196,760},{196,756},{191,760}},
	   {{196,756},{191,760},{188,756}},
	   {{191,760},{188,756},{187,760}},
	   {{188,756},{187,760},{181,758}},
	   {{187,760},{181,758},{183,763}},
	   {{181,758},{183,763},{177,762}},
	   {{183,763},{177,762},{176,765}},
	   {{183,763},{176,765},{182,766}},

    };
    int t[7][3][2] = {
	   {{222,787},{228,786},{228,745}},
	   {{222,787},{228,745},{222,748}},
	   {{217,761},{233,755},{233,761}},
	   {{217,761},{233,755},{217,755}},
	   {{222,787},{228,786},{226,791}},
	   {{228,786},{233,791},{226,791}},
	   {{228,786},{233,791},{233,787}},
    };
    int i_n_u_r[2][3][2] = {
	   {{244,790},{250,757},{250,790}},
	   {{244,790},{250,757},{244,757}},
    };
    int I[2][3][2] = {
	   {{244,749},{244,744},{250,744}},
	   {{244,749},{250,749},{250,744}},
    };
    int o_e[19][3][2] = {
	   {{278,792},{283,786},{290,786}},
	   {{283,786},{290,786},{288,781}},
	   {{290,786},{288,781},{294,781}},

	   //{{290,786},{288,781},{295,779}},
	   //{{288,781},{295,779},{289,773}},
	   //{{295,779},{289,773},{294,766}},

	   {{294,766},{289,773},{295,773}},
	   {{289,773},{294,766},{288,766}},
	   {{294,766},{288,766},{290,759}},
	   {{288,766},{290,759},{283,762}},
	   {{290,759},{283,762},{280,756}},
	   {{283,762},{280,756},{276,761}},
	   {{280,756},{276,761},{268,759}},
	   {{276,761},{268,759},{269,766}},
	   {{268,759},{269,766},{262,766}},
	   {{269,766},{262,766},{268,774}},
	   {{262,766},{268,774},{262,779}},
	   {{268,774},{262,779},{269,782}},
	   {{262,779},{269,782},{266,787}},
	   {{269,782},{266,787},{275,787}},
	   {{266,787},{275,787},{278,792}},
	   {{275,787},{278,792},{283,786}},
    };
    int o[2][3][2] = {
	   {{288,781},{295,773},{294,781}},
	   {{288,781},{295,773},{289,773}},
    };
    int n[8][3][2] = {
	   {{331,790},{336,762},{336,790}},
	   {{331,790},{336,762},{331,765}},

	   {{331,765},{331,756},{336,762}},
	   {{331,765},{331,756},{329,762}},

	   {{329,762},{320,756},{331,756}},
	   {{329,762},{320,756},{321,760}},

	   {{312,766},{320,756},{321,760}},
	   {{312,766},{320,756},{311,762}},
    };
    int L[2][3][2] = {
	   {{394,790},{399,744},{394,744}},
	   {{394,790},{399,744},{399,790}},
    };

public:
    National(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   //float alldata[LEN];
	   //GLint first[PRICOUNT];
	   //GLsizei count[PRICOUNT];
	   int k = 0, primcount = 0;
	   glColor3f(0.007, 0.4375, 0.589);
	   //first[primcount] = 0;
	   for (int i = 0; i < 7; i++) {

		  draw_en(N[i], 0);
	   }
	   for (int i = 0; i < 26; i++) {

		  draw_en(a[i], 0);
		  draw_en(a[i], 173);
	   }
	   for (int i = 0; i < 7; i++) {
		  draw_en(t[i], 0);
		  draw_en(t[i], 957);
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_en(i_n_u_r[i], 0);//i
		  draw_en(I[i], 0);
		  draw_en(i_n_u_r[i], 63);//n
		  draw_en(i_n_u_r[i], 306);//u
		  draw_en(i_n_u_r[i], 317 + 8);//n
		  draw_en(i_n_u_r[i], 393);//g
		  draw_en(i_n_u_r[i], 523);//i
		  draw_en(I[i], 523);
		  draw_en(i_n_u_r[i], 543);//n
		  draw_en(i_n_u_r[i], 611);//g
		  draw_en(i_n_u_r[i], 701);//n
		  draw_en(i_n_u_r[i], 744);//i
		  draw_en(I[i], 744);
		  draw_en(i_n_u_r[i], 846);//r
		  draw_en(i_n_u_r[i], 913);//i
		  draw_en(I[i], 913);
		  draw_en(L[i], 0);
		  draw_en(L[i], 91);
		  draw_en(L[i], 281);
		  draw_en(L[i], 315);
		  draw_en(o[i], 0);
	   }
	   for (int i = 0; i < 19; i++) {
		  draw_en(o_e[i], 0);
		  draw_en(o_e[i], 783);
	   }
	   for (int i = 0; i < 8; i++) {
		  draw_en(n[i], 0);
		  //draw_en(n[i], 63-63);//n
		  draw_en(n[i], 317 - 63 + 8);//n
		  draw_en(n[i], 543 - 63);//n
		  draw_en(n[i], 701 - 63);//n
		  draw_en(n[i], 177);//n
	   }
    }
};
class Chung :protected NCHU_en {
private:
    int C[21][3][2] = {
	   {{472,776},{465,776},{467,785}},
	   {{465,776},{467,785},{460,783}},
	   {{467,785},{460,783},{457,791}},
	   {{460,783},{457,791},{451,786}},
	   {{457,791},{451,786},{445,792}},
	   {{451,786},{445,792},{441,783}},
	   {{445,792},{441,783},{433,785}},
	   {{441,783},{433,785},{435,775}},
	   {{433,785},{435,775},{428,773}},
	   {{435,775},{428,773},{434,766}},
	   {{428,773},{434,766},{429,758}},
	   {{434,766},{429,758},{436,757}},
	   {{429,758},{436,757},{434,747}},
	   {{434,766},{434,747},{441,750}},
	   {{434,747},{441,750},{445,743}},
	   {{441,750},{445,743},{451,749}},
	   {{445,743},{451,749},{458,742}},
	   {{451,749},{458,742},{460,750}},
	   {{458,742},{460,750},{467,748}},
	   {{460,750},{467,748},{465,757}},
	   {{467,748},{465,757},{470,756}},
    };
    int u[10][3][2] = {
	   {{527,783},{533,756},{527,756}},
	   {{527,783},{533,756},{533,781}},

	   {{527,783},{536,786},{533,781}},
	   {{527,783},{536,786},{530,788}},
	   //{{533,781},{536,786},{530,788}},

	   {{536,786},{530,788},{536,792}},
	   {{530,788},{536,792},{541,787}},
	   {{536,792},{541,787},{545,791}},
	   {{541,787},{545,791},{547,785}},
	   {{545,791},{547,785},{552,784}},
	   {{547,785},{552,784},{553,778}},
    };
    int g[28][3][2] = {
	   {{613,796},{618,795},{614,800}},
	   {{618,795},{614,800},{620,799}},
	   {{614,800},{620,799},{621,805}},
	   {{620,799},{621,805},{626,801}},
	   {{621,805},{626,801},{630,806}},
	   {{626,801},{630,806},{632,800}},
	   {{630,806},{632,800},{639,801}},
	   {{632,800},{639,801},{637,795}},
	   {{639,801},{637,795},{643,795}},
	   {{643,795},{637,787},{637,795}},
	   {{643,795},{637,787},{644,787}},

	   {{637,780},{637,787},{632,786}},
	   {{637,787},{632,786},{631,791}},
	   {{632,786},{631,791},{627,787}},
	   {{631,791},{627,787},{622,790}},
	   {{627,787},{622,790},{622,785}},

	   {{622,790},{622,785},{615,785}},
	   {{622,785},{615,785},{618,781}},
	   {{615,785},{618,781},{612,780}},

	   {{618,781},{612,780},{618,767}},

	   {{612,780},{618,767},{612,769}},
	   {{618,767},{612,769},{615,761}},

	   {{618,767},{615,761},{622,762}},
	   {{615,761},{622,762},{622,756}},
	   {{622,762},{622,756},{628,760}},
	   {{622,756},{628,760},{633,757}},
	   {{628,760},{633,757},{636,764}},
	   {{633,757},{636,764},{639,764}},
    };
public:
    Chung(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 21; i++) {
		  draw_en(C[i], 0);
	   }
	   for (int i = 0; i < 10; i++) {
		  draw_en(u[i], 0);
	   }
	   for (int i = 0; i < 28; i++) {
		  draw_en(g[i], 0);
		  draw_en(g[i], 218);
	   }
    }
};
class Hsing :protected NCHU_en {
private:
    int H[2][3][2] = {
	   {{710,768},{678,763},{710,763}},
	   {{710,768},{678,763},{678,768}},
    };
    int s[26][3][2] = {
	   {{726,781},{731,781},{729,787}},
	   {{731,781},{729,787},{734,786}},
	   {{729,787},{734,786},{735,791}},

	   {{734,786},{735,791},{739,788}},
	   {{735,791},{739,788},{742,792}},
	   {{739,788},{742,792},{745,788}},

	   {{742,792},{745,788},{751,790}},
	   {{745,788},{751,790},{750,784}},
	   {{751,790},{750,784},{756,784}},

	   {{750,784},{755,778},{756,784}},
	   {{750,784},{755,778},{750,780}},

	   {{750,780},{750,774},{755,778}},
	   {{750,780},{750,774},{745,778}},

	   {{737,775},{750,774},{745,778}},
	   {{737,775},{750,774},{737,769}},

	   {{737,775},{737,769},{730,772}},
	   {{737,769},{730,772},{733,766}},
	   {{730,772},{733,766} ,{726,767}},
	   {{726,767},{735,761},{733,766}},
	   {{726,767},{735,761},{728,760}},
	   {{735,761},{728,760},{736,756}},

	   {{736,756},{746,761},{735,760}},
	   {{736,756},{746,761},{745,755}},

	   {{746,761},{745,755},{752,760}},

	   {{746,761},{754,764},{752,760}},
	   {{746,761},{754,764},{749,766}},

    };

public:
    Hsing(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 2; i++) {
		  draw_en(H[i], 0);
	   }
	   for (int i = 0; i < 26; i++) {
		  draw_en(s[i], 0);
		  draw_en(s[i], 389);
	   }
    }
};
class University :protected NCHU_en {
private:
    int U[13][3][2] = {
	   {{891,745},{897,780},{891,780}},
	   {{891,745},{897,780},{897,745}},
	   {{891,780},{897,780},{896,787}},
	   {{897,780},{896,787},{903,785}},
	   {{896,787},{903,785},{904,792}},
	   {{903,785},{904,792},{911,787}},
	   {{904,792},{911,787},{916,792}},
	   {{911,787},{916,792},{918,785}},
	   {{916,792},{918,785},{926,787}},
	   {{918,785},{926,787},{923,779}},
	   {{926,787},{923,779},{930,780}},
	   {{930,780},{923,745},{930,745}},
	   {{930,780},{923,745},{923,780}},
    };
    int v[4][3][2] = {
	   {{1024,791},{1004,757},{1018,791}},
	   {{1024,791},{1004,757},{1010,757}},
	   {{1018,791},{1038,757},{1031,757}},
	   {{1018,791},{1038,757},{1024,791}},
    };
    int e[2][3][2] = {
	   {{1078,774},{1046,771},{1078,771}},
	   {{1078,774},{1046,771},{1046,774}},
    };
    int r[6][3][2] = {
	   {{1110,758},{1108,762},{1106,756}},
	   {{1108,762},{1106,756},{1103,761}},
	   {{1106,756},{1103,761},{1101,756}},
	   {{1103,761},{1101,756},{1098,764}},
	   {{1101,756},{1098,764},{1095,762}},
	   {{1098,764},{1095,762},{1095,770}},

    };
    int y[6][3][2] = {
	   {{1200,800},{1205,805},{1205,800}},
	   {{1200,800},{1205,805},{1200,805}},
	   {{1205,805},{1205,800},{1210,803}},
	   {{1210,803},{1211,791},{1205,800}},
	   {{1210,803},{1211,791},{1213,799}},
	   {{1211,791},{1213,799},{1217,790}},
    };
public:
    University(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 13; i++) {
		  draw_en(U[i], 0);
	   }
	   for (int i = 0; i < 4; i++) {
		  draw_en(v[i], 0);
		  draw_en(v[i], 193);
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_en(e[i], 0);
	   }
	   for (int i = 0; i < 6; i++) {
		  draw_en(r[i], 0);
		  draw_en(y[i], 0);
	   }
    }
};
static GLint fogMode;

void init(void)
{
    /*
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat position[] = { 0.0, 3.0, 2.0, 0.0 };
    GLfloat lmodel_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
    GLfloat local_view[] = { 0.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
    */
    
    //glClearColor(1, 1, 1, 1);
    //glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClearColor(float(28/255), float(74 / 255), float(13 / 255),1.0);
    glClearColor(0.109,0.188,0.078,0.0);
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    //glEnable(GL_LIGHT0);
    //glEnable(GL_NORMALIZE);
    //glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    
}
void drawHealth(float health) {
    glBegin(GL_QUADS);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0,0);
    glVertex3f(0, 0.2,0);
    glVertex3f(health,0.2,0);
    glVertex3f(health,0,0);
    glEnd();
}
void DrawSnipe(float cx, float cy, float r, int num_segments) {
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);
    for (int ii = 0; ii < num_segments; ii++) {
	   float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
	   float x = r * cosf(theta);//calculate the x component 
	   float y = r * sinf(theta);//calculate the y component 
	   glVertex3f(x + cx, y + cy,0.0);//output vertex 
    }
    glEnd();
    glBegin(GL_LINES);
	   glVertex3f(cx,cy - float(r / 5),0);
	   glVertex3f(cx, cy-r , 0);
	   
	   glVertex3f(cx, cy+ float(r / 5), 0);
	   glVertex3f(cx, cy+r, 0);
	   
	   glVertex3f(cx - float(r / 5), cy, 0);
	   glVertex3f(cx - r, cy, 0);

	   glVertex3f(cx+ float(r / 5), cy, 0);
	   glVertex3f(cx + r, cy, 0);
    glEnd();
    glPointSize(5);
    glBegin(GL_POINTS);
	   glVertex3f(cx,cy,0);
    glEnd();

}
void hurt(float a,int color) {
    glEnable(GL_BLEND); 
    glDisable(GL_DEPTH_TEST); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
    //glColor4f(0.9, 0.4, 0.4,0.8);
    if(color==0) glColor4f(0.8,0,0, a);
    else if(color==1) glColor4f(0.8, 0.5, 0, a);
    else  glColor4f(0.6, 0.6, 0, a);
    //if (hurt == 1) {
	   glBegin(GL_POLYGON);
		  glVertex3f(10,10,0.1);
		  glVertex3f(10, -10, 0.1);
		  glVertex3f(-10, -10, 0.1);
		  glVertex3f(-10, 10, 0.1);
	   
	   glEnd();
    //}
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}
void display()
{
    if (fog_open==1) {
	   glEnable(GL_FOG);
	   {
		  GLfloat fogColor[4] = { 0.5, 0.5, 0.5, 1.0 };

		  fogMode = GL_EXP;
		  glFogi(GL_FOG_MODE, fogMode);
		  glFogfv(GL_FOG_COLOR, fogColor);
		  glFogf(GL_FOG_DENSITY, 0.35);
		  glHint(GL_FOG_HINT, GL_DONT_CARE);
		  glFogf(GL_FOG_START, 1.0);
		  glFogf(GL_FOG_END, 5.0);
	   }
	   glClearColor(0.5, 0.5, 0.5, 1.0);  /* fog color */
    }
    else  {
	   glDisable(GL_FOG);
	   glClearColor(0.109, 0.188, 0.078, 0.0);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    A a(WIDTH, HEIGHT);
    C c(WIDTH, HEIGHT);
    D d(WIDTH, HEIGHT);
    E e(WIDTH, HEIGHT);
    F f(WIDTH, HEIGHT);
    B b(WIDTH, HEIGHT);
    Logo logo(WIDTH, HEIGHT);
    National national(WIDTH, HEIGHT);
    Chung chung(WIDTH, HEIGHT);
    Hsing hsing(WIDTH, HEIGHT);
    University university(WIDTH, HEIGHT);
    glColor3f(0.0, 0.0, 0.0);
    glPushMatrix();//lookat and walk 4
	   glLoadIdentity();            //clear the matrix
	   gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
	   glPushMatrix();//latter 8
		  glDisable(GL_CULL_FACE);
		  glDisable(GL_LIGHT0);
		  glDisable(GL_NORMALIZE);
		  glDisable(GL_COLOR_MATERIAL);
		  glDisable(GL_LIGHTING);
		  a.display_ch();
		  c.display_ch();
		  d.display_ch();
		  e.display_ch();
		  f.display_ch();
		  b.display_ch();
		  glPushMatrix();
			 glDisable(GL_DEPTH_TEST);
			 logo.display_ch();
			 glEnable(GL_DEPTH_TEST);
		  glPopMatrix();
		  national.display_en();
		  chung.display_en();
		  hsing.display_en();
		  university.display_en();
		  
	   glPopMatrix();//latter 8
	   //xyz-------------------------
	   glPushMatrix();
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
	   hurt(hurt_a,0);
	   hurt(hurt_b,1);
	   hurt(hurt_c,2);
	   // ------------------------xyz
	   //std::cout << "x hi: " << ((523 - mX) / sX) << "	x low: " << ((667 - mX) / sX) << "	y hi: " << ((mY - 600) / sY) << "  y low: " << ((mY - 500) / sY) << std::endl;
	   /*glPushMatrix();
		  glColor3f(1.0, 0.0, 0.0);
		  glBegin(GL_LINES);
		  glVertex2f(((420 - mX) / sX), -10.0);
		  glVertex2f(((420 - mX) / sX), 10.0);
		  glEnd();

		  glColor3f(0.0, 1.0, 0.0);
		  glBegin(GL_LINES);
		  glVertex3f(((770 - mX) / sX), -10.0, 0.0);
		  glVertex3f(((770 - mX) / sX), 10.0, 0.0);
		  glEnd();
		  glColor3f(0.0, 0.0, 1.0);
		  glBegin(GL_LINES);
		  glVertex3f(-10.0, ((mY - 800) / sY), 0.0);
		  glVertex3f(10.0, ((mY - 800) / sY), 0.0);
		  glEnd();
		  glBegin(GL_LINES);
		  glVertex3f(-10.0, ((mY - 500) / sY), 0.0);
		  glVertex3f(10.0, ((mY - 500) / sY), 0.0);
		  glEnd();
	   glPopMatrix();*/

	   glPushMatrix();
		  glTranslatef(2, 1.5, 0.0);
		  drawHealth(myHP);
	   glPopMatrix();
		  
		  
	   //glPushMatrix();
		  
	   glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
	   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_black);
	   glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_black);
	   glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	   glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	   glPushMatrix();//monster 1
	   if (M1_die == 0) {
		  glTranslatef(M1_X, M1_Y, M1_Z);
		  glPushMatrix();
		  glTranslatef(-0.12, 0.4, 0);
		  glScalef(0.5, 0.2, 0.5);
		  drawHealth(M1_HP);
		  glEnable(GL_CULL_FACE);
		  glEnable(GL_LIGHT0);
		  glEnable(GL_NORMALIZE);
		  glEnable(GL_COLOR_MATERIAL);
		  glEnable(GL_LIGHTING);
		  glPopMatrix();
		  if (!pmodel) {
			 char filename[] = "al.obj";
			 pmodel = glmReadOBJ(filename);
			 if (!pmodel) exit(0);
			 glmUnitize(pmodel);
			 glmFacetNormals(pmodel);
			 glmVertexNormals(pmodel, 90.0);
		  }
		  glColor3f(0.5, 0.5, 0.5);
		  glScalef(0.3, 0.3, 0.3);
		  glmDraw(pmodel, GLM_SMOOTH);
	   }
	   glPopMatrix();//monster 1
	   glPushMatrix();//monster 2
	   if (M2_die == 0) {
		  //glTranslatef(1,0,0);
		  glTranslatef(M2_X, M2_Y, M2_Z);
		  glPushMatrix();
		  glDisable(GL_CULL_FACE);
		  glDisable(GL_LIGHT0);
		  glDisable(GL_NORMALIZE);
		  glDisable(GL_COLOR_MATERIAL);
		  glDisable(GL_LIGHTING);
		  glTranslatef(-0.12, 0.3, 0);
		  glScalef(0.5, 0.2, 0.5);
		  drawHealth(M2_HP);
		  glEnable(GL_CULL_FACE);
		  glEnable(GL_LIGHT0);
		  glEnable(GL_NORMALIZE);
		  glEnable(GL_COLOR_MATERIAL);
		  glEnable(GL_LIGHTING);
		  glPopMatrix();
		  if (!pmodel2) {
			 char filename[] = "cow.obj";
			 pmodel2 = glmReadOBJ(filename);
			 if (!pmodel2) exit(0);
			 glmUnitize(pmodel2);
			 glmFacetNormals(pmodel2);
			 glmVertexNormals(pmodel2, 90.0);
		  }
		  glColor3f(0.8, 0.8, 0.8);
		  glScalef(0.3, 0.3, 0.3);
		  glRotatef(270,0.0,1.0,0.0);
		  glmDraw(pmodel2, GLM_SMOOTH);
	   }
	   glPopMatrix();//monster 2
	   glPushMatrix();//monster 3
	   if (M2_die == 0) {
		  //glTranslatef(1,0,0);
		  glTranslatef(M3_X, M3_Y, M3_Z);
		  glPushMatrix();
		  glDisable(GL_CULL_FACE);
		  glDisable(GL_LIGHT0);
		  glDisable(GL_NORMALIZE);
		  glDisable(GL_COLOR_MATERIAL);
		  glDisable(GL_LIGHTING);
		  glTranslatef(-0.12, 0.3, 0);
		  glScalef(0.5, 0.2, 0.5);
		  drawHealth(M3_HP);
		  glEnable(GL_CULL_FACE);
		  glEnable(GL_LIGHT0);
		  glEnable(GL_NORMALIZE);
		  glEnable(GL_COLOR_MATERIAL);
		  glEnable(GL_LIGHTING);
		  glPopMatrix();
		  if (!pmodel3) {
			 char filename[] = "pig.obj";
			 pmodel3 = glmReadOBJ(filename);
			 if (!pmodel3) exit(0);
			 glmUnitize(pmodel3);
			 glmFacetNormals(pmodel3);
			 glmVertexNormals(pmodel3, 90.0);
		  }
		  glColor3f(1.0, 0.5, 0.5);
		  glScalef(0.3, 0.3, 0.3);
		  glRotatef(270, 0.0, 1.0, 0.0);
		  glmDraw(pmodel3, GLM_SMOOTH);
	   }
	   glPopMatrix();//monster 3
	   glPushMatrix();// blend
		  glDisable(GL_CULL_FACE);
		  glDisable(GL_LIGHT0);
		  glDisable(GL_NORMALIZE);
		  glDisable(GL_COLOR_MATERIAL);
		  glDisable(GL_LIGHTING);
		  glTranslatef(bullet_X, bullet_Y, bullet_Z-zoom);
		  glTranslatef(bullet_X, bullet_Y, 0.1);
		  glColor3f(1, 1, 1);
		  DrawSnipe(0, 0, 0.7, 100);
		  glEnable(GL_CULL_FACE);
		  glEnable(GL_LIGHT0);
		  glEnable(GL_NORMALIZE);
		  glEnable(GL_COLOR_MATERIAL);
		  glEnable(GL_LIGHTING);
	   glPopMatrix();//blend
	   glPushMatrix();//bullet
		  //glTranslatef(0,0, - fly);
		  glTranslatef(bullet_X,bullet_Y,bullet_Z-fly);
		  glColor3f(1, 1, 1);
		  glutSolidSphere(0.03, 16, 16);
	   glPopMatrix();//bullet
    glPopMatrix();
    glutSwapBuffers();
}
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);
    gluPerspective(90.0, (GLfloat)w / (GLfloat)h, 1.0, 700.0);
    glMatrixMode(GL_MODELVIEW);
}
int c = 0,c2=0,c3=0;
void Idle_function(void) {
    if (myHP <= 0) {
	   return;
    }
    //shoot
    if (shootstate == 1) {
	   
	   fly = fly + 0.1;
	   //std::cout << "x hi: " << ((420 - mX) / sX) << "	x low: " << ((770 - mX) / sX) << "	y hi: " << ((mY - 800) / sY)<<"  y low: "<< ((mY - 500) / sY) << std::endl;
	   //std::cout << "x: " <<bullet_X<<"    y: "<<bullet_Y<<"   y: "<< (bullet_Z - fly) << std::endl;
	   //momster die
	   if (M1_die = 0 &&(M1_X > ((420 - mX) / sX) && M1_X <((770 - mX) / sX) && M2_Y>((mY - 800) / sY) && M2_Y < ((mY - 500) / sY)) && (((bullet_Z-fly) > M1_Z-0.05)&&((bullet_Z - fly) < M1_Z+0.05))) {
		  M1_HP -= 0.1;
		  //std::cout << "kill" << std::endl;
		  if (M1_HP <= 0) M1_die = 1;
	   }
	   if (M2_die = 0 && (M2_X > ((420 - mX) / sX) && M2_X <((770 - mX) / sX) && M2_Y>((mY - 800) / sY) && M2_Y < ((mY - 500) / sY)) && (((bullet_Z - fly) > M2_Z - 0.05) && ((bullet_Z - fly) < M2_Z + 0.05))) {
		  M2_HP -= 0.1;
		  //std::cout << "kill" << std::endl;
		  if (M2_HP <= 0) M2_die = 1;
	   }
	   if (fly > 10) {
		  shootstate = 2;
		  //cout << "arrive" << endl;
		  fly = 0;
		  
	   }
    }
    if (shootstate == 2) {
	   //cout << "shootstate == 2" << endl;
	   //fly = 0;
	   
    }
    
    
    /* 固定亂數種子 */
    srand(time(NULL));

    /* 指定亂數範圍 */
    double min = -0.2;
    double max = 0.2;
    double min_z = 0;
    // double max_z = 0.05;

    /* 產生 [min , max) 的浮點數亂數 */
    //double x = (max - min) * rand() / (RAND_MAX + 1.0) + min;
    double z3 = (0.1) * rand() / (RAND_MAX + 1.0) + min_z;
    double x3 = (0.2) * rand() / (RAND_MAX + 1.0) -0.1;
    double y3 = (0.2) * rand() / (RAND_MAX + 1.0) -0.1;
    double z2 = (0.05) * rand() / (RAND_MAX + 1.0) + min_z;
    double x2 = (0.1) * rand() / (RAND_MAX + 1.0) -0.05;
    double y2 = (0.1) * rand() / (RAND_MAX + 1.0) -0.05;
    double z = (0.025) * rand() / (RAND_MAX + 1.0) + min_z;
    double x = (0.5) * rand() / (RAND_MAX + 1.0) - 0.025;
    double y = (0.5) * rand() / (RAND_MAX + 1.0) - 0.025;
    //double z2 = (max - min_z) * rand() / (RAND_MAX + 1.0) + min_z;
    //double x2 = (max - min) * rand() / (RAND_MAX + 1.0) + min;
    //double y2 = (max - min) * rand() / (RAND_MAX + 1.0) + min;
    //double x =0;
    //double y = 0;
    //std::cout << "x: " << x << "	y: " << y << "	z: " << z << std::endl;
    if ((M1_X > -2.0 && x < 0) || (M1_X < 2.0 && x > 0) && M1_die==0) {
	   M1_X += x;
    }
    if ((M1_Y > -1.0 && y < 0) || (M1_Y < 1.0 && y > 0) && M1_die == 0) {
	   M1_Y += y;
    }
    if ((M1_Z > -3.5 && z < 0) || (M1_Z < 1.8 && z > 0) && M1_die == 0) {
	   M1_Z += z;
    }
    if (M1_Z > 1.6 && M1_die == 0 && c2==0 && c3==0) {
	   
	   if (c==0) {
		  
		  if (hurt_a < 0.9) hurt_a += 0.1;
		  else c = 1;
	   }
	   else { 
		  if (hurt_a > 0.0) hurt_a -= 0.1;
		  else {
			 myHP = myHP - 0.1;
			 M1_Z -= 2;
			 c = 0;
		  }
		  
		  
	   }
    }
    
    if ((M2_X > -2.0 && x2 < 0) || (M2_X < 2.0 && x2 > 0) && M2_die == 0) {
	   M2_X += x2;
    }
    if ((M2_Y > -1.0 && y2 < 0) || (M2_Y < 1.0 && y2 > 0) && M2_die == 0) {
	   M2_Y += y2;
    }
    if ((M2_Z > -3.5 && z2 < 0) || (M2_Z < 1.4 && z2 > 0) && M2_die == 0) {
	   M2_Z += z2;
    }
    if (M2_Z > 1.2 && M2_die == 0 &&c==0 && c3==0) {

	   if (c2 == 0) {
		  //cout << "hurt start" << endl;
		  if (hurt_b < 0.9) hurt_b += 0.1;
		  else c2 = 1;
	   }
	   else {
		  //cout << "hurt end" << endl;
		  if (hurt_b > 0.0) hurt_b -= 0.1;
		  else {
			// cout << "back" << endl;
			 myHP = myHP - 0.02;
			 M2_Z -= 3;
			 M3_Z -= 5;
			 c2 = 0;
		  }


	   }
    }
    if ((M3_X > -0.05 && x3< 0) || (M3_X < 0.05 && x3 > 0) && M3_die == 0) {
	   M3_X += x3;
    }
    if ((M3_Y > -0.05 && y3 < 0) || (M3_Y < 0.05 && y3 > 0) && M3_die == 0) {
	   M3_Y += y3;
    }
    if ((M3_Z > -3.5 && z3 < 0) || (M3_Z < 1.4 && z3 > 0) && M3_die == 0) {
	   M3_Z += z3;
    }
    if (M3_Z > 1.2 && M3_die == 0 && c==0 && c2==0) {

	   if (c3 == 0) {
		  //cout << "hurt start" << endl;
		  if (hurt_c < 0.9) hurt_c += 0.1;
		  else c3 = 1;
	   }
	   else {
		  //cout << "hurt end" << endl;
		  if (hurt_c > 0.0) hurt_c -= 0.1;
		  else {
			 // cout << "back" << endl;
			 myHP = myHP - 0.02;
			 M3_Z -= 5;
			 M2_Z -= 3;
			 c3 = 0;
		  }


	   }
    }
    //std::cout << "M2_X: " << M2_X << "	M2_Y: " << M2_Y << "	M2_Z: " << M2_Z << std::endl;
    //std::cout << "M1_X: " << M1_X << "	M1_Y: " << M1_Y << "	M1_Z: " << M1_Z << std::endl;
    
    
    glutPostRedisplay();
}
/* 釋放鍵盤時會call此function */
void keyboardUp(unsigned char key, int x, int y) {
    glutIgnoreKeyRepeat(1); //ignore key repeat
    switch (key) {
    case 'b':
    case 'B':
	   //cout << "B" << endl;
	   zoom = 0;
	   meZ = 3;
	   shootstate = 1;
	   break;
    case 27: // “esc” on keyboard
	   exit(0);
	   break;
    default:
	   break;
    }
    glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y){
    switch (key) {
    case 'r':
    case 'R':
	   myHP = 1.0;
	   M1_HP = 0.5;
	   M1_die=0;
	   break;
    case 'b':
    case 'B':
	   //cout << "B down" << endl;
	   zoom = 3.5;
	   meZ = 2;
	   break;
    case 'c':
    case 'C':
	   break;
    case 'f':
    case 'F':
	   if (fog_open == 0) fog_open = 1;
	   else fog_open = 0;
	   //cout << fog_open << endl;
	   break;
    case 27:
	   exit(0);
	   break;
    }
    glutPostRedisplay();
}
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("week06");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(Idle_function);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMainLoop();
}
