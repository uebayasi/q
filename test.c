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

/* a > 5 */
struct cond cond_a = {
	.op = Q_SEL_OP_LT,
	.off = offsetof(struct x, a),
	.param = 5,
};

/* b > 4 */
struct cond cond_b = {
	.op = Q_SEL_OP_LT,
	.off = offsetof(struct x, b),
	.param = 4,
};

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

int
main(int c, char *v[])
{
	struct cond *conds[2];

	q_open(&tab_x);

	dump();

	printf("cond-a\n");
	conds[0] = &cond_a;
	q_query(&tab_x, cb_x, 1, conds);

	printf("cond-b\n");
	conds[0] = &cond_b;
	q_query(&tab_x, cb_x, 1, conds);

	printf("cond-a AND cond-b\n");
	conds[0] = &cond_a;
	conds[1] = &cond_b;
	q_query(&tab_x, cb_x, 2, conds);

	q_close(&tab_x);

	return 0;
}

/******************************************************************************/
