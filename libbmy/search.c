#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <json-c/json.h>
#include "config.h"
#include "ytht/strlib.h"
#include "ytht/msg.h"
#include "memorystruct.h"
#include "bmy/search.h"
#include "bmy/convcode.h"

static const char *ILLEGAL_CMD_CHARS = "\\><'\";`()[]{}$#|";
static const char *SEARCH_CMD = "LD_LIBRARY_PATH=" MY_BBS_HOME "/lib java -cp \"" MY_BBS_HOME "/java/*\" edu.xjtu.bmybbs.App search";
static const char *FIELD_TITLE = "title";
static const char *FIELD_OWNER = "owner";
static const char *FIELD_AID   = "timestamp";
static const char *FIELD_TID   = "thread";

struct fileheader_utf *bmy_search_board(const char *board, const char *whattosearch, size_t *search_size) {
	char *dup_whattosearch, *cmd, buf[256], *ptr;
	size_t i, len, cmdlen, bufread, search_length = 0;
	FILE *fp;
	struct json_object *search_results, *search_item, *search_field;
	struct MemoryStruct chunk;
	struct fileheader_utf *articles = NULL;

	if (!search_size)
		return NULL;

	*search_size = 0;

	chunk.memory = malloc(1);
	chunk.size = 0;

	if (chunk.memory == NULL) {
		return NULL;
	}

	if ((dup_whattosearch = strdup(whattosearch)) != NULL) {
		cmdlen = 5 /* two spaces + two quotes + terminator */ + strlen(SEARCH_CMD) + strlen(board) + strlen(dup_whattosearch);
		if ((cmd = malloc(cmdlen)) != NULL) {
			for (i = 0, len = strlen(dup_whattosearch); i < len; i++) {
				if (strchr(ILLEGAL_CMD_CHARS, dup_whattosearch[i]) != NULL) {
					dup_whattosearch[i] = ' ';
				}
			}
			snprintf(cmd, cmdlen, "%s %s \"%s\"", SEARCH_CMD, board, dup_whattosearch);
			if ((fp = popen(cmd, "r")) != NULL) {
				while ((bufread = fread(buf, 1, sizeof(buf), fp)) != 0) {
					if ((ptr = realloc(chunk.memory, chunk.size + bufread + 1)) != NULL) {
						chunk.memory = ptr;
						memcpy(&(chunk.memory[chunk.size]), buf, bufread);
						chunk.size += bufread;
						chunk.memory[chunk.size] = 0;
					} else {
						newtrace("[bmy/search] cannot realloc");
					}
					if (bufread < sizeof(buf)) {
						if (feof(fp)) {
							break;
						} else if (ferror(fp)) {
							newtrace("[bmy/search] has problem of reading stdout");
							break;
						}
					}
				}
				pclose(fp);
			}
			free(cmd);

			if ((search_results = json_tokener_parse(chunk.memory)) != NULL) {
				if (json_object_is_type(search_results, json_type_array)) {
					if ((search_length = json_object_array_length(search_results)) > 0) {
						if ((articles = calloc(search_length, sizeof(struct fileheader_utf))) != NULL) {
							*search_size = search_length;
							for (i = 0; i < search_length; i++) {
								search_item = json_object_array_get_idx(search_results, i);
								ytht_strsncpy(articles[i].boardname_en, board, sizeof(articles[i].boardname_en));
								if ((search_field = json_object_object_get(search_item, FIELD_TITLE)) != NULL) {
									ytht_strsncpy(articles[i].title, json_object_get_string(search_field), sizeof(articles[i].title));
								}
								if ((search_field = json_object_object_get(search_item, FIELD_OWNER)) != NULL) {
									ytht_strsncpy(articles[i].owner, json_object_get_string(search_field), sizeof(articles[i].owner));
								}
								if ((search_field = json_object_object_get(search_item, FIELD_AID)) != NULL) {
									ytht_strsncpy(buf, json_object_get_string(search_field), sizeof(buf));
									articles[i].filetime = atol(buf);
								}
								if ((search_field = json_object_object_get(search_item, FIELD_TID)) != NULL) {
									ytht_strsncpy(buf, json_object_get_string(search_field), sizeof(buf));
									articles[i].thread = atol(buf);
								}
							}
						}
					}
				}
				json_object_put(search_results);
			}
		}
		free(dup_whattosearch);
	}

	if (chunk.memory) {
		free(chunk.memory);
	}

	return articles;
}

struct fileheader_utf *bmy_search_board_gbk(const char *board, const char *whattosearch_gbk, size_t *search_size) {
	char *whattosearch;
	struct fileheader_utf *articles = NULL;
	size_t len;

	len = strlen(whattosearch_gbk);
	if ((whattosearch = calloc(len * 2, 1)) != NULL) {
		g2u(whattosearch_gbk, len, whattosearch, len * 2);
		articles = bmy_search_board(board, whattosearch, search_size);
		free(whattosearch);
	}

	return articles;
}

