#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);


#pragma pack(push,1)

struct MotorCommand
{
    int16_t left_rpm;
    int16_t right_rpm;
};

struct MotorFeedback
{
    int16_t rpm;
    int16_t current_x10;
    int16_t duty_x1000;
    uint8_t status;
};

#pragma pack(pop)


MotorCommand cmd;
int16_t target_rpm = 0;
unsigned long last_send = 0;


void setup()
{
    Serial.begin(115200);
    delay(2000);

    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    {
        Serial.println("CAN init OK");
    }
    else
    {
        Serial.println("CAN init FAILED");
        while (1);
    }

    CAN.setMode(MCP_NORMAL);
}

void loop()
{
    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        unsigned long id;
        unsigned char len;
        unsigned char buf[8];

        CAN.readMsgBuf(&id, &len, buf);
        if (id == 0x100)
        {
            memcpy(&cmd, buf, sizeof(cmd));

            target_rpm = cmd.left_rpm;

            Serial.print("Target RPM = ");
            Serial.println(target_rpm);
        }
        if (len == sizeof(MotorCommand))
        {
            MotorCommand cmd;

            memcpy(&cmd, buf, sizeof(cmd));

            Serial.print("CAN ID: 0x");
            Serial.println(id, HEX);

            Serial.print("Left RPM  = ");
            Serial.println(cmd.left_rpm);

            Serial.print("Right RPM = ");
            Serial.println(cmd.right_rpm);

            Serial.println("----------------");
        }
    }

    if (millis() - last_send >= 100)
    {
        MotorFeedback fb;

        fb.rpm = target_rpm;
        fb.current_x10 = 253;
        fb.duty_x1000 = 876;
        fb.status = 1;

        CAN.sendMsgBuf(
            0x101,
            0,
            sizeof(fb),
            (byte*)&fb);

        last_send = millis();
    }
}

