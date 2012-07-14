#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	if (max_len < strlen(line))
		max_len=strlen(line);
    }

    w.buf=buf;
    w.y_size=line_no;
    w.x_size=max_len;
    return w;
}

int main()
{
    world w;
    w=read_map();

    fprintf(stderr,"x: %d y: %d\n",w.x_size,w.y_size);

    return EXIT_SUCCESS;
}
