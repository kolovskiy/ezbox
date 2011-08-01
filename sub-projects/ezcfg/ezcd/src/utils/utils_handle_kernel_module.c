/* ============================================================================
 * Project Name : ezbox Configuration Daemon
 * Module Name  : utils_handle_kernel_module.c
 *
 * Description  : ezcfg install/remove kernel modules function
 *
 * Copyright (C) 2008-2011 by ezbox-project
 *
 * History      Rev       Description
 * 2011-07-28   0.1       Write it from scratch
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

typedef struct mod_dep_s {
	char *name;
	char *list;
} mod_dep_t;

/* directly depends list */
static mod_dep_t mod_depends[] = {
	{ "ahci", "libahci" },
	{ "cfg80211", "compat" },
	{ "ehci-hcd", "usbcore" },
	{ "fat", "nls_cp437,nls_iso8859-1" },
	{ "ohci-hcd", "nls_base,usbcore" },
	{ "mac80211", "compat,cfg80211" },
	{ "nls_cp437", "nls_base" },
	{ "nls_iso8859-1", "nls_base" },
	{ "rt2x00lib", "compat,compat_firmware_class,cfg80211,mac80211" },
	{ "rt2x00usb", "usbcore,mac80211,rt2x00lib" },
	{ "rt73usb", "crc-itu-t,rt2x00lib,rt2x00usb" },
	{ "uhci-hcd", "usbcore" },
	{ "usbcore", "nls_base" },
	{ "vfat", "nls_base,nls_cp437,nls_iso8859-1,fat" },
};

int utils_install_kernel_module(char *name, char *args)
{
	mod_dep_t *mdp;
	//char buf[128];
	char *p, *q, *l;
	char *kver;
	int i;
	int ret = EXIT_FAILURE;

	if (name == NULL) {
		return ret;
	}

	/* first check if we should insmod related kernel modules */
	for (i = 0; i < ARRAY_SIZE(mod_depends); i++) {
		mdp = &mod_depends[i];
		if (strcmp(mdp->name, name) == 0) {
			l = strdup(mdp->list);
			if (l != NULL) {
				p = l;
				while(p != NULL) {
					q = strchr(p, ',');
					if (q != NULL)
						*q = '\0';

					/* check it recursively */
					utils_install_kernel_module(p, NULL);

					if (q != NULL)
						p = q+1;
					else
						p = NULL;
				}
				free(l);
			}
		}
        }

	kver = utils_get_kernel_version();
	if (kver == NULL) {
		return ret;
	}

	/* then insmod the kernel module directly */
	q = (args == NULL) ? "" : args;
	i = strlen(CMD_INSMOD);
	i += 14; /* strlen(" /lib/modules/") */
	i += (strlen(kver) + 1); /* %s/ */
	i += (strlen(name) + 3);
	i += (strlen(q) + 2); /* " %s", one more for '\0' */
	p = malloc(i);
	if (p != NULL) {
		snprintf(p, i, "%s /lib/modules/%s/%s.ko %s", CMD_INSMOD, kver, name, q);
		system(p);
		free(p);
	}
	free(kver);
	ret = EXIT_SUCCESS;

	return ret;
}

int utils_remove_kernel_module(char *name)
{
	mod_dep_t *mdp;
	//char buf[128];
	char *p, *q, *l;
	int i;
	int ret = EXIT_FAILURE;

	if (name == NULL) {
		return ret;
	}

	/* first rmmod the kernel module directly */
	i = strlen(CMD_RMMOD) + strlen(name) + 2;
	p = malloc(i);
	if (p == NULL) {
		return ret;
	}
	snprintf(p, i, "%s %s", CMD_RMMOD, name);
	system(p);
	free(p);

	/* then check if we can rmmod related kernel modules */
	for (i = 0; i < ARRAY_SIZE(mod_depends); i++) {
		mdp = &mod_depends[i];
		if (strcmp(mdp->name, name) == 0) {
			l = strdup(mdp->list);
			if (l != NULL) {
				p = l;
				while(p != NULL) {
					q = strrchr(p, ',');
					if (q != NULL) {
						p = q+1;
					}

					utils_remove_kernel_module(p);

					if (q != NULL) {
						*q = '\0';
						p = l;
					}
					else {
						p = NULL;
					}
				}
				free(l);
			}
		}
        }

	ret = EXIT_SUCCESS;

	return ret;
}