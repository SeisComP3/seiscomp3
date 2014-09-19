#ifndef LOCKUTIL_H
#define LOCKUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

int acquire_lock(const char *lockfile);
void release_lock(int fd);

#ifdef __cplusplus
}
#endif

#endif /* LOCKUTIL_H */

