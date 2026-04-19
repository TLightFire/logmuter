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
#ifndef PM_PARSER_H
#define PM_PARSER_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct procmaps_struct {
    unsigned long addr_start;  // start address of the memory region
    unsigned long addr_end;    // end address of the memory region
    char perm[5];              // permissions: r-xp
    unsigned long offset;      // offset
    dev_t dev;                 // device
    ino_t inode;               // inode
    char *pathname;            // pathname of the library
    struct procmaps_struct *next;
};

typedef struct procmaps_struct procmaps_iterator;

/**
 * Parse the maps file of the pid
 * @param pid the pid to parse, 0 for self
 * @return the iterator to the maps list
 */
procmaps_iterator* pmparser_parse(int pid);

/**
 * Free the maps list
 * @param p_procmaps_it the iterator to free
 */
void pmparser_free(procmaps_iterator* p_procmaps_it);

#ifdef __cplusplus
}
#endif

#endif
