// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "debug.h"

#ifdef DEBUG

#include <stdio.h>

#include "convex.h"
#include "convexFig.h"
#include "convexSpace.h"
#include "script.h"
#include "scriptFigure.h"
#include "figure.h"

void convexFigPrint() { // mark unsafe
	printf("digraph{");
	convexFigMarkReset(convexFigMarkIdLayer);
	convexFigPrintR(convexFigure, "convexFigure");
	convexFigPrintR(convexFreeVertices, "convexFreeVertices");
	printf("}\n");
	convexFigBstPrint(convexSpaces); // print even convexSpaces bst tree
}

void convexFigPrintR(struct convexFigList *list, char *parent) {
	char str[50];
	while (list) {
		if (list->fig->space->dim>=0) {
			sprintf(str, "%dD - %02d - %08x",list->fig->space->dim,list->fig->index,list->fig->hash);
			printf("\"%s\"->\"%s\";", parent, str);
			if (!convexFigMarkGet(list->fig, convexFigMarkIdLayer)) {
				convexFigPrintR(list->fig->boundary, str);
				convexFigMarkSet(list->fig, convexFigMarkIdLayer);
			}
		}
		list=list->next;
	}
}

void convexFigBstPrint(struct convexFigBst *bst) {
	printf("digraph{");
	convexFigBstPrintR(bst, "BST","");
	printf("}\n");
}

struct convexFigBst {
	struct convexFig *fig;
	struct convexFigBst *left;
	struct convexFigBst *right;
	int leaning;
};
void convexFigBstPrintR(struct convexFigBst *bst, char *parent, char *attr) {
	char str[50];
	if (!bst)
		return;
	sprintf(str, "%dD - %02d - %08x - %f (%d)",
		bst->fig->space->dim,
		bst->fig->index,
		bst->fig->hash,
		bst->fig->space->hash,
		bst->leaning);
	printf("\"%s\"->\"%s\" [%s];", parent, str, attr);
	convexFigBstPrintR(bst->left, str, "label=L");
	convexFigBstPrintR(bst->right, str, "label=R");
}

int convexFigBstCheck(struct convexFigBst *bst, char *str) {
	int depth;
	GLdouble min, max;
	return convexFigBstCheckR(bst, &min, &max, &depth, str);
}

int convexFigBstCheckR(struct convexFigBst *bst, GLdouble *min, GLdouble *max, int *depth, char *str) {
	if (!bst) {
		*depth=0;
		return 0;
	}
	int ldepth, lret, rdepth, rret;
	GLdouble lmin, lmax, rmin, rmax;
	if (lret=convexFigBstCheckR(bst->left, &lmin, &lmax, &ldepth, str))
		return lret;
	if (rret=convexFigBstCheckR(bst->right, &rmin, &rmax, &rdepth, str))
		return rret;
	if (ldepth)
		*min=lmin;
	else
		*min=bst->fig->space->hash;
	if (rdepth)
		*max=rmax;
	else
		*max=bst->fig->space->hash;
	*depth=(rdepth>ldepth?rdepth:ldepth)+1;
	if ((ldepth && (lmax>bst->fig->space->hash)) || (rdepth && (rmin<bst->fig->space->hash))) {
		printf("bst sort err: %s\n", str);
		convexFigBstPrint(bst);
		exit(30);
	}
	return 0;
	if (bst->leaning!=rdepth-ldepth) {
		printf("bst leaning err: %s\n", str);
		convexFigBstPrint(bst);
		exit(31);
	}
	return 0;

}

void convexLoopDetectPrint(int count, int parent, int child, int index, int next) {
	printf("convexLoopDetect: %dx(%08x-%08x) - %d/%d\n",
		count,
		parent,
		child,
		index,
		next);
}

// consoleCmds.c:
struct trie {
	char c;
	struct trie *sibling;
	struct trie *child;
	char *scriptExpr;
	char *paramsFlags;
	int params; // -1 for variable
};

void triePrint(struct trie *trie, int indent) {
	if (!trie)
		return;
	printf("%c%c", (trie->c?trie->c:'_'), (trie->scriptExpr || trie->paramsFlags || trie->params? '*':'-'));
	triePrint(trie->child, indent+2);
	while (trie=trie->sibling) {
		printf("\n%*c%c%c", indent, '|', (trie->c?trie->c:'_'), (trie->scriptExpr || trie->paramsFlags || trie->params? '*':'-'));
		triePrint(trie->child, indent+2);
	}
}
void consoleCmdsTriePrint(struct trie *trie) {
	printf("--- TRIE START ---\n--");
	triePrint(trie, 2);
	printf("\n--- TRIE  END  ---\n");
}


// Convex hull progress

void debugProgrStart(struct figureData *figure, struct convexFig ***shadow) {
	char *figExpr=scriptFigureToPythonExpr(figure);
	printf("import gf\nimport debugPlayback\n");
	if (figExpr) {
		printf("debugPlayback.open(%s, {", figExpr);
		for (int i=0; i<=figure->dim; i++) {
			for (int j=0; j<figure->count[i]; j++) {
				if (shadow[i][j])
					printf("%u:(%d,%d),", shadow[i][j]->hash, i, j);
			}
		}
		printf("})\n");
	} else {
		printf("debugPlayback.figureOpen(None, None)\n");
	}
}

void debugProgrAttach(struct convexFig *parent, struct convexFig *child) {
	printf("debugPlayback.attach(%u,%u)\n", parent->hash, child->hash);
}

void debugProgrDetach(struct convexFig *parent, struct convexFig *child) {
	printf("debugPlayback.detach(%u,%u)\n", parent->hash, child->hash);
}

void debugProgrHashChange(unsigned int oldHash, unsigned int newHash) {
	printf("debugPlayback.rename(%u,%u)\n", oldHash, newHash);
}

#endif
