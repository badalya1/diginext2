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

/**
 * Set n'th  bit to one on the inode bitmap. 
 */
void mark_inode_used(int n);

/**
 * Return next available block index. Return -1 if all blocks are taken
 */
int get_available_block();

/**
 * Set n'th  bit to one on the block bitmap. 
 */
void mark_block_used(int n);

/**
 * Return the record length in bytes given the name_len 
 */
int get_rec_len(int name_len);

/**
 * *inode_pt must point to an inode of a directory.
 * 
 * Returns 0 on success, ENOENT if name does not exist.
 */
int go_to(inode_t **inode_pt,unsigned int* inode_number,  char* name);

/**
 * Insert directory entries into the first available block.
 * Return 0 on sucess, -1 otherwise
 */
int insert_dir_entry(dir_entry_t* entry, inode_t* inode, const char* name);

/**
 * The type indicator occupies the top hex digit (bits 15 to 12) of `mode`s 16-bit field
 * Return 0 if `mode` is of type `type`, 1 otherwise.
 */
int is_fmode_type(unsigned short mode, unsigned short type);

void print_error(int errnum, const char* origin,const char* extra);
#endif