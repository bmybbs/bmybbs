#include <stdbool.h>
#include <string.h>

bool is_system_board(const char *boardname) {
	return (!strcasecmp(boardname, "newcomers")
			|| !strcasecmp(boardname, "millionairesrec")
			|| !strcasecmp(boardname, "syssecurity"));
}

