#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Pair {
    void* fst;
    void* snd;
};

struct X {
    int a;
};

#define first(pair, type) ((type*)pair->fst)
#define second(pair, type) ((type*)pair->snd)

struct Pair* _new_pair(void* fst,void* snd, size_t fst_s, size_t snd_s) {
    struct Pair* p;
    p = (struct Pair*)malloc(sizeof(struct Pair));
    memset(p, 0, sizeof(struct Pair));
    p->fst = malloc(fst_s);
    p->snd = malloc(snd_s);
    memcpy(p->fst, fst, fst_s);
    memcpy(p->snd, snd, snd_s);
    return p;
}

#define new_pair(fst, snd) _new_pair(&fst, &snd, sizeof(fst), sizeof(snd))
#define new_pair1(fst, snd)({ \
    typeof(fst) a; \
    typeof(snd) b; \
    a = fst; \
    b = snd; \
    _new_pair (&a, &b, sizeof(a), sizeof(b)); \
})

void delete_pair(struct Pair* pair) {
    free(pair->fst);
    free(pair->snd);
    free(pair);
}

int main(void) {
    struct Pair* p;
    struct Pair* p2;
    struct X x;

    x.a = 9;
    p = new_pair1(5, 'a');
    p2 = new_pair(x, x);

    printf("%d\n", *first(p, int));
    printf("%c\n", *second(p, char));

    printf("%d\n", first(p2, struct X)->a);

    delete_pair(p);
    delete_pair(p2);
}
