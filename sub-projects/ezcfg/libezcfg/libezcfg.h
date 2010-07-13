/* ============================================================================
 * Project Name : ezbox configuration utilities
 * File Name    : libezcfg.c
 *
 * Description  : interface to configurate ezbox information
 *
 * Copyright (C) 2010 by ezbox-project
 *
 * History      Rev       Description
 * 2010-07-12   0.1       Write it from scratch
 * ============================================================================
 */

#ifndef _LIBEZCFG_H_
#define _LIBEZCFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ezcfg - library context
 *
 * load/save the ezbox config and system environment
 * allows custom logging
 */

/*
 * unified socket address. For IPv6 support, add IPv6 address structure
 * in the union u.
 */
struct usa;

/*
 * structure used to describe listening socket, or socket which was
 * accept()-ed by the monitor thread and queued for future handling
 * by the worker thread.
 */
struct socket;

/*
 * ezbox config context
 */
struct ezcfg;


int ezcfg_get_log_priority(struct ezcfg *ezcfg);
void ezcfg_set_log_priority(struct ezcfg *ezcfg, int priority);

/*
 * ezcfg_list
 *
 * access to libezcfg generated lists
 */
struct ezcfg_list_entry;
struct ezcfg_list_entry *ezcfg_list_entry_get_next(struct ezcfg_list_entry *list_entry);
struct ezcfg_list_entry *ezcfg_list_entry_get_by_name(struct ezcfg_list_entry *list_entry, const char *name);
const char *ezcfg_list_entry_get_name(struct ezcfg_list_entry *list_entry);
const char *ezcfg_list_entry_get_value(struct ezcfg_list_entry *list_entry);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
