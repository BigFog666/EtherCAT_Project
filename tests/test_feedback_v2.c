/* 正式反馈检查：检查固定字节布局、模型故障锁存与复位反馈。 */
#include "joint.h"
#include "pdo.h"
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int same_feedback(const JointFeedback *a, const JointFeedback *b)
{
    /* 不比较结构体填充；只比较接口字段。 */
    return a->statusword == b->statusword && a->position == b->position &&
           a->velocity == b->velocity && a->mode == b->mode &&
           a->error_code == b->error_code;
}

static int model_round(const char *stage, Joint *model, const JointCommand *command,
                       uint32_t now, int receive)
{
    uint8_t packet[PROJECT_TXPDO_BYTES];
    JointFeedback decoded = {0};
    if (receive) joint_receive(model, command, now);
    joint_update(model, now, 1);
    project_encode_feedback(packet, &model->feedback);
    CHECK(project_decode_feedback(packet, sizeof(packet), &decoded));
    CHECK(model->feedback.error_code == model->error_code);
    CHECK(same_feedback(&decoded, &model->feedback));
    printf("%-8s model=%04X snapshot=%04X decoded=%04X tail=%02X %02X sw=%04X\n",
           stage, (unsigned)model->error_code, (unsigned)model->feedback.error_code,
           (unsigned)decoded.error_code, (unsigned)packet[12], (unsigned)packet[13],
           (unsigned)decoded.statusword);
    return 0;
}

int main(void)
{
    /* 独立固定字节，防止发送和接收同时写错却相互抵消。 */
    static const uint8_t golden[14] = {
        0x08, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x04, 0xFF
    };
    static const uint16_t errors[] = {0, 0xFF01, 0xFF04, 0x8611};
    JointFeedback source = {0}, decoded = {0}, saved;
    JointCommand command = {0x0006, 0, 1000, 9}, command_decoded;
    Joint model;
    uint8_t storage[PROJECT_TXPDO_BYTES + 2u], command_bytes[PROJECT_RXPDO_BYTES];
    uint8_t *packet = storage + 1;
    uint32_t now = 0;
    int32_t stopped_position;
    size_t i;

    CHECK(PROJECT_RXPDO_BYTES == 12u && PROJECT_TXPDO_BYTES == 14u);
    puts("Formal feedback validation: common C sources; PC only, no board access");
    source.statusword = 0x0008;
    source.position = 1000;
    source.mode = 8;
    source.error_code = 0xFF04;
    memset(storage, 0xA5, sizeof(storage));
    project_encode_feedback(packet, &source);
    CHECK(memcmp(packet, golden, sizeof(golden)) == 0);
    CHECK(storage[0] == 0xA5 && storage[sizeof(storage) - 1] == 0xA5);
    CHECK(project_decode_feedback(golden, sizeof(golden), &decoded));
    CHECK(same_feedback(&source, &decoded));
    printf("golden: error=%04X tail=%02X %02X; bounds=OK\n",
           (unsigned)decoded.error_code, (unsigned)packet[12], (unsigned)packet[13]);

    for (i = 0; i < sizeof(errors) / sizeof(errors[0]); i++) {
        source.error_code = errors[i];
        source.position = -1000;
        source.velocity = -500;
        source.mode = -128;
        project_encode_feedback(packet, &source);
        CHECK(packet[12] == (uint8_t)errors[i] && packet[13] == (uint8_t)(errors[i] >> 8));
        CHECK(project_decode_feedback(packet, PROJECT_TXPDO_BYTES, &decoded));
        CHECK(same_feedback(&source, &decoded));
    }
    saved = decoded;
    CHECK(!project_decode_feedback(packet, 12u, &decoded));
    CHECK(!project_decode_feedback(packet, 13u, &decoded));
    CHECK(!project_decode_feedback(packet, 15u, &decoded));
    CHECK(same_feedback(&decoded, &saved));
    CHECK(!project_decode_feedback(NULL, PROJECT_TXPDO_BYTES, &decoded));
    CHECK(!project_decode_feedback(packet, PROJECT_TXPDO_BYTES, NULL));
    project_encode_command(command_bytes, &command);
    CHECK(project_decode_command(command_bytes, sizeof(command_bytes), &command_decoded));
    CHECK(command_decoded.controlword == command.controlword &&
          command_decoded.target_position == command.target_position &&
          command_decoded.target_velocity == command.target_velocity &&
          command_decoded.mode == command.mode);
    /* 使用确有14字节的缓冲区测试拒绝，避免测试自己越界。 */
    CHECK(!project_decode_command(golden, sizeof(golden), &command_decoded));

    joint_init(&model, now);
    CHECK(model.error_code == 0 && model.feedback.error_code == 0);
    CHECK(model_round("ready", &model, &command, now += 10000u, 1) == 0);
    command.controlword = 0x0007;
    CHECK(model_round("switched", &model, &command, now += 10000u, 1) == 0);
    command.controlword = 0x000F;
    CHECK(model_round("moving", &model, &command, now += 10000u, 1) == 0);
    CHECK(model.state == JD_ENABLED && model.feedback.velocity == 1000);
    stopped_position = model.feedback.position;
    CHECK(model_round("timeout", &model, &command, now += JOINT_TIMEOUT_US, 0) == 0);
    CHECK(model.state == JD_FAULT_REACTION && model.error_code == 0xFF04);
    CHECK(model.feedback.velocity == 0 && model.feedback.position == stopped_position);
    CHECK(model_round("fault", &model, &command, now += 10000u, 0) == 0);
    CHECK(model.state == JD_FAULT && model.timeout_count == 1);
    CHECK(model_round("resumed", &model, &command, now += 10000u, 1) == 0);
    CHECK(model.error_code == 0xFF04 && model.state == JD_FAULT);
    command.controlword = 0x0080;
    CHECK(model_round("reset", &model, &command, now += 10000u, 1) == 0);
    CHECK(model.error_code == 0 && model.state == JD_DISABLED);

    command.controlword = 0x800F;
    CHECK(model_round("inject", &model, &command, now += 10000u, 1) == 0);
    CHECK(model.error_code == 0xFF01);
    command.controlword = 0;
    CHECK(model_round("latched", &model, &command, now += 10000u, 1) == 0);
    command.controlword = 0x0080;
    CHECK(model_round("clear", &model, &command, now += 10000u, 1) == 0);
    CHECK(model.error_code == 0 && model.feedback.error_code == 0);

    /* 越界原因在运动分支后半段产生，必须也到达反馈，而非只同步超时。 */
    joint_init(&model, 0);
    now = 0;
    command.mode = 9;
    command.target_velocity = 1000;
    model.position_micro = (int64_t)JOINT_LIMIT * 1000000;
    command.controlword = 6;
    CHECK(model_round("limit-r", &model, &command, now += 10000u, 1) == 0);
    command.controlword = 7;
    CHECK(model_round("limit-s", &model, &command, now += 10000u, 1) == 0);
    command.controlword = 15;
    CHECK(model_round("limit", &model, &command, now += 10000u, 1) == 0);
    CHECK(model.error_code == 0x8611 && model.feedback.error_code == 0x8611);
    printf("PASS: %u checks; formal C core command=12 feedback=14; PC only\n", checks);
    return 0;
}
