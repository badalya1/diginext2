/*
 *------------
 * This code is provided solely for the personal and private use of
 * students taking the CSC369H5 course at the University of Toronto.
 * Copying for purposes other than this use is expressly prohibited.
 * All forms of distribution of this code, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2019 MCS @ UTM
 * -------------
 */

#ifndef CSC369_E2FS_H
#define CSC369_E2FS_H



#include "ext2.h"
#include <string.h>

/**
 * TODO: add in here prototypes for any helpers you might need.
 * Implement the helpers in e2fs.c
 */

 // .....
typedef struct ext2_inode inode_t;
typedef struct ext2_dir_entry dir_entry_t;


unsigned char* disk;
struct ext2_super_block* sb;
struct ext2_group_desc* g;
unsigned char* inode_bitmap;
unsigned char* block_bitmap;
inode_t* inode_table;


//BreadCrumb types to easly parse out paths
typedef struct breadcrumb {
    struct breadcrumb_node* start;
    struct breadcrumb_node* end;
    int count;
} bc_t;

typedef struct breadcrumb_node {
    char name[EXT2_NAME_LEN+1];
    struct breadcrumb_node* next;
} bc_node_t;

//Functions for BreadCrumbs
bc_t* extract_path(const char*);
int add_bc_node(bc_t*,char *);

void free_bc(bc_t* bc);

/**
 * Pops and returns the last element from a breadcrumb
 */
bc_node_t * bc_pop(bc_t*);


/**
 * Return the pointer to the inode corresponding to inode_index
 */
inode_t* get_inode(int inode_index);

/**
 * Return next available inode index. Return -1 if all inodes are taken
 */
int get_available_inode();

int go_to(inode_t **inode, char* dirname);

void print_error(int errnum, const char* origin,const char* extra);
#endif