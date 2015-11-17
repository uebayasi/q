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

struct tab {
	const char *name;
	void *data;
	int nrows;
	int ncols;
	int colsize;
	int **idxs;
	int (**idxcmps)(const void *, const void *);
};

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
cond_LT(void *v, int off, int p)
{
	return (idx_int(v, off) > p);
}

static int
cond_GT(void *v, int off, int p)
{
	return (idx_int(v, off) < p);
}

static int
cond_LE(void *v, int off, int p)
{
	return (idx_int(v, off) >= p);
}

static int
cond_GE(void *v, int off, int p)
{
	return (idx_int(v, off) <= p);
}

static void
q_sel(struct tab *tab, struct sel *sel)
{
	int i, j;

	(void)cond_LT;
	(void)cond_GT;
	(void)cond_LE;
	(void)cond_GE;

	for (i = 0; i < sel->idx.len; i++) {
		void *v;

		v = (char *)tab->data + tab->colsize * sel->idx.vec[i];
		if ((*sel->cond->cond)(v, sel->cond->off, sel->cond->param))
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

#define	ITER_CB_DECL(f)	void (*f)(struct tab *, int, int, struct sel *)

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
q_iter(struct tab *tab, ITER_CB_DECL(cb), int dim, struct sel *sels)
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
		(*cb)(tab, dim, i, sels);
	}
}

static void
q_query(struct tab *tab, ITER_CB_DECL(cb), int dim, struct cond *conds[])
{
	struct sel sels[dim];
	int i;

	for (i = 0; i < dim; i++) {
		sels[i].idx.vec = tab->idxs[i];
		sels[i].idx.len = tab->nrows;
		sels[i].cond = conds[i];
		q_sel(tab, &sels[i]);
	}
	q_iter(tab, cb, dim, sels);
	for (i = 0; i < dim; i++)
		q_sel_done(&sels[i]);
}

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

static struct tab tab_x;

static int
cmp_x_a(const void *p, const void *q)
{
	const int pi = *(const int *)p;
	const int qi = *(const int *)q;
	const int pv = xx(tab_x.data, pi)->a;
	const int qv = xx(tab_x.data, qi)->a;
	return (pv < qv) ? -1 : (pv > qv) ? 1 : 0;
}

static int
cmp_x_b(const void *p, const void *q)
{
	const int pi = *(const int *)p;
	const int qi = *(const int *)q;
	const int pv = xx(tab_x.data, pi)->b;
	const int qv = xx(tab_x.data, qi)->b;
	return (pv < qv) ? -1 : (pv > qv) ? 1 : 0;
}

static void
q_idx(int n, int (*cmp)(const void *, const void *), int **ridx)
{
	int i;

	int *idx = malloc(sizeof(int) * tab_x.nrows);
	for (i = 0; i < tab_x.nrows; i++)
		idx[i] = i;
	qsort(idx, tab_x.nrows, sizeof(int), cmp);
	*ridx = idx;
}

static void
q_open(void)
{
	struct stat st;
	int i;

	int fd = open("d", O_RDONLY);
	if (fd < 0)
		exit(1);
	if (fstat(fd, &st) < 0)
		exit(1);
	tab_x.nrows = st.st_size / sizeof(struct x);
	tab_x.data = mmap(NULL, sizeof(struct x) * tab_x.nrows, PROT_READ,
	    MAP_FILE | MAP_SHARED, fd, 0);
	if (tab_x.data == (void *)-1)
		exit(1);
	for (i = 0; i < tab_x.ncols; i++)
		q_idx(tab_x.nrows, tab_x.idxcmps[i], &tab_x.idxs[i]);
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
cb_x(struct tab *tab, int dim, int idx, struct sel *sels)
{
	struct x *x = xx(tab_x.data, idx);
	int first = 1;
	int i;

	printf("%d: (", idx);
	for (i = 0; i < dim; i++) {
		printf("%s%d",
		    first ? "" : ", ",
		    idx_int(x, sels[i].cond->off));
		first = 0;
	}
	printf(") matches!\n");
}

static int *tab_x_idxs[2];
static int (*tab_x_idxcmps[2])(const void *, const void *) = {
	cmp_x_a,
	cmp_x_b,
};
static struct tab tab_x = {
	.name = "d",
	.ncols = 2,
	.colsize = sizeof(struct x),
	.idxs = tab_x_idxs,
	.idxcmps = tab_x_idxcmps,
};

void
dump(void)
{
	struct x *x = tab_x.data;
	int i;

	for (i = 0; i < tab_x.nrows; i++) {
		printf("%d: (%d, %d)\n", i, x->a, x->b);
		x++;
	}
}

int
main(int c, char *v[])
{
	struct cond *conds1[1];
	struct cond *conds2[2];

	q_open();

	dump();

	printf("cond-a\n");
	conds1[0] = &cond_a;
	q_query(&tab_x, cb_x, 1, conds1);

	printf("cond-b\n");
	conds1[0] = &cond_b;
	q_query(&tab_x, cb_x, 1, conds1);

	printf("cond-a AND cond-b\n");
	conds2[0] = &cond_a;
	conds2[1] = &cond_b;
	q_query(&tab_x, cb_x, 2, conds2);

	return 0;
}

/******************************************************************************/
