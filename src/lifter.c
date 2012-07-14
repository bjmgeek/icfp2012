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
    int waterproof;
    int water_steps;
} robot;

typedef struct {
    char ** buf;
    int x_size;
    int y_size;
    int water;
    int flooding;
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

    fprintf(stderr,"buffer line -3: \"%s\"\n",buf[line_no-3]);
    if (line_no > 3 && strstr(buf[line_no-3],"Water") != NULL) {
	    sscanf(buf[line_no-3],"Water %d",&(map.water));
	    sscanf(buf[line_no-2],"Flooding %d",&(map.flooding));
	    sscanf(buf[line_no-1],"Waterproof %d",&(lLifter.waterproof));
        map.y_size -= 4;
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

/* print the map so we can watch our robot get squashed by rocks */
void print_map(){
int x;
    for (x=0; x<map.y_size;x++) {
        fprintf(stderr,"%d \"%s\"\n",x,map.buf[x]);
    }
    fprintf(stderr,"robot position (x,y): %d,%d\n",lLifter.x,lLifter.y);
}


/* updates map based on robot's movement
 * returns 1 if move is successful
 * returns 0 if move is unsuccessful
 * returns -1 if robot dies
 */
int update_map(char robot_dir) {
	int x_prime = lLifter.x, y_prime=lLifter.y;
	int movement_result = 0;
	int lambda_count = 0, lift_x = -1, lift_y = -1;
	int x,y;
	
	switch(robot_dir) {
		case 'D': y_prime ++; break;
		case 'U': y_prime --; break;
		case 'R': x_prime ++; break;
		case 'L': x_prime --; break;
	}
	
	lLifter.steps ++;
	
	/** update robot */
	
	/* if the robot is trying to move off the map, WAIT */
	if(x_prime >= map.x_size || x_prime < 0
	  || y_prime >= map.y_size || y_prime < 0)
	  movement_result = 0;
	/* if a robot is trying to move a rock, move rock if there is an empty space behind the rock */
	else if(map.buf[y_prime][x_prime] == '*' && (robot_dir == 'L' || robot_dir == 'R'))
	{
		if(robot_dir == 'L' && map.buf[y_prime][x_prime-1] == ' ')
		{
			map.buf[y_prime][x_prime-1] = '*';
			map.buf[lLifter.y][lLifter.x] = ' ';
			lLifter.y = y_prime;
			lLifter.x = x_prime;
			map.buf[lLifter.y][lLifter.x] = 'R';
			movement_result = 1;
		}
		else if(robot_dir == 'R' && map.buf[y_prime][x_prime+1] == ' ')
		{
			map.buf[y_prime][x_prime+1] = '*';
			map.buf[lLifter.y][lLifter.x] = ' ';
			lLifter.y = y_prime;
			lLifter.x = x_prime;
			map.buf[lLifter.y][lLifter.x] = 'R';
			movement_result = 1;
		}
		else
			movement_result = 0;
	}
	/* else if the robot is trying to move, and the move is valid move the robot */	
	else if((robot_dir == 'D' || robot_dir=='U' || robot_dir=='R' || robot_dir=='L') &&
			(map.buf[y_prime][x_prime] == ' ' || map.buf[y_prime][x_prime] == 'O' || map.buf[y_prime][x_prime] == '.' || map.buf[y_prime][x_prime] == '\\'))
	{
		map.buf[lLifter.y][lLifter.x] = ' ';
		
		/* if the robot is moving onto a lambda, pick it up */
		if(map.buf[y_prime][x_prime] == '\\')
			lLifter.lambdas ++;		
		
		lLifter.y = y_prime;
		lLifter.x = x_prime;
		map.buf[lLifter.y][lLifter.x] = 'R';
		movement_result = 1;
	}
	
	/** update map */
	for(y = 0; y < map.y_size; y++)
		for(x = 0; x < map.x_size; x++)
		{
			if(map.buf[y][x] == '\\') lambda_count ++;
			if(map.buf[y][x] == 'L')
			{
				lift_x = x;
				lift_y = y;
			}
			if(map.buf[y][x] == '*')
			{	/* unsupported rocks fall */
				if(map.buf[y+1][x] == ' ')
				{
					map.buf[y][x] = ' ';
					map.buf[y+1][x] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x] == 'R')
						movement_result = -1;
				}
				/* balanced rocks slide */
				else if(map.buf[y+1][x] == '*' && map.buf[y][x+1] == ' ' && map.buf[y+1][x+1] == ' ')
				{
					map.buf[y][x] = ' ';
					map.buf[y+1][x+1] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x+1] == 'R')
						movement_result = -1;
				}
				else if(map.buf[y+1][x] == '*' && map.buf[y][x-1] == ' ' && map.buf[y+1][x-1] == ' ')
				{
					map.buf[y][x] = ' ';
					map.buf[y+1][x-1] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x-1] == 'R')
						movement_result = -1;
				}
				else if(map.buf[y+1][x] == '\\' && map.buf[y][x+1] == ' ' && map.buf[y+1][x+1] == ' ')
				{
					map.buf[y][x] = ' ';
					map.buf[y+1][x+1] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x+1] == 'R')
						movement_result = -1;
				}
				
			}
		}
	
	/* if all the lambdas are collected, the lift opens */	
	if(lambda_count == 0)
		map.buf[lift_y][lift_x] = 'O';
	
    print_map();
	return movement_result;	
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
    read_map();
    init_robot();

    signal(SIGINT,sig_handler);
    signal(SIGALRM,sig_handler);





    return EXIT_SUCCESS;
}
