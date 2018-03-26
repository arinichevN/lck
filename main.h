
#ifndef LCK_H
#define LCK_H

#include "lib/app.h"
#include "lib/gpio.h"
#include "lib/timef.h"
#include "lib/udp.h"
#include "lib/tsv.h"

#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/acp/lck.h"



#define APP_NAME lck
#define APP_NAME_STR TOSTRING(APP_NAME)

#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./config/"
#endif

#define KEY_FILE "" CONF_DIR "key.tsv"
#define CONFIG_FILE "" CONF_DIR "main.tsv"

typedef struct {
    int pin;
    int value;
} Lock;

DEC_LIST(Lock)

extern void serverRun(int *state) ;

extern void initApp() ;

extern void freeApp() ;

extern void exit_nicely() ;

extern void exit_nicely_e(char *s) ;

#endif 