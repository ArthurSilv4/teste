#include <iostream>
#include <unistd.h>
#include <linux/can.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h> 

int main() {
    int socket_desc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_desc < 0) {
        std::cerr << "Erro ao abrir o socket CAN" << std::endl;
        return -1;
    }

    struct sockaddr_can addr;
    struct ifreq ifr;

    strcpy(ifr.ifr_name, "can0");
    ioctl(socket_desc, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_desc, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Erro ao associar o socket à interface CAN" << std::endl;
        return -1;
    }

    struct can_frame frame;
    frame.can_id = 0x123;  
    frame.can_dlc = 1;    
    frame.data[0] = 0;     

    while (true) {
        frame.data[0] = (frame.data[0] == 0) ? 1 : 0;

        if (write(socket_desc, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            std::cerr << "Erro ao enviar a mensagem CAN" << std::endl;
            return -1;
        }

        std::cout << "Mensagem enviada: " << (frame.data[0] ? "Ligar" : "Desligar") << std::endl;

        sleep(1);
    }

    close(socket_desc);

    return 0;
}
