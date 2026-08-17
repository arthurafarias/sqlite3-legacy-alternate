#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* No type belongs to this header: fnv1a_hash is a plain utility function
 * used from more than one compilation unit (db.c and stmt.c), so it gets
 * its own standalone pair instead of being duplicated or made extern from
 * whichever .c happened to need it first. */
unsigned fnv1a_hash(const char *s);

#ifdef __cplusplus
}
#endif
