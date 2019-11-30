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

int32_t ext2_fsal_mkdir(const char *path) {

    if (path[0] != '/') { // path is not absolute
        print_error(ENOENT,path,"Path must be absolute.");
        return ENOENT;
    }

    if (strlen(path)==1) { // path is just root so dir already exists
        print_error(EEXIST,path,NULL);
        return EEXIST;
    }
    
    bc_t * bc = extract_path(path); //turn path into a breadcrumb

    bc_node_t* name_node = bc_pop(bc); //extract out the name

    inode_t* walker = inode_table + EXT2_ROOT_INO-1; // start at root
    
    for (bc_node_t* guide = bc->start; guide!=NULL; guide = guide->next){
        int result = go_to(&walker, guide->name);
        printf("__________________________\n");
        if (result){
            print_error(result,guide->name,"Could not navigate");
            return result;
        }
    }

    printf("Creating directory\n");



    /*
    struct ext2_inode root_inode = (inode_table[EXT2_ROOT_INO - 1]); // root inode entry (-1 for index)
    int dir_entry_block = root_inode.i_block[0];
    
    bc_node_t curr = *bc->start;
    int i = 0;
    while (i < bc->count) {
        //if (root_inode.i_blocks == 1) { // only implementation for first block
            struct ext2_dir_entry * dir_entries = (struct ext2_dir_entry *)(disk + dir_entry_block * EXT2_BLOCK_SIZE);

            int found = 0;
            int offset = 0;
            while (offset < EXT2_BLOCK_SIZE) { // loop through same level directory entries

                if (strlen(curr.name) == dir_entries->name_len) { 
	                if (strncmp(curr.name, dir_entries->name, strlen(curr.name)) == 0) {
	                    found = 1;
	                    break;
	                }
            	}
                offset = offset + dir_entries->rec_len;
                dir_entries = (struct ext2_dir_entry*)((disk + dir_entry_block * EXT2_BLOCK_SIZE) + offset);

            }

            if (!found) {
                if (i < bc->count - 1) { // not last entry of path (because that one obviously doesnt exist)
                    fprintf(stderr, "[usage error] Directory '%s' in the path, doesnt exists.\n", curr.name);
                    free_bc_nodes_linked_list(bc);
                    return ENOENT;
                } else { // last entry
                    //create directory 
                    printf("[success] Reached end of path successfully, would create '%s' here.\n", curr.name);
                }
            } else {
                if (get_directory_entry_file_type(dir_entries->file_type) == 'd') { // check if file is a directory
                    if (i != bc->count -1) { // check if last element of path
	                    struct ext2_inode next_path_inode = (inode_table[dir_entries->inode - 1]);
	                    dir_entry_block = next_path_inode.i_block[0];
	                } else {
	                	fprintf(stderr, "[usage error] Directory '%s' already exists.\n", curr.name);
	                	free_bc_nodes_linked_list(bc);
                    	return ENOENT;
	                }
                } else {
                    fprintf(stderr, "[usage error] File '%s' already exists, and is not a directory.\n", curr.name);
                    free_bc_nodes_linked_list(bc);
                    return ENOENT;
                }
            }
            
        //}
        i++;
        if (curr.next != NULL) {
        	curr = *(curr.next);
        }
    }  */
 
    printf("Next available inode: %d\n",get_available_inode());
    return 0;
}