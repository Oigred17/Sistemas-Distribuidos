#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    pid_t mipid;

    mipid = fork(); /* Creación de un proceso hijo. */

    if (mipid == -1) { /* Error en la llamada a fork() */
        perror("Error en la llamada a fork()");
    } else if (mipid == 0) {
        /* Código que ejecuta solamente el proceso HIJO */
        printf("Hijo: mi pid es=%d\n", getpid());
    } else {
        /* Código que ejecuta solamente el proceso PADRE */
        printf("Padre: mi pid es=%d\n", getpid());
    }

    /* Código común a los procesos PADRE e HIJO. */
    printf("Finalizando...\n");

    return 0;
}