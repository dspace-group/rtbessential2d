#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <ethercat.h>

#include "rtbessential2d.h"
#include "rtbessential2d_private.h"

typedef struct RtbStruct {
  int a;
} tRtb;
 
tRtb * rtb_init() {
  tRtb * h = malloc(sizeof(tRtb));
  memset(h, 0, sizeof(tRtb));

  return h;
}
tRtbResult rtb_term(tRtb * h) {
  free(h);
  return RTB_OK;
}
tRtbResult rtb_showInterfaces(tRtb * h) {
  ec_adaptert * adapter = NULL;
  adapter = ec_find_adapters();

  while (adapter != NULL) {
    printf("%s;%s\n", adapter->name, adapter->desc);
    adapter = adapter->next;
  }
  return RTB_OK;
}

tRtbResult rtb_start(tRtb * h, char * ifname) {
    int rc;
    assert(ifname != NULL);

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
        rtb_showdetectedslaves(h, rc);

        ec_close();
        fprintf(stderr, "Error: Cannot enumerate and init slaves\n");
        return RTB_EC_INIT;
    }
    
    for(int idx = 0; idx < EC_SLAVES_Count; idx++) {
        int id = ecSlaveDescs[idx].id;
        char * name = ecSlaveDescs[idx].name;

        if(strcmp(name, ec_slave[id].name) != 0) {
            rtb_showdetectedslaves(h, rc);

            ec_close();
            fprintf(stderr, "Error: Slave missmatch. Expected '%s', found '%s'\n", name, ec_slave[id].name);
            return RTB_EC_SLAVE;
        }
    }
    return RTB_OK;
}
APIFCN tRtbResult rtb_showdetectedslaves(tRtb * h, int nSlaves) {
    fprintf(stdout, "---\n");
    fprintf(stdout, "| List of detected slaves: \n");
    for(int idx = 0; idx < nSlaves; idx++) {
        int id = ecSlaveDescs[idx].id;
        char * name = ecSlaveDescs[idx].name;
        fprintf(stdout, "| %d. %d '%s'\n", idx, id, name);
    }
    fprintf(stdout, "---\n");

    return RTB_OK;
}