#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "cg/geometry.h"

enum { JOINT_COUNT = 4, PRIZE_COUNT = 3, MAX_SOLIDS = 24, ARM_PART_COUNT = 9 };
typedef struct {
    const char *name;
    float min, max, initial, speed;
} JointConfig;
typedef struct {
    JointConfig joints[JOINT_COUNT];
    float shoulder_height, upper_length, fore_length, grip_offset;
    float finger_min, finger_max, finger_speed;
    float prize_size, capture_x, capture_y, capture_z;
    float max_step, collision_tolerance, gravity;
    float prize_yaw[PRIZE_COUNT], drop_yaw[PRIZE_COUNT];
    Color prize_colors[PRIZE_COUNT];
    Vec3 tray_center, tray_size;
    AABB workspace; /* Interior safety envelope, including the open sides. */
} MachineConfig;
extern const MachineConfig MACHINE_CONFIG;
/* Known FK pose used to place the prizes and verify the documented tutorial.
 * No inverse kinematics is used at runtime. */
extern const float PICKUP_POSE[JOINT_COUNT];

#endif
