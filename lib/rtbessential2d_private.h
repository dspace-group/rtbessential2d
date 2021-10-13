#ifndef __RTBESSENTIAL2D_PRIVATE_H__
#define __RTBESSENTIAL2D_PRIVATE_H__

typedef struct {
  int id;
  char * name;
} tEcSlaveDesc;

typedef enum {
  EK1100,
  MOTOR1,
  MOTOR2,

  EC_SLAVES_Count,
} tRtbEssential2DEcSlaves;

tEcSlaveDesc ecSlaveDescs[] = {
    {.id = 1, .name = "Term 3 (EK1100)" },
    {.id = 2, .name = "Motor 1" },
    {.id = 3, .name = "Motor 2" }
};


#endif /* __RTBESSENTIAL2D_PRIVATE_H__ */