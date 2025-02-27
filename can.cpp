#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <cstring>
#include <pthread.h>

// Desabilitamos a lógica CAN e utilizamos a porta paralela
// Variável global para o descritor da porta paralela
int parport_fd;

// Mutex para sincronizar o acesso aos buffers de saída
pthread_mutex_t writeOutputsBufferLock = PTHREAD_MUTEX_INITIALIZER;

// Vetores para habilitar módulos e armazenar os estados das saídas (16 módulos)
unsigned char BC8S_enabled[16];
int BC8S_outputBuffers[16];

/*
@func:      setDigitalOutput | Define estado de saída utilizando a porta paralela
@param:     int moduleAddress | Endereço do módulo (nesse exemplo indica qual módulo está sendo acessado)
@param:     int value | Valor da saída (cada bit pode representar um canal)
@area:      Comunicação em porta paralela
*/
void setDigitalOutput(int moduleAddress, int value) {
    // Se desejar, você pode usar moduleAddress para modificar o valor transmitido
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
    // Abre a porta paralela
    parport_fd = open("/dev/parport0", O_RDWR);
    if (parport_fd < 0) {
        std::cerr << "Erro ao abrir /dev/parport0" << std::endl;
        return -1;
    }

    // Reivindica o dispositivo
    if (ioctl(parport_fd, PPCLAIM) < 0) {
        std::cerr << "Erro ao reivindicar /dev/parport0" << std::endl;
        close(parport_fd);
        return -1;
    }

    // Inicializa os 16 módulos: todos habilitados e as saídas iniciadas com 0.
    for (int i = 0; i < 16; i++) {
        BC8S_enabled[i] = 1;       // Habilita o módulo
        BC8S_outputBuffers[i] = 0; // Saída inicial
    }
    
    // Variável auxiliar para controlar o toggle de todos os módulos
    int toggle = 0;
    
    // Loop principal para atualizar as saídas digitais dos módulos
    while (true) {
        // Atualiza todos os módulos para alternar entre 0 e 1
        pthread_mutex_lock(&writeOutputsBufferLock);
            for (int i = 0; i < 16; i++) {
                BC8S_outputBuffers[i] = toggle;
            }
            toggle = (toggle == 0) ? 1 : 0;
        pthread_mutex_unlock(&writeOutputsBufferLock);
        
        ////////////////////////////////////////////////////////////
        // 16 BC8S Modules - 8 digital outputs per module
        ////////////////////////////////////////////////////////////
        for (int BC8S_address = 0; BC8S_address < 16; BC8S_address++) {
            unsigned char BC8SoutputEnabled;
            int outputStates;
            
            pthread_mutex_lock(&writeOutputsBufferLock);
                BC8SoutputEnabled = BC8S_enabled[BC8S_address];
                outputStates = BC8S_outputBuffers[BC8S_address];
            pthread_mutex_unlock(&writeOutputsBufferLock);
            
            if (BC8SoutputEnabled) {
                setDigitalOutput(BC8S_address, outputStates);
            }
        }

        // Leitura opcional do estado da porta
        std::cout << "Valor atual da porta: " << getDigitalInput() << std::endl;
        sleep(1);
    }

    // Libera a porta paralela (nunca alcançado devido ao loop infinito)
    ioctl(parport_fd, PPRELEASE);
    close(parport_fd);
    return 0;
}
