#include "uftpd.h"

int open_socket(int port, int type, const char *desc)
{
    int sd, err, val = 1;
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in server;

    sd = socket(AF_INET, type | SOCK_NONBLOCK, 0);
    if (sd < 0) {
        ERR(errno, "Failed creating %s server socket", desc);
        return -1;
    }

    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));
    if (err != 0)
        WARN(errno, "Failed setting SO_REUSEADDR on %s socket", desc);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(sd, (struct sockaddr *)&server, len) < 0) {
        if (EACCES != errno) {
            WARN(errno, "Failed binding to port %d, maybe another %s server is already running", port, desc);
        }
        close(sd);
        return -1;
    }
    return sd;
}