#include "uftpd.h"

#define TFTP_DEFAULT_PORT 69
#define TFTP_SERVICE_NAME "tftp"
#define TFTP_PROTO_NAME   "udp"

int do_syslog = 0;

/* global daemon settings */
static char *prognm = "uftpd";
static char *home = NULL;
static int do_tftp = TFTP_DEFAULT_PORT;

static uev_t tftp_watcher;

static int usage(int code)
{
	printf("\nUsage: %s [-hnsv] [-l LEVEL] [-o OPTS] [-p FILE] [PATH]\n\n", prognm);
	printf("  -h         Show this help text\n"
	       "  -l LEVEL   Set log level: none, err, notice (default), info, debug\n");

	return code;
}

static void tftp_cb(uev_t *w, void *arg, int events)
{
    uev_io_stop(w);
    
    if (UEV_ERROR == events || UEV_HUP == events) {
        uev_io_start(w);
        return;
    }

    /* create new sesion */
    INFO("new session");
}

static int start_service(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, 
                int port, int type, char *desc)
{
    int sd;

    sd = open_socket(port, type, desc);
    if (sd < 0) {
        ERR(errno, "Not allow to start %s service at port %d", desc, port);
        return -1;
    }

    INFO("Starting %s server on port %d ...", desc, port);
    uev_io_init(ctx, w, cb, ctx, sd, UEV_READ);
    return 0;
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

    if (uev_init(&ctx)) {
        ERR(0, "Failed initializing, exiting.");
		exit(1);
    }

	INFO("Serving files from %s ...", home);

    if (start_service(&ctx, &tftp_watcher, tftp_cb, do_tftp, SOCK_DGRAM, "TFTP")) {
        ERR(0, "Failed Serving files, exiting.");
        exit(1);
    }

	uev_run(&ctx, 0);
    INFO("Bye.");
}