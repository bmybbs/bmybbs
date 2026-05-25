#include "importer.h"

#include <stdio.h>
#include <stdlib.h>

static void bmy_log_importer_print_usage(const char *argv0) {
	fprintf(stderr, "Usage: %s [--dry-run] YYYY-MM-DD\n", argv0);
}

static void bmy_log_importer_print_summary(
	const struct bmy_log_importer_config *config,
	const struct bmy_log_import_summary *summary) {
	printf("source_file: %s\n", config->source_file ? config->source_file : "(null)");
	printf("dry_run: %s\n", config->dry_run ? "yes" : "no");
	printf("total_lines: %lu\n", summary->total_lines);
	if (config->dry_run)
		printf("accepted: %lu\n", summary->inserted);
	else
		printf("inserted: %lu\n", summary->inserted);
	printf("already_imported: %lu\n", summary->already_imported);
	printf("discarded: %lu\n", summary->discarded);
	printf("unrecognized: %lu\n", summary->unrecognized);
	printf("failed: %lu\n", summary->failed);
}

int main(int argc, char **argv) {
	struct bmy_log_importer_config config = {0};
	struct bmy_log_import_summary summary = {0};
	int rc;

	rc = bmy_log_importer_parse_args(argc, argv, &config);
	if (rc != 0) {
		bmy_log_importer_print_usage(argv[0]);
		bmy_log_importer_config_cleanup(&config);
		return EXIT_FAILURE;
	}

	rc = bmy_log_importer_run(&config, &summary);
	bmy_log_importer_print_summary(&config, &summary);
	bmy_log_importer_config_cleanup(&config);

	return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
