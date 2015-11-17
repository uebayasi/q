/******************************************************************************/

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "q.h"

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
sel_x_a(struct cond *cond, struct tab *tab, struct sel *sel)
{
	void *v;
	int i;

	for (i = 0; i < sel->idx.len; i++) {
		v = (char *)tab->data + tab->colsize * sel->idx.vec[i];
		if (cond_LT(v, cond->off, cond->param))
			break;
	}
	if (i == sel->idx.len)
		exit(1);
	sel->set.p = i;
	sel->set.q = sel->idx.len - 1;
}

static void
sel_x_b(struct cond *cond, struct tab *tab, struct sel *sel)
{
	void *v;
	int i;

	for (i = 0; i < sel->idx.len; i++) {
		v = (char *)tab->data + tab->colsize * sel->idx.vec[i];
		if (cond_LT(v, cond->off, cond->param))
			break;
	}
	if (i == sel->idx.len)
		exit(1);
	sel->set.p = i;
	sel->set.q = sel->idx.len - 1;
}

static void
cb_x(struct tab *tab, int dim, int idx, struct sel *sels, struct cond *conds[])
{
	struct x *x = xx(tab_x.data, idx);
	int first = 1;
	int i;

	printf("%d: (", idx);
	for (i = 0; i < dim; i++) {
		printf("%s%d",
		    first ? "" : ", ",
		    idx_int(x, conds[i]->off));
		first = 0;
	}
	printf(") matches!\n");
}

static void
dump(void)
{
	struct x *x = tab_x.data;
	int i;

	for (i = 0; i < tab_x.nrows; i++) {
		printf("%d: (%d, %d)\n", i, x->a, x->b);
		x++;
	}
}

/* a > 5 */
static struct cond cond_a = {
	.sel = sel_x_a,
	.op = Q_SEL_OP_LT,
	.off = offsetof(struct x, a),
	.param = 5,
};

/* b > 4 */
struct cond cond_b = {
	.sel = sel_x_b,
	.op = Q_SEL_OP_LT,
	.off = offsetof(struct x, b),
	.param = 4,
};

static int (*tab_x_idxcmps[2])(const void *, const void *) = {
	cmp_x_a,
	cmp_x_b,
};
static int *tab_x_idxs[2];
static struct tab tab_x = {
	.name = "d",
	.ncols = 2,
	.colsize = sizeof(struct x),
	.idxcmps = tab_x_idxcmps,
	.idxs = tab_x_idxs,
};

int
main(int c, char *v[])
{
	struct cond *conds[2] = {
		&cond_a,
		&cond_b,
	};

	q_open(&tab_x);

	dump();

	printf("cond-a\n");
	q_query(&tab_x, cb_x, 1, &tab_x.idxs[0], &conds[0]);

	printf("cond-b\n");
	q_query(&tab_x, cb_x, 1, &tab_x.idxs[1], &conds[1]);

	printf("cond-a AND cond-b\n");
	q_query(&tab_x, cb_x, 2, &tab_x.idxs[0], &conds[0]);

	q_close(&tab_x);

	return 0;
}

/******************************************************************************/
