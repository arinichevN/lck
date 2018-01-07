#include "main.h"

char pid_path[LINE_SIZE];
int app_state = APP_INIT;
int pid_file = -1;
int proc_id = -1;
int sock_port = -1;
int sock_fd = -1; //udp socket file descriptor
Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};

LockList lock_list = {NULL, 0};
int locked = 1;

#include "init_f.c"
#include "util.c"

void serverRun(int *state) {
    SERVER_HEADER
    if (ACP_CMD_IS(ACP_CMD_APP_EXIT)) {
        *state = APP_EXIT;
        return;
    } else if (ACP_CMD_IS(ACP_CMD_APP_PING)) {
        acp_responseSendStr(ACP_RESP_APP_BUSY, ACP_LAST_PACK, &response, &peer_client);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_APP_PRINT)) {
        printData(&response);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_APP_HELP)) {
        printHelp(&response);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_LCK_LOCK)) {
        lockClose(&lock_list);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_LCK_UNLOCK)) {
        lockOpen(&lock_list);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_GET_DATA)) {
        char q[LINE_SIZE];
        snprintf(q, sizeof q, "%d" ACP_DELIMITER_ROW_STR, locked);
        SEND_STR_L_P(q)
        return;
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
    if (!gpioSetup()) {
        exit_nicely_e("initApp: failed to initialize GPIO\n");
    }
    if (!initLock(&lock_list)) {
        exit_nicely_e("initApp: failed to initialize lock\n");
    }
    if (!checkLock(&lock_list)) {
        exit_nicely_e("initApp: failed to check lock\n");
    }
    
    lockPrep(&lock_list);
}

void freeApp() {
    lockClose(&lock_list);
    FREE_LIST(&lock_list);
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
    if (geteuid() != 0) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s: root user expected\n", APP_NAME_STR);
#endif
        return (EXIT_FAILURE);
    }
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

