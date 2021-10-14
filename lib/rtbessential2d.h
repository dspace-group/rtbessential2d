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
  RTB_OK,
  RTB_EC_INIT, // Cannot initialize EtherCAT on this interface
  RTB_EC_SLAVES, // Wrong amount of EtherCAT slaves found
  RTB_EC_SLAVE, // Wrong slave found
  
} tRtbResult;

APIFCN tRtb * rtb_init();
APIFCN tRtbResult rtb_term(tRtb * h);
APIFCN tRtbResult rtb_showInterfaces(tRtb * h);
APIFCN tRtbResult rtb_start(tRtb * h, char * ifname);
APIFCN tRtbResult rtb_showdetectedslaves(tRtb * h, int nSlaves);

#endif /* __RTBESSENTIAL2D_H__ */