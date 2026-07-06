#include <SPI.h>
#include <mcp_can.h>

// ---------- ขา SPI สำหรับ MCP2515 ----------
const int SPI_CS_PIN = 10;   // เปลี่ยนตามการต่อจริง (GPIO10 = SS)
MCP_CAN CAN(SPI_CS_PIN);

// ---------- โครงสร้างข้อมูล ----------
#pragma pack(push, 1)
struct MotorCommand {
    int16_t left_rpm;   // 2 bytes
    int16_t right_rpm;  // 2 bytes
};  // รวม 4 bytes

struct MotorFeedback {
    int16_t rpm;          // 2 bytes
    int16_t current_x10;  // 2 bytes (ค่าจริง ×10)
    int16_t duty_x1000;   // 2 bytes (ค่าจริง ×1000)
    uint8_t status;       // 1 byte
};  // รวม 7 bytes
#pragma pack(pop)

// ---------- ตัวแปรสำหรับการทำงาน ----------
MotorCommand cmd;
int16_t target_rpm = 0;
unsigned long last_send = 0;

void setup() {
    // เริ่ม Serial Monitor
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n===== Arduino Nano ESP32 CAN Receiver =====");

    // เริ่มต้น CAN Bus ที่ 500kbps ด้วยคริสตัล 8MHz
    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        Serial.println("CAN init OK");
    } else {
        Serial.println("CAN init FAILED!");
        while (1);  // หยุดทำงานถ้าเริ่มต้นไม่ได้
    }

    // ตั้งค่า MCP2515 ให้ทำงานในโหมด Normal
    CAN.setMode(MCP_NORMAL);
    Serial.println("CAN is ready, waiting for messages...\n");
}

void loop() {
    // ---------- 1. รับข้อมูลจาก CAN Bus ----------
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        unsigned long id;
        unsigned char len;
        unsigned char buf[8];

        // อ่านข้อมูลจาก CAN buffer
        CAN.readMsgBuf(&id, &len, buf);

        // ตรวจสอบว่าเป็น ID 0x100 หรือไม่
        if (id == 0x100) {
            // คัดลอกข้อมูล 4 bytes ลงใน struct MotorCommand
            memcpy(&cmd, buf, sizeof(cmd));

            // อัปเดต target_rpm จาก left_rpm
            target_rpm = cmd.left_rpm;

            // แสดงผลทาง Serial Monitor
            Serial.println("===== CAN Message Received =====");
            Serial.print("CAN ID: 0x");
            Serial.println(id, HEX);
            Serial.print("Left RPM  = ");
            Serial.println(cmd.left_rpm);
            Serial.print("Right RPM = ");
            Serial.println(cmd.right_rpm);
            Serial.println("================================");
        }
    }

    // ---------- 2. ส่งข้อมูล Feedback ทุก 100ms ----------
    if (millis() - last_send >= 100) {
        MotorFeedback fb;

        // จำลองข้อมูลสถานะ
        fb.rpm          = target_rpm;        // RPM ปัจจุบัน (จากคำสั่งล่าสุด)
        fb.current_x10  = 253;               // 25.3A (ค่าจริง ×10)
        fb.duty_x1000   = 876;               // 87.6% (ค่าจริง ×1000)
        fb.status       = 1;                 // 1 = OK

        // ส่งข้อมูลผ่าน CAN Bus ที่ ID 0x101
        CAN.sendMsgBuf(
            0x101,          // CAN ID
            0,              // ไม่ใช้ extended frame
            sizeof(fb),     // ขนาดข้อมูล (7 bytes)
            (byte*)&fb      // pointer ไปยังข้อมูล
        );

        last_send = millis();
    }
}