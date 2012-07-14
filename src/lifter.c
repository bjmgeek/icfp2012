#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

typedef struct {
    int x;
    int y;
    int steps;
    int lambdas;
} robot;

typedef struct {
    char ** buf;
    int x_size;
    int y_size;
} world;

/* global variables */
robot lLifter;
world map;

/* pad the map with spaces */
void space_pad (char *st,int n){
	int i;
	for (i=strlen(st); i<n; i++)
		st[i]=' ';
}

/* populate the global world variable, and allocate the array of 
 * strings for its buffer */
void read_map() {
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

    free(line);

    map.buf=buf;
    map.y_size=line_no;
    map.x_size=max_len;

    /* The width of the mine is the number of characters in the longest line.  
     * shorter lines are assumed to be padded out with spaces */
    for (n=0;n<map.y_size;n++) {
        map.buf[n]=realloc(map.buf[n],max_len * sizeof (char*));
        space_pad(map.buf[n],max_len);
    }
}

/* find the robot on the map, and populate the global robot variable */
void init_robot() {
    int x,y;

    lLifter.steps=0;
    lLifter.lambdas=0;
    for (x=0; x<map.x_size; x++)
        for (y=0; y<map.y_size; y++)
            if (map.buf[y][x]=='R') {
                lLifter.x=x;
                lLifter.y=y;
            }
}


void update_map(char robot_dir) {
	int x_prime = lLifter.x, y_prime=lLifter.y;
	
	switch(robot_dir) {
		case 'N': y_prime ++; break;
		case 'S': y_prime --; break;
		case 'E': x_prime ++; break;
		case 'W': x_prime --; break;
	}
	
	
	
}	


/* calculate the score if we abort now */
int calc_abort_score() {
  /* need to keep track of how many lambdas we've picked up already
   * need to keep track of how many steps we've taken 
   * score = lambdas * 50 - steps
   */
    return (lLifter.lambdas * 50) - lLifter.steps;
}

void last_second(){
    /* if score is positive (or would be with the bonus), send Abort command */
    fprintf(stderr,"checking for last second abort\n");
    if (calc_abort_score() > 0) {
        putchar('A');
        fflush(stdout);
    }
}

void sig_handler(int signum) {
    if (signum==SIGINT){
        fprintf(stderr,"caught SIGINT\n");
        alarm(9);
    } else if (signum==SIGALRM) {
        fprintf(stderr,"caught SIGALRM\n");
        last_second();
    }
}


int main() {
    int x=0;

    read_map();
    init_robot();

    signal(SIGINT,sig_handler);
    signal(SIGALRM,sig_handler);




    fprintf(stderr,"x: %d y: %d\n",map.x_size,map.y_size);
    for (x=0; x<map.y_size;x++) {
        fprintf(stderr,"%d \"%s\"\n",x,map.buf[x]);
    }
    fprintf(stderr,"robot position (x,y): %d,%d\n",lLifter.x,lLifter.y);

    return EXIT_SUCCESS;
}
