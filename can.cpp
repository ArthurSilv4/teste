#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <cstring>

// Desabilitamos a lógica CAN e utilizamos a porta paralela
// Variável global para o descritor da porta paralela
int parport_fd;

// Define o estado de saída digital
/*
@func:      setDigitalOutput | Define estado de saída utilizando a porta paralela
@param:     int moduleAddress | Endereço do módulo (não utilizado, apenas para compatibilidade)
@param:     int value | Valor da saída (cada bit pode representar um canal)
@area:      Comunicação em porta paralela
*/
void setDigitalOutput(int moduleAddress, int value) {
    (void)moduleAddress; // Suprime warning de variável não utilizada
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

    int currentValue = 0;
    while (true) {
        currentValue = (currentValue == 0) ? 1 : 0;
        std::cout << "Valor atual da porta: " << getDigitalInput() << std::endl;

        setDigitalOutput(0, currentValue);
        sleep(1);
    }

    // Libera a porta paralela (não alcançado devido ao loop infinito)
    ioctl(parport_fd, PPRELEASE);
    close(parport_fd);
    return 0;
}
