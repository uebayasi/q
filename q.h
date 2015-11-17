/******************************************************************************/

struct set;
struct cond;
struct vec;
struct sel;
struct tab;

struct set {
	int p;
	int q;
};

struct cond {
	void (*sel)(struct cond *, struct tab *, struct sel *);
	int op;
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
};

struct tab {
	const char *name;
	int fd;
	void *data;
	int colsize;
	int ncols;
	int nrows;
	int (**idxcmps)(const void *, const void *);
	int **idxs;
};

/******************************************************************************/

#define	ITER_CB_DECL(f)	\
	void (*f)(struct tab *, int, int, struct sel *, struct cond *[])

void q_query(struct tab *, ITER_CB_DECL(), int, int *[], struct cond *[]);
void q_open(struct tab *);
void q_close(struct tab *);

/******************************************************************************/

enum Q_SEL_OP {
	Q_SEL_OP_LT,
	Q_SEL_OP_GT,
	Q_SEL_OP_LE,
	Q_SEL_OP_GE,
};

static inline int
idx_int(void *x, int off)
{
	char *cp = (char *)x + off;

	return (*(int *)cp);
}

static inline int
cond_LT(void *v, int off, int p)
{
	return (idx_int(v, off) > p);
}

static inline int
cond_GT(void *v, int off, int p)
{
	return (idx_int(v, off) < p);
}

static inline int
cond_LE(void *v, int off, int p)
{
	return (idx_int(v, off) >= p);
}

static inline int
cond_GE(void *v, int off, int p)
{
	return (idx_int(v, off) <= p);
}

/******************************************************************************/
