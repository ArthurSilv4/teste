#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <cstring>
#include <chrono>
#include <thread>

#define PARPORT_DEVICE "/dev/parport0"

// Mapeia os pinos da porta paralela para CAN_H e CAN_L
#define CAN_H 0x01  // Bit 0 da porta paralela (Pino 2)
#define CAN_L 0x02  // Bit 1 da porta paralela (Pino 3)

// Define tempos do barramento (Ajuste conforme necessário)
#define BIT_TIME_US 8  // Tempo de um bit a 125 kbps (~8 µs)

int parport_fd;

// Simula um bit no barramento CAN
void sendCANBit(int bit) {
    int signal = (bit == 0) ? CAN_H : CAN_L;  // CAN_H = Dominante (0), CAN_L = Recessivo (1)
    ioctl(parport_fd, PPWDATA, &signal);
    std::this_thread::sleep_for(std::chrono::microseconds(BIT_TIME_US));
}

// Envia um quadro CAN básico
void sendCANFrame(int id, int data) {
    // SOF (Start of Frame) - Sempre 0
    sendCANBit(0);

    // Envia o ID (11 bits)
    for (int i = 10; i >= 0; i--) {
        sendCANBit((id >> i) & 1);
    }

    // RTR (Remote Transmission Request) - Sempre 0 para mensagens normais
    sendCANBit(0);

    // IDE (Identifier Extension) - 0 para CAN padrão
    sendCANBit(0);

    // DLC (Data Length Code) - 4 bits (fixado em 1 byte por simplicidade)
    sendCANBit(0);
    sendCANBit(0);
    sendCANBit(0);
    sendCANBit(1);  // 1 byte de dados

    // Dados (8 bits)
    for (int i = 7; i >= 0; i--) {
        sendCANBit((data >> i) & 1);
    }

    // CRC (Simulado, não válido)
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);
    sendCANBit(1);

    // ACK (Deveria ser lido do barramento, mas simulamos)
    sendCANBit(1);

    // EOF (7 bits recessivos)
    for (int i = 0; i < 7; i++) {
        sendCANBit(1);
    }
}

int main() {
    parport_fd = open(PARPORT_DEVICE, O_RDWR);
    if (parport_fd < 0) {
        std::cerr << "Erro ao abrir /dev/parport0" << std::endl;
        return -1;
    }

    if (ioctl(parport_fd, PPCLAIM) < 0) {
        std::cerr << "Erro ao reivindicar /dev/parport0" << std::endl;
        close(parport_fd);
        return -1;
    }

    while (true) {
        sendCANFrame(0x123, 0xAB);  // Envia um quadro CAN com ID 0x123 e dado 0xAB
        std::cout << "Enviado quadro CAN (ID: 0x123, Data: 0xAB)" << std::endl;
        sleep(1);
    }

    ioctl(parport_fd, PPRELEASE);
    close(parport_fd);
    return 0;
}
