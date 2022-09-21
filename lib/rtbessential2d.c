#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <ethercat.h>

#include "rtbessential2d.h"
#include "rtbessential2d_private.h"
#include "rtblogic.h"

char IOmap[4096];

typedef struct RtbStruct {
    OSAL_THREAD_HANDLE worker;
    int worker_loop;
    char ** interfaceNames;
    char ** interfaceDesc;
    uint32_t interfacesCnt;
    ec_timet tStart;
    unsigned cnt;
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

APIFCN tRtbResult rtb_setCorrectionFactor(tRtb * h, double m1, double m2) {
    Sa5_CorrectionFactor_M1 = m1;
    Sa5_CorrectionFactor_M2 = m2;

    return RTB_OK;
}

APIFCN tRtbResult rtb_setAngles(tRtb * h, double az_deg, double el_deg) {
    Sa1_angle_az_deg_ = az_deg;
    Sa1_angle_el_deg_ = el_deg;

    return RTB_OK;
}

APIFCN tRtbResult rtb_enableTestbench(tRtb * h, boolean enable) {
    Sa1_Testbench___rol_Enable_0_1_ = enable;
    return RTB_OK;
}

APIFCN tRtbResult rtb_enableSwEnpo(tRtb * h, boolean enable) {
    Sa1_Enable_SW_ENPO_0_1_ = enable;
    return RTB_OK;
}

APIFCN tRtbResult rtb_ackError(tRtb * h) {
    Sa1_Quit_error__0_1_ = 1;
    return RTB_OK;
}

APIFCN tRtbResult rtb_setOperationMode(tRtb * h, tRtbOperationMode moo) {
    if((unsigned) moo > RTB_OM_POSITION_JOG)
        return RTB_ARG;
    Sa1_OperationModes___ = moo;

    return RTB_OK;
}

APIFCN tRtbResult rtb_enableHoming(tRtb * h, boolean enable) {
    Sa1_Start_Homing_0_1_ = enable;
    return RTB_OK;
}

APIFCN tRtbResult rtb_getSimulationTime(tRtb * h, double * t, unsigned * steps) {
    ec_timet d;
    ec_timet now = osal_current_time();
    
    osal_time_diff(&h->tStart, &now, &d);
    *t = d.sec * 1e6 + d.usec;
    *steps = h->cnt;

    return RTB_OK;
}

APIFCN tRtbResult rtb_getMotorStatus(tRtb * h, tRtbMotorStatus * m1, tRtbMotorStatus * m2) {
    m1->statusword = Sa1_Motor_1_Statusword;
    m1->modesOfOperationDisplay = Sa1_Motor_1_Mo__eration_display;
    m1->positionActualValue = Sa1_Motor_1_Po__on_actual_value;
    m1->velocityActualValue = Sa1_Motor_1_VelocityActualValue;
    
    m1->controlword             = Sa1_Motor_1_Controlword;
    m1->targetPosition          = Sa1_Motor_1_Target_Position;
    m1->motorDriveSubmodeSelect = Sa1_Motor_1_Mo___submode_select;
    m1->modesOfOperation        = Sa1_Motor_1_Modes_of_operation;

    m2->statusword = Sa1_Motor_2_Statusword;
    m2->modesOfOperationDisplay = Sa1_Motor_2_Mo__eration_display;
    m2->positionActualValue = Sa1_Motor_2_Po__on_actual_value;
    m2->velocityActualValue = Sa1_Motor_2_VelocityActualValue;

    m2->controlword             = Sa1_Motor_2_Controlword;
    m2->targetPosition          = Sa1_Motor_2_Target_Position;
    m2->motorDriveSubmodeSelect = Sa1_Motor_2_Mo___submode_select;
    m2->modesOfOperation        = Sa1_Motor_2_Modes_of_operation;
    
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

    ui32 = EOE_HTONL(0x10004160); //0x60410010;
    retval += ec_SDOwrite(slave, 6656, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui32 = EOE_HTONL(0x20006460); // 0x60640020;
    retval += ec_SDOwrite(slave, 6656, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui32 = EOE_HTONL(0x08006160); // 0x60610008;
    retval += ec_SDOwrite(slave, 6656, 3, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui32 = EOE_HTONL(0x20006c60); // 0x606c0020;
    retval += ec_SDOwrite(slave, 6656, 4, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry

    ui8 = 0x04;
    retval += ec_SDOwrite(slave, 6656, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A00 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6657, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A01 entries

    ui32 = EOE_HTONL(0x20006460); // 0x60640020;
    retval += ec_SDOwrite(slave, 6657, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A01 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 6657, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A01 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6658, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A02 entries

    ui32 = EOE_HTONL(0x10004460); // 0x60440010;
    retval += ec_SDOwrite(slave, 6658, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A02 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 6658, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A02 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 6659, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1A03 entries

    ui32 = EOE_HTONL(0x2000fd60); // 0x60fd0020;
    retval += ec_SDOwrite(slave, 6659, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry

    ui32 = EOE_HTONL(0x20012033); // 0x33200120;
    retval += ec_SDOwrite(slave, 6659, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry

    ui32 = EOE_HTONL(0x20022033); // 0x33200220;
    retval += ec_SDOwrite(slave, 6659, 3, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry

    ui8 = 0x03;
    retval += ec_SDOwrite(slave, 6659, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1A03 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5632, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1600 entries

    ui32 = EOE_HTONL(0x10004060); // 0x60400010;
    retval += ec_SDOwrite(slave, 5632, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui32 = EOE_HTONL(0x20007a60); // 0x607a0020;
    retval += ec_SDOwrite(slave, 5632, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui32 = EOE_HTONL(0x20000232); // 0x32020020;
    retval += ec_SDOwrite(slave, 5632, 3, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui32 = EOE_HTONL(0x08006060); // 0x60600008;
    retval += ec_SDOwrite(slave, 5632, 4, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1600 entry

    ui8 = 0x04;
    retval += ec_SDOwrite(slave, 5632, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1600 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5633, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1601 entries

    ui32 = EOE_HTONL(0x20007a60); // 0x607a0020;
    retval += ec_SDOwrite(slave, 5633, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1601 entry

    ui32 = EOE_HTONL(0x20008160); // 0x60810020;
    retval += ec_SDOwrite(slave, 5633, 2, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1601 entry

    ui8 = 0x02;
    retval += ec_SDOwrite(slave, 5633, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1601 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5634, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1602 entries

    ui32 = EOE_HTONL(0x10004260); // 0x60420010;
    retval += ec_SDOwrite(slave, 5634, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1602 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 5634, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1602 entry count

    ui8 = 0x00;
    retval += ec_SDOwrite(slave, 5635, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // clear pdo 0x1603 entries

    ui32 = EOE_HTONL(0x10004260); // 0x60420010;
    retval += ec_SDOwrite(slave, 5635, 1, FALSE, sizeof(ui32), &ui32, EC_TIMEOUTSAFE); // download pdo 0x1603 entry

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 5635, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1603 entry count

    ui16 = EOE_HTONS(0x0016); // 0x1600;
    retval += ec_SDOwrite(slave, 7186, 1, FALSE, sizeof(ui16), &ui16, EC_TIMEOUTSAFE); // download pdo 0x1C12:01 index

    ui8 = 0x01;
    retval += ec_SDOwrite(slave, 7186, 0, FALSE, sizeof(ui8), &ui8, EC_TIMEOUTSAFE); // download pdo 0x1C12 count

    ui16 = EOE_HTONS(0x001a); // 0x1a00;
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
     * Initialize the model
     */
    rtb_setCorrectionFactor(h, 0.881, 0.883333333333);
    rtb_setAngles(h, 0.0, 0.0);
    Sa1_Quit_error__0_1_ = 0;
    rtb_setOperationMode(h, RTB_OM_HOMING);

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
    h->tStart = osal_current_time();
    h->cnt = 0;
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
            tN5DriveIn * motor_in1 = (tN5DriveIn *) ec_slave[2].inputs;
            tN5DriveOut * motor_out1 = (tN5DriveOut *) ec_slave[2].outputs;
            tN5DriveIn * motor_in2 = (tN5DriveIn *) ec_slave[3].inputs;
            tN5DriveOut * motor_out2 = (tN5DriveOut *) ec_slave[3].outputs;

            Sa1_Motor_1_Statusword          = EOE_HTONS(motor_in1->Statusword);
            Sa1_Motor_1_Po__on_actual_value = EOE_HTONL(motor_in1->Position_actual_value);
            Sa1_Motor_1_Mo__eration_display = motor_in1->Modes_of_operation_display;
            Sa1_Motor_1_VelocityActualValue = EOE_HTONL(motor_in1->VelocityActualValue);

            Sa1_Motor_2_Statusword          = EOE_HTONS(motor_in2->Statusword);
            Sa1_Motor_2_Po__on_actual_value = EOE_HTONL(motor_in2->Position_actual_value);
            Sa1_Motor_2_Mo__eration_display = motor_in2->Modes_of_operation_display;
            Sa1_Motor_2_VelocityActualValue = EOE_HTONL(motor_in2->VelocityActualValue);

            rtblogic();
            Sa1_Quit_error__0_1_ = 0;
            h->cnt++;

            motor_out1->Controlword                = EOE_NTOHS(Sa1_Motor_1_Controlword);
            motor_out1->Target_Position            = EOE_NTOHL(Sa1_Motor_1_Target_Position);
            motor_out1->Motor_drive_submode_select = EOE_NTOHL(Sa1_Motor_1_Mo___submode_select);
            motor_out1->Modes_of_operation         = Sa1_Motor_1_Modes_of_operation;

            motor_out2->Controlword                = EOE_NTOHS(Sa1_Motor_2_Controlword);
            motor_out2->Target_Position            = EOE_NTOHL(Sa1_Motor_2_Target_Position);
            motor_out2->Motor_drive_submode_select = EOE_NTOHL(Sa1_Motor_2_Mo___submode_select);
            motor_out2->Modes_of_operation         = Sa1_Motor_2_Modes_of_operation;
        }        
    }
}
