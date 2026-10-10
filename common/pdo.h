#ifndef PROJECT_PDO_H
#define PROJECT_PDO_H
#include <stddef.h>
#include "joint.h"
#define PROJECT_RXPDO_BYTES 12u
#define PROJECT_TXPDO_BYTES 14u
#define PROJECT_PDO_BYTES PROJECT_RXPDO_BYTES

int project_decode_command(const uint8_t *data, size_t size, JointCommand *command);
int project_decode_feedback(const uint8_t *data, size_t size, JointFeedback *feedback);
void project_encode_command(uint8_t *data, const JointCommand *command);
void project_encode_feedback(uint8_t *data, const JointFeedback *feedback);
#endif
