/* 
 * libuserbindmount: bindmount as a user and create a user-namespace if needed
 * Copyright (C) 2017  Renzo Davoli University of Bologna
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include "userbindmount.h"

int userbindmount_set_cap_sysadm(void) {
	cap_t caps=cap_get_proc();
	if (caps == NULL)
		return -1;
	cap_value_t cap = CAP_SYS_ADMIN;
	cap_set_flag(caps, CAP_INHERITABLE, 1, &cap, CAP_SET);
	if (cap_set_proc(caps) < 0)
		return -1;
	cap_free(caps);
	if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
		return -1;
	return 0;
}

static void uid_gid_map(pid_t pid) {
	char map_file[PATH_MAX];
	FILE *f;
	uid_t euid = geteuid();
	gid_t egid = getegid();
	snprintf(map_file, PATH_MAX, "/proc/%d/uid_map", pid);
	f = fopen(map_file, "w");
	if (f) {
		fprintf(f,"%d %d 1\n",euid,euid);
		fclose(f);
	}
	snprintf(map_file, PATH_MAX, "/proc/%d/setgroups", pid);
	f = fopen(map_file, "w");
	if (f) {
		fprintf(f,"deny\n");
		fclose(f);
	}
	snprintf(map_file, PATH_MAX, "/proc/%d/gid_map", pid);
	f = fopen(map_file, "w");
	if (f) {
		fprintf(f,"%d %d 1\n",egid,egid);
		fclose(f);
	}
}

int userbindmount_unshare(void) {
	int pipe_fd[2];
	pid_t child_pid;
	char buf[1];
	if (pipe2(pipe_fd, O_CLOEXEC) == -1)
		return -1;
	switch (child_pid = fork()) {
		case 0:
			close(pipe_fd[1]);
			read(pipe_fd[0], &buf, sizeof(buf));
			uid_gid_map(getppid());
			exit(0);
		default:
			close(pipe_fd[0]);
			if (unshare(CLONE_NEWUSER | CLONE_NEWNS) == -1)
				return -1;
			close(pipe_fd[1]);
			if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
				return -1;
			break;
		case -1:
			return -1;
	}
	return 0;
}

int userbindmount(const char *source, const char *target) {
	int retval;
	if ((retval = mount(source, target, "", MS_BIND, NULL) < 0) && errno == EPERM) {
		userbindmount_unshare();
		retval = mount(source, target, "", MS_BIND, NULL);
	}
	return retval;
}

#define BUFSIZE 1024
static int __common_userbindmount_tmp(int fd_in, const void *data, size_t count, const char *target, mode_t mode) {
	char tmpname[] = "/tmp/userbindmountXXXXXX";
	int fd = mkstemp(tmpname);
	int retval;
	if (fd < 0)
		return -1;
	fchmod(fd, 0600);
	if (fd_in >= 0) {
		char buf[BUFSIZE];
		while ((retval = read(fd_in, buf, BUFSIZE)) > 0)
			write(fd, buf, retval);
	} else 
		write(fd, data, count);
	fchmod(fd, mode);
	close(fd);
	retval = userbindmount(tmpname, target);
	unlink(tmpname);
	return retval;
}

int userbindmount_data(const void *data, size_t count, const char *target, mode_t mode) {
	return __common_userbindmount_tmp(-1, data, count, target, mode);
}

int userbindmount_string(const char *string, const char *target, mode_t mode) {
	return __common_userbindmount_tmp(-1, string, strlen(string), target, mode);
}

int userbindmount_fd(int fd,  const char *target, mode_t mode) {
	return __common_userbindmount_tmp(fd, NULL, 0, target, mode);
}
