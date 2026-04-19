#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "pmparser.h"

/**
 * Internal function to split a line from /proc/pid/maps
 * */
void _pmparser_split_line(
        char*buf,
        unsigned long *addr_start,
        unsigned long *addr_end,
        char *perm,
        unsigned long *offset,
        dev_t *dev,
        ino_t *inode,
        char **pathname) {
    // line example:
    // 08048000-0804c000 r-xp 00000000 08:01 01395214 /lib/ld-uClibc.so.0
    //
    int i=0, orig=0;
    //addr_start
    while(*buf != '-') {
        perm[i] = *buf;
        buf++;
        i++;
    }
    perm[i] = '\0';
    *addr_start = strtoul(perm, NULL, 16);
    buf++;
    //addr_end
    i=0;
    while(*buf != ' ') {
        perm[i] = *buf;
        buf++;
        i++;
    }
    perm[i] = '\0';
    *addr_end = strtoul(perm, NULL, 16);
    //perm
    i=0;
    while(*buf != ' ') {
        perm[i] = *buf;
        buf++;
        i++;
    }
    perm[i] = '\0';
    //offset
    i=0;
    while(*buf != ' ') {
        dev[i] = *buf;
        buf++;
        i++;
    }
    dev[i]='\0';
    *offset = strtoul(dev, NULL, 16);
    //dev
    i=0;
    while(*buf != ' ') {
        dev[i] = *buf;
        buf++;
        i++;
    }
    dev[i]='\0';
    //now parse dev:
    unsigned int maj, min;
    if(sscanf(dev, "%x:%x", &maj, &min) == 2) {
        *dev = makedev(maj, min);
    } else {
        *dev = 0;
    }
    //inode
    i=0;
    while(*buf != ' ') {
        pathname[i] = *buf;
        buf++;
        i++;
    }
    pathname[i]='\0';
    *inode = strtoul(pathname, NULL, 10);
    //pathname
    if(*buf == ' ') buf++;
    *pathname = buf;
    return;
}

/**
 * proc maps parser: parse the maps file of the pid
 * */
int pmparser_parse(pid_t pid, struct pmparser **procmaps) {
    // open the file
    char maps_path[512];
    if(pid == 0) {
        sprintf(maps_path, "/proc/self/maps");
    } else {
        sprintf(maps_path, "/proc/%d/maps", pid);
    }
    FILE *f = fopen(maps_path, "r");
    if(!f) {
        perror("fopen");
        return -1;
    }
    // read line by line
    char buf[1024];
    int n = 0;
    struct pmparser *maps = NULL;
    while(fgets(buf, 1024, f)) {
        // parse the line
        unsigned long addr_start, addr_end, offset;
        char perm[5], pathname[1024];
        dev_t dev;
        ino_t inode;
        _pmparser_split_line(buf, &addr_start, &addr_end, perm, &offset, &dev, &inode, &pathname);
        // add to the list
        struct pmparser *tmp = malloc(sizeof(struct pmparser));
        tmp->addr_start = addr_start;
        tmp->addr_end = addr_end;
        strncpy(tmp->perm, perm, 4);
        tmp->perm[4] = '\0';
        tmp->offset = offset;
        tmp->dev = dev;
        tmp->inode = inode;
        tmp->pathname = strdup(pathname);
        tmp->next = maps;
        maps = tmp;
        n++;
    }
    fclose(f);
    *procmaps = maps;
    return n;
}

/**
 * Free the list
 * */
void pmparser_free(struct pmparser *maps) {
    struct pmparser *tmp;
    while(maps) {
        tmp = maps;
        maps = maps->next;
        free(tmp->pathname);
        free(tmp);
    }
}

#ifdef PM_PARSER_DEBUG
int main(int argc, char *argv[]) {
    pid_t pid = atoi(argv[1]);
    struct pmparser *maps;
    int n = pmparser_parse(pid, &maps);
    printf("n: %d\n", n);
    struct pmparser *tmp = maps;
    while(tmp) {
        printf("0x%lx-0x%lx %s 0x%lx %d:%d %lu %s\n",
                tmp->addr_start,
                tmp->addr_end,
                tmp->perm,
                tmp->offset,
                major(tmp->dev),
                minor(tmp->dev),
                tmp->inode,
                tmp->pathname);
        tmp = tmp->next;
    }
    pmparser_free(maps);
    return 0;
}
#endif
