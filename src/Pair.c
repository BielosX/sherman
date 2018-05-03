#include <stdlib.h>
#include <string.h>

#include "Pair.h"

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

void delete_pair(struct Pair* pair) {
    free(pair->fst);
    free(pair->snd);
    free(pair);
}
