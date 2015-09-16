// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "figure.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>

#include "convex.h"
#include "safe.h"
#include "matrix.h"
#include "console.h"
#include "script.h"
#include "drawer.h"

struct figureData figureData;

GLfloat *figureRotMatrix=0;
GLfloat figureScale=1;

static bool boundaryChanged=1;

static bool checkTopology(struct figureData *figure);
static void updateScale();

void figureInit() {
	figureData.dim=-1;
	figureData.count=0;
	figureData.vertices=0;
	figureData.boundary=0;
}

void figureNew(int dim) {
	convexDetach();
	boundaryChanged=1;
	figureDestroy(&figureData, false);
	free(figureRotMatrix);
	figureData.dim=dim;
	if (dim>=0) {
		figureRotMatrix=safeMalloc(dim*dim*sizeof(GLfloat));
		figureResetRotation();
		figureData.count=safeCalloc(dim+1, sizeof(GLint));
		figureData.boundary=safeCalloc(dim+1, sizeof(GLfloat **));
	} else {
		figureRotMatrix=0;
	}
	if (convexHull)
		convexAttach();
	drawerInvokeRedisplay();
}

bool figureWrite(char *path, struct figureData *figure, bool rotated) {
	FILE *f;
	int dim, i;
	GLfloat pos[figure->dim];
	if (figure->dim<0)
		return false;
	f=fopen(path, "wb");
	if (f==0)
		return false;
	fwrite(&figure->dim, sizeof(GLint), 1, f);
	fwrite(figure->count, sizeof(GLint), 1, f);
	for (i=0; i<figure->count[0]; i++) {
		if (rotated) {
			matrixProduct(figureRotMatrix, figure->vertices[i], pos, figure->dim, figure->dim, 1);
			fwrite(pos, sizeof(GLfloat), figure->dim, f);
		} else {
			fwrite(figure->vertices[i], sizeof(GLfloat), figure->dim, f);
		}
	}
	for (dim=1; dim<=figure->dim; dim++) {
		fwrite(figure->count+dim, sizeof(GLint), 1, f);
		for (i=0; i<figure->count[dim]; i++)
			fwrite(figure->boundary[dim][i], sizeof(GLint), figure->boundary[dim][i][0]+1, f);
	}
	fclose(f);
	return 1;
}

bool figureOpen(struct figureData *figure, bool preserveRotation) {
	if (preserveRotation && (figure->dim != figureData.dim)) {
		scriptThrowException("Wrong number of dimensions");
		return false;
	}
	if (!checkTopology(figure)) {
		scriptThrowException("Broken figure topology");
		return false;
	}

	drawerInvokeRedisplay();
	convexDetach();
	figureDestroy(&figureData, false);

	figureData.dim=figure->dim;
	figureData.count=figure->count;
	figureData.vertices=figure->vertices;
	figureData.boundary=figure->boundary;
	boundaryChanged=1;
	free(figure);
	if (!preserveRotation) {
		free(figureRotMatrix);
		figureRotMatrix=0;
	}
	if (figureData.dim>=0) {
		if (!preserveRotation) {
			figureRotMatrix=safeMalloc(figureData.dim*figureData.dim*sizeof(GLfloat));
			figureResetRotation();
			drawerSetDim(figureData.dim);
		}

		updateScale();

		if (!convexAttach()) {
			scriptThrowException("Several faces generating spaces of wrong dimension removed");
			return false;
		}
	}
	return true;
}

struct figureData *figureRead(char *path) {
	struct figureData *figure=safeCalloc(1, sizeof(struct figureData));
	FILE *f;
	GLint i, j, k;
	f=fopen(path, "rb");
	if (f==0) {
		figureDestroy(figure, true);
		return NULL;
	}
	fread(&i, sizeof(GLint), 1, f);
	if ((i<0) || (i>safeMaxDim)) {
		fclose(f);
		figureDestroy(figure, true);
		return NULL;
	}
	figure->dim=i;
	figure->count=safeMalloc(sizeof(GLint) * (figure->dim+1));
	fread(figure->count, sizeof(GLint), 1, f);
	if (figure->count[0]<0) {
		fclose(f);
		figureDestroy(figure, true);
		return NULL;
	}
	figure->vertices=safeCalloc(figure->count[0], sizeof(GLfloat *));
	for (i=0; i<figure->count[0]; i++) {
		figure->vertices[i]=safeMalloc(figure->dim*sizeof(GLfloat));
		fread(figure->vertices[i], sizeof(GLfloat), figure->dim, f);
		if (!safeCheckPos(figure->vertices[i], figure->dim)) {
			fclose(f);
			figureDestroy(figure, true);
			return NULL;
		}
	}
	figure->boundary=safeCalloc(figure->dim+1, sizeof(GLint **));
	for (i=1; i<=figure->dim; i++) {
		fread(figure->count+i, sizeof(GLint), 1, f);
		if (figure->count[i]<0) {
			fclose(f);
			figureDestroy(figure, true);
			return NULL;
		}
		figure->boundary[i]=safeCalloc(figure->count[i], sizeof(GLint *));
		for (j=0; j<figure->count[i]; j++) {
			fread(&k, sizeof(GLint), 1, f);
			if (k<0) {
				fclose(f);
				figureDestroy(figure, true);
				return NULL;
			}
			figure->boundary[i][j]=safeMalloc((k+1)*sizeof(GLint));
			figure->boundary[i][j][0]=k;
			fread(figure->boundary[i][j]+1, sizeof(GLint), k, f);
		}
	}
	fclose(f);
	return figure;
}

static bool checkTopology(struct figureData *figure) {
	int dim, i, j;
	for (dim=1; dim<=figure->dim; dim++)
		for (i=0; i<figure->count[dim]; i++)
			for (j=1; j<=figure->boundary[dim][i][0]; j++)
				if ((figure->boundary[dim][i][j]<0) || (figure->boundary[dim][i][j]>=figure->count[dim-1]))
					return false;
	return true;
}

void figureBoundaryChanged() {
	boundaryChanged=1;
	drawerInvokeRedisplay();
}

void figureResetBoundary() {
	if (figureData.dim<0) {
		scriptThrowException("Nothing opened");
		return;
	}
	int i, j;
	boundaryChanged=1;
	if (convexAttached) {
		convexDestroyHull();
	} else {
		for (i=1; i<=figureData.dim; i++) {
			for (j=0; j<figureData.count[i]; j++)
				free(figureData.boundary[i][j]);
			free(figureData.boundary[i]);
			figureData.boundary[i]=0;
			figureData.count[i]=0;
		}
	}
	if (convexHull) {
		if (!convexAttached)
			convexAttach();
		convexUpdateHull();
	}
	drawerInvokeRedisplay();
}

void figureResetRotation() {
	if (figureData.dim<0) {
		scriptThrowException("Nothing opened");
		return;
	}
	matrixIdentity(figureRotMatrix, figureData.dim);
	drawerInvokeRedisplay();
}

void figureDestroy(struct figureData *figure, bool hard) {
	if (figure->vertices) {
		for (int i=0; i<figure->count[0]; i++)
			free(figure->vertices[i]);
		free(figure->vertices);
	}
	if (figure->boundary) {
		for (int i=1; i<=figure->dim; i++) {
			if (figure->boundary[i]) {
				for (int j=0; j<figure->count[i]; j++)
					free(figure->boundary[i][j]);
				free(figure->boundary[i]);
				}
		}
		free(figure->boundary);
	}
	free(figure->count);
	if (hard) {
		free(figure);
	} else {
		figure->count=NULL;
		figure->vertices=NULL;
		figure->boundary=NULL;
		figure->dim=-1;
	}
}


void figureRotate(int axis1, int axis2, GLfloat angle) {
	static GLfloat *matrix=0;
	static int lsize=0;
	GLfloat *matrix2;
	if (lsize!=figureData.dim) {
		free(matrix);
		lsize=figureData.dim;
		matrix=safeMalloc(lsize*lsize*sizeof(GLfloat));
	}
	matrixProduct(matrixRotation(lsize, axis1, axis2, angle), figureRotMatrix, matrix, lsize, lsize, lsize);
	matrix2=figureRotMatrix;
	figureRotMatrix=matrix;
	matrix=matrix2;

	drawerInvokeRedisplay();
}

static void updateScale() {
	GLint i, j;
	GLfloat sum;
	GLfloat farest=0;
	for (i=0; i<figureData.count[0]; i++) {
		sum=0;
		for (j=0; j<figureData.dim; j++)
			sum+=figureData.vertices[i][j]*figureData.vertices[i][j];
		sum=sqrt(sum);
		if (farest<sum)
			farest=sum;
	}
	if (farest==0)
		farest=1;
	figureScale=1/farest;
}

int figureVerticesOfFaces(int ***facevertOut) {
	int (*neighbour)[2]=safeMalloc(2*(figureData.count[0])*sizeof(int));
	static int **facevert=0;
	static int faces=0, validFaces=0;
	int i, j, e, v1, v2;
	int broken;
	if (!boundaryChanged) {
		*facevertOut=facevert;
		return validFaces;
	}
	for (i=0; i<faces; i++)
		free(facevert[i]);
	free(facevert);
	faces=figureData.count[2];
	facevert=safeCalloc(faces, sizeof(int *));
	for (i=0, validFaces=0; i<faces; i++) {
		broken=0;
		free(facevert[validFaces]);
		for (j=0; j<figureData.count[0]; j++) {
			neighbour[j][0]=-1;
			neighbour[j][1]=-1;
		}
		facevert[validFaces]=safeMalloc((figureData.boundary[2][i][0]+1)*sizeof(int));
		facevert[validFaces][0]=figureData.boundary[2][i][0];
		for (j=1; j<=facevert[validFaces][0]; j++) {
			e=figureData.boundary[2][i][j];
			v1=figureData.boundary[1][e][1];
			v2=figureData.boundary[1][e][2];
			if (neighbour[v1][0]==-1)
				neighbour[v1][0]=v2;
			else if (neighbour[v1][1]==-1)
				neighbour[v1][1]=v2;
			else {
				consolePrintErr("Inconsistent border of 2D faces, skipping drawing");
				broken=1;break;
			}
			if (neighbour[v2][0]==-1)
				neighbour[v2][0]=v1;
			else if (neighbour[v2][1]==-1)
				neighbour[v2][1]=v1;
			else {
				consolePrintErr("Inconsistent border of 2D faces, skipping drawing");
				broken=1;break;
			}
		}
		if (broken)
			continue;
		for (j=0; j<facevert[validFaces][0]; j++) {
			if (v1<0) {
				consolePrintErr("Inconsistent border of 2D faces, skipping drawing");
				broken=1;break;
			}
			facevert[validFaces][j+1]=v1;
			if (neighbour[v1][0]!=v2) {
				v2=v1;
				v1=neighbour[v1][0];
			} else {
				v2=v1;
				v1=neighbour[v1][1];
			}
			neighbour[v2][0]=-1;
			neighbour[v2][1]=-1;
		}
		if (broken)
			continue;
		validFaces++;
	}
	free(neighbour);
	*facevertOut=facevert;
	boundaryChanged=0;
	return validFaces;
}

void figureVertexMove(int vertex, GLfloat *pos) {
	if (!convexAttached)
		convexAttach();
	convexVertexMove(vertex, pos);
	matrixCopy(pos, figureData.vertices[vertex], figureData.dim);
	updateScale();
	drawerInvokeRedisplay();
}

int figureVertexAdd(GLfloat *pos) {
	if (!convexAttached)
		convexAttach();
	figureData.vertices=safeRealloc(figureData.vertices, ++figureData.count[0]*sizeof(GLfloat *));
	GLfloat *pos2=safeMalloc(figureData.dim*sizeof(GLfloat));
	matrixCopy(pos, pos2, figureData.dim);
	figureData.vertices[figureData.count[0]-1]=pos2;
	updateScale();
	convexVertexAdd(figureData.count[0]-1);
	drawerInvokeRedisplay();
	return figureData.count[0]-1;
}

void figureVertexRm(int vertex) {
	int i;
	if (!convexAttached)
		convexAttach();
	convexVertexRm(vertex);
	free(figureData.vertices[vertex]);
	for (i=vertex; i<figureData.count[0]-1; i++)
		figureData.vertices[i]=figureData.vertices[i+1];
	figureData.count[0]--;
	figureData.vertices=safeRealloc(figureData.vertices, figureData.count[0]*sizeof(GLfloat *));
	updateScale();
	drawerInvokeRedisplay();
}


int figureDistCmpZero(GLfloat distance, GLfloat tolerance) {
	return (fabs(distance)*figureScale>tolerance?(distance>0?1:-1):0);
}

int figureDistCmpZeroSq(GLfloat squaredDistance, GLfloat tolerance) {
	return squaredDistance*figureScale*figureScale>tolerance*tolerance;
}

int figureDistCmp(GLfloat a, GLfloat b, GLfloat tolerance) {
	return figureDistCmpZero(a-b, tolerance);
}
