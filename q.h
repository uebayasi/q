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

int cmp_int(const void *, const void *);
int idx_int(void *, int);
int cond_LT(void *, int, int);
int cond_GT(void *, int, int);
int cond_LE(void *, int, int);
int cond_GE(void *, int, int);
void q_sel(struct tab *, struct sel *);
void q_sel_done(struct sel *);
void q_iter(struct tab *, ITER_CB_DECL(), int, struct sel *);
void q_query(struct tab *, ITER_CB_DECL(), int, struct cond *[]);
void q_idx(struct tab *, int);
void q_open(struct tab *);
void q_close(struct tab *);

/******************************************************************************/
