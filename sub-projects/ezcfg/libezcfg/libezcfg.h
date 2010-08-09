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

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EZCFG_INVALID_SOCKET	-1

#define EZCFG_NVRAM_SPACE	0x20000 /* 128K Bytes */
#define EZCFG_SOCKET_DIR	"/tmp/ezcfg"
#define EZCFG_SOCKET_PATH	"/tmp/ezcfg/ezcfg.sock"
//#define EZCFG_CTRL_SOCK_PATH	"@/org/kernel/ezcfg/ctrl"
#define EZCFG_CTRL_SOCK_PATH	"/tmp/ezcfg/ctrl"
//#define EZCFG_MASTER_SOCK_PATH	"@/org/kernel/ezcfg/master"
#define EZCFG_MASTER_SOCK_PATH	"/tmp/ezcfg/master"
#define EZCFG_MASTER_SOCKET_QUEUE_LENGTH	20

/* ezcfg supported protocols */
#define EZCFG_PROTO_UNKNOWN	0
#define EZCFG_PROTO_HTTP	1
#define EZCFG_PROTO_IGRS	2
#define EZCFG_PROTO_ISDP	3

/* ezcfg xml definitions */
#define EZCFG_XML_MAX_ELEMENTS	128
#define EZCFG_XML_ELEMENT_ATTRIBUTE_TAIL 0
#define EZCFG_XML_ELEMENT_ATTRIBUTE_HEAD 1

/* ezcfg soap element names */
#define EZCFG_SOAP_ENV_ELEMENT_NAME	"SOAP-ENV:Envelope"
#define EZCFG_SOAP_BODY_ELEMENT_NAME	"SOAP-ENV:Body"
#define EZCFG_SOAP_ENV_NS_NAME		"xmlns:SOAP-ENV"
#define EZCFG_SOAP_ENV_NS_VALUE		"http://www.w3.org/2002/12/soap-envelope"
#define EZCFG_SOAP_ENV_ENC_NAME		"SOAP-ENV:encodingStyle"
#define EZCFG_SOAP_ENV_ENC_VALUE	"http://schemas.xmlsoap.org/soap/encoding/"

/* ezcfg http definitions */
#define EZCFG_HTTP_MAX_HEADERS	64 /* must be less than 256 */

/* ezcfg uuid definitions */
#define EZCFG_UUID_BINARY_LEN	16 /* 128/8 */
#define EZCFG_UUID_STRING_LEN	36 /* 8+1+4+1+4+1+4+1+12 */
#define EZCFG_UUID_NIL_STRING	"00000000-0000-0000-0000-000000000000"

/* ezcfg igrs soap element names */
#define EZCFG_IGRS_ENVELOPE_ELEMENT_NAME	"SOAP-ENV:Envelope"
#define EZCFG_IGRS_BODY_ELEMENT_NAME	"SOAP-ENV:Body"
#define EZCFG_IGRS_SESSION_ELEMENT_NAME	"Session"
#define EZCFG_IGRS_SOURCE_CLIENT_ID_ELEMENT_NAME	"SourceClientId"
#define EZCFG_IGRS_TARGET_SERVICE_ID_ELEMENT_NAME	"TargetServiceId"
#define EZCFG_IGRS_SEQUENCE_ID_ELEMENT_NAME	"SequenceId"
#define EZCFG_IGRS_USER_INFO_ELEMENT_NAME	"UserInfo"
#define EZCFG_IGRS_SOURCE_USER_ID_ELEMENT_NAME	"SourceUserId"
#define EZCFG_IGRS_SERVICE_SECURITY_ID_ELEMENT_NAME	"ServiceSecurityId"
#define EZCFG_IGRS_TOKEN_ELEMENT_NAME	"Token"

/* ezcfg igrs soap element attributes */
#define EZCFG_IGRS_ENVELOPE_ATTR_NS_NAME		"xmlns:SOAP-ENV"
#define EZCFG_IGRS_ENVELOPE_ATTR_NS_VALUE		"http://www.w3.org/2002/12/soap-envelope"
#define EZCFG_IGRS_ENVELOPE_ATTR_ENC_NAME		"SOAP-ENV:encodingStyle"
#define EZCFG_IGRS_ENVELOPE_ATTR_ENC_VALUE	"http://schemas.xmlsoap.org/soap/encoding/"
#define EZCFG_IGRS_SESSION_ATTR_NS_NAME		"xmlns"
#define EZCFG_IGRS_SESSION_ATTR_NS_VALUE	"www.igrs.org/spec1.0"

/*
 * ezcfg - library context
 *
 * load/save the ezbox config and system environment
 * allows custom logging
 */

/*
 * libezcfg.c
 * ezbox config context
 */
struct ezcfg;

void ezcfg_set_log_fn(struct ezcfg *ezcfg,
                      void (*log_fn)(struct ezcfg *ezcfg,
                                    int priority, const char *file, int line, const char *fn,
                                    const char *format, va_list args));
int ezcfg_get_log_priority(struct ezcfg *ezcfg);
void ezcfg_set_log_priority(struct ezcfg *ezcfg, int priority);

struct ezcfg *ezcfg_new(void);
void ezcfg_delete(struct ezcfg *ezcfg);

/*
 * libezcfg-list.c
 * ezcfg_list
 *
 * access to libezcfg generated lists
 */
struct ezcfg_list_entry;
struct ezcfg_list_entry *ezcfg_list_entry_get_next(struct ezcfg_list_entry *list_entry);
struct ezcfg_list_entry *ezcfg_list_entry_get_by_name(struct ezcfg_list_entry *list_entry, const char *name);
const char *ezcfg_list_entry_get_name(struct ezcfg_list_entry *list_entry);
const char *ezcfg_list_entry_get_value(struct ezcfg_list_entry *list_entry);

/**
 * ezcfg_list_entry_foreach:
 * @list_entry: entry to store the current position
 * @first_entry: first entry to start with
 *
 * Helper to iterate over all entries of a list.
 */
#define ezcfg_list_entry_foreach(list_entry, first_entry) \
	for (list_entry = first_entry; \
	     list_entry != NULL; \
	     list_entry = ezcfg_list_entry_get_next(list_entry))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
