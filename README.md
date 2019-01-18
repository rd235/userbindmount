# userbindmount / libuserbindmount

**libuserbindmount** is a library providing support for bind mount in user namespaces.
**userbindmount** is a utility command based on libuserbindmount.

e.g. 
```
userbindmount("/tmp/resolv.conf", "/etc/resolv.conf");
```
mounts the file /tmp/resolv.conf instead of /etc/resolv.conf: the purpose of this
example is to redefine the name servers for the name resolution.

libuserbindmount requires user namespace to be included and enabled in the kernel of the hosting system.
Kernel config file must include the following option.
```
CONFIG_USER_NS=y
```

Debian users should enable user namespaces using the following command:
```
$ sudo echo 1 > /proc/sys/kernel/unprivileged_userns_clone
```

## libuserbindmount: the library

### userbindmount
```
int userbindmount(const char *source, const char *target);
```
bind mount source on destination. If it is not permitted in the current namespace it
creates (unshare) a new user-namespace.
If a program reqires several bind mounts like in the following chunk of code e.g.:
```
userbindmount("/tmp/resolv.conf", "/etc/resolv.conf");
userbindmount("/tmp/passwd", "/etc/passwd");
userbindmount("/tmp/hosts", "/etc/hosts");
```
the first userbindmount creates the namespace if required, while the others will
simply mount the other files given that the operation is permitted in the current namespace.

### userbindmount\_unshare
```
int userbindmount_unshare(void);
```
create a new user-namespace where bind mount is allowed.

### userbindmount\_set\_cap\_sysadm
```
int userbindmount_set_cap_sysadm(void);
```
add the ```CAP_SYS_ADMIN``` ambient capability to the current namespace so that the ability
of bind mount files and directories can be exported to new programs (ambient capabilities 
survive to execve(2)).

This program unshare_sysadm.c uses both ```userbindmount_unshare``` and ```userbindmount_set_cap_sysadm```; 
```
#include <stdio.h>
#include <unistd.h>
#include <userbindmount.h>

int main(int argc, char *argv[]) {
  userbindmount_unshare();
  userbindmount_set_cap_sysadm();
  execvp(argv[1], argv+1);
}
```

It can be compiled and tested in the following way:
```
$ gcc -o unshare_sysadm unshare_sysadm.c -luserbindmount
$ unshare_sysadm bash
$ cat /etc/resolv.conf 
nameserver 127.0.0.1
$ echo "nameserver 9.9.9.9" > /tmp/resolv.conf
$ busybox mount --bind /tmp/resolv.conf /etc/resolv.conf 
$ cat /etc/resolv.conf
nameserver 9.9.9.9
$ exit
$
```

(please note that in the example the mount command by busybox has been used instead of the
 standard mount by util-linux. In fact the standard mount command has not been updated to
 support the capabilities, and forbids the access to the mount system call if the effective user
 is not root, denying in this way a legal operation).

### userbindmount\_string
```
int userbindmount_string(const char *string, const char *target, mode_t mode);
```
The contents of the file is provided by a string.
e.g.
```
userbindmount_string("nameserver 9.9.9.9\n", "/etc/resolv.conf", 0600);
```

### userbindmount\_data
```
int userbindmount_data(const void *data, size_t count, const char *target, mode_t mode);
```
The contents of the file is provided by (binary) data from the memory of the process. 
e.g.
```
static char fakeargv[] = {'c','m','d',0,
  'a','r','g','1',0,
  'a','r','g','2',0,
  0};
userbindmount_data(fakeargv, sizeof(fakeargv), "/proc/self/cmdline", 0600);
```

### userbindmount\_fd
```
int userbindmount_fd(int fd, const char *target, mode_t mode);
```
The contents of the file is provided by reading from the file decriptor.
e.g.
```
userbindmount_fd(STDIN_FILENO, "/etc/resolv.conf", 0600);
```
takes the contents of the file from standard input.

## userbindmount: the command

userbindmount is a utility command to bind mount files and directories, creating a
new user-namespace when requested.

Examples of usage:
```
$ userbindmount -h
Usage: userbindmount OPTIONS [source target [source target [...]]] [-- [cmd [args]]] 
OPTIONS:
  -n | --newns   : create a new namespace
  -s | --sysadm  : set cap_sys_admin ambient capability
  -v | --verbose : verbose mode
  -h | --help    : print this short help message
a new user_namespace is created if -- arg is present
it runs $SHELL if cmd omitted
the contents of the mounted file is read from stdin if source is '-'
```

```
$ userbindmount -s -- bash
$ cat /etc/resolv.conf 
nameserver 127.0.0.1
$ echo "nameserver 9.9.9.9" > /tmp/resolv.conf
$ busybox mount --bind /tmp/resolv.conf /etc/resolv.conf 
$ cat /etc/resolv.conf
nameserver 9.9.9.9
$ exit
$
```
Alternative equivalent commands for ```userbindmount -s -- bash``` are ```userbindmount -sn``` or ```userbindmount -s --```.


```
$ echo "nameserver 9.9.9.9" > /tmp/resolv.conf
$ userbindmount -v /tmp/resolv.conf /etc/resolv.conf -- cat /etc/resolv.conf
creating a user_namespace
mounting /tmp/resolv.conf on /etc/resolv.conf
starting cat
nameserver 9.9.9.9
```

```
$ userbindmount -sn
$ echo "nameserver 9.9.9.9" | userbindmount - /etc/resolv.conf
$ userbindmount /tmp/passwd /etc/passwd
$ cat /etc/resolv.conf
nameserver 9.9.9.9
$ exit
```

