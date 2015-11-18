/******************************************************************************

 q - minimal query language for C

 ******************************************************************************/

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX2(a,b) (((a)>(b))?(a):((a)<(b))?(b):(a))
#define MIN2(a,b) (((a)>(b))?(b):((a)<(b))?(a):(a))

#include "q.h"

/******************************************************************************/

static int
cmp_int(const void *p, const void *q)
{
	const int pi = *(const int *)p;
	const int qi = *(const int *)q;

	return (pi < qi) ? -1 : (pi > qi) ? 1 : 0;
}

static void
q_sel(struct tab *tab, struct sel *sel, struct cond *cond)
{
	int i, j;

	(*cond->sel)(cond, tab, sel);
	sel->ord.len = (sel->set.q - sel->set.p + 1);
	sel->ord.vec = malloc(sizeof(int) * sel->ord.len);
	for (i = sel->set.p, j = 0; i <= sel->set.q; i++, j++)
		sel->ord.vec[j] = sel->idx.vec[i];
	qsort(sel->ord.vec, sel->ord.len, sizeof(int), cmp_int);
}

static void
q_sel_done(struct sel *sel)
{
	free(sel->ord.vec);
}

static int
minord(struct sel *sels, int dim)
{
	int min = sels[0].ord.vec[0];
	int i;

	for (i = 0; i < dim; i++)
		min = MAX2(min, sels[i].ord.vec[0]);
	return min;
}

static int
maxord(struct sel *sels, int dim)
{
	int max = sels[0].ord.vec[sels[0].ord.len - 1];
	int i;

	for (i = 0; i < dim; i++)
		max = MIN2(max, sels[i].ord.vec[sels[i].ord.len - 1]);
	return max;
}

static void
q_iter(struct tab *tab, ITER_CB_DECL(cb), int dim, struct sel *sels,
    struct cond *conds[])
{
	int min = minord(sels, dim);
	int max = maxord(sels, dim);
	int *curs[dim];
	int i, j, matched;

	for (i = 0; i < dim; i++)
		curs[i] = sels[i].ord.vec;
	for (i = min; i <= max; i++) {
		matched = 0;
		for (j = 0; j < dim; j++) {
			while (*curs[j] < i)
				curs[j]++;
			if (*curs[j] > i)
				break;
			matched++;
		}
		if (matched == dim)
			(*cb)(tab, dim, i, sels, conds);
	}
}

static void
q_idx(struct tab *tab, int dim)
{
	int *v;
	int i;

	v = malloc(sizeof(int) * tab->nrows);
	for (i = 0; i < tab->nrows; i++)
		v[i] = i;
	qsort(v, tab->nrows, sizeof(int), tab->idxcmps[dim]);
	tab->idxs[dim] = v;
}

/******************************************************************************/

void
q_query(struct tab *tab, ITER_CB_DECL(cb), int dim, struct cond *conds[])
{
	struct sel sels[dim];
	int i;

	for (i = 0; i < dim; i++) {
		sels[i].idx.vec = tab->idxs[conds[i]->colidx];
		sels[i].idx.len = tab->nrows;
		q_sel(tab, &sels[i], conds[i]);
	}
	q_iter(tab, cb, dim, sels, conds);
	for (i = 0; i < dim; i++)
		q_sel_done(&sels[i]);
}

void
q_open(struct tab *tab)
{
	struct stat st;
	int i;

	tab->fd = open(tab->name, O_RDONLY);
	if (tab->fd < 0)
		exit(1);
	if (fstat(tab->fd, &st) < 0)
		exit(1);
	if (st.st_size % tab->colsize != 0)
		exit(1);
	tab->nrows = st.st_size / tab->colsize;
	tab->data = mmap(NULL, tab->colsize * tab->nrows, PROT_READ,
	    MAP_FILE | MAP_SHARED, tab->fd, 0);
	if (tab->data == (void *)-1)
		exit(1);
	for (i = 0; i < tab->ncols; i++)
		q_idx(tab, i);
}

void
q_close(struct tab *tab)
{
	(void)munmap(tab->data, tab->colsize * tab->nrows);
	(void)close(tab->fd);
}

/******************************************************************************/
