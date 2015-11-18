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

/******************************************************************************/

void open_x(void);
void close_x(void);
void query_x_a(ITER_CB_DECL());
void query_x_b(ITER_CB_DECL());
void query_x_a_b(ITER_CB_DECL());

/******************************************************************************/

extern struct tab tab_x;
