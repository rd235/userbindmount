#ifndef USERBINDMOUNT_C
#define USERBINDMOUNT_C
#include <sys/types.h>

int userbindmount_unshare(void);

int userbindmount_set_cap_sysadm(void);

int userbindmount(const char *source, const char *target); 

int userbindmount_string(const char *string, const char *target, mode_t mode);

int userbindmount_data(const void *data, size_t count, const char *target, mode_t mode);

int userbindmount_fd(int fd, const char *target, mode_t mode);

#endif
