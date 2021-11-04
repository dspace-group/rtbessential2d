#ifndef __RTBESSENTIAL2D_PRIVATE_H__
#define __RTBESSENTIAL2D_PRIVATE_H__

typedef struct {
  int id;
  char * name;
  uint32_t manId;
  uint32_t prodId;
} tEcSlaveDesc;

typedef enum {
  EK1100,
  MOTOR1,
  MOTOR2,

  EC_SLAVES_Count,
} tRtbEssential2DEcSlaves;

tEcSlaveDesc ecSlaveDescs[] = {
    {.id = 1, .name = "EK1100", .manId = 0x00000002, .prodId = 0x044c2c52 },
    {.id = 2, .name = "N5-1-1", .manId = 0x0000026c, .prodId = 0x00000007 },
    {.id = 3, .name = "N5-1-1", .manId = 0x0000026c, .prodId = 0x00000007 }
};


#endif /* __RTBESSENTIAL2D_PRIVATE_H__ */