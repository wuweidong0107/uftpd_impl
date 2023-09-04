#include "uftpd.h"
//#include <arpa/tftp.h>

static int do_send(session_t *session, size_t len)
{
    ssize_t c;
    ssize_t sa_len = sizeof(struct sockaddr_in);
    if ((c = sendto(session->sd, &session->msg, 4+len, 0, (struct sockaddr *) &session->client_sa, sa_len)) < 0) {
        perror("sendto");
        return -1;
    }
    return 0;
}

static int send_ERROR(session_t *session)
{
    return 0;
}

static int send_DATA(session_t *session, uint16_t block)
{
    size_t len;

    session->msg.opcode = htons(DATA);
    session->msg.data.block_number = htons(block);
    len = fread(session->msg.data.data, 1, sizeof(session->msg.data.data), session->fp);
    if (len < 512)
        session->to_close = 1;
    return do_send(session, len);
}

static int handle_RRQ(session_t *session)
{
    session->fp = fopen(session->filename, "r");
    if (!session->fp) {
        ERR(errno, "%s: Failed openning %s", inet_ntoa(session->client_sa.sin_addr), session->filename);
        send_ERROR(session);
        return -1;
    }
    return send_DATA(session, session->block_number);
}

static int handle_ACK(session_t *session)
{
    if (session->block_number != session->msg.ack.block_number) {
        send_ERROR(session);
        return -1;
    }
    session->block_number++;
    return send_DATA(session, session->block_number);
}

void tftp_read_cb(uev_t *w, void *arg, int events)
{
    session_t *session = (session_t *)arg;
    ssize_t len;
    struct sockaddr *addr = (struct sockaddr *)&session->client_sa;
    socklen_t addr_len = sizeof(session->client_sa);
    uint16_t opcode;

    /* Reset inactivity timer. */
	uev_timer_set(&session->timeout_watcher, INACTIVITY_TIMER, 0);

    INFO("Reading tftp...");
    len = recvfrom(session->sd, &session->msg, sizeof(session->msg), 0, addr, &addr_len);
    if (-1 == len) {
        if (errno != EINTR)
            ERR(errno, "Failed reading command/status from client");
        uev_exit(w->ctx);
        return;
    }

    INFO("Reading tftp, %d bytes...", len);
    opcode = ntohs(session->msg.opcode);
    switch(opcode) {
        case RRQ:
            session->filename = strdup((char *)session->msg.request.filename_and_mode);
            session->block_number = 1;
            session->to_close = 0;
            INFO("tftp RRQ '%s' from %s:%d", session->filename, 
                inet_ntoa(session->client_sa.sin_addr), ntohs(session->client_sa.sin_port));
            handle_RRQ(session);
        case ACK:
            INFO("tftp ACK, block %u", session->msg.ack.block_number);
            handle_ACK(session);
        default:
            break;
    }
}

int tftp_session(uev_ctx_t *ctx, int sd)
{
    int pid = 0, ret;
    session_t *session;

    session = new_session(ctx, sd, &pid);
    if (!session) {
        // parent
        return pid;
    }
    
    // child process
    uev_io_init(session->ctx, &session->io_watcher, tftp_read_cb, session, session->sd, UEV_READ);
    uev_run(session->ctx, 0);

    ret = del_session(session);
    exit(ret);
}