/*
 * lck
 */

#include "main.h"

char pid_path[LINE_SIZE];
int app_state = APP_INIT;
int pid_file = -1;
int proc_id = -1;
int sock_port = -1;
int sock_fd = -1; //udp socket file descriptor
size_t sock_buf_size = 0;
Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};

LockList lock_list = {NULL, 0};
int locked = 1;

I1List i1l = {NULL, 0};

#include "init_f.c"
#include "util.c"

void serverRun(int *state) {
    char buf_in[sock_buf_size];
    char buf_out[sock_buf_size];
    uint8_t crc;
    crc = 0;
    memset(buf_in, 0, sizeof buf_in);
    acp_initBuf(buf_out, sizeof buf_out);
    if (recvfrom(sock_fd, buf_in, sizeof buf_in, 0, (struct sockaddr*) (&(peer_client.addr)), &(peer_client.addr_size)) < 0) {
#ifdef MODE_DEBUG
        perror("serverRun: recvfrom() error");
#endif
    }
#ifdef MODE_DEBUG
    dumpBuf(buf_in, sizeof buf_in);
#endif    
    if (!crc_check(buf_in, sizeof buf_in)) {
#ifdef MODE_DEBUG
        fputs("WARNING: serverRun: crc check failed\n", stderr);
#endif
        return;
    }
    switch (buf_in[1]) {
        case ACP_CMD_APP_EXIT:
            *state = APP_EXIT;
            return;
        case ACP_CMD_APP_PING:
            sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_APP_BUSY);
            return;
        case ACP_CMD_APP_PRINT:
            printData(&lock_list);
            return;
        case ACP_CMD_APP_HELP:
            printHelp();
            return;
        case ACP_CMD_LCK_LOCK:
            lockClose(&lock_list);
            return;
        case ACP_CMD_LCK_UNLOCK:
            lockOpen(&lock_list);
            return;
        case ACP_CMD_LCK_GET_DATA:
            if (!bufCatData(locked, buf_out, sock_buf_size)) {
                sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_BUF_OVERFLOW);
                return;
            }
            if (!sendBufPack(buf_out, ACP_QUANTIFIER_BROADCAST, ACP_RESP_REQUEST_SUCCEEDED)) {
                sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_BUF_OVERFLOW);
                return;
            }
            return;
        default:
            break;
    }

}

void initApp() {
    if (!readSettings()) {
        exit_nicely_e("initApp: failed to read settings\n");
    }
    if (!initPid(&pid_file, &proc_id, pid_path)) {
        exit_nicely_e("initApp: failed to initialize pid\n");
    }
    if (!initServer(&sock_fd, sock_port)) {
        exit_nicely_e("initApp: failed to initialize udp server\n");
    }
    i1l.item = (int *) malloc(sock_buf_size * sizeof *(i1l.item));
    if (i1l.item == NULL) {
        exit_nicely_e("initApp: failed to initialize I1 list\n");
    }
    if (!initLock(&lock_list)) {
        exit_nicely_e("initApp: failed to initialize lock\n");
    }
    if (!checkLock(&lock_list)) {
        exit_nicely_e("initApp: failed to check lock\n");
    }
    if (!gpioSetup()) {
        exit_nicely_e("initApp: failed to initialize GPIO\n");
    }
    lockPrep(&lock_list);

}

void freeApp() {
    lockClose(&lock_list);
    FREE_LIST(&lock_list);
    FREE_LIST(&i1l);
#ifndef PLATFORM_ANY
    gpioFree();
#endif
    freeSocketFd(&sock_fd);
    freePid(&pid_file, &proc_id, pid_path);
}

void exit_nicely() {
    freeApp();
#ifdef MODE_DEBUG
    puts("\nBye...");
#endif
    exit(EXIT_SUCCESS);
}

void exit_nicely_e(char *s) {
    fprintf(stderr, "%s", s);
    freeApp();
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
#ifndef MODE_DEBUG
    daemon(0, 0);
#endif
    conSig(&exit_nicely);
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("main: memory locking failed");
    }
#ifndef MODE_DEBUG
    setPriorityMax(SCHED_FIFO);
#endif
    while (1) {
        switch (app_state) {
            case APP_INIT:
#ifdef MODE_DEBUG
                puts("MAIN: init");
#endif
                initApp();
                app_state = APP_RUN;
                break;
            case APP_RUN:
#ifdef MODE_DEBUG
                puts("MAIN: run");
#endif
                serverRun(&app_state);
                break;
            case APP_EXIT:
#ifdef MODE_DEBUG
                puts("MAIN: exit");
#endif
                exit_nicely();
                break;
            default:
                exit_nicely_e("main: unknown application state");
                break;
        }
    }
    freeApp();
    return (EXIT_SUCCESS);
}

