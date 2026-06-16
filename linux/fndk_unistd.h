#pragma once

#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t fndk_read(int fd, void *buf, size_t count);
ssize_t fndk_write(int fd, const void *buf, size_t count);
int fndk_close(int fd);

#ifdef __cplusplus
};
#endif
