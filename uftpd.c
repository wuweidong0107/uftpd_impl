#include "uftpd.h"

#define TFTP_DEFAULT_PORT 69
#define TFTP_SERVICE_NAME "tftp"
#define TFTP_PROTO_NAME   "udp"

/* global daemon settings */
static char *prognm = "uftpd";
static char *home = NULL;
static int do_syslog = 0;
static int do_tftp = TFTP_DEFAULT_PORT;

static int usage(int code)
{
	printf("\nUsage: %s [-hnsv] [-l LEVEL] [-o OPTS] [-p FILE] [PATH]\n\n", prognm);
	printf("  -h         Show this help text\n"
	       "  -l LEVEL   Set log level: none, err, notice (default), info, debug\n");

	return code;
}

int main(int argc, char **argv)
{
    uev_ctx_t ctx;
    int c;

    if (argc == 1) {
        return usage(1);
    }

    while((c = getopt(argc, argv, "hl:n")) != EOF) {
        switch (c) {
        case 'h':
            return usage(0);
        default:
            return usage(1);
        }
    }

    if (optind < argc) {
        home = realpath(argv[optind], NULL);
        if (!home || access(home, F_OK)) {
            fprintf(stderr, "Invalid root directory %s,", argv[optind]);
            exit(1);
        }
    }

    INFO("home directory: %s", home);
}