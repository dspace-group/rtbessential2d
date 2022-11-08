#ifndef __RTBESSENTIAL2D_H__
#define __RTBESSENTIAL2D_H__

#ifdef _MSC_VER
  #ifdef LIBEXPORT_EXPORTS 
    #define APIFCN __declspec(dllexport)
  #else
    #define APIFCN __declspec(dllimport)
  #endif
#else
  #define APIFCN
#endif

struct RtbStruct;
typedef struct RtbStruct tRtb;

typedef enum {
    RTB_Initialized,
    RTB_Starting,
    RTB_Started,
} tRtbState;

typedef enum {
  RTB_OK,
  RTB_STATE, // Library is in wrong state for this command
  RTB_ARG, // Wrong argument
  RTB_EC_INIT, // Cannot initialize EtherCAT on this interface
  RTB_EC_SLAVES, // Wrong amount of EtherCAT slaves found
  RTB_EC_SLAVE, // Wrong slave found
} tRtbResult;

typedef enum {
    RTB_OM_HOMING = 6,
    RTB_OM_POSITION_CONTROL = 1,
    RTB_OM_POSITION_JOG = 3,
} tRtbOperationMode;

typedef struct {
    uint16_t controlword;
    int32_t  targetPosition;
    uint32_t motorDriveSubmodeSelect;
    int8_t  modesOfOperation;
    uint16_t statusword;
    int8_t modesOfOperationDisplay;
    int32_t positionActualValue;
    int32_t velocityActualValue;
} tRtbMotorStatus;

APIFCN tRtb * rtb_init();
APIFCN tRtbResult rtb_term(tRtb * h);
APIFCN tRtbResult rtb_getState(tRtb * h, tRtbState * state);
APIFCN tRtbResult rtb_showInterfaces(tRtb * h);
APIFCN tRtbResult rtb_getNumberOfInterfaces(tRtb * h, uint32_t * n);
APIFCN tRtbResult rtb_getInterface(tRtb * h, uint32_t idx, char * name, char * desc);
APIFCN tRtbResult rtb_start(tRtb * h, const char * ifname);
APIFCN tRtbResult rtb_stop(tRtb * h);
APIFCN tRtbResult rtb_getNumberOfDetectedSlaves(tRtb * h, unsigned * n);
APIFCN tRtbResult rtb_getSlaveInformation(tRtb * h, int idx, char * name, unsigned * configAdr, unsigned * manId, unsigned * prodId);
APIFCN tRtbResult rtb_setCorrectionFactor(tRtb * h, double cf_m1, double cf_m2, double offset_m1, double offset_m2);
APIFCN tRtbResult rtb_setAngles(tRtb * h, double az_deg, double el_deg);
APIFCN tRtbResult rtb_enableTestbench(tRtb * h, boolean enable);
APIFCN tRtbResult rtb_enableSwEnpo(tRtb * h, boolean enable);
APIFCN tRtbResult rtb_ackError(tRtb * h);
APIFCN tRtbResult rtb_setOperationMode(tRtb * h, tRtbOperationMode moo);
APIFCN tRtbResult rtb_enableHoming(tRtb * h, boolean enable);
APIFCN tRtbResult rtb_getSimulationTime(tRtb * h, double * t, unsigned * steps);
APIFCN tRtbResult rtb_getMotorStatus(tRtb * h, tRtbMotorStatus * m1, tRtbMotorStatus * m2);
#endif /* __RTBESSENTIAL2D_H__ */