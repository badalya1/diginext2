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

#include "ext2fsal.h"
#include "e2fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int32_t ext2_fsal_rm(const char *path)
{
    if (path[0] != '/')
    { // path is not absolute
        print_error(ENOENT, path, "Path must be absolute.");
        return ENOENT;
    }

    if (strlen(path) == 1)
    {
        print_error(EISDIR, path, NULL);
        return EISDIR;
    }

    bc_t *bc = extract_path(path); //turn path into a breadcrumb

    bc_node_t *file_node = bc_pop(bc); //extract out the file

    unsigned int walker_inode = EXT2_ROOT_INO;
    inode_t *walker = inode_table + walker_inode - 1;

    for (bc_node_t *guide = bc->start; guide != NULL; guide = guide->next)
    {
        int result = go_to(&walker, &walker_inode, guide->name);
        if (result)
        {
            print_error(result, guide->name, NULL);
            return result;
        }
        else if (is_fmode_type(walker->i_mode, EXT2_S_IFDIR))
        {
            print_error(ENOENT, guide->name, "Not a directory");
            return ENOENT;
        }
    }

    inode_t *dir = walker;
    if (go_to(&dir, &walker_inode, file_node->name))
    { //File doesnt exist
        print_error(ENOENT, file_node->name, NULL);
        return ENOENT;
    }

    if (!is_fmode_type(dir->i_mode, EXT2_S_IFDIR))
    {
        print_error(EISDIR, path, NULL);
        return EISDIR;
    }

    dir->i_links_count -= 1;
    if (dir->i_links_count <= 0)
    {
        //free the blocks and set i_dtime
        int block_num;
        for (int i = 0; i < 12; i++)
        {
            block_num = dir->i_block[i];
            if (block_num != 0)
            {
                mark_block_unused(block_num);
            }
        }
        dir->i_dtime = (unsigned int)time(NULL);
        mark_inode_unused(walker_inode-1);
    }

    //Remove Direntry from walker
    dir_entry_t *entry = NULL;
    dir_entry_t *prev_entry;
    int name_len = strlen(file_node->name);
    for (int i = 0; i < 12; i++)
    {
        int dir_entry_block_index = walker->i_block[i];

        if (dir_entry_block_index == 0)
        {
            break;
        }

        unsigned int marker = 0;
        while (marker < EXT2_BLOCK_SIZE)
        {
            prev_entry = entry;
            entry = (dir_entry_t *)(disk + dir_entry_block_index * EXT2_BLOCK_SIZE + marker);
            if (entry->inode != 0 && name_len == entry->name_len && strncmp(file_node->name, entry->name, name_len) == 0)
            {
                break;
            }
            marker += entry->rec_len;
        }
        prev_entry->rec_len += entry->rec_len;
    }

    free_bc(bc);
    free(file_node);
    return 0;
}