#include "headers.h"

void insert_token(char* str, tok** token_list) {
	*token_list = malloc(sizeof(tok));
	size_t str_len = strlen(str);
	(*token_list)->value = calloc(str_len, sizeof(char));
	strncpy((*token_list)->value, str, str_len);
}

tok** lex_line(char* buffer) {
	// Remove eventually end-len char (\n)
	if(buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = '\0';
	// Allocate space for max 128 token
	tok** token_list = calloc(128, sizeof(tok*));
	int tl_ind = 0;
	char* ptr = strtok(buffer, " ");
	while (ptr != NULL) {
		// Pass to insert_token() the string and the index where to store the new token
		insert_token(ptr, &token_list[tl_ind]);
		tl_ind++;
		ptr = strtok(NULL, " ");
	}
	return token_list;
}