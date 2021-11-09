#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <ethercat.h>

#include "rtbessential2d.h"
#include "rtbessential2d_private.h"

char IOmap[4096];

typedef struct RtbStruct {
  OSAL_THREAD_HANDLE worker;
  int worker_loop;
  char ** interfaceNames;
  char ** interfaceDesc;
  uint32_t interfacesCnt;

  tRtbState libstate;
} tRtb;

tRtb * _h = NULL;
OSAL_THREAD_FUNC _rtb_worker(void * arg);

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

APIFCN tRtbResult rtb_getNumberOfDetectedSlaves(tRtb * h, unsigned * n) {
    *n = ec_slavecount;
    return RTB_OK;
}

APIFCN tRtbResult rtb_getSlaveInformation(tRtb * h, int idx, char * name, unsigned * configAdr, unsigned * manId, unsigned * prodId) {
    if(idx > ec_slavecount-1) return RTB_ARG;

    strcpy(name, ec_slave[idx+1].name);
    *configAdr = ec_slave[idx+1].configadr;
    *manId = ec_slave[idx+1].eep_man;
    *prodId = ec_slave[idx+1].eep_id;

    return RTB_OK;
}

static void setup_motor(uint16 slave) {
    uint8 ui8;
    uint16 ui16;
    uint32 ui32;
    int retval = 0;

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 7186, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear sm pdos (0x1C12)

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 7187, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear sm pdos (0x1C13)

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6656, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A00 entries

    ui32 = 0x10004160;
    retval += ec_SDOwrite(slave, 6656, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui32 = 0x20006460;
    retval += ec_SDOwrite(slave, 6656, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui32 = 0x08006160;
    retval += ec_SDOwrite(slave, 6656, 3, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui32 = 0x20006c60;
    retval += ec_SDOwrite(slave, 6656, 4, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui8 = 0x04;
    retval += ec_SDOwrite(slave, 6656, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6657, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A01 entries

    ui32 = 0x20006460;
    retval += ec_SDOwrite(slave, 6657, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A01 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 6657, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A01 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6658, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A02 entries

    ui32 = 0x10004460;
    retval += ec_SDOwrite(slave, 6658, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A02 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 6658, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A02 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6659, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A03 entries

    ui32 = 0x2000fd60;
    retval += ec_SDOwrite(slave, 6659, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry

    ui32 = 0x20012033;
    retval += ec_SDOwrite(slave, 6659, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry

    ui32 = 0x20022033;
    retval += ec_SDOwrite(slave, 6659, 3, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry

    ui8 = 0x03;
    retval += ec_SDOwrite(slave, 6659, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5632, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1600 entries

    ui32 = 0x10004060;
    retval += ec_SDOwrite(slave, 5632, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui32 = 0x20007a60;
    retval += ec_SDOwrite(slave, 5632, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui32 = 0x20000232;
    retval += ec_SDOwrite(slave, 5632, 3, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui32 = 0x08006060;
    retval += ec_SDOwrite(slave, 5632, 4, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui8 = 0x04;
    retval += ec_SDOwrite(slave, 5632, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1600 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5633, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1601 entries

    ui32 = 0x20007a60;
    retval += ec_SDOwrite(slave, 5633, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1601 entry

    ui32 = 0x20008160;
    retval += ec_SDOwrite(slave, 5633, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1601 entry

    ui8 = 0x02;
    retval += ec_SDOwrite(slave, 5633, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1601 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5634, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1602 entries

    ui32 = 0x10004260;
    retval += ec_SDOwrite(slave, 5634, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1602 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 5634, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1602 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5635, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1603 entries

    ui32 = 0x10004260;
    retval += ec_SDOwrite(slave, 5635, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1603 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 5635, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1603 entry count

    ui16 = 0x0016;
    retval += ec_SDOwrite(slave, 7186, 1, FALSE, sizeof(ui16), &ui16, EC_TIMEOUTSAFE); // download pdo 0x1C12:01 index

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 7186, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1C12 count

    ui16 = 0x001a;
    retval += ec_SDOwrite(slave, 7187, 1, FALSE, sizeof(ui16), &ui16, EC_TIMEOUTSAFE); // download pdo 0x1C13:01 index

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 7187, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1C13 count

    printf("Nanotec Electronic GmbH - N5 EtherCAT Drive, slave %d set, retval = %d\n", slave, retval);
}

tRtbResult rtb_start(tRtb * h, const char * ifname) {
    int rc;
    assert(ifname != NULL);
    if(h->libstate == RTB_Started) {
        fprintf(stderr, "Error: Library is already started. Can't start again.\n");
        return RTB_STATE;
    }

    /*
     * Initialize EtherCAT
     * - init SOEM library
     * - find and auto-config
     * - verify slave by slave
     */
    rc = ec_init(ifname);
    if(rc <= 0) {
        fprintf(stderr, "Error: Cannot initialize on '%s'\n", ifname);
        return RTB_EC_INIT;
    }

    rc = ec_config_init(FALSE);
    if(rc < EC_SLAVES_Count) {
        fprintf(stderr, "Error: Cannot enumerate and init slaves (found: %d, required: %d)\n", rc, EC_SLAVES_Count);
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

    setup_motor(2);
    setup_motor(3);

    ec_config_map(&IOmap);

    /*
     * Switch to SAFE_OP
     */
    ec_slave[0].state = EC_STATE_SAFE_OP;
    ec_writestate(0);
    rc = (int) ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
    if(rc != EC_STATE_SAFE_OP) {
        fprintf(stderr, "Error: Can't switch slaves to SAFE_OP\n");
        ec_readstate();
        for(unsigned idx = 1; idx <= ec_slavecount ; idx++) {
            printf("Slave %d (%s) State=0x%2.2x StatusCode=0x%4.4x : %s\n", idx, 
                                                                            ecSlaveDescs[idx-1].name, 
                                                                            ec_slave[idx].state, 
                                                                            ec_slave[idx].ALstatuscode, 
                                                                            ec_ALstatuscode2string(ec_slave[idx].ALstatuscode));
        }
        return RTB_EC_SLAVES;
    } else {
        printf("Slaves in SAFE_OP\n");
    }

    /*
     * Worker thread
     */
    signal(SIGINT, sigfunc);
    printf("Start working loop..."); fflush(stdout);
    h->worker_loop = 1;
    osal_thread_create_rt(&h->worker, 128*1024, _rtb_worker, (void *) h);
    printf("done\n");

    /*
     * Switch to OP
     */
    ec_slave[0].state = EC_STATE_OPERATIONAL;
    ec_writestate(0);
    rc = (int) ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE * 4);
    if(rc != EC_STATE_OPERATIONAL) {
        fprintf(stderr, "Error: Can't switch slaves to EC_STATE_OP\n");
        ec_readstate();
        for(unsigned idx = 1; idx <= ec_slavecount ; idx++) {
            printf("Slave %d (%s) State=0x%2.2x StatusCode=0x%4.4x : %s\n", idx, 
                                                                            ecSlaveDescs[idx-1].name, 
                                                                            ec_slave[idx].state, 
                                                                            ec_slave[idx].ALstatuscode, 
                                                                            ec_ALstatuscode2string(ec_slave[idx].ALstatuscode));
        }
        h->worker_loop = 0; // stop worker thread
        return RTB_EC_SLAVES;
    } else {
        printf("Slaves in OP\n");
    }

    h->libstate = RTB_Started;
    return RTB_OK;
}

tRtbResult rtb_stop(tRtb * h) {
    if(h->libstate == RTB_Started) {
        /*
         * Switch slaves to INIT
         */
        ec_slave[0].state = EC_STATE_INIT;
        ec_writestate(0);
        int rc = (int) ec_statecheck(0, EC_STATE_INIT,  EC_TIMEOUTSTATE * 4);
        if(rc != EC_STATE_INIT) {
            fprintf(stderr, "Error: Can't switch slaves to INIT\n");
        }

        /*
         * Stop worker
         */
        h->worker_loop = 0;
    }

    h->libstate = RTB_Initialized;
    ec_close();

    return RTB_OK;
}

OSAL_THREAD_FUNC _rtb_worker(void * arg) {
    tRtb * h = (tRtb*) arg;

    while(h->worker_loop) {
        osal_usleep(10 * 1000);
        ec_send_processdata();
        int wkc = ec_receive_processdata(EC_TIMEOUTRET);

        /*
         * Check if slaves still available
         */
        if(h->libstate == RTB_Started && wkc != EC_SLAVES_Count*2) {
            ec_readstate();
            for (int slave = 1; slave <= EC_SLAVES_Count; slave++) {
                if (!ec_slave[slave].islost) continue;

                if(ec_slave[slave].state == EC_STATE_NONE) {
                    if (ec_recover_slave(slave, 500)) {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d recovered\n",slave);   
                    }
                } else {
                    ec_slave[slave].islost = FALSE;
                    printf("MESSAGE : slave %d found\n",slave);   
                }
            }
        }

        /*
         * Process data
         */
        if(h->libstate == RTB_Started && wkc == EC_SLAVES_Count*2) {
            tN5DriveIn * motor_in = NULL;

            motor_in = (tN5DriveIn *) ec_slave[2].inputs;
            printf("motor_1:\n"
                   "- Statusword: %d\n"
                   "- Position_actual_value: %d\n"
                   "- Modes_of_operation_display: %d\n"
                   "- VelocityActualValue: %d\n", motor_in->Statusword, motor_in->Position_actual_value, motor_in->Modes_of_operation_display, motor_in->VelocityActualValue);

            motor_in = (tN5DriveIn *) ec_slave[3].inputs;
            printf("motor_2:\n"
                   "- Statusword: %d\n"
                   "- Position_actual_value: %d\n"
                   "- Modes_of_operation_display: %d\n"
                   "- VelocityActualValue: %d\n", motor_in->Statusword, motor_in->Position_actual_value, motor_in->Modes_of_operation_display, motor_in->VelocityActualValue);
            printf("\n");
        }        
    }
}
