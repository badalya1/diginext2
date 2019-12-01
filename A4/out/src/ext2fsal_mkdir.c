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
#include <stdint.h>

int32_t ext2_fsal_mkdir(const char *path)
{

    if (path[0] != '/')
    { // path is not absolute
        print_error(ENOENT, path, "Path must be absolute.");
        return ENOENT;
    }

    if (strlen(path) == 1)
    { // path is just root so dir already exists
        print_error(EEXIST, path, NULL);
        return EEXIST;
    }

    bc_t *bc = extract_path(path); //turn path into a breadcrumb

    bc_node_t *name_node = bc_pop(bc); //extract out the name

    inode_t *walker = inode_table + EXT2_ROOT_INO - 1; // start at root

    for (bc_node_t *guide = bc->start; guide != NULL; guide = guide->next)
    {
        int result = go_to(&walker, guide->name);
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
    if (go_to(&dir, name_node->name)==0)
    {
        if (~is_fmode_type(dir->i_mode, EXT2_S_IFDIR))
        {
            print_error(EEXIST, name_node->name, NULL);
            return EEXIST;
        }
        else
        {
            print_error(ENOENT, name_node->name, "A file exists with that name.");
            return ENOENT;
        }
        }

        printf("Creating directory\n");

        printf("Next available inode: %d\n", get_available_inode());
        return 0;
    }