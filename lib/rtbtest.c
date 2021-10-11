#include "rtbtest.h"
#include <ethercat.h>

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
    printf("-    %s    (description: %s)\n", adapter->name, adapter->desc);
    adapter = adapter->next;
  }
  return RTB_OK;
}