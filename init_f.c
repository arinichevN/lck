#include "main.h"

int readSettings(int *sock_port, const char *data_path) {
    TSVresult tsv = TSVRESULT_INITIALIZER;
    TSVresult* r = &tsv;
    if (!TSVinit(r, data_path)) {
        TSVclear(r);
        return 0;
    }
    int _port = TSVgetis(r, 0, "port");
    if (TSVnullreturned(r)) {
        TSVclear(r);
        return 0;
    }
    *sock_port = _port;
    TSVclear(r);
    return 1;
}

int initLock(LockList *list, const char *data_path) {
    TSVresult tsv = TSVRESULT_INITIALIZER;
    TSVresult* r = &tsv;
    if (!TSVinit(r, data_path)) {
        TSVclear(r);
        return 0;
    }
    int n = TSVntuples(r);
    if (n <= 0) {
        TSVclear(r);
        return 1;
    }
    RESIZE_M_LIST(list, n);

    if (LML != n) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failure while resizing list\n", F);
#endif
        TSVclear(r);
        return 0;
    }
    NULL_LIST(list);
    for (int i = 0; i < LML; i++) {
        LIi.pin = TSVgetis(r, i, "pin");
        LIi.value = TSVgetis(r, i, "value");
        if (TSVnullreturned(r)) {
            break;
        }
        LL++;
    }
    TSVclear(r);
    if (LL != LML) {
#ifdef MODE_DEBUG

        fprintf(stderr, "%s(): failure while reading rows\n", F);
#endif
        return 0;
    }
    return 1;
}
