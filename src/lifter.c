#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

typedef struct {
    int x;
    int y;
} robot;

typedef struct {
    char ** buf;
    int x_size;
    int y_size;
} world;

world read_map()
{
    /* returns a newly allocated array of (newly allocated) strings */
    world w={NULL,0,0};
    char **buf=NULL;
    char *line=NULL;
    size_t n=0;
    int line_no=0;
    int max_len=0;

    while (getline(&line,&n,stdin) != -1){
        /* replace newline with null */
        line[strchr(line,'\n')-line]='\0';
        buf=realloc(buf,(1+line_no) * sizeof (char*));
        buf[line_no]=calloc(strlen(line),sizeof (char*));
        strcpy(buf[line_no],line);
        line_no++;
        if (max_len < strlen(line)) {
            max_len=strlen(line);
        }
    }

    w.buf=buf;
    w.y_size=line_no;
    w.x_size=max_len;
    return w;
}

int calc_abort_score() {
    fprintf(stderr,"FIXME: calc_abort_score()\n");
    return 1;
}

void last_second(){
    /* if score is positive (or would be with the bonus), send Abort command */
    fprintf(stderr,"checking for last second abort\n");
    if (calc_abort_score() > 0) {
        putchar('A');
        fflush(stdout);
    }
}

void sig_handler(int signum)
{
    if (signum==SIGINT){
        fprintf(stderr,"caught SIGINT\n");
        alarm(9);
    } else if (signum==SIGALRM) {
        fprintf(stderr,"caught SIGALRM\n");
        last_second();
    }
}

int main()
{
    world w=read_map();
    int x=0;

    signal(SIGINT,sig_handler);
    signal(SIGALRM,sig_handler);


    while (1){
        x++;
    }

    fprintf(stderr,"x: %d y: %d\n",w.x_size,w.y_size);

    return EXIT_SUCCESS;
}
