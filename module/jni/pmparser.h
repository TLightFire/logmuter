#ifndef PMPARSER_H
#define PMPARSER_H

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#define PROCMAPS_LINE_MAX_LENGTH 1024
#define RBUF 1024

#ifdef __cplusplus
extern "C" {
#endif

typedef struct procmaps_struct {
    void *addr_start; // start address of the area
    void *addr_end;   // end address
    size_t length;    // length of the area
    char perm[5];     // permissions: r w x p
    int is_r;
    int is_w;
    int is_x;
    int is_p;
    off_t offset;     // offset into the file
    char dev[10];     // device major:minor
    ino_t inode;      // inode number
    char pathname[PATH_MAX]; // pathname
    struct procmaps_struct *next;
} procmaps_struct;

typedef struct procmaps_iterator {
    procmaps_struct *head;
    procmaps_struct *current;
} procmaps_iterator;

procmaps_iterator* pmparser_parse(int pid);
procmaps_struct* pmparser_next(procmaps_iterator* p_procmaps_it);
void pmparser_free(procmaps_iterator* p_procmaps_it);
void pmparser_print(procmaps_struct* map, int order);

#ifdef __cplusplus
}
#endif

#endif
