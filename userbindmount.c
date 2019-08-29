/* 
 * userbindmount: bindmount as a user (using user-namespaces).
 * Copyright (C) 2016  Renzo Davoli, University of Bologna
 * 
 * userbindmount is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <libgen.h>
#include <sys/mount.h>

#include "userbindmount.h"

char *progname;
int sysadmin_flag = 0;
int verbose_flag = 0;
int newnamespace_flag = 0;

#define errExit(msg)    do {\
	perror(msg); \
	exit(EXIT_FAILURE); \
} while(0);

void usage_n_exit(void) {
	fprintf(stderr, 
			"Usage: %s OPTIONS [source target [source target [...]]] [-- [cmd [args]]] \n"
			"OPTIONS:\n"
			"\t-n | --newns   : create a new namespace\n"
			"\t-s | --sysadm  : set cap_sys_admin ambient capability\n"
			"\t-v | --verbose : verbose mode\n" 
			"\t-h | --help    : print this short help message\n" 
			"a new user_namespace is created if -- arg is present\n"
			"it runs $SHELL if cmd omitted\n"
			"the contents of the mounted file is read from stdin if source is '-'\n\n",
			progname);
	exit(EXIT_SUCCESS);
}

void userbindmount_mount(const char *source, const char *target) {
	int retval;
	if (*source == 0 || strcmp(source, "-") == 0)
		retval = userbindmount_fd(0, target, 0600);
	else {
		int len = strlen(source);
		if (source[0] == '"' && len > 1 && source[len - 1] == '"') {
			char string[len];
			snprintf(string, len, "%*.*s", len - 2, len - 2, source + 1);
			retval = userbindmount_string(string, target, 0600);
		} else
			retval = mount(source, target, "", MS_BIND, NULL);
	}
	if (retval < 0)
		errExit("mount");
}

static int there_is_dash_dash(char *argv[]) {
	for (; *argv != NULL; argv++) {
		if (strcmp(argv[0], "--") == 0)
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	static char *short_options = "+shvn";
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"newns", no_argument, 0, 'n'},
		{"sysadm", no_argument, 0, 's'},
		{"verbose", no_argument, 0, 'v'},
		{0, 0, 0, 0}};
	progname = basename(argv[0]);
	while (1) {
		int c;
		if ((c = getopt_long(argc, argv, short_options, long_options, NULL)) == -1)
			break;
		switch (c) {
			case 's': sysadmin_flag = 1; break;
			case 'v': verbose_flag = 1; break;
			case 'n': newnamespace_flag = 1; break;
			case '?':
			case 'h': 
			default: usage_n_exit();
		}
	}
	if (strcmp(argv[optind - 1], "--") == 0)
		optind--;
	argc -= optind;
	argv += optind;
	if (there_is_dash_dash(argv))
		newnamespace_flag = 1;
	if (newnamespace_flag) {
		if (verbose_flag)
			fprintf(stderr, "creating a user_namespace\n");
		if (userbindmount_unshare() < 0)
			errExit("unshare");
	}
	if (sysadmin_flag) {
		if (verbose_flag)  
			fprintf(stderr, "setting sysadm ambient capability\n");   
		if (userbindmount_set_cap_sysadm() < 0)
			errExit("set_cap_sysadm");
	}
	while (argc >= 2 && strcmp(argv[0],"--") != 0) {
		if (verbose_flag) 
			fprintf(stderr, "mounting %s on %s\n", argv[0], argv[1]);
		userbindmount_mount(argv[0], argv[1]);
		argc -= 2;
		argv += 2;
	}
	if (newnamespace_flag || argc > 0) {
		char **cmdargv;
		char *argvsh[]={getenv("SHELL"),NULL};
		if (argc > 0 && strcmp(argv[0],"--") == 0) {
			argc--;
			argv++;
		}
		cmdargv = (argc == 0) ? argvsh : argv;
		if (cmdargv[0] == NULL)
			errExit("$SHELL env variable not set\n");
		if (verbose_flag) 
			fprintf(stderr, "starting %s\n", cmdargv[0]);   
		execvp(cmdargv[0], cmdargv);
	}
}
