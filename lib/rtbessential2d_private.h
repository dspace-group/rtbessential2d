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

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct PACKED {
    uint16_t Statusword;
    int32_t Position_actual_value;
    int8_t Modes_of_operation_display;
    int32_t VelocityActualValue;
} tN5DriveIn;

typedef struct PACKED {
    uint16_t Controlword;
    int32_t Target_Position;
    uint32_t Motor_drive_submode_select;
    int8_t Modes_of_operation;
} tN5DriveOut;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif /* __RTBESSENTIAL2D_PRIVATE_H__ */