

int readSettings() {
    FILE* stream = fopen(CONFIG_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s()", F); perror("");
#endif
        return 0;
    }
    skipLine(stream);
    int n;
    n = fscanf(stream, "%d\n", &sock_port);
    if (n != 1) {
        fclose(stream);
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): bad row format\n", F);
#endif
        return 0;
    }
    fclose(stream);
#ifdef MODE_DEBUG
    printf("%s(): \n\tsock_port: %d\n",F, sock_port);
#endif
    return 1;
}

#define KEY_ROW_FORMAT "%d\t%d\n"
#define KEY_FIELD_COUNT 2

int initLock(LockList *list) {
    FILE* stream = fopen(KEY_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s()", F); perror("");
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
        printf("%s(): count: pin = %d, value = %d\n",F, x1, x2);
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
            fprintf(stderr,"%s(): failed to allocate memory\n", F);
#endif
            fclose(stream);
            return 0;
        }
        skipLine(stream);
        int done = 1;
        FORLIST(i){
            int n;
            n = fscanf(stream, KEY_ROW_FORMAT, &LIi.pin, &LIi.value);
            if (n != KEY_FIELD_COUNT) {
                done = 0;
            }
#ifdef MODE_DEBUG
            printf("%s(): read: pin = %d, value = %d\n",F, LIi.pin, LIi.value);
#endif
        }
        if (!done) {
            FREE_LIST(list);
            fclose(stream);
#ifdef MODE_DEBUG
            fprintf(stderr,"%s(): failure while reading rows\n", F);
#endif
            return 0;
        }
    }
    fclose(stream);
    return 1;
}
