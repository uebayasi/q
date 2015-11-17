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
	int fd;
	void *data;
	int nrows;
	int ncols;
	int colsize;
	int **idxs;
	int (**idxcmps)(const void *, const void *);
};

/******************************************************************************/
