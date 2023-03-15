#include <stdio.h>

struct point {
    int x;
    int y;
};

struct rect {
    struct point p1;
    struct point p2;
};

void Enlarge_Rectangle(struct rect *r, struct point p) {
	if (r->p1.x > p.x) {
		r->p1.x = p.x;
	}
	if (r->p1.y > p.y) {
		r->p1.y = p.y;
	}
	if (r->p2.x < p.x) {
		r->p2.x = p.x;
	}
	if (r->p2.y < p.y) {
		r->p2.y = p.y;
	}
}

void main() {
	struct point a = {3,4};
	struct rect r = {(0,0), (1,1)};

	struct rect *p1;
	p1 = &r;

	Enlarge_Rectangle(p1, a);
	printf("p1: (%d,%d), p2: (%d, %d)\n", r.p1.x, r.p1.y, r.p2.x, r.p2.y);
}
