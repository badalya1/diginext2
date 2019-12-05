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
#include <time.h>

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
	if (go_to(&dir, NULL, name_node->name)==0)
	{
		if (!is_fmode_type(dir->i_mode, EXT2_S_IFDIR))
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

		//Create a new directory 
		printf("Creating directory\n");
		unsigned int new_dir_inode_index;
		new_dir_inode_index = get_available_inode(); //the index of the next available inode
		if (new_dir_inode_index==-1){
			print_error(-1, NULL, "Ran out of inodes.");
			return -1;
		}
		//Lock Inode Table
		dir = inode_table + (new_dir_inode_index);
		mark_inode_used(new_dir_inode_index);
		//Unlock Inode Table
		//Lock specific Inode
		dir->i_mode = EXT2_S_IFDIR;
		dir->i_uid = 0;
		dir->i_atime = (unsigned int)time(NULL);
		dir->i_ctime = (unsigned int)time(NULL);
		dir->i_mtime = (unsigned int)time(NULL);
		dir->i_gid = 0;
		dir->i_links_count = 0;
		dir->i_blocks = 0;
		for(int i = 0; i < 15; i++){
			dir->i_block[i] = 0; 
		}
		dir->osd1 = 0;
		dir->i_generation = 0;
		dir->i_file_acl = 0;
		dir->i_dir_acl = 0;
		dir->i_faddr = 0;

		dir_entry_t dir_entry;
		dir_entry.inode = new_dir_inode_index+1;
		dir_entry.name_len = strlen(name_node->name);
		dir_entry.file_type = EXT2_FT_DIR;
		dir_entry.rec_len = get_rec_len(dir_entry.name_len);

		insert_dir_entry(&dir_entry, walker, name_node->name);

		dir_entry_t self;
		self.inode = new_dir_inode_index+1;
		self.name_len = 1;
		self.file_type = EXT2_FT_DIR;
		self.rec_len = get_rec_len(dir_entry.name_len);
		insert_dir_entry(&self, dir, ".");
		

		dir_entry_t parent;
		parent.inode = walker_inode;
		parent.name_len = 2;
		parent.file_type = EXT2_FT_DIR;
		parent.rec_len = get_rec_len(dir_entry.name_len);
		insert_dir_entry(&parent, dir, "..");

		free_bc(bc);
		free(name_node);
		return 0;
	}