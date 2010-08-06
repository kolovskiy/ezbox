/* ============================================================================
 * Project Name : ezbox configuration utilities
 * File Name    : libezcfg-util.c
 *
 * Description  : interface to configurate ezbox information
 *
 * Copyright (C) 2010 by ezbox-project
 *
 * History      Rev       Description
 * 2010-07-12   0.1       Write it from scratch
 * ============================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libezcfg.h"
#include "libezcfg-private.h"

int ezcfg_util_log_priority(const char *priority)
{
	char *endptr;
	int prio;

	prio = strtol(priority, &endptr, 10);
	if (endptr[0] == '\0' || isspace(endptr[0]))
		return prio;
	if (strncmp(priority, "err", 3) == 0)
		return LOG_ERR;
	if (strncmp(priority, "info", 4) == 0)
		return LOG_INFO;
	if (strncmp(priority, "debug", 5) == 0)
		return LOG_DEBUG;
	return 0;
}

void ezcfg_util_remove_trailing_char(char *s, char c)
{
	size_t len;

	if (s == NULL)
		return;
	len = strlen(s);
	while (len > 0 && s[len-1] == c)
		s[--len] = '\0';
}

void ezcfg_util_remove_trailing_charlist(char *s, char *l)
{
	size_t len;

	if (s == NULL || l == NULL)
		return;
	len = strlen(s);
	while (len > 0 && strchr(l, s[len-1]) != NULL)
		s[--len] = '\0';
}

char *ezcfg_util_skip_leading_char(char *s, char c)
{
	if (s == NULL)
		return NULL;
	if (c == '\0')
		return s;
	while (*s == c) s++;
	return s;
}

char *ezcfg_util_skip_leading_charlist(char *s, char *l)
{
	if (s == NULL)
		return NULL;
	if (l == NULL)
		return s;
	while (*s != '\0' && strchr(l, *s) != NULL) s++;
	return s;
}

