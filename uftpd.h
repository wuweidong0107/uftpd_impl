#ifndef UFTPD_H_
#define UFTPD_H_

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

/* tftp message structure */
typedef union {

     uint16_t opcode;

     struct {
          uint16_t opcode; /* RRQ or WRQ */             
          uint8_t filename_and_mode[514];
     } request;     

     struct {
          uint16_t opcode; /* DATA */
          uint16_t block_number;
          uint8_t data[512];
     } data;

     struct {
          uint16_t opcode; /* ACK */             
          uint16_t block_number;
     } ack;

     struct {
          uint16_t opcode; /* ERROR */     
          uint16_t error_code;
          uint8_t error_string[512];
     } error;

} tftp_message_t;

typedef struct {
    int sd;
    uev_ctx_t *ctx;
    uev_t timeout_watcher, io_watcher;
    tftp_message_t msg;

    struct sockaddr_in client_sa;;
    struct sockaddr_in server_sa;
} session_t;

/* doesn't expect 5 second inactivity */
#define INACTIVITY_TIMER  5 * 1000

#define LOGIT(severity, code, fmt, args...)				\
	do {								\
		if (code)						\
			logit(severity, fmt ". Error %d: %s%s",		\
			      ##args, code, strerror(code),		\
			      do_syslog ? "" : "\n");			\
		else							\
			logit(severity, fmt "%s", ##args,		\
			      do_syslog ? "" : "\n");			\
	} while (0)
#define ERR(code, fmt, args...)  LOGIT(LOG_ERR, code, fmt, ##args)
#define WARN(code, fmt, args...) LOGIT(LOG_WARNING, code, fmt, ##args)
#define LOG(fmt, args...)        LOGIT(LOG_NOTICE, 0, fmt, ##args)
#define INFO(fmt, args...)       LOGIT(LOG_INFO, 0, fmt, ##args)
#define DBG(fmt, args...)        LOGIT(LOG_DEBUG, 0, fmt, ##args)

extern int do_syslog; 
int open_socket(int port, int type, const char *desc);
void logit(int severity, const char *fmt, ...);
int tftp_session(uev_ctx_t *ctx, int sd);
session_t* new_session(uev_ctx_t *ctx, int sd, int *rc);
int del_session(session_t *session);
#endif