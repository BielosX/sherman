#ifndef __ERROR_H__
#define __ERROR_H__

#define TRY(f, g) \
    if (f < 0) {\
        goto g; \
    } \

#define TRY_CATCH(f, e) \
    if (f < 0) { \
        e \
    } \

#endif
