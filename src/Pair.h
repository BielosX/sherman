#ifndef __PAIR_H__
#define __PAIR_H__

struct Pair {
    void* fst;
    void* snd;
};

struct Pair* _new_pair(void* fst,void* snd, size_t fst_s, size_t snd_s);
void delete_pair(struct Pair* pair);

#define first(pair, type) ((type*)pair->fst)
#define second(pair, type) ((type*)pair->snd)


#define new_pair(fst, snd) _new_pair(&fst, &snd, sizeof(fst), sizeof(snd))
#define new_pair1(fst, snd)({ \
    typeof(fst) a; \
    typeof(snd) b; \
    a = fst; \
    b = snd; \
    _new_pair (&a, &b, sizeof(a), sizeof(b)); \
})

#endif
