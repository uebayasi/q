/******************************************************************************/

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX2(a,b) (((a)>(b))?(a):((a)<(b))?(b):(a))
#define MIN2(a,b) (((a)>(b))?(b):((a)<(b))?(a):(a))

/******************************************************************************/

struct set {
	int p;
	int q;
};

struct cond {
	int (*cond)(void *, int, int);
	int off;
	int param;
};

struct vec {
	int *vec;
	int len;
};

struct sel {
	struct vec idx;
	struct vec ord;
	struct set set;
	const struct cond *cond;
};

/******************************************************************************/

struct x {
	int a;
	int b;
};

static inline struct x *
xx(void *v, int idx)
{
	struct x *x = (struct x *)v;
	return &x[idx];
}

struct tab {
	const char *name;
	void *data;
	int nrows;
	int ncols;
	int *idxs[2];
};

static struct tab x_tab;

/******************************************************************************/

static int
cmp_int(const void *p, const void *q)
{
	const int pi = *(const int *)p;
	const int qi = *(const int *)q;

	return (pi < qi) ? -1 : (pi > qi) ? 1 : 0;
}

static int
idx_int(void *x, int off)
{
	char *cp = (char *)x + off;

	return (*(int *)cp);
}

static int
cond_LT(void *x, int off, int p)
{
	return (idx_int(x, off) > p);
}

static int
cond_GT(void *x, int off, int p)
{
	return (idx_int(x, off) < p);
}

static int
cond_LE(void *x, int off, int p)
{
	return (idx_int(x, off) >= p);
}

static int
cond_GE(void *x, int off, int p)
{
	return (idx_int(x, off) <= p);
}

static void
q_sel(struct sel *sel)
{
	int i, j;

	(void)cond_LT;
	(void)cond_GT;
	(void)cond_LE;
	(void)cond_GE;

	for (i = 0; i < sel->idx.len; i++) {
		if ((*sel->cond->cond)(xx(x_tab.data, sel->idx.vec[i]),
		    sel->cond->off, sel->cond->param))
			break;
	}
	if (i == sel->idx.len)
		exit(1);
	sel->set.p = i;
	sel->set.q = sel->idx.len - 1;
	sel->ord.len = (sel->set.q - sel->set.p + 1);
	sel->ord.vec = malloc(sizeof(int) * sel->ord.len);
	for (i = sel->set.p, j = 0; i <= sel->set.q; i++, j++)
		sel->ord.vec[j] = sel->idx.vec[i];
	qsort(sel->ord.vec, sel->ord.len, sizeof(int), cmp_int);
}

static void
q_sel_done(struct sel *sel)
{
	if (sel->ord.vec != NULL)
		free(sel->ord.vec);
}

#define	ITER_CB_DECL(f)	void (*f)(int, int, struct sel *)

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
q_iter(ITER_CB_DECL(cb), int dim, struct sel *sels)
{
	int min = minord(sels, dim);
	int max = maxord(sels, dim);
	struct cur {
		int *cur;
	} curs[dim];
	int i, j;
	for (j = 0; j < dim; j++)
		curs[j].cur = sels[j].ord.vec;
	for (i = min; i <= max; i++) {
		for (j = 0; j < dim; j++) {
			while (*curs[j].cur < i)
				curs[j].cur++;
			if (*curs[j].cur > i)
				continue;
		}
		(*cb)(dim, i, sels);
	}
}

static void
q_query(ITER_CB_DECL(cb), int dim, struct cond *conds[])
{
	struct sel sels[dim];
	int i;

	for (i = 0; i < dim; i++) {
		sels[i].idx.vec = x_tab.idxs[i];
		sels[i].idx.len = x_tab.nrows;
		sels[i].cond = conds[i];
		q_sel(&sels[i]);
	}
	q_iter(cb, dim, sels);
	for (i = 0; i < dim; i++)
		q_sel_done(&sels[i]);
}

/******************************************************************************/

static int
cmp_x_a(const void *p, const void *q)
{
	const int pi = *(const int *)p;
	const int qi = *(const int *)q;

	const int pv = xx(x_tab.data, pi)->a;
	const int qv = xx(x_tab.data, qi)->a;
	return (pv < qv) ? -1 : (pv > qv) ? 1 : 0;
}

static int
cmp_x_b(const void *p, const void *q)
{
	const int pi = *(const int *)p;
	const int qi = *(const int *)q;

	const int pv = xx(x_tab.data, pi)->b;
	const int qv = xx(x_tab.data, qi)->b;
	return (pv < qv) ? -1 : (pv > qv) ? 1 : 0;
}

static void
q_idx(int n, int **ridx, int (*cmp)(const void *, const void *))
{
	int i;

	int *idx = malloc(sizeof(int) * x_tab.nrows);
	for (i = 0; i < x_tab.nrows; i++)
		idx[i] = i;
	qsort(idx, x_tab.nrows, sizeof(int), cmp);
	*ridx = idx;
}

static void
q_open(void)
{
	/* database creation */
	struct stat st;

	int fd = open("d", O_RDONLY);
	if (fd < 0)
		exit(1);
	printf("open'ed!\n");

	if (fstat(fd, &st) < 0)
		exit(1);
	x_tab.nrows = st.st_size / sizeof(struct x);
	x_tab.data = mmap(NULL, sizeof(struct x) * x_tab.nrows, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
	if (x_tab.data == (void *)-1)
		exit(1);
	printf("mmap'ed!\n");

	q_idx(x_tab.nrows, &x_tab.idxs[0], cmp_x_a);
	q_idx(x_tab.nrows, &x_tab.idxs[1], cmp_x_b);

	struct x *x = x_tab.data;
	int i;
	for (i = 0; i < x_tab.nrows; i++) {
		printf("%d: (%d, %d)\n", i, x->a, x->b);
		x++;
	}
}

/******************************************************************************/

/* a > 5 */
struct cond cond_a = {
	.cond = cond_LT,
	.off = offsetof(struct x, a),
	.param = 5,
};

/* b > 4 */
struct cond cond_b = {
	.cond = cond_LT,
	.off = offsetof(struct x, b),
	.param = 4,
};

void
iter_cb1(int dim, int idx, struct sel *sels)
{
	struct x *x = xx(x_tab.data, idx);

	printf("%d: (%d) matches!\n", idx,
	    idx_int(x, sels[0].cond->off));
}

void
iter_cb2(int dim, int idx, struct sel *sels)
{
	struct x *x = xx(x_tab.data, idx);

	printf("%d: (%d, %d) matches!\n", idx,
	    idx_int(x, sels[0].cond->off),
	    idx_int(x, sels[1].cond->off));
}

static struct tab x_tab = {
	.name = "d",
	.ncols = 2,
};

int
main(int c, char *v[])
{
	struct cond *conds1[1];
	struct cond *conds2[2];

	q_open();

	printf("cond-a\n");
	conds1[0] = &cond_a;
	q_query(iter_cb1, 1, conds1);

	printf("cond-b\n");
	conds1[0] = &cond_b;
	q_query(iter_cb1, 1, conds1);

	printf("cond-a AND cond-b\n");
	conds2[0] = &cond_a;
	conds2[1] = &cond_b;
	q_query(iter_cb2, 2, conds2);

	return 0;
}

/******************************************************************************/
