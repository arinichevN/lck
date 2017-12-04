

int readSettings() {
    FILE* stream = fopen(CONFIG_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        perror("readSettings()");
#endif
        return 0;
    }
    skipLine(stream);
    int n;
    n = fscanf(stream, "%d\t%255s\n", &sock_port, pid_path);
    if (n != 2) {
        fclose(stream);
#ifdef MODE_DEBUG
        fputs("ERROR: readSettings: bad row format\n", stderr);
#endif
        return 0;
    }
    fclose(stream);
#ifdef MODE_DEBUG
    printf("readSettings: \n\tsock_port: %d, \n\tpid_path: %s\n", sock_port, pid_path);
#endif
    return 1;
}

#define KEY_ROW_FORMAT "%d\t%d\n"
#define KEY_FIELD_COUNT 2

int initLock(LockList *list) {
    FILE* stream = fopen(KEY_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fputs("ERROR: initDevice: fopen", stderr);
#endif
        return 0;
    }
    skipLine(stream);
    int rnum = 0;
    while (1) {
        int n = 0, x1, x2;
        n = fscanf(stream, KEY_ROW_FORMAT, &x1, &x2);
        if (n != KEY_FIELD_COUNT) {
            break;
        }
#ifdef MODE_DEBUG
        printf("initLock: count: pin = %d, value = %d\n", x1, x2);
#endif
        rnum++;

    }
    rewind(stream);

    list->length = rnum;
    if (list->length > 0) {
        list->item = (Lock *) malloc(list->length * sizeof *(list->item));
        if (list->item == NULL) {
            list->length = 0;
#ifdef MODE_DEBUG
            fputs("ERROR: initLock: failed to allocate memory for pins\n", stderr);
#endif
            fclose(stream);
            return 0;
        }
        skipLine(stream);
        int done = 1;
        size_t i;
        FORL{
            int n;
            n = fscanf(stream, KEY_ROW_FORMAT, &LIi.pin, &LIi.value);
            if (n != KEY_FIELD_COUNT) {
                done = 0;
            }
#ifdef MODE_DEBUG
            printf("initLock: read: pin = %d, value = %d\n", LIi.pin, LIi.value);
#endif
        }
        if (!done) {
            FREE_LIST(list);
            fclose(stream);
#ifdef MODE_DEBUG
            fputs("ERROR: initLock: failure while reading rows\n", stderr);
#endif
            return 0;
        }
    }
    fclose(stream);
    return 1;
}
