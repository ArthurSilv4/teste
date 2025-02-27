#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/io.h>

int main()
{
    // Habilita acesso à porta I/O (endereço padrão para LPT1: 0x378)
    if (ioperm(0x378, 3, 1))
    {
        perror("ioperm");
        exit(1);
    }

    // Abre o dispositivo /dev/lp0 para escrita
    int fd = open("/dev/lp0", O_WRONLY);
    if (fd == -1)
    {
        perror("Erro ao abrir /dev/lp0");
        return 1;
    }

    // Envia um byte de teste (0xFF)
    char data = 0xFF;
    if (write(fd, &data, 1) != 1)
    {
        perror("Erro ao escrever na porta");
        close(fd);
        return 1;
    }

    printf("Escrita bem-sucedida na porta LPT1 (/dev/lp0)!\n");
    close(fd);
    return 0;
}
