#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

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
    //Serial.println(CAN.checkReceive());
    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        unsigned long id;
        unsigned char len;
        unsigned char buf[8];

        //CAN.readMsgBuf(&id, &len, buf);
        byte ret = CAN.readMsgBuf(&id, &len, buf);

        Serial.print("ret=");
        Serial.println(ret);

        Serial.print("id=");
        Serial.println(id);

        Serial.print("len=");
        Serial.println(len);
        Serial.print("ID = 0x");
        Serial.println(id, HEX);

        Serial.print("Message = ");

        for (int i = 0; i < len; i++)
        {
            Serial.print(buf[i], HEX);
            Serial.print(" ");
        }

        Serial.println();
        Serial.println("----------------");
    }
}

