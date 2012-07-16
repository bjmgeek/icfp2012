#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

typedef struct {
    int x;
    int y;
    int steps;
    int lambdas;
    int waterproof;
    int water_steps;
    int razors;
} robot;

typedef struct {
    char source;
    char target;
    int target_x;
    int target_y;
} trampoline;

typedef struct {
    char ** buf;
    int x_size;
    int y_size;
    int initial_lambdas;
    int water;
    int flooding;
    trampoline *tramps;
    int num_tramps;
    int growth;
} world;


/* global variables */
robot lLifter;
world map;
int **beards;
const int LONGEST_PATH=21;

/* calculate the score of a success */
int calc_success_score() {
	/* need to keep track of how many lambdas we've picked up already
   * need to keep track of how many steps we've taken 
   * score = lambdas * 75 - steps
   */
    return (lLifter.lambdas * 75) - lLifter.steps;
}
/* pad the map with spaces */
void space_pad (char *st,int n){
	int i;
	for (i=strlen(st); i<n; i++)
		st[i]=' ';
}

int count_lambdas() {
    int x,y;
    int count=0;
    for (y=0; y<map.y_size; y++)
        for (x=0; x<map.x_size; x++)
            if (map.buf[y][x]=='\\')
                count++;
    fprintf(stderr,"found %d lambdas in map\n",count);
    return count;
}

void find_all_tramp_dest() {
	int x, y, i;
	
	 for (y=0; y<map.y_size; y++)
        for (x=0; x<map.x_size; x++)
            if (map.buf[y][x]>='1' && map.buf[y][x] <= '9')
				for(i = 0; i < map.num_tramps; i ++)
					if(map.buf[y][x] == map.tramps[i].target)
					{
						map.tramps[i].target_x = x;
						map.tramps[i].target_y = y;
					}
}

trampoline find_tramp(char t) {
	int i;
	trampoline tramp;
	
	for(i = 0; i < map.num_tramps; i ++)
		if(t == map.tramps[i].source)
			tramp = map.tramps[i];
			
	return tramp;
}

/* parses a buffer containing all the lines of input and updates metadata
 * in the global world and robot variables */
int get_metadata(char ** buf,int num_lines) {
    int i;
    int count=0;
    map.growth=25; /* the default if not specified in the input */
    for (i=0; i< num_lines; i++) {
        if (strstr(buf[i],"Water ") != NULL) {
            sscanf(buf[i],"Water %d",&(map.water));
            count++;
        }
        if (strstr(buf[i],"Flooding") != NULL) {
            sscanf(buf[i],"Flooding %d",&(map.flooding));
            count++;
        }
        if (strstr(buf[i],"Waterproof") != NULL) {
            sscanf(buf[i],"Waterproof %d",&(lLifter.waterproof));
            count++;
        }
        if (strstr(buf[i],"Trampoline") != NULL) {
            map.tramps=realloc(map.tramps,(1 + map.num_tramps) * sizeof (trampoline));
            sscanf(buf[i],"Trampoline %c targets %c",&(map.tramps[map.num_tramps].source),&(map.tramps[map.num_tramps].target));
            map.num_tramps++;
            count++;
        }
        if (strstr(buf[i],"Growth") != NULL) {
            sscanf(buf[i],"Growth %d",&(map.growth));
            count++;
        }
        if (strstr(buf[i],"Razors ") != NULL) {
            sscanf(buf[i],"Razors %d",&(lLifter.razors));
            count++;
        }
    }
    fprintf(stderr,"found %d lines of metadata\n",count);
    return count;
}

/* find the robot on the map, and populate the global robot variable */
void init_robot() {
    int x,y;

    lLifter.steps=0;
    lLifter.lambdas=0;
    lLifter.water_steps=0;
    lLifter.waterproof=10;  /* 10 is the default if not specified */
    for (x=0; x<map.x_size; x++)
        for (y=0; y<map.y_size; y++)
            if (map.buf[y][x]=='R') {
                lLifter.x=x;
                lLifter.y=y;
            }
}
/* populate the global world variable, and allocate the array of 
 * strings for its buffer */
void read_map(FILE *f) {
    char **buf=NULL;
    char *line=NULL;
    size_t n=0;
    int line_no=0;
    int max_len=0;
    int x,y;

    map.flooding=0;  /* default per spec if not listed */
    map.water=0;  /* default per spec if not listed */

    while (getline(&line,&n,f) != -1){
        /* replace newline with null */
        line[strchr(line,'\n')-line]='\0';
        buf=realloc(buf,(1+line_no) * sizeof (char*));
        buf[line_no]=calloc(strlen(line),sizeof (char*));
        strcpy(buf[line_no],line);
        line_no++;
    }

    free(line);

    map.buf=buf;
    map.y_size=line_no;
    map.tramps=NULL;
    map.num_tramps=0;

    /* check for metadata */
    n=get_metadata(buf,line_no);
    if (n > 0)
        map.y_size -= (1+n);

    /* find max length of the map lines only */
    for (n=0;n<map.y_size;n++) {
        if (max_len < strlen(map.buf[n])) {
            max_len=strlen(map.buf[n]);
        }
    }
    map.x_size=max_len;

    /* The width of the mine is the number of characters in the longest line.  
     * shorter lines are assumed to be padded out with spaces */
    for (n=0;n<map.y_size;n++) {
        map.buf[n]=realloc(map.buf[n],max_len * sizeof (char*));
        space_pad(map.buf[n],max_len);
    }

    /* initialize beards */
    beards=calloc(map.y_size,sizeof (int*));
    for (y=0; y<map.y_size; y++) {
        beards[y]=calloc(map.x_size, sizeof (int));
        for (x=0; x<map.x_size; x++){
            if (map.buf[y][x]=='W')
                beards[y][x]=map.growth-1;
            else
                beards[y][x]=INT_MAX;
        }
    }

    map.initial_lambdas=count_lambdas();
	init_robot();
	find_all_tramp_dest();
}

/* print the map so we can watch our robot get squashed by rocks */
void print_map(){
int x;
    fprintf(stderr,"map size: %d rows of %d columns each\n",map.y_size,map.x_size);
    for (x=0; x<map.y_size;x++) {
        fprintf(stderr,"%02d \"%s\"\n",x,map.buf[x]);
    }
    fprintf(stderr,"robot position (x,y): %d,%d\n",lLifter.x,lLifter.y);
    fprintf(stderr,"water level: %d\n",map.water);
    fprintf(stderr,"water steps: %d out of %d\n",lLifter.water_steps, lLifter.waterproof);
    fprintf(stderr,"robot has %d razors\n",lLifter.razors);
    fprintf(stderr,"robot has %d lambdas out of %d\n",lLifter.lambdas,map.initial_lambdas);
    fprintf(stderr,"steps: %d\n",lLifter.steps);
    for (x=0; x<map.num_tramps; x++)
        fprintf(stderr,"trampoline %c to target %c\n",map.tramps[x].source,map.tramps[x].target);
}

/* allocates a new array of n strings (each of length l) and copies from
 * src to dest */
char** copy_buf (char **src,int n,int l) {
    int i;
    char **dest;
    dest=malloc(n*sizeof (char*));
    for (i=0; i<n; i++) {
        dest[i]=malloc((1+l)*sizeof(char));
        strcpy(dest[i],src[i]);
    }
    return dest;
}

void cleanup(char **buf,int lines){
    int i;
    for (i=0; i<lines; i++)
        free(buf[i]);
    free(buf);
}

void shave() {
    int x,y;
    for (x=-1; x<2; x++) {
        for (y=-1; y<2; y++) {
            if (((x + lLifter.x) > 0) && ((x + lLifter.x) < map.x_size) 
                    && ((y + lLifter.y) > 0) && ((y + lLifter.y) < map.y_size)
                    && map.buf[y+lLifter.y][x+lLifter.x]=='W') {
                map.buf[y+lLifter.y][x+lLifter.x]=' ';
            } 
        }
    }
}

/* updates map based on robot's movement
 * returns 1 if move is successful
 * returns 0 if move is unsuccessful
 * returns -1 if robot dies
 */
int update_map(char robot_dir) {
	int x_prime = lLifter.x, y_prime=lLifter.y;
	int movement_result = 0;
	int lift_x = -1, lift_y = -1;
	int x,y,i,j;
	char robot_target = '0';
    char **new_buf=NULL;
	
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
	/* if robot is trying to use a trampoline, jump */
	else if(map.buf[y_prime][x_prime] >= 'A' && map.buf[y_prime][x_prime] <= 'I')
	{
		for(i=0;i<map.num_tramps;i++)
			if(map.tramps[i].source == map.buf[y_prime][x_prime])
			{
				robot_target = map.tramps[i].target;
				map.buf[lLifter.y][lLifter.x] = ' ';
				map.buf[y_prime][x_prime] = ' ';
				lLifter.y = map.tramps[i].target_y;
				lLifter.x = map.tramps[i].target_x;
				map.buf[lLifter.y][lLifter.x] = 'R';
			}
	}	
	else if(map.buf[y_prime][x_prime] == 'O')
	{
		fprintf(stderr," SUCCESS! score: %d \n",calc_success_score());
		exit(EXIT_SUCCESS);
	}
	/* else if the robot is trying to move, and the move is valid move the robot */	
	else if((robot_dir == 'D' || robot_dir=='U' || robot_dir=='R' || robot_dir=='L') &&
			(map.buf[y_prime][x_prime] == ' ' || map.buf[y_prime][x_prime] == '.' || map.buf[y_prime][x_prime] == '\\' || map.buf[y_prime][x_prime] == '!'))
	{
		map.buf[lLifter.y][lLifter.x] = ' ';
		
		/* if the robot is moving onto a lambda, pick it up */
		if(map.buf[y_prime][x_prime] == '\\') {
			lLifter.lambdas ++;		
        }

		/* if the robot is moving onto a razor, pick it up */
		if(map.buf[y_prime][x_prime] == '!') {
			lLifter.razors++;		
        }
		
		lLifter.y = y_prime;
		lLifter.x = x_prime;
		map.buf[lLifter.y][lLifter.x] = 'R';
		movement_result = 1;
	} else if (robot_dir == 'S')
        shave();

    /* done moving robot, subsequent map updates read from map.buf and 
     * write to new_buf */
    new_buf=copy_buf(map.buf,map.y_size,map.x_size);
	
	/** update map */
	for(y = map.y_size -1; y >= 0; y--)
		for(x = 0; x < map.x_size; x++)
		{
			if(map.buf[y][x] == 'L' || map.buf[y][x] == 'O')
			{
				lift_x = x;
				lift_y = y;
			}
			else if(map.buf[y][x] >= 'A' && map.buf[y][x] <= 'I')
			{
				/* trampolines with used targets disappear */
				for(i=0;i<map.num_tramps;i++)
					if(map.tramps[i].source == map.buf[y][x] && map.tramps[i].target == robot_target)
						new_buf[y][x] = ' ';
			}	
			else if(map.buf[y][x] == '*')
			{	/* unsupported rocks fall */
				if(map.buf[y+1][x] == ' ')
				{
					new_buf[y][x] = ' ';
					new_buf[y+1][x] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x] == 'R')
						movement_result = -1;
				}
				/* balanced rocks slide */
				else if(map.buf[y+1][x] == '*' && map.buf[y][x+1] == ' ' && map.buf[y+1][x+1] == ' ')
				{
					new_buf[y][x] = ' ';
					new_buf[y+1][x+1] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x+1] == 'R')
						movement_result = -1;
				}
				else if(map.buf[y+1][x] == '*' && map.buf[y][x-1] == ' ' && map.buf[y+1][x-1] == ' ')
				{
					new_buf[y][x] = ' ';
					new_buf[y+1][x-1] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x-1] == 'R')
						movement_result = -1;
				}
				else if(map.buf[y+1][x] == '\\' && map.buf[y][x+1] == ' ' && map.buf[y+1][x+1] == ' ')
				{
					new_buf[y][x] = ' ';
					new_buf[y+1][x+1] = '*';
					/* falling rocks can kill robots */
					if(map.buf[y+2][x+1] == 'R')
						movement_result = -1;
				}
				
			} else if (map.buf[y][x] == 'W') {
                /* beard growth */
                if (beards[y][x] > 0)
                    beards[y][x]--;
                else {
                    beards[y][x]=map.growth-1;
                    for (i=-1; i<2; i++)
                        for (j=-1; j<2; j++) {
                            if (((i + lLifter.x) > 0) && ((i + lLifter.x) < map.x_size) 
                                    && ((j + lLifter.y) > 0) && ((j + lLifter.y) < map.y_size)
                                    && map.buf[j][i]==' ') {
                                new_buf[j][i]='W';
                                beards[j][i]=map.growth-1;
                            }
                        }
                }

            }
		}
		
    /* increase flooding if necessary */
    if ((map.flooding > 0) && (lLifter.steps % map.flooding)==0)
        map.water++;
    if (lLifter.y >= (map.y_size - map.water))
        lLifter.water_steps++;
    else
		lLifter.water_steps = 0;
    if (lLifter.water_steps > lLifter.waterproof)
        return -1;

	/* if all the lambdas are collected, the lift opens */	
	if(map.initial_lambdas==lLifter.lambdas)
		new_buf[lift_y][lift_x] = 'O';

	

    cleanup(map.buf,map.y_size);
    map.buf=new_buf;
    
	return movement_result;	
}	

int search(int y, int x, int steps, char dir, int razors)
{
	int min_steps = LONGEST_PATH, test_steps;
	
	/*fprintf(stderr,"checking dir %c there is a %c here\n",dir, map.buf[y][x]);*/
	
	/* if safe location or not too many steps(more than LONGEST_PATH), continue searching */
	if(map.buf[y][x] != '#' && map.buf[y][x] != 'W' && map.buf[y][x] != 'L' && map.buf[y][x] != '*' && steps < LONGEST_PATH )
	{
		steps++;
		
		/* if lambda or exit(& no lambdas left), return steps +1 */
		if(map.buf[y][x] == '\\' || map.buf[y][x] == '!' || (map.initial_lambdas == lLifter.lambdas && map.buf[y][x] == 'O'))
			return steps;
			
		/* if trampoline, "jump" */
		if(map.buf[y][x] >= 'A' && map.buf[y][x] <= 'I')
		{
			trampoline tramp = find_tramp(map.buf[y][x]);
			return search(tramp.target_y, tramp.target_x, steps, 'T', razors);
		}
			
		/* if lambda or exit nearby , return steps +2*/
		if(map.buf[y][x+1] == '\\' || map.buf[y][x] == '!' || (map.initial_lambdas == lLifter.lambdas && map.buf[y][x+1] == 'O') ||
			map.buf[y][x-1] == '\\' || map.buf[y][x] == '!' || (map.initial_lambdas == lLifter.lambdas && map.buf[y][x-1] == 'O') ||
			map.buf[y-1][x] == '\\' || map.buf[y][x] == '!' || (map.initial_lambdas == lLifter.lambdas && map.buf[y-1][x] == 'O') ||
			(map.buf[y-1][x] != '*' && (map.buf[y+1][x] == '\\' || map.buf[y][x] == '!' || (map.initial_lambdas == lLifter.lambdas && map.buf[y+1][x] == 'O'))))
			return steps+1;
		/* else search */
		/* recursively check all 4 directions for closest safe lambda or exit*/
		if(dir != 'L' && min_steps > steps+1) {
			test_steps = search(y, x+1, steps, 'R', razors);
			if (test_steps < min_steps) min_steps = test_steps;
		}
		if(dir != 'R' && min_steps > steps+1) {
			test_steps = search(y, x-1, steps, 'L', razors);
			if (test_steps < min_steps) min_steps = test_steps;
		}
		if(dir != 'D' && min_steps > steps+1) {
			test_steps = search(y-1, x, steps, 'U', razors);
			if (test_steps < min_steps) min_steps = test_steps;
		}
		if(dir != 'U' && min_steps > steps+1 && map.buf[y-1][x] != '*') {
			test_steps = search(y+1, x, steps, 'D', razors);
			if (test_steps < min_steps) min_steps = test_steps;
		}
	}
	
	/* if unable to move, check to see if there is a rock here that can be moved 
	 * without blocking the lift */
	if(min_steps == LONGEST_PATH && map.buf[y][x] == '*')
	{
		steps++;
		if(dir == 'L' && map.buf[y][x-1] == ' ' && map.buf[y][x-2] !='O' && map.buf[y][x-2] != 'L'){
			min_steps = search(y, x-1, steps, 'L', razors);
		}
		else if(dir == 'R' && map.buf[y][x+1] == ' ' && map.buf[y][x+2] !='O' && map.buf[y][x+2] != 'L'){
			min_steps = search(y, x+1, steps, 'R', razors);
		}
		else
			return LONGEST_PATH;
	}
	
	/* if unable to move, check to see if there is a beard here that can be shaved */
	if(min_steps == LONGEST_PATH && map.buf[y][x] == 'W' && razors > 0)
	{
		steps++;
		if(dir != 'L' && min_steps > steps+1) {
			test_steps = search(y, x+1, steps, 'R', razors-1);
			if (test_steps < min_steps) min_steps = test_steps;
		}
		if(dir != 'R' && min_steps > steps+1) {
			test_steps = search(y, x-1, steps, 'L', razors-1);
			if (test_steps < min_steps) min_steps = test_steps;
		}
		if(dir != 'D' && min_steps > steps+1) {
			test_steps = search(y-1, x, steps, 'U', razors-1);
			if (test_steps < min_steps) min_steps = test_steps;
		}
		if(dir != 'U' && min_steps > steps+1 && map.buf[y-1][x] != '*') {
			test_steps = search(y+1, x, steps, 'D', razors-1);
			if (test_steps < min_steps) min_steps = test_steps;
		}
	}
	
	return min_steps;
}

/* tells the robot where to move next 
 * currently: moves towards the closest lambda or the exit
 * needs to: 
 *  - look for falling rocks
 *  - know how to push rocks
 *  - get out of the water before drowning
 *  - strategize getting complicated lambdas
 * */
char move_robot()
{
	int U = LONGEST_PATH, D = LONGEST_PATH, L = LONGEST_PATH, R = LONGEST_PATH, x, y;
	
	/* if under water and out of waterproofing 
	 * pop up if at the surface, otherwise abort */
	if(lLifter.water_steps == lLifter.waterproof)
	{
		if((map.water == map.y_size - lLifter.y) && 
			(map.buf[lLifter.y-1][lLifter.x] == '.' || map.buf[lLifter.y-1][lLifter.x] == ' '))
			return 'U';
		else
			return 'A';
	}
	
	/* recursively check all 4 directions for closest safe lambda or exit*/
	R = search(lLifter.y, lLifter.x+1, 0, 'R', lLifter.razors);
	L = search(lLifter.y, lLifter.x-1, 0, 'L', lLifter.razors);
	U = search(lLifter.y-1, lLifter.x, 0, 'U', lLifter.razors);
	if(map.buf[lLifter.y-1][lLifter.x] != '*')
		D = search(lLifter.y+1, lLifter.x, 0, 'D', lLifter.razors);
	
	/* fprintf(stderr,"R has a lambda in %i steps, L has one in %i steps, U has one in %i steps, D has one in %i steps\n",R, L, U, D); */
	
	if(R < LONGEST_PATH && R <= U && R <= D && R <= L)
	{
		if(map.buf[lLifter.y][lLifter.x+1] == 'W')
			return 'S';
		else
			return 'R';
	}
	if(L < LONGEST_PATH && L <= U && L <= D)
	{
		if(map.buf[lLifter.y][lLifter.x-1] == 'W')
			return 'S';
		else
			return 'L';
		}
	if(U < LONGEST_PATH && U <= D)
	{
		if(map.buf[lLifter.y-1][lLifter.x] == 'W')
			return 'S';
		else
			return 'U';
		}
	if(D < LONGEST_PATH)
	{
		if(map.buf[lLifter.y+1][lLifter.x] == 'W')
			return 'S';
		else
			return 'D';
		}
		
	/* if robot still can't move check for falling rocks */
	for(y = 0; y < map.y_size; y++)
		for(x = 0; x < map.x_size; x++)
		{
			if(map.buf[y][x] == '*')
			{	/* unsupported rocks fall, wait to see if it opens a path */
				if(map.buf[y+1][x] == ' ')
					return 'W';
				/* balanced rocks slide, wait to see if it opens a path */
				else if((map.buf[y+1][x] == '*' && map.buf[y][x+1] == ' ' && map.buf[y+1][x+1] == ' ') ||
					(map.buf[y+1][x] == '*' && map.buf[y][x-1] == ' ' && map.buf[y+1][x-1] == ' ') ||
					(map.buf[y+1][x] == '\\' && map.buf[y][x+1] == ' ' && map.buf[y+1][x+1] == ' '))
					return 'W';
				
			}
		}
	
	/* if map is stable, and there is nowhere to go, abort */
	return 'A';
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
    int score = calc_abort_score();
    /* if score is positive (or would be with the bonus), send Abort command */
    fprintf(stderr,"checking for last second abort\n");
    fprintf(stderr,"current score: %d score if abort: %d\n",lLifter.lambdas *25 - lLifter.steps, score);
    if (calc_abort_score() > 0) {
        fprintf(stderr,"sending abort instruction\n");
        putchar('A');
        fprintf(stderr,"score: %d \n",calc_abort_score());
		fflush(stdout);
        exit(EXIT_SUCCESS);
    }
    else {
        fprintf(stderr,"score too low, not aborting\n");
        sleep(2); /* use up the clock */
        exit(EXIT_FAILURE);
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

int main(int argc,char **argv) {
    char move;
    FILE *infile;

    signal(SIGINT,sig_handler);
    signal(SIGALRM,sig_handler);

    /* interactive mode */
    if ((argc == 3) && !strcmp(argv[2],"-i")) {
        fprintf(stderr,"starting interactive mode\n");
        infile=fopen(argv[1],"r");
        read_map(infile);
        while (lLifter.steps < (map.x_size * map.y_size)) {
            print_map();
            move=move_robot();
            fprintf(stderr,"about to execute move: %c\n",move);
            fprintf(stderr,"press enter:\n");
            getchar();            
            if(move == 'A')
            {
				fprintf(stderr," score: %d \n",calc_abort_score());
				exit(EXIT_SUCCESS);
			}
            if (update_map(move)==-1) {
                fprintf(stderr," robot broken\n");
                exit (EXIT_FAILURE);
            }
        }
        fprintf(stderr," robot exceeded maximum number of moves\n");
        exit(EXIT_FAILURE);
    } else if (argc==1) {
        read_map(stdin);
        while (lLifter.steps < (map.x_size * map.y_size)) {
            move=move_robot();
            putchar(move);
            fflush(stdout);
            if(move == 'A')
            {
				fprintf(stderr," score: %d \n",calc_abort_score());
				exit(EXIT_SUCCESS);
			}
            else if (update_map(move)==-1) {
                fprintf(stderr," robot broken\n");
                exit (EXIT_FAILURE);
            }
        }
        fprintf(stderr," robot exceeded maximum number of moves\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr,"usage: %s\n%s <file> -i\n",argv[0],argv[0]);
        exit (EXIT_FAILURE);
    }



    return EXIT_SUCCESS;
}
