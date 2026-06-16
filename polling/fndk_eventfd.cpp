#include <cstdint>
#include <psp2/kernel/threadmgr.h>
#include <cerrno>
#include <cstdio>
#include <sys/unistd.h>
#include "FalsoNDK_Utils.h"

#include "fndk_eventfd.h"

#define EVENTFD_MARGIN 256
#define EVENTFD_MAX 64

typedef struct eventfd_internal {
    int fd = -1; // >=0 indicates that it's in use
    uint64_t value{};
    int flags{};
} eventfd_internal;

static eventfd_internal eventfd_pool[EVENTFD_MAX];
SceKernelLwMutexWork eventfd_pool_mutex{};

__attribute__((constructor))
static void eventfd_pool_init() {
    sceKernelCreateLwMutex(&eventfd_pool_mutex, "eventfd_pool_mutex", 0, 0, NULL);
}

int fndk_eventfd(unsigned int initval, int flags) {
    sceKernelLockLwMutex(&eventfd_pool_mutex, 1, NULL);

    eventfd_internal * fd = nullptr;
    for (int i = 0; i < EVENTFD_MAX; ++i) {
        if (eventfd_pool[i].fd == -1) {
            eventfd_pool[i].fd = i + EVENTFD_MARGIN;
            fd = &eventfd_pool[i];
            break;
        }
    }

    if (!fd) {
        sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
        errno = EMFILE;
        return -1;
    }

    fd->value = initval;
    fd->flags = flags;

    sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
#ifdef DEBUG_POLL_AND_WAKE
    ALOGD("Created eventfd #%i from addr %p", fd->fd, __builtin_return_address(0));
#endif
    return fd->fd;
}

bool is_eventfd(int fd) {
#ifdef FNDK_SAFER_SLOWER
    eventfd_internal * p = nullptr;

    sceKernelLockLwMutex(&eventfd_pool_mutex, 1, NULL);

    for (int i = 0; i < EVENTFD_MAX; ++i) {
        if (eventfd_pool[i].fd == fd) {
            p = &eventfd_pool[i];
            break;
        }
    }

    sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);

    return p != nullptr;
#else
    return (fd >= EVENTFD_MARGIN) && (fd < (EVENTFD_MARGIN + EVENTFD_MAX));
#endif
}

ssize_t fndk_eventfd_read(int fd, void *buf, size_t count) {
    sceKernelLockLwMutex(&eventfd_pool_mutex, 1, NULL);

    eventfd_internal * efd = nullptr;

    for (int i = 0; i < EVENTFD_MAX; ++i) {
        if (eventfd_pool[i].fd == fd) {
            efd = &eventfd_pool[i];
            break;
        }
    }

    if (!efd) {
        sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
        errno = EINVAL;
        return -1;
    }

    if (count < 8 || !buf) {
        sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
        errno = EINVAL;
        return -1;
    }

    if (efd->value == 0) {
        if (efd->flags & FNDK_EFD_NONBLOCK) {
            sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
            errno = EAGAIN;
            return -1;
        } else {
            for (;;) {
                sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
                usleep(10000);
                sceKernelLockLwMutex(&eventfd_pool_mutex, 1, NULL);

                if (efd->value != 0) {
                    break;
                }
            }
        }
    }

    if (efd->flags & FNDK_EFD_SEMAPHORE) {
        *(uint64_t *)buf = (uint64_t) 1;
        efd->value--;
        sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
        return 8;
    }

    *(uint64_t *)buf = efd->value;
    efd->value = 0;
    sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
    return 8;
}

ssize_t fndk_eventfd_write(int fd, const void *buf, size_t count) {
    sceKernelLockLwMutex(&eventfd_pool_mutex, 1, NULL);

    uint64_t val;
    eventfd_internal * efd = nullptr;

    for (int i = 0; i < EVENTFD_MAX; ++i) {
        if (eventfd_pool[i].fd == fd) {
            efd = &eventfd_pool[i];
            break;
        }
    }

    if (!efd) {
        sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
        errno = EINVAL;
        return -1;
    }

    if (count < 8 || !buf) {
        sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
        errno = EINVAL;
        return -1;
    }

    val = *(uint64_t *) buf;
    if (0xfffffffffffffffe - efd->value < val) {
        if (efd->flags & FNDK_EFD_NONBLOCK) {
            sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
            errno = EAGAIN;
            return -1;
        } else {
            for (;;) {
                sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
                usleep(10000);
                sceKernelLockLwMutex(&eventfd_pool_mutex, 1, NULL);

                if (0xfffffffffffffffe - efd->value >= val) {
                    break;
                }
            }
        }
    }

    efd->value += val;
    sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
    return 8;
}

void fndk_eventfd_status(int fd, bool * is_readable, bool * is_writeable) {
    sceKernelLockLwMutex(&eventfd_pool_mutex, 1, nullptr);

    for (int u = 0; u < EVENTFD_MAX; ++u) {
        if (eventfd_pool[u].fd == fd) {
            *is_readable = eventfd_pool[u].value > 0;
            *is_writeable = eventfd_pool[u].value < 0xfffffffffffffffe;
            break;
        }
    }

    sceKernelUnlockLwMutex(&eventfd_pool_mutex, 1);
}
