

int checkLock(LockList *list) {
    size_t i;
    FORL{
        if (!checkPin(LIi.pin)) {
            fprintf(stderr, "ERROR: checkLock: bad pin where pin = %d\n", LIi.pin);
            return 0;
        }
        if (!(LIi.value == 1 || LIi.value == 0)) {
            fprintf(stderr, "ERROR: checkLock: bad value where pin = %d (1 or 0 expected)\n", LIi.pin);
            return 0;
        }
    }
    return 1;
}

void lockClose(const LockList *list) {
    extern int locked;
    size_t i;
    FORL{
        if (LIi.value) {
            pinHigh(LIi.pin);
        } else {
            pinLow(LIi.pin);
        }
    }
    locked = 1;
}

void lockPrep(const LockList *list) {
    size_t i;
    FORL{
        pinPUD(LIi.pin, PUD_OFF);
        pinModeOut(LIi.pin);
    }
    lockClose(list);
}

void lockOpen(const LockList *list) {
    extern int locked;
    size_t i;
    FORL{
        if (LIi.value) {
            pinLow(LIi.pin);
        } else {
            pinHigh(LIi.pin);
        }
    }
    locked = 0;
}

int bufCatData(int locked, char *buf, size_t buf_size) {
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "%d" ACP_DELIMITER_ROW_STR, locked);
    if (bufCat(buf, q, buf_size) == NULL) {
        return 0;
    }
    return 1;
}

int sendStrPack(char qnf, char *cmd) {
    extern Peer peer_client;
    return acp_sendStrPack(qnf, cmd, &peer_client);
}

int sendBufPack(char *buf, char qnf, char *cmd_str) {
    extern Peer peer_client;
    return acp_sendBufPack(buf, qnf, cmd_str, &peer_client);
}

void sendStr(const char *s, uint8_t *crc) {
    acp_sendStr(s, crc, &peer_client);
}

void sendFooter(int8_t crc) {
    acp_sendFooter(crc, &peer_client);
}

void printData(LockList *list) {
    int i = 0;
    char q[LINE_SIZE];
    uint8_t crc = 0;
    snprintf(q, sizeof q, "pid_path: %s\n", pid_path);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "app_state: %s\n", getAppState(app_state));
    sendStr(q, &crc);
    snprintf(q, sizeof q, "PID: %d\n", proc_id);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "port: %d\n", sock_port);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "sock_buf_size: %d\n", sock_buf_size);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "locked: %d\n", locked);
    sendStr(q, &crc);
    sendStr("+-----------------------+\n", &crc);
    sendStr("|        device         |\n", &crc);
    sendStr("+-----------+-----------+\n", &crc);
    sendStr("|    pin    |   value   |\n", &crc);
    sendStr("+-----------+-----------+\n", &crc);
    FORL{
        snprintf(q, sizeof q, "|%11d|%11d|\n",
        LIi.pin,
        LIi.value
        );
        sendStr(q, &crc);
    }
    sendStr("+-----------+-----------+\n", &crc);

    sendFooter(crc);
}

void printHelp() {
    char q[LINE_SIZE];
    uint8_t crc = 0;
    sendStr("COMMAND LIST\n", &crc);
    snprintf(q, sizeof q, "%c\tterminate process\n", ACP_CMD_APP_EXIT);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tget state of process; response: B - process is in active mode, I - process is in standby mode\n", ACP_CMD_APP_PING);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tget some variable's values; response will be packed into multiple packets\n", ACP_CMD_APP_PRINT);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tget this help; response will be packed into multiple packets\n", ACP_CMD_APP_HELP);
    sendStr(q, &crc);

    snprintf(q, sizeof q, "%c\tlock\n", ACP_CMD_LCK_LOCK);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tunlock\n", ACP_CMD_LCK_UNLOCK);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tresponse: 1-locked, 0-unlocked\n", ACP_CMD_LCK_GET_DATA);
    sendStr(q, &crc);
    sendFooter(crc);
}