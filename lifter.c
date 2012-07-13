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

    FILE *f;
    f=fopen("contest7.map","r");

    while (getline(&line,&n,f) != -1){
        /* replace newline with null */
        line[strchr(line,'\n')-line]='\0';
        printf ("line: \"%s\"\n",line);
        buf=realloc(buf,(1+line_no) * sizeof (char*));
        buf[line_no]=line;
        printf("buf[%d]: \"%s\"\n",line_no,buf[line_no]);
        line_no++;
    }

    w.buf=buf;
    w.y_size=line_no;
    return w;
}

int main()
{
    int y;
    world w;
    w=read_map();

    printf("x: %d y: %d\n",w.x_size,w.y_size);
    for (y=0;y<w.y_size;y++)
        printf("w.buf[%d]: \"%s\"\n",y,w.buf[y]);

    return EXIT_SUCCESS;
}
