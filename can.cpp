#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <cstring>
#include <pthread.h>

#define BC8S_IEC_BASE_ADDRESS 600

// Variável global para o descritor da porta paralela
int parport_fd;

// Mutex para sincronizar o acesso aos buffers de saída
pthread_mutex_t writeOutputsBufferLock = PTHREAD_MUTEX_INITIALIZER;

// Vetores para habilitar módulos e armazenar os estados das saídas (16 módulos)
unsigned char BC8S_enabled[16];
unsigned char BC8S_outputBuffers[16];

/*
@func:      setDigitalOutput | Define estado de saída utilizando a porta paralela
@param:     int moduleAddress | Endereço do módulo (calculado a partir de BC8S_IEC_BASE_ADDRESS)
@param:     int value | Valor da saída (cada bit pode representar um canal)
@area:      Comunicação em porta paralela
*/
void setDigitalOutput(int moduleAddress, int value) {
    if (ioctl(parport_fd, PPWDATA, &value) < 0) {
        std::cerr << "Erro ao enviar dados para /dev/parport0 em setDigitalOutput" << std::endl;
    } else {
        std::cout << "Módulo " << moduleAddress << " saída definida para " << value << std::endl;
    }
}

int getDigitalInput() {
    unsigned char value;
    if (ioctl(parport_fd, PPRDATA, &value) < 0) {
        std::cerr << "Erro ao ler /dev/parport0" << std::endl;
        return -1;
    }
    return value;
}

int main() {
    parport_fd = open("/dev/parport0", O_RDWR);
    if (parport_fd < 0) {
        std::cerr << "Erro ao abrir /dev/parport0" << std::endl;
        return -1;
    }

    if (ioctl(parport_fd, PPCLAIM) < 0) {
        std::cerr << "Erro ao reivindicar /dev/parport0" << std::endl;
        close(parport_fd);
        return -1;
    }

    // Inicializa os 16 módulos: todos habilitados e saídas iniciadas como 0.
    for (int i = 0; i < 16; i++) {
        BC8S_enabled[i] = 1;
        BC8S_outputBuffers[i] = 0;
    }
    
    int toggle = 0;
    
    while (true) {
        pthread_mutex_lock(&writeOutputsBufferLock);
            for (int i = 0; i < 16; i++) {
                BC8S_outputBuffers[i] = toggle;
            }
            toggle = (toggle == 0) ? 1 : 0;
        pthread_mutex_unlock(&writeOutputsBufferLock);
        
        // Atualiza todos os módulos, utilizando o endereço base + índice
        for (int idx = 0; idx < 16; idx++) {
            unsigned char moduleEnabled;
            int output;
            
            pthread_mutex_lock(&writeOutputsBufferLock);
                moduleEnabled = BC8S_enabled[idx];
                output = BC8S_outputBuffers[idx];
            pthread_mutex_unlock(&writeOutputsBufferLock);
            
            if (moduleEnabled) {
                // Calcula o endereço real do módulo
                int moduleAddress = BC8S_IEC_BASE_ADDRESS + idx;
                setDigitalOutput(moduleAddress, output);
            }
        }

        std::cout << "Valor atual da porta: " << getDigitalInput() << std::endl;
        sleep(1);
    }

    ioctl(parport_fd, PPRELEASE);
    close(parport_fd);
    return 0;
}
