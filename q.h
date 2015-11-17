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

#define	ITER_CB_DECL(f)	void (*f)(struct tab *, int, int, struct sel *)

/******************************************************************************/

int
cmp_int(const void *p, const void *q);
int
idx_int(void *x, int off);
int
cond_LT(void *v, int off, int p);
int
cond_GT(void *v, int off, int p);
int
cond_LE(void *v, int off, int p);
int
cond_GE(void *v, int off, int p);
void
q_sel(struct tab *tab, struct sel *sel);
void
q_sel_done(struct sel *sel);
void
q_iter(struct tab *tab, ITER_CB_DECL(cb), int dim, struct sel *sels);
void
q_query(struct tab *tab, ITER_CB_DECL(cb), int dim, struct cond *conds[]);
void
q_idx(struct tab *tab, int dim);
void
q_open(struct tab *tab);
void
q_close(struct tab *tab);

/******************************************************************************/
