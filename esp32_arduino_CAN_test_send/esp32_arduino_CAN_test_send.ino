#include <SPI.h>
#include <mcp_can.h>

// MCP2515 CS pin
const int SPI_CS_PIN = 10;

// CAN object
MCP_CAN CAN(SPI_CS_PIN);



void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ))
    {
        Serial.println("CAN init failed, retrying...");
        delay(1000);
    }

    Serial.println("CAN init OK");

    // Normal mode
    CAN.setMode(MCP_NORMAL);
}

void loop()
{
    byte data[8];

    data[0] = 0x11;
    data[1] = 0x22;
    data[2] = 0x33;
    data[3] = 0x44;
    data[4] = 0x55;
    data[5] = 0x66;
    data[6] = 0x77;
    data[7] = 0x88;

    byte result = CAN.sendMsgBuf(0x123, 0, 8, data);

    if (result == CAN_OK)
    {
        Serial.println("Message Sent");
    }
    else
    {
        Serial.println("Send Error");
    }

    delay(1000);
}

