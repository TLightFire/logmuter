/*
 * proc maps parser
 *
 * This software is under the MIT license
 *
 * Copyright (c) 2012-2016, Andrea Borruso <andrea@borruso.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include "pmparser.h"

/**
 * Internal function to split a line from /proc/pid/maps
 * */
static void _pmparser_split_line(
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
    char tmp[1024];
    while(*buf != ' ') {
        tmp[i] = *buf;
        buf++;
        i++;
    }
    tmp[i]='\0';
    *offset = strtoul(tmp, NULL, 16);
    //dev
    i=0;
    while(*buf != ' ') {
        tmp[i] = *buf;
        buf++;
        i++;
    }
    tmp[i]='\0';
    //now parse dev:
    unsigned int maj, min;
    if(sscanf(tmp, "%x:%x", &maj, &min) == 2) {
        *dev = makedev(maj, min);
    } else {
        *dev = 0;
    }
    //inode
    i=0;
    while(*buf != ' ') {
        tmp[i] = *buf;
        buf++;
        i++;
    }
    tmp[i]='\0';
    *inode = strtoul(tmp, NULL, 10);
    //pathname
    if(*buf == ' ') buf++;
    *pathname = buf;
    return;
}

/**
 * proc maps parser: parse the maps file of the pid
 * */
procmaps_iterator* pmparser_parse(int pid) {
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
        return NULL;
    }
    // read line by line
    char buf[1024];
    procmaps_iterator *maps = NULL;
    while(fgets(buf, 1024, f)) {
        // parse the line
        unsigned long addr_start, addr_end, offset;
        char perm[5], *pathname;
        dev_t dev;
        ino_t inode;
        _pmparser_split_line(buf, &addr_start, &addr_end, perm, &offset, &dev, &inode, &pathname);
        // add to the list
        procmaps_iterator *tmp = malloc(sizeof(procmaps_iterator));
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
    }
    fclose(f);
    return maps;
}

/**
 * Free the list
 * */
void pmparser_free(procmaps_iterator *maps) {
    procmaps_iterator *tmp;
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
    procmaps_iterator *maps = pmparser_parse(pid);
    if(!maps) {
        perror("pmparser_parse");
        return -1;
    }
    procmaps_iterator *tmp = maps;
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
