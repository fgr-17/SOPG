#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


#define SLEEP_PADRE_SEG     5
#define SLEEP_HIJO_SEG      5


void sigint_handler(int sig)
{
    int rv;

    write(0, "PARENT: Ahhh! SIGCHLD!\n", 14);
    sleep(SLEEP_PADRE_SEG);
    wait(&rv);
    write(0, "PARENT: Termino el wait, salgo del handler\n", 14);
    exit(0);
    return;
}


int main(void)
{
    pid_t pid;
//    int rv;
    int fdPipe[2];

    char mensajePipe[] = "Mensaje del CHILD al PARENT x pipe";
    char buf[50];

    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; //SA_RESTART;
    sigemptyset(&sa.sa_mask);


    pipe(fdPipe);



    switch(pid = fork()) {
    case -1:
        perror("fork");  /* something went wrong */
        exit(1);         /* parent exits */

    case 0:     // proceso hijo


        close(fdPipe[0]);

        printf(" CHILD: This is the child process!\n");
        printf(" CHILD: My PID is %d\n", getpid());
        printf(" CHILD: My parent's PID is %d\n", getppid());
        sleep(SLEEP_HIJO_SEG);        
        printf(" CHILD: Termine de esperar y mando msj\n");
        write(fdPipe[1], mensajePipe, sizeof(mensajePipe));
        sleep(SLEEP_HIJO_SEG);        
        sleep(SLEEP_HIJO_SEG);        
        printf(" CHILD: Termine de esperar y termino \n");
	    exit(0);



    default: 	// proceso padre

       close(fdPipe[1]);   


       if (sigaction(SIGCHLD, &sa, NULL) == -1) {
          perror("sigaction");
          exit(1);
       }


        printf("PARENT: This is the parent process!\n");
        printf("PARENT: My PID is %d\n", getpid());
        printf("PARENT: My child's PID is %d\n", pid);
        printf("PARENT: Espero que CHILD me mande un msj x pipe\n");
        

        while(1){
            int ret = read(fdPipe[0], buf, sizeof(mensajePipe));

		    if(ret>0)
		    {
			    printf("PARENT: llego de hijo:%s\n",buf);
		    }
		    else if(ret==0)
		    {
			    printf("PARENT: hay EOF!\n");
			    break;
		    }
		    else
		    {
			    perror("PARENT: read error");
		    }



            sleep(1);        
        }


    }
}
