#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>

struct MotorCommand
{
    int16_t left_rpm;
    int16_t right_rpm;
};

int main()
{
    // Create SocketCAN socket
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (s < 0)
    {
        perror("socket");
        return 1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");

    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl");
        return 1;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    // Build our C++ struct
    MotorCommand cmd;

    cmd.left_rpm = 1200;
    cmd.right_rpm = 1180;

    // Create CAN frame
    struct can_frame frame;

    frame.can_id = 0x100;
    frame.can_dlc = sizeof(cmd);

    memcpy(frame.data, &cmd, sizeof(cmd));

    // Send
    int nbytes = write(s, &frame, sizeof(frame));

    if (nbytes != sizeof(frame))
    {
        perror("write");
        return 1;
    }

    std::cout << "MotorCommand sent!" << std::endl;
    std::cout << "Left  = " << cmd.left_rpm << std::endl;
    std::cout << "Right = " << cmd.right_rpm << std::endl;

    close(s);

    return 0;
}
