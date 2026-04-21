#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

int main(void) {
    pid_t mipid;
    printf("Proceso padre va a crear un proceso hijo\n");
    mipid = fork();
    if (mipid == -1) {
        perror("Error al crear el proceso hijo");
        printf("Código de error: %d\n", errno);
        return -1;
    } else if (mipid == 0) {
        printf("HIJO: Mi pid es %d, el de mi padre es %d\n", getpid(), getppid());
        while (1) {
            sleep(1);
            printf(" HIJO: zzz...\n");
        }
    } else {
        for (int i = 0; i < 7; i++) {
            printf("PADRE: Despierta, hijo!\n");
            sleep(2);
        }
        printf("PADRE Voy a matar a mi hijo %d\n", mipid);
        if (kill(mipid, SIGKILL) == -1) {
            perror("Error al matar al hijo");
        }
        return 0;
    }
}
