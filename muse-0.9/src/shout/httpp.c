/* Httpp.c
**
** http parsing engine
*/

#define _GNU_SOURCE 1

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "thread.h"
#include "avl.h"
#include "httpp.h"

#ifdef _WIN32
#define strcasecmp stricmp
#endif

#define MAX_HEADERS 32

/* internal functions */

/* misc */
char *_lowercase(char *str);

/* for avl trees */
int _compare_vars(void *compare_arg, void *a, void *b);
int _free_vars(void *key);

http_parser_t *httpp_create_parser(void)
{
	return (http_parser_t *)malloc(sizeof(http_parser_t));
}

void httpp_initialize(http_parser_t *parser, http_varlist_t *defaults)
{
	http_varlist_t *list;

	parser->req_type = httpp_req_none;
	parser->uri = NULL;
	parser->vars = avl_tree_new(_compare_vars, NULL);

	/* now insert the default variables */
	list = defaults;
	while (list != NULL) {
		httpp_setvar(parser, list->var.name, list->var.value);
		list = list->next;
	}
}

static int split_headers(char *data, unsigned long len, char **line)
{
	/* first we count how many lines there are 
	** and set up the line[] array	 
	*/
	int i, lines = 0;
	line[lines] = data;
	for (i = 0; i < len && lines < MAX_HEADERS; i++) {
		if (data[i] == '\r')
			data[i] = '\0';
		if (data[i] == '\n') {
			lines++;
			data[i] = '\0';
			if (i + 1 < len) {
				if (data[i + 1] == '\n' || data[i + 1] == '\r')
					break;
				line[lines] = &data[i + 1];
            }
		}
	}

	i++;
	while (data[i] == '\n') i++;

    return lines;
}

static void parse_headers(http_parser_t *parser, char **line, int lines)
{
    int i,l;
	int whitespace, where, slen;
	char *name = NULL;
	char *value = NULL;

	/* parse the name: value lines. */
	for (l = 1; l < lines; l++) {
		where = 0;
		whitespace = 0;
		name = line[l];
		value = NULL;
		slen = strlen(line[l]);
		for (i = 0; i < slen; i++) {
			if (line[l][i] == ':') {
				whitespace = 1;
				line[l][i] = '\0';
			} else {
				if (whitespace) {
					whitespace = 0;
					while (i < slen && line[l][i] == ' ')
						i++;

					if (i < slen)
						value = &line[l][i];
					
					break;
				}
			}
		}
		
		if (name != NULL && value != NULL) {
			httpp_setvar(parser, _lowercase(name), value);
			name = NULL; 
			value = NULL;
		}
	}
}

int httpp_parse_response(http_parser_t *parser, char *http_data, unsigned long len, char *uri)
{
    char *data;
    char *line[MAX_HEADERS];
    int lines, slen,i, whitespace=0, where=0,code;
    char *version=NULL, *resp_code=NULL, *message=NULL;
    
    if(http_data == NULL)
        return 0;

	/* make a local copy of the data, including 0 terminator */
	data = (char *)malloc(len+1);
	if (data == NULL) return 0;
	memcpy(data, http_data, len);
    data[len] = 0;

    lines = split_headers(data, len, line);

    /* In this case, the first line contains:
     * VERSION RESPONSE_CODE MESSAGE, such as
     * HTTP/1.0 200 OK
     */
    slen = strlen(line[0]);
    version = line[0];
    for(i=0; i < slen; i++) {
        if(line[0][i] == ' ') {
            line[0][i] = 0;
            whitespace = 1;
        }
        else if(whitespace) {
            whitespace = 0;
            where++;
            if(where == 1)
                resp_code = &line[0][i];
            else {
                message = &line[0][i];
                break;
            }
        }
    }

    if(version == NULL || resp_code == NULL || message == NULL) {
        free(data);
        return 0;
    }

    httpp_setvar(parser, HTTPP_VAR_ERROR_CODE, resp_code);
    code = atoi(resp_code);
    if(code < 200 || code >= 300) {
        httpp_setvar(parser, HTTPP_VAR_ERROR_MESSAGE, message);
    }

    httpp_setvar(parser, HTTPP_VAR_URI, uri);
	httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "NONE");

    parse_headers(parser, line, lines);

	free(data);

	return 1;
}

int httpp_parse(http_parser_t *parser, char *http_data, unsigned long len)
{
	char *data, *tmp;
	char *line[MAX_HEADERS]; /* limited to 32 lines, should be more than enough */
	int i;
	int lines;
	char *req_type = NULL;
	char *uri = NULL;
	char *version = NULL;
    int whitespace, where, slen;

	if (http_data == NULL)
		return 0;

	/* make a local copy of the data, including 0 terminator */
	data = (char *)malloc(len+1);
	if (data == NULL) return 0;
	memcpy(data, http_data, len);
    data[len] = 0;

    lines = split_headers(data, len, line);

	/* parse the first line special
	** the format is:
	** REQ_TYPE URI VERSION
	** eg:
	** GET /index.html HTTP/1.0
	*/
	where = 0;
	whitespace = 0;
	slen = strlen(line[0]);
	req_type = line[0];
	for (i = 0; i < slen; i++) {
		if (line[0][i] == ' ') {
			whitespace = 1;
			line[0][i] = '\0';
		} else {
			/* we're just past the whitespace boundry */
			if (whitespace) {
				whitespace = 0;
				where++;
				switch (where) {
				case 1:
					uri = &line[0][i];
					break;
				case 2:
					version = &line[0][i];
					break;
				}
			}
		}
	}

	if (strcasecmp("GET", req_type) == 0) {
		parser->req_type = httpp_req_get;
	} else if (strcasecmp("POST", req_type) == 0) {
		parser->req_type = httpp_req_post;
	} else if (strcasecmp("HEAD", req_type) == 0) {
		parser->req_type = httpp_req_head;
	} else if (strcasecmp("SOURCE", req_type) == 0) {
		parser->req_type = httpp_req_source;
	} else if (strcasecmp("PLAY", req_type) == 0) {
		parser->req_type = httpp_req_play;
	} else if (strcasecmp("STATS", req_type) == 0) {
		parser->req_type = httpp_req_stats;
	} else {
		parser->req_type = httpp_req_unknown;
	}

	if (uri != NULL && strlen(uri) > 0)
		parser->uri = strdup(uri);
	else
		parser->uri = NULL;

	if ((version != NULL) && ((tmp = strchr(version, '/')) != NULL)) {
		tmp[0] = '\0';
		if ((strlen(version) > 0) && (strlen(&tmp[1]) > 0)) {
			httpp_setvar(parser, HTTPP_VAR_PROTOCOL, version);
			httpp_setvar(parser, HTTPP_VAR_VERSION, &tmp[1]);
		} else {
			free(data);
			return 0;
		}
	} else {
		free(data);
		return 0;
	}

	if (parser->req_type != httpp_req_none && parser->req_type != httpp_req_unknown) {
		switch (parser->req_type) {
		case httpp_req_get:
			httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "GET");
			break;
		case httpp_req_post:
			httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "POST");
			break;
		case httpp_req_head:
			httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "HEAD");
			break;
		case httpp_req_source:
			httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "SOURCE");
			break;
		case httpp_req_play:
			httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "PLAY");
			break;
		case httpp_req_stats:
			httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "STATS");
			break;
		default:
			break;
		}
	} else {
		free(data);
		return 0;
	}

    if (parser->uri != NULL) {
		httpp_setvar(parser, HTTPP_VAR_URI, parser->uri);
	} else {
		free(data);
		return 0;
	}

    parse_headers(parser, line, lines);

	free(data);

	return 1;
}

void httpp_setvar(http_parser_t *parser, char *name, char *value)
{
	http_var_t *var;

	if (name == NULL || value == NULL)
		return;

	var = (http_var_t *)malloc(sizeof(http_var_t));
	if (var == NULL) return;

	var->name = strdup(name);
	var->value = strdup(value);

	if (httpp_getvar(parser, name) == NULL) {
		avl_insert(parser->vars, (void *)var);
	} else {
		avl_delete(parser->vars, (void *)var, _free_vars);
		avl_insert(parser->vars, (void *)var);
	}
}

char *httpp_getvar(http_parser_t *parser, char *name)
{
	http_var_t var;
	http_var_t *found;

	var.name = name;
        var.value = NULL;

	if (avl_get_by_key(parser->vars, (void *)&var, (void **)&found) == 0)
		return found->value;
	else
		return NULL;
}

void httpp_clear(http_parser_t *parser)
{
	parser->req_type = httpp_req_none;
	if (parser->uri)
		free(parser->uri);
	parser->uri = NULL;
	avl_tree_free(parser->vars, _free_vars);
	parser->vars = NULL;
}

void httpp_destroy(http_parser_t *parser)
{
    httpp_clear(parser);
    free(parser);
}

char *_lowercase(char *str)
{
	long i;
	for (i = 0; i < strlen(str); i++)
		str[i] = tolower(str[i]);

	return str;
}

int _compare_vars(void *compare_arg, void *a, void *b)
{
	http_var_t *vara, *varb;

	vara = (http_var_t *)a;
	varb = (http_var_t *)b;

	return strcmp(vara->name, varb->name);
}

int _free_vars(void *key)
{
	http_var_t *var;

	var = (http_var_t *)key;

	if (var->name)
		free(var->name);
	if (var->value)
		free(var->value);
	free(var);

	return 1;
}

