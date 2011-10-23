/* ============================================================================
 * Project Name : ezbox Configuration Daemon
 * Module Name  : rc_ezcd.c
 *
 * Description  : ezbox run ezcfg daemon service
 *
 * Copyright (C) 2008-2011 by ezbox-project
 *
 * History      Rev       Description
 * 2010-06-13   0.1       Write it from scratch
 * 2011-10-21   0.2       Modify it to use rcso frame
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
#include "pop_func.h"

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

#ifdef _EXEC_
int main(int argc, char **argv)
#else
int rc_ezcd(int argc, char **argv)
#endif
{
	char buf[256];
	proc_stat_t *pidList;
	int flag, ret;

	if (argc < 2) {
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[0], "ezcd")) {
		return (EXIT_FAILURE);
	}

	ret = EXIT_SUCCESS;
	flag = utils_get_rc_act_type(argv[1]);

	switch (flag) {
	case RC_ACT_BOOT :
		/* FIXME: nvram is not ready now!!! */
		pop_etc_ezcd_conf(RC_ACT_BOOT);
		/* ezcd config file should be prepared */
		snprintf(buf, sizeof(buf), "%s -d", CMD_EZCD);
		system(buf);

		/* wait until nvram is ready */
		if (utils_ezcd_wait_up(0) == false) {
			ret = EXIT_FAILURE;
		}

		break;

	case RC_ACT_STOP :
		pidList = utils_find_pid_by_name("ezcd");
		while (pidList) {
			int i;
			for (i = 0; pidList[i].pid > 0; i++) {
				kill(pidList[i].pid, SIGTERM);
			}
			free(pidList);
			pidList = utils_find_pid_by_name("ezcd");
		}

		/* wait until ezcd is exit */
		if (utils_ezcd_wait_down(0) == false) {
			ret = EXIT_FAILURE;
		}

		break;

	case RC_ACT_RELOAD :
		/* re-generate ezcd config file */
		pop_etc_ezcd_conf(flag);
		/* set nvram config file path */
		ezcfg_api_nvram_set_config_file(EZCD_CONFIG_FILE_PATH);
		/* save the nvram changes before doing reload */
		ezcfg_api_nvram_commit();
		/* send signal to ezcd to reload config */
		pidList = utils_find_pid_by_name("ezcd");
		if (pidList) {
			int i;
			for (i = 0; pidList[i].pid > 0; i++) {
				kill(pidList[i].pid, SIGUSR1);
			}
			free(pidList);
		}

		/* wait until nvram is ready */
		if (utils_ezcd_wait_up(0) == false) {
			ret = EXIT_FAILURE;
		}

		break;

	default:
		ret = EXIT_FAILURE;
		break;
	}

	return (ret);
}
