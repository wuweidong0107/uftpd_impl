#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>		/*  PRIu64/PRI64, etc. for stdint.h types */
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>		/* isset(), setbit(), etc. */
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <uev/uev.h>

#include "uftpd.h"

/* global daemon settings */
char *prognm = "uftpd";
char *home = NULL;

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
        if (!home) {
            fprintf(stderr, "Invalid root directory %s,", argv[optind]);
            exit(1);
        }
    }
}