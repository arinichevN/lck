#include "main.h"

int app_state = APP_INIT;
int pid_file = -1;
int sock_port = -1;
int sock_fd = -1; //udp socket file descriptor
Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};

LockList lock_list = LIST_INITIALIZER;
FTS locked;

#include "init_f.c"
#include "util.c"

void serverRun(int *state) {
    SERVER_HEADER
    DEF_SERVER_I1LIST
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
        lockClose(&lock_list, &locked);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_LCK_UNLOCK)) {
        lockOpen(&lock_list, &locked);
        return;
    } else if (ACP_CMD_IS(ACP_CMD_GET_DATA)) {
        char q[LINE_SIZE];
        snprintf(q, sizeof q, "%.0f" ACP_DELIMITER_ROW_STR, locked.value);
        SEND_STR_L_P(q)
        return;
    } else if (ACP_CMD_IS(ACP_CMD_GET_FTS)) {
        acp_requestDataToI1List(&request, &i1l);
        if (i1l.length <= 0) {
            return;
        }
        for (int i = 0; i < i1l.length; i++) {
            int r = acp_responseFTSCat(i1l.item[i], locked.value, locked.tm, locked.state, &response);
            if (!r) {
                return;
            }
        }
        acp_responseSend(&response, &peer_client);
        return;
    }
}

void initApp() {
    if (!readSettings(&sock_port, CONFIG_FILE)) {
        exit_nicely_e("initApp: failed to read settings\n");
    }
    if (!initServer(&sock_fd, sock_port)) {
        exit_nicely_e("initApp: failed to initialize udp server\n");
    }
    if (!gpioSetup()) {
        exit_nicely_e("initApp: failed to initialize GPIO\n");
    }
    if (!initLock(&lock_list, KEY_FILE)) {
        exit_nicely_e("initApp: failed to initialize lock\n");
    }
    if (!checkLock(&lock_list)) {
        exit_nicely_e("initApp: failed to check lock\n");
    }
    lockPrep(&lock_list, &locked);
    lockOpen(&lock_list, &locked);
}

void freeApp() {
    lockClose(&lock_list, &locked);
    FREE_LIST(&lock_list);
    freeSocketFd(&sock_fd);
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
    while (1) {
#ifdef MODE_DEBUG
        printf("%s(): %s\n", F, getAppState(app_state));
#endif
        switch (app_state) {
            case APP_INIT:
                initApp();
                app_state = APP_RUN;
                break;
            case APP_RUN:
                serverRun(&app_state);
                break;
            case APP_EXIT:
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

