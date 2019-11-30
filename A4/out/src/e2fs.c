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

/**
 * TODO: Make sure to add all necessary includes here.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "e2fs.h"

int add_bc_node(bc_t *bc, char *name)
{

    int name_len = strlen(name);

    //Add error checking here
    bc_node_t *node = malloc(sizeof(bc_node_t));

    strncpy(node->name, name, name_len);
    node->name[name_len] = '\0';
    node->next = NULL;

    if (bc->start == NULL)
    {
        bc->start = node;
        bc->end = node;
    }
    else
    {
        bc->end->next = node;
        bc->end = node;
    }
    return 0;
}

bc_t *extract_path(const char *path)
{
    int size = strlen(path);
    char path_str[size + 1];

    strncpy(path_str, path, size);
    path_str[size] = '\0';

    bc_t *bc = malloc(sizeof(bc_t));
    bc->start = NULL;
    bc->end = NULL;
    bc->count = 0;

    char *rest = path_str;
    char *name;

    while ((name = strtok_r(rest, "/", &rest)))
    {
        //Add error checking here
        add_bc_node(bc, name);
    }

    return bc;
}

void free_bc(bc_t *bc)
{

    bc_node_t *first = bc->start;
    bc_node_t *next;
    while (first != NULL)
    {
        next = first->next;
        free(first);
        first = next;
    }
    free(bc);
}

bc_node_t *bc_pop(bc_t *bc)
{
    bc_node_t *node = bc->start;
    bc_node_t *last = bc->end;
    if (last == NULL)
    {
        return NULL;
    }
    if (node == last)
    {
        bc->start = NULL;
        bc->end = NULL;
    }
    else
    {
        for (; node != NULL && node->next != last; node = node->next)
            ;

        node->next = NULL;
    }
    return last;
}

struct ext2_inode *get_inode(int inode_index)
{
    return inode_table + inode_index;
}

int get_available_inode()
{
    unsigned char *byte;
    for (int i = EXT2_GOOD_OLD_FIRST_INO; i < sb->s_inodes_count; i++)
    {
        byte = inode_bitmap + i / 8;
        if (~(*byte >> (i % 8)) & 1)
        {
            return i;
        }
    }
return -1;
}

/**
 * Navigates i to point to the directory named dirname, inside the directory of i.
 * 
 * *i must point to an inode of a directory.
 * 
 * Returns 0 on success, ENOENT if dirname does not exist or is not a directory.
 */
int go_to(inode_t **inode_pt, char* dirname)
{
    inode_t* inode = *inode_pt;
    dir_entry_t* entry;
    int dirname_length = strlen(dirname);
    for (int i = 0; i < 1; i++)
    {
        int dir_entry_block_index = inode->i_block[i];
        

        unsigned int marker = 0;
        while (marker<EXT2_BLOCK_SIZE){
            entry = (dir_entry_t *)(disk + dir_entry_block_index * EXT2_BLOCK_SIZE + marker);
            printf("%d: %.*s              %d\n", entry->inode,entry->name_len, entry->name, entry->file_type);
            if (dirname_length==entry->name_len && strncmp(dirname,entry->name,dirname_length)==0){
                if (entry->file_type == EXT2_FT_DIR){
                    *inode_pt =  inode_table + (entry->inode -1);
                    return 0;
                }else{
                    return ENOENT;
                }
            }
            marker+= entry->rec_len;
        }

    }
    
    

    return ENOENT;
}

void print_error(int errnum, const char *origin, const char *extra)
{
    if (origin)
    {
        fprintf(stderr, "%s", origin);
    }
    switch (errnum)
    {
    case ENOENT:
        fprintf(stderr, ": No such file or directory. ");
        break;
    case EEXIST:
        fprintf(stderr, " already exists. ");
        break;
    default:
        fprintf(stderr, ": unknown error. ");
    }
    if (extra)
    {
        fprintf(stderr, "%s. \n", extra);
    }
}