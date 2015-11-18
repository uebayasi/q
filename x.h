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

#define	QUERY_X_CB_DECL(cb) void (*cb)(struct x *)

void open_x(void);
void close_x(void);
void query_x_a(QUERY_X_CB_DECL());
void query_x_b(QUERY_X_CB_DECL());
void query_x_a_b(QUERY_X_CB_DECL());

/******************************************************************************/

extern struct tab tab_x;
