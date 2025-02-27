#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <chrono>
#include <thread>

#define PARPORT_DEVICE "/dev/parport0"

// Mapeia os pinos da porta paralela para CAN_H e CAN_L
#define CAN_H 0x01  // Pino 2 (Bit 0 da porta paralela)
#define CAN_L 0x02  // Pino 3 (Bit 1 da porta paralela)

// Tempo de um bit no barramento CAN (ajustado para 125 kbps)
#define BIT_TIME_US 8

int parport_fd;

// Simula a transmissão de um bit CAN na porta paralela
void sendCANBit(int bit) {
    int signal = (bit == 0) ? CAN_H : CAN_L;  // CAN_H = Dominante (0), CAN_L = Recessivo (1)
    ioctl(parport_fd, PPWDATA, &signal);
    std::this_thread::sleep_for(std::chrono::microseconds(BIT_TIME_US));
}

// Envia um quadro CAN (ID 0x123, Dado 0xAB)
void sendCANFrame() {
    std::cout << "Enviando quadro CAN...\n";

    // SOF (Start of Frame) - Sempre 0
    sendCANBit(0);

    // ID do Quadro (11 bits)
    int id = 0x123;
    for (int i = 10; i >= 0; i--) {
        sendCANBit((id >> i) & 1);
    }

    // RTR (Remote Transmission Request) - Sempre 0 para mensagens normais
    sendCANBit(0);

    // IDE (Identifier Extension) - 0 para CAN padrão
    sendCANBit(0);

    // DLC (Data Length Code) - 4 bits (fixado em 1 byte)
    sendCANBit(0);
    sendCANBit(0);
    sendCANBit(0);
    sendCANBit(1);  

    // Dados (8 bits)
    int data = 0xAB;
    for (int i = 7; i >= 0; i--) {
        sendCANBit((data >> i) & 1);
    }

    // CRC (Simulado)
    for (int i = 0; i < 15; i++) {
        sendCANBit(1);
    }

    // ACK (Deveria ser lido do barramento, mas simulamos)
    sendCANBit(1);

    // EOF (7 bits recessivos)
    for (int i = 0; i < 7; i++) {
        sendCANBit(1);
    }

    std::cout << "Quadro CAN enviado com sucesso!\n";
}

int main() {
    parport_fd = open(PARPORT_DEVICE, O_RDWR);
    if (parport_fd < 0) {
        std::cerr << "Erro ao abrir /dev/parport0\n";
        return -1;
    }

    if (ioctl(parport_fd, PPCLAIM) < 0) {
        std::cerr << "Erro ao reivindicar /dev/parport0\n";
        close(parport_fd);
        return -1;
    }

    while (true) {
        sendCANFrame();
        sleep(1);
    }

    ioctl(parport_fd, PPRELEASE);
    close(parport_fd);
    return 0;
}
