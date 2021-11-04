#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <ethercat.h>

#include "rtbessential2d.h"
#include "rtbessential2d_private.h"

char IOmap[4096];

static void *_rtb_worker(void * arg);

typedef struct RtbStruct {
  pthread_t worker;
  int worker_loop;
  char ** interfaceNames;
  char ** interfaceDesc;
  uint32_t interfacesCnt;

  tRtbState libstate;
} tRtb;

tRtb * _h = NULL;

void sigfunc(int sig) {
   if(sig == SIGINT) {
       _h->worker_loop = 0;
   }
}

tRtb * rtb_init() {
    tRtb * h = malloc(sizeof(tRtb));
    memset(h, 0, sizeof(tRtb));
    _h = h;

    /*
     * Obtain interfaces
     */
    h->interfacesCnt = 0;
    ec_adaptert * adapter = ec_find_adapters();
    while (adapter != NULL) {
        h->interfacesCnt++;
        adapter = adapter->next;
    }
    h->interfaceNames = (char **) malloc(h->interfacesCnt * sizeof(char *));
    h->interfaceDesc = (char **) malloc(h->interfacesCnt * sizeof(char *));
    adapter = ec_find_adapters();
    for(unsigned idx = 0; idx < h->interfacesCnt; idx++) {
        h->interfaceNames[idx] = strdup(adapter->name);
        h->interfaceDesc[idx] = strdup(adapter->desc);
        adapter = adapter->next;
    }

    h->libstate = RTB_Initialized;
    return h;
}

tRtbResult rtb_term(tRtb * h) {
    pthread_join(h->worker, NULL);

    for(unsigned idx = 0; idx < h->interfacesCnt; idx++) {
        free(h->interfaceNames[idx]);
        free(h->interfaceDesc[idx]);
    }
    free(h->interfaceNames);
    free(h);
    return RTB_OK;
}

tRtbResult rtb_getState(tRtb * h, tRtbState * state) {
    *state = h->libstate;
    return RTB_OK;
}

tRtbResult rtb_showInterfaces(tRtb * h) {
    for(unsigned idx = 0; idx < h->interfacesCnt; idx++)
        printf("%s;%s\n", h->interfaceNames[idx], h->interfaceDesc[idx]);
    return RTB_OK;
}

tRtbResult rtb_getNumberOfInterfaces(tRtb * h, uint32_t * n) {
    *n = h->interfacesCnt;
    return RTB_OK;
}
tRtbResult rtb_getInterface(tRtb * h, uint32_t idx, char * name, char * desc) {
    strcpy(name, h->interfaceNames[idx]);
    strcpy(desc, h->interfaceDesc[idx]);
    return RTB_OK;
}

APIFCN tRtbResult rtb_showdetectedslaves(tRtb * h, char * out) {
    if(out == NULL) return RTB_OK;

    sprintf(out, "| List of detected slaves (%d)\n"
                 "| Id | ConfigAddr | Name                 | Man id     | Product Id |\n"
                 "|----|------------|----------------------|------------|------------|\n", ec_slavecount);
    size_t n = strlen(out);

    for(int idx = 0; idx < ec_slavecount; idx++) {
        char * name = ec_slave[idx+1].name;
        uint16 configadr = ec_slave[idx+1].configadr;
        uint32 manId = ec_slave[idx+1].eep_man;
        uint32 prodId = ec_slave[idx+1].eep_id;

        sprintf(out+n, "| #%1.d | 0x%04x     | %-20.20s | 0x%08x | 0x%08x |\n", idx+1, configadr, name, manId, prodId);
    }

    return RTB_OK;
}

tRtbResult rtb_start(tRtb * h, char * ifname) {
    int rc;
    assert(ifname != NULL);
    h->libstate = RTB_Starting;

    char * buf = (char *) malloc(sizeof(char) * 2*1024*1024);

    /*
     * Initialize EtherCAT
     * - init SOEM library
     * - find and auto-config
     * - verify slave by slave
     */
    if(h->libstate == RTB_Initialized) {
        rc = ec_init(ifname);
        if(rc <= 0) {
            fprintf(stderr, "Error: Cannot initialize on '%s'\n", ifname);
            return RTB_EC_INIT;
        }
    }

    rc = ec_config_init(FALSE);
    if(rc < EC_SLAVES_Count) {
        fprintf(stderr, "Error: Cannot enumerate and init slaves\n");
        return RTB_EC_SLAVES;
    }
    
    for(int idx = 0; idx < EC_SLAVES_Count; idx++) {
        int id = ecSlaveDescs[idx].id;
        char * name = ecSlaveDescs[idx].name;
        uint32_t manId = ecSlaveDescs[idx].manId;
        uint32_t prodId = ecSlaveDescs[idx].prodId;

        if(manId != ec_slave[id].eep_man) {
            fprintf(stderr, "Error: Slave missmatch on slave #%d. Expected ManId '0x%08x', found '0x%08x'\n", id, manId, ec_slave[id].eep_man);
            return RTB_EC_SLAVE;
        }
        if(prodId != ec_slave[id].eep_id) {
            fprintf(stderr, "Error: Slave missmatch on slave #%d. Expected ProdId '0x%08x', found '0x%08x'\n", id, prodId, ec_slave[id].eep_id);
            return RTB_EC_SLAVE;
        }
    }
    rtb_showdetectedslaves(h, buf);
    printf(buf);
    free(buf);

    ec_config_map(&IOmap);
    printf("Slaves mapped\n");

    rc = (int) ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
    if(rc != EC_STATE_SAFE_OP) {
        fprintf(stderr, "Error: Can't set slaves in SAFEOF\n");
    }
    printf("Slaves in SAFEOF state\n");

    signal(SIGINT, sigfunc);

    printf("Start working loop..."); fflush(stdout);
    h->worker_loop = 1;
    rc = pthread_create(&h->worker, NULL, _rtb_worker, (void *) h);
    if(rc != 0) {
        fprintf(stderr, "Error: Was not possible to create worker thread (%s)\n", strerror(errno));
    }
    printf("done\n");

    h->libstate = RTB_Started;
    return RTB_OK;
}

tRtbResult rtb_stop(tRtb * h) {
    h->libstate = RTB_Initialized;
    ec_close();
}

static void *_rtb_worker(void * arg) {
    tRtb * h = (tRtb*) arg;

    while(h->worker_loop) {
        printf("."); fflush(stdout);
        sleep(1);
    }
    return arg;
}
