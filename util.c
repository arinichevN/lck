

int checkLock(LockList *list) {
    FORLIST(i){
        if (!checkPin(LIi.pin)) {
            fprintf(stderr, "%s(): bad pin where pin = %d\n",F, LIi.pin);
            return 0;
        }
        if (!(LIi.value == 1 || LIi.value == 0)) {
            fprintf(stderr, "%s(): bad value where pin = %d (1 or 0 expected)\n",F, LIi.pin);
            return 0;
        }
    }
    return 1;
}

void lockClose(const LockList *list) {
    extern int locked;
    FORLIST(i){
        if (LIi.value) {
            pinHigh(LIi.pin);
        } else {
            pinLow(LIi.pin);
        }
    }
    locked = 1;
}

void lockPrep(const LockList *list) {
    FORLIST(i){
        pinPUD(LIi.pin, PUD_OFF);
        pinModeOut(LIi.pin);
    }
    lockClose(list);
}

void lockOpen(const LockList *list) {
    extern int locked;
    FORLIST(i){
        if (LIi.value) {
            pinLow(LIi.pin);
        } else {
            pinHigh(LIi.pin);
        }
    }
    locked = 0;
}

void printData(ACPResponse *response) {
    LockList *list=&lock_list;
    int i = 0;
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "app_state: %s\n", getAppState(app_state));
    SEND_STR(q)
    snprintf(q, sizeof q, "PID: %d\n", getpid());
    SEND_STR(q)
    snprintf(q, sizeof q, "port: %d\n", sock_port);
    SEND_STR(q)
    snprintf(q, sizeof q, "locked: %d\n", locked);
    SEND_STR(q)
    SEND_STR("+-----------------------+\n")
    SEND_STR("|        device         |\n")
    SEND_STR("+-----------+-----------+\n")
    SEND_STR("|    pin    |   value   |\n")
    SEND_STR("+-----------+-----------+\n")
    FORL{
        snprintf(q, sizeof q, "|%11d|%11d|\n",
        LIi.pin,
        LIi.value
        );
        SEND_STR(q)
    }
    SEND_STR_L("+-----------+-----------+\n")
}

void printHelp(ACPResponse *response) {
    char q[LINE_SIZE];
    SEND_STR("COMMAND LIST\n")
    snprintf(q, sizeof q, "%s\tterminate process\n", ACP_CMD_APP_EXIT);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget state of process; response: B - process is in active mode, I - process is in standby mode\n", ACP_CMD_APP_PING);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget some variable's values; response will be packed into multiple packets\n", ACP_CMD_APP_PRINT);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget this help; response will be packed into multiple packets\n", ACP_CMD_APP_HELP);
    SEND_STR(q)

    snprintf(q, sizeof q, "%s\tlock\n", ACP_CMD_LCK_LOCK);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tunlock\n", ACP_CMD_LCK_UNLOCK);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tresponse: 1-locked, 0-unlocked\n", ACP_CMD_GET_DATA);
    SEND_STR_L(q)
}