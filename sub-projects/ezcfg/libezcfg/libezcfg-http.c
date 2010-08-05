/* ============================================================================
 * Project Name : ezbox configuration utilities
 * File Name    : libezcfg-http.c
 *
 * Description  : implement HTTP/1.1 protocol (RFC 2616)
 *                and it's extension (RFC 2774)
 *
 * Copyright (C) 2010 by ezbox-project
 *
 * History      Rev       Description
 * 2010-07-12   0.1       Write it from scratch
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/un.h>
#include <assert.h>
#include <pthread.h>

#include "libezcfg.h"
#include "libezcfg-private.h"

struct http_header {
	char *name; /* HTTP header name */
	char *value; /* HTTP header value */
	struct http_header *next;
	bool is_known_header; /* true if name in header_strings */
};

struct ezcfg_http {
	struct ezcfg *ezcfg;

	unsigned char num_methods; /* Number of supported methods */
	const char **method_strings;
	unsigned char method_index; /* index of method_strings, should be > 0, 0 means error */

	char *request_uri; /* URL-decoded URI */
	char *query_string; /* \0 - terminated, use same buf with request_uri !!! */

	unsigned short version_major;
	unsigned short version_minor;

	unsigned short status_code; /* HTTP response status code */
	char *reason_phrase; /* HTTP response Reason-Phrase */

	unsigned char num_known_headers; /* Number of supported headers */
	const char **known_header_strings;

	struct http_header *header_head; /* HTTP header list head */
	struct http_header *header_tail; /* HTTP header list tail */

	int message_body_len; /* HTTP message body length */
	char *message_body; /* HTTP message bosy buffer */

	bool is_ssl; /* true if SSL-ed, false if not */
	bool is_extension; /* true if HTTP extension framework */
};

static const char *default_method_strings[] = {
	/* bad method string */
	NULL,
	/* HTTP/1.1 (RFC2616) defined methods */
	"OPTIONS",
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"CONNECT",
	/* HTTP/1.1 extension (RFC2774) defined methods */
	"M-OPTIONS",
	"M-GET",
	"M-HEAD",
	"M-POST",
	"M-PUT",
	"M-DELETE",
	"M-TRACE",
	"M-CONNECT",
};

static const char *default_header_strings[] = {
	/* bad header string */
	NULL,
	/* HTTP/1.1 (RFC2616) General headers */
	"Cache-Control",
	"Connection",
	"Date",
	"Pragma",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"Via",
	"Warning",
	/* HTTP/1.1 (RFC2616) request headers */
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Authorization",
	"Expect",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Max-Forwards",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"TE",
	"User-Agent",
	/* HTTP/1.1 (RFC2616) response headers */
	"Accept-Ranges",
	"Age",
	"ETag",
	"Location",
	"Proxy-Authenticate",
	"Retry-After",
	"Server",
	"Vary",
	"WWW-Authenticate",
	/* HTTP/1.1 (RFC2616) entity headers */
	"Allow",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Expires",
	"Last-Modified",
	/* HTTP/1.1 extension (RFC2774) headers */
	"Man",
	"Opt",
	"C-Man",
	"C-Opt",
	/* HTTP/1.1 extension (RFC2774) responese headers */
	"Ext",
	"C-Ext",
};

/**
 * Private functions
 **/

static void clear_http_headers(struct ezcfg_http *http)
{
	struct ezcfg *ezcfg;
	struct http_header *h;

	assert(http != NULL);

	ezcfg = http->ezcfg;
	while(http->header_head != NULL) {
		h = http->header_head;
		http->header_head = h->next;
		if (h->is_known_header == false) {
			free(h->name);
		}
		if (h->value != NULL) {
			free(h->value);
		}
	}
	http->header_tail = NULL;
}

static const char *find_known_header_name(struct ezcfg_http *http, char *name)
{
	struct ezcfg *ezcfg;
	int i;

	assert(http != NULL);
	assert(name != NULL);

	ezcfg = http->ezcfg;

	for (i = http->num_known_headers; i > 0; i--) {
		if (strcasecmp(http->known_header_strings[i], name) == 0) {
			return http->known_header_strings[i];
		}
	}
	return NULL;
}

/**
 * Parse HTTP headers from the given buffer, advance buffer to the point
 * where parsing stopped.
 **/
static bool parse_http_headers(struct ezcfg_http *http, char *buf)
{

	struct ezcfg *ezcfg;
	char *header, *end, *name, *value;

	assert(http != NULL);
	assert(buf != NULL);

	ezcfg = http->ezcfg;

	/* header end with CRLF */
	header = buf;

	do {
		/* find header line */
		end = strstr(header, "\r\n");
		if (end == NULL) {
			err(ezcfg, "HTTP header parse error.\n");
			return false;
		}
		*end = '\0';

		/* find header name field */
		name = header;
		value = strchr(header, ':');
		if (value == NULL) {
			err(ezcfg, "HTTP header parse error 2.\n");
			return false;
		}
		*value = '\0';

		/* find header value field */
		value++;
		while (*value == ' ' || *value == '\t') value++; /* skip LWS */

		if (name[0] != '\0') {
			if (ezcfg_http_add_header(http, name, value) == false) {
				err(ezcfg, "HTTP add header error.\n");
				return false;
			}
		}

		header = end+2; /* skip CRLF */
	} while(name[0] != '\0');

	return true;
}

static unsigned char find_method_index(struct ezcfg_http *http, const char *method)
{
	struct ezcfg *ezcfg;
	int i;

	assert(http != NULL);
	assert(method != NULL);

	ezcfg = http->ezcfg;

	for (i = http->num_methods; i > 0; i--) {
		if (strcmp(http->method_strings[i], method) == 0)
			return i;
	}
	return 0;
}

/**
 * Public functions
 **/
void ezcfg_http_delete(struct ezcfg_http *http)
{
	assert(http != NULL);

	clear_http_headers(http);

	if (http->request_uri != NULL) {
		free(http->request_uri);
	}
	if (http->message_body != NULL) {
		free(http->message_body);
	}

	free(http);
}

/**
 * ezcfg_http_new:
 *
 * Create ezcfg http info structure.
 *
 * Returns: a new ezcfg http info structure
 **/
struct ezcfg_http *ezcfg_http_new(struct ezcfg *ezcfg)
{
	struct ezcfg_http *http;

	assert(ezcfg != NULL);

	/* initialize http info structure */
	http = calloc(1, sizeof(struct ezcfg_http));
	if (http == NULL) {
		return NULL;
	}

	memset(http, 0, sizeof(struct ezcfg_http));

	http->ezcfg = ezcfg;
	http->num_methods = ARRAY_SIZE(default_method_strings) - 1; /* first item is NULL */
	http->method_strings = default_method_strings;

	http->num_known_headers = ARRAY_SIZE(default_header_strings) - 1; /* first item is NULL */
	http->known_header_strings = default_header_strings;

	return http;
}

void ezcfg_http_reset_attributes(struct ezcfg_http *http)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	clear_http_headers(http);

	if (http->request_uri != NULL) {
		free(http->request_uri);
		http->request_uri = NULL;
	}

	if (http->message_body != NULL) {
		free(http->message_body);
		http->message_body = NULL;
		http->message_body_len = 0;
	}

}

bool ezcfg_http_parse_request(struct ezcfg_http *http, char *buf)
{
	struct ezcfg *ezcfg;
	char *p;
	char *method, *uri, *version, *headers;

	assert(http != NULL);
	assert(buf != NULL);

	ezcfg = http->ezcfg;

	method = buf;
	/* split HTTP Request-Line (RFC2616 section 5.1) */
	p = strstr(buf, "\r\n");
	if (p == NULL) {
		err(ezcfg, "no HTTP request line\n");
		return false;
	}

	headers = p+2; /* skip CRLF */
	*p = '\0';
	dbg(ezcfg, "request_line=[%s]\n", method);

	/* get request method string */
	p = strchr(method, ' ');
	if (p == NULL)
		return false;

	/* 0-terminated method string */
	*p = '\0';
	http->method_index = find_method_index(http, method);
	if (http->method_index == 0) {
		*p = ' ';
		return false;
	}

	/* restore the SP charactor */
	*p = ' ';

	/* get uri string */
	uri = p+1;
	if (uri[0] != '/' && uri[0] != '*')
		return false;

	p = strchr(uri, ' ');
	if (p == NULL)
		return false;

	/* 0-terminated method string */
	*p = '\0';
	http->request_uri = strdup(uri);
	if (http->request_uri == NULL)
		return false;

	/* restore the SP charactor */
	*p = ' ';

	/* get http version */
	version = p+1;
	if (strncmp(version, "HTTP/", 5) != 0)
		return false;

	p = version+5;
	if (sscanf(p, "%hd.%hd",
	           &http->version_major,
	            &http->version_minor) != 2)
		return false;

	/* parse http headers */
	return parse_http_headers(http, headers);
}

unsigned short ezcfg_http_get_version_major(struct ezcfg_http *http)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	return http->version_major;
}

unsigned short ezcfg_http_get_version_minor(struct ezcfg_http *http)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	return http->version_minor;
}

bool ezcfg_http_set_version_major(struct ezcfg_http *http, unsigned short major)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	http->version_major = major;

	return true;
}

bool ezcfg_http_set_version_minor(struct ezcfg_http *http, unsigned short minor)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	http->version_minor = minor;

	return true;
}

void ezcfg_http_set_status_code(struct ezcfg_http *http, int status_code)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	http->status_code = status_code;
}

char *ezcfg_http_get_header(struct ezcfg_http *http, char *name)
{
	struct ezcfg *ezcfg;
	struct http_header *h;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	h = http->header_head;
	while(h != NULL) {
		if (strcasecmp(name, h->name) == 0)
			return h->name;
		h = h->next;
	}

	return NULL;
}

void ezcfg_http_dump(struct ezcfg_http *http)
{
	struct ezcfg *ezcfg;
	struct http_header *h;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	info(ezcfg, "request_method=[%s]\n", http->method_strings[http->method_index]);
	info(ezcfg, "uri=[%s]\n", http->request_uri);
	info(ezcfg, "query_string=[%s]\n", http->query_string);
	info(ezcfg, "http_version=[%d.%d]\n", http->version_major, http->version_minor);
	info(ezcfg, "message_body_len=[%d]\n", http->message_body_len);
	info(ezcfg, "message_body=[%s]\n", http->message_body);
	info(ezcfg, "status_code=[%d]\n", http->status_code);
	info(ezcfg, "is_ssl=[%d]\n", http->is_ssl);
	h = http->header_head;
	while (h != NULL) {
		info(ezcfg, "header(known=%d)=[%s:%s]\n", h->is_known_header, h->name, h->value);
		h = h->next;
	}
}

bool ezcfg_http_set_method_strings(struct ezcfg_http *http, const char **method_strings, unsigned char num_methods)
{
	struct ezcfg *ezcfg;

	assert(http != NULL);
	assert(method_strings != NULL);

	ezcfg = http->ezcfg;

	http->num_methods = num_methods;
	http->method_strings = method_strings;

	return true;
}

unsigned char ezcfg_http_set_request_method(struct ezcfg_http *http, const char *method)
{
	struct ezcfg *ezcfg;
	int i;

	assert(http != NULL);
	assert(method != NULL);

	ezcfg = http->ezcfg;

	for (i = http->num_methods; i > 0; i++) {
		if (strcmp(http->method_strings[i], method) == 0) {
			http->method_index = i;
			if (method[0] == 'M' && method[1] == '-') {
				http->is_extension = true;
			}
			return i;
		}
	}

	return 0;
}

bool ezcfg_http_set_request_uri(struct ezcfg_http *http, const char *uri)
{
	struct ezcfg *ezcfg;
	char *request_uri;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	request_uri = strdup(uri);
	if (request_uri == NULL) {
		return false;
	}

	if (http->request_uri != NULL) {
		free(http->request_uri);
	}

	http->request_uri = request_uri;

	return true;
}

bool ezcfg_http_set_message_body(struct ezcfg_http *http, const char *body, int len)
{
	struct ezcfg *ezcfg;
	char *message_body;

	assert(http != NULL);

	ezcfg = http->ezcfg;

	message_body = calloc(len, sizeof(char));
	if (message_body == NULL) {
		return false;
	}
	memcpy(message_body, body, len);

	if (http->message_body != NULL) {
		free(http->message_body);
	}

	http->message_body = message_body;
	http->message_body_len = len;

	return true;
}

int ezcfg_http_write_request_line(struct ezcfg_http *http, char *buf, int len)
{
	struct ezcfg *ezcfg;
	int n;

	assert(http != NULL);
	assert(buf != NULL);
	assert(len > 0);

	ezcfg = http->ezcfg;

	if (http->method_index == 0) {
		err(ezcfg, "unknown http method\n");
		return -1;
	}

	n = snprintf(buf, len, "%s %s HTTP/%d.%d\r\n",
	         http->method_strings[http->method_index],
	         http->request_uri,
	         http->version_major,
	         http->version_minor);
	if (n < 0) {
		return -1;
	}
	return n;
}

int ezcfg_http_write_headers(struct ezcfg_http *http, char *buf, int len)
{
	struct ezcfg *ezcfg;
	struct http_header *h;
	int n, count;

	assert(http != NULL);
	assert(buf != NULL);
	assert(len > 0);

	ezcfg = http->ezcfg;

	if (http->method_index == 0) {
		err(ezcfg, "unknown http method\n");
		return -1;
	}

	h = http->header_head;
	count = 0;
	while (h != NULL) {
		n = snprintf(buf+count, len-count, "%s:%s\r\n",
		         h->name,
		         h->value ? h->value : "");
		if (n < 0) {
			return -1;
		}
		count += n;
		h = h->next;
	}
	return count;
}

bool ezcfg_http_add_header(struct ezcfg_http *http, char *name, char *value)
{
	struct ezcfg *ezcfg;
	struct http_header *h;

	assert(http != NULL);
	assert(name != NULL);
	assert(value != NULL);

	ezcfg = http->ezcfg;

	h = calloc(1, sizeof(struct http_header));
	if (h == NULL) {
		err(ezcfg, "HTTP add header no memory.\n");
		return false;
	}
	memset(h, 0, sizeof(struct http_header));

	h->name = find_known_header_name(http, name);
	if (h->name != NULL) {
		/* A known header */
		h->is_known_header = true;
	} else {
		h->is_known_header = false;
		h->name = strdup(name);
		if (h->name == NULL) {
			err(ezcfg, "HTTP add header no memory 2.\n");
			free(h);
			return false;
		}
	}
				
	if (value[0] != '\0') {
		h->value = strdup(value);
		if (h->name == NULL) {
			err(ezcfg, "HTTP add header no memory 3.\n");
			if (h->is_known_header == false) {
				free(h->name);
			}
			free(h);
			return false;
		}
	}

	/* add to header list tail */
	if (http->header_tail == NULL) {
		http->header_tail = h;
		http->header_head = h;
		h->next = NULL;
	} else {
		http->header_tail->next = h;
		http->header_tail = h;
		h->next = NULL;
	}
	return true;
}
