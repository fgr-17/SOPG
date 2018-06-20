/**
 * @file manejoThreads.c
 *
 * @author Federico G. Roux (rouxfederico@gmail.com)
 *
 */

/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>

#include <pthread.h>
#include <signal.h>


/* --------------------------------------- prototipos -------------------------------------- */

int bloquearSign(void);
int desbloquearSign(void);

/* --------------------------------------- funciones -------------------------------------- */

/**
 * @fn int bloquearSign(void)
 *
 * @brief Bloqueo set de signals que me interesan
 * 
 */

int bloquearSign(void) {

    sigset_t set;

    if(sigemptyset(&set)) {
        perror("Error de sigemptyset()");
        return errno;
    }
        
    if(sigaddset(&set, SIGINT)) {
        perror("Error de sigaddset()");
        return errno;
    }

    if(pthread_sigmask(SIG_BLOCK, &set, NULL)) {
        perror("Error de pthread_sigmask()");
        return errno;
    }

    return 0;
}

/**
 * @fn int desbloquearSign(void)
 *
 * @brief Desbloqueo set de signals que me interesan
 * 
 */

int desbloquearSign(void) {

    sigset_t set;

    if(sigemptyset(&set)) {
        perror("Error de sigemptyset()");
        return errno;
    }
        
    if(sigaddset(&set, SIGINT)) {
        perror("Error de sigaddset()");
        return errno;
    }

    if(pthread_sigmask(SIG_UNBLOCK, &set, NULL)) {
        perror("Error de pthread_sigmask()");
        return errno;
    }

    return 0;
}

