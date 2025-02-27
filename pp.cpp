#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <chrono>
#include <thread>

#define PARPORT_DEVICE "/dev/parport0"

// Pino que será usado para gerar PWM (exemplo: Bit 0 da porta paralela → Pino 2)
#define PWM_PIN 0x01  

// Frequência do PWM (250 Hz) → Período = 4 ms (4000 µs)
#define PWM_FREQUENCY 250
#define PWM_PERIOD_US (1000000 / PWM_FREQUENCY)  // 4000 µs

int parport_fd;

// Função para gerar o sinal PWM
void generatePWM(int dutyCycle) {
    int highTime = (PWM_PERIOD_US * dutyCycle) / 100;  // Tempo ALTO
    int lowTime = PWM_PERIOD_US - highTime;           // Tempo BAIXO

    int value = PWM_PIN;  // Define variável para o valor a ser escrito
    ioctl(parport_fd, PPWDATA, &value);  // Define o pino como ALTO
    std::this_thread::sleep_for(std::chrono::microseconds(highTime));

    value = 0x00;  // Define variável para nível BAIXO
    ioctl(parport_fd, PPWDATA, &value);  // Define o pino como BAIXO
    std::this_thread::sleep_for(std::chrono::microseconds(lowTime));
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

    int dutyCycle = 50;  // Define ciclo de trabalho inicial (50%)
    std::cout << "Gerando PWM com duty cycle de " << dutyCycle << "%\n";

    while (true) {
        generatePWM(dutyCycle);
    }

    ioctl(parport_fd, PPRELEASE);
    close(parport_fd);
    return 0;
}
