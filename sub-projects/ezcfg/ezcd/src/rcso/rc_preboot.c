/* ============================================================================
 * Project Name : ezbox Configuration Daemon
 * Module Name  : rc_preboot.c
 *
 * Description  : ezbox prepare to boot system
 *
 * Copyright (C) 2008-2011 by ezbox-project
 *
 * History      Rev       Description
 * 2011-07-28   0.1       Write it from scratch
 * 2011-10-16   0.2       Modify it to use rcso framework
 * ============================================================================
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <stdarg.h>

#include "ezcd.h"
#include "utils.h"

#if 0
#define DBG(format, args...) do {\
	FILE *fp = fopen("/dev/kmsg", "a"); \
	if (fp) { \
		fprintf(fp, format, ## args); \
		fclose(fp); \
	} \
} while(0)
#else
#define DBG(format, args...)
#endif

#define DBG2() do {\
	pid_t pid = getpid(); \
	FILE *fp = fopen("/dev/kmsg", "a"); \
	if (fp) { \
		char buf[32]; \
		FILE *fp2; \
		int i; \
		for(i=pid; i<pid+30; i++) { \
			snprintf(buf, sizeof(buf), "/proc/%d/stat", i); \
			fp2 = fopen(buf, "r"); \
			if (fp2) { \
				if (fgets(buf, sizeof(buf)-1, fp2) != NULL) { \
					fprintf(fp, "pid=[%d] buf=%s\n", i, buf); \
				} \
				fclose(fp2); \
			} \
		} \
		fclose(fp); \
	} \
} while(0)

#ifdef _EXEC_
int main(int argc, char **argv)
#else
int rc_preboot(int argc, char **argv)
#endif
{
	char buf[KERNEL_COMMAND_LINE_SIZE];
	int ret;
	char *p, *q;
	int flag = RC_ACT_UNKNOWN;

	if (argc < 2) {
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[0], "preboot")) {
		return (EXIT_FAILURE);
	}

	flag = utils_get_rc_act_type(argv[1]);

	switch (flag) {
	case RC_ACT_BOOT :
		/* /proc */
		mkdir("/proc", 0555);
		mount("proc", "/proc", "proc", MS_MGC_VAL, NULL);

		/* sysfs -> /sys */
		mkdir("/sys", 0755);
		mount("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);

		/* /dev */
		mkdir("/dev", 0755);
		mount("devfs", "/dev", "tmpfs", MS_MGC_VAL, NULL);

		/* /etc */
		mkdir("/etc", 0755);
		mount("tmpfs", "/etc", "tmpfs", MS_MGC_VAL, NULL);

		/* /boot */
		mkdir("/boot", 0777);

		/* /var */
		mkdir("/var", 0777);
		mkdir("/var/lock", 0777);
		mkdir("/var/log", 0777);
		mkdir("/var/run", 0777);
		mkdir("/var/tmp", 0777);

		/* useful /var directories */
		mkdir("/var/lib", 0777);
		mkdir("/var/lib/misc", 0777);

		/* /tmp */
		snprintf(buf, sizeof(buf), "%s -rf /tmp", CMD_RM);
		utils_system(buf);
		ret = symlink("/var/tmp", "/tmp");

		/* init shms */
		mkdir("/dev/shm", 0777);

		/* Mount /dev/pts */
		mkdir("/dev/pts", 0777);
		mount("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);

		mknod("/dev/console", S_IRUSR|S_IWUSR|S_IFCHR, makedev(5, 1));
		mknod("/dev/null", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH|S_IFCHR, makedev(1, 3));

		/* run in root HOME path */
		mkdir(ROOT_HOME_PATH, 0755);
		setenv("HOME", ROOT_HOME_PATH, 1);
		ret = chdir(ROOT_HOME_PATH);

		/* get the kernel module name from /proc/cmdline */
		ret = utils_get_kernel_modules(buf, sizeof(buf));
		if (ret > 0) {
			/* load preboot kernel modules */
			p = buf;
			while(p != NULL) {
				q = strchr(p, ',');
				if (q != NULL)
					*q = '\0';

				utils_install_kernel_module(p, NULL);

				if (q != NULL)
					p = q+1;
				else
					p = NULL;
			}
		}

		/* init /dev/ nodes */
		utils_udev_pop_nodes();

		/* prepare boot device path */
		utils_mount_boot_partition_readonly();

		ret = EXIT_SUCCESS;
		break;

	default:
		ret = EXIT_FAILURE;
		break;
	}

	return (ret);
}
