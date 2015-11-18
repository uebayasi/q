/******************************************************************************/

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "q.h"
#include "x.h"

/******************************************************************************/

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

int
main(int c, char *v[])
{
	open_x();

	query_x_a(cb_x);
	query_x_b(cb_x);
	query_x_a_b(cb_x);

	close_x();

	return 0;
}

/******************************************************************************/
