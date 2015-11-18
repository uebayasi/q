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

int
main(int c, char *v[])
{
	open_x();

	query_x_a(NULL);
	query_x_b(NULL);
	query_x_a_b(NULL);

	close_x();

	return 0;
}

/******************************************************************************/
