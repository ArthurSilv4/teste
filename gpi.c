#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <stdlib.h>

int main(void)
{
    int fd;
    unsigned char data;

    // Abre a porta paralela
    fd = open("/dev/parport0", O_RDWR);
    if (fd < 0)
    {
        perror("Erro ao abrir /dev/parport0");
        exit(1);
    }

    // Inicialmente, coloca todos os pinos em 0 (nível baixo)
    data = 0x00;
    if (ioctl(fd, PPWDATA, &data) == -1)
    {
        perror("Erro ao escrever na porta");
        close(fd);
        exit(1);
    }
    printf("Pinos zerados (0x00).\n");

    // Aguarda 2 segundos
    sleep(2);

    // Exemplo: liga o pino D0 (bit 0) configurando data = 0x01
    data = 0x01;
    if (ioctl(fd, PPWDATA, &data) == -1)
    {
        perror("Erro ao escrever na porta");
        close(fd);
        exit(1);
    }
    printf("Pino D0 ativado (0x01).\n");

    // Aguarda 2 segundos para observar a mudança
    sleep(2);

    // Desliga novamente os pinos
    data = 0x00;
    if (ioctl(fd, PPWDATA, &data) == -1)
    {
        perror("Erro ao escrever na porta");
        close(fd);
        exit(1);
    }
    printf("Pinos zerados novamente (0x00).\n");

    close(fd);
    return 0;
}
