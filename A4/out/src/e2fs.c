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

void mark_inode_used(int n)
{
	unsigned char *byte = inode_bitmap + n / 8;
	*byte |= (1 << (n % 8));
}

void mark_inode_unused(int n){
	unsigned char *byte = inode_bitmap + n / 8;
	*byte &= ~(1 << (n % 8));
}

int get_available_block()
{
	unsigned char *byte;
	for (int i = 0; i < sb->s_blocks_count; i++)
	{
		byte = block_bitmap + i / 8;
		if (~(*byte >> (i % 8)) & 1)
		{
			return i;
		}
	}
	return -1;
}

void mark_block_used(int n)
{
	unsigned char *byte = block_bitmap + n / 8;
	*byte |= (1 << (n % 8));
}

void mark_block_unused(int n)
{
	unsigned char *byte = block_bitmap + n / 8;
	*byte &= ~(1 << (n % 8));
}
int get_rec_len(int name_len)
{
	int total = sizeof(dir_entry_t) + name_len;
	if (total % 4 != 0)
	{
		total += 4 - (total % 4);
	}
	return total;
}

int go_to(inode_t **inode_pt,unsigned int* inode_number, char *name)
{
	inode_t *inode = *inode_pt;
	dir_entry_t *entry;
	int name_len = strlen(name);
	for (int i = 0; i < 12; i++)
	{
		int dir_entry_block_index = inode->i_block[i];

		if (dir_entry_block_index == 0)
		{
			break;
		}

		unsigned int marker = 0;
		while (marker < EXT2_BLOCK_SIZE)
		{
			entry = (dir_entry_t *)(disk + dir_entry_block_index * EXT2_BLOCK_SIZE + marker);
			if (entry->inode != 0 && name_len == entry->name_len && strncmp(name, entry->name, name_len) == 0)
			{
				*inode_pt = inode_table + (entry->inode - 1);
				if(inode_number!=NULL){
				*inode_number = entry->inode;
				}
				return 0;
			}
			marker += entry->rec_len;
		}
	}

	return ENOENT;
}

int insert_dir_entry(dir_entry_t *entry, inode_t *inode, const char *name)
{
	int which_block = ((inode->i_blocks + 1) / 2) - 1; //Which i_block is last
	int dir_entry_block_index;
	dir_entry_t *last_entry;
	int last_entry_size;
	dir_entry_t *new_entry;
	unsigned int marker;
	int backtrack;

	if (which_block == -1)
	{
		dir_entry_block_index = get_available_block()+1;
		if (dir_entry_block_index==0){
				print_error(-1, name, "A new block is required but all blocks are taken");
				return -1;
			}
		inode->i_block[which_block+1] = dir_entry_block_index;
		inode->i_blocks+=2;
		inode->i_size+=EXT2_BLOCK_SIZE;
		mark_block_used(dir_entry_block_index-1);
		marker = 0;
		backtrack = EXT2_BLOCK_SIZE;
	}
	else
	{
		dir_entry_block_index = inode->i_block[which_block];

		for (marker = 0; marker < EXT2_BLOCK_SIZE; marker += last_entry->rec_len)
		{
			last_entry = (dir_entry_t *)(disk + dir_entry_block_index * EXT2_BLOCK_SIZE + marker);
		}

		last_entry_size = get_rec_len(last_entry->name_len); //Actual record size

		if (last_entry->rec_len - last_entry_size < entry->rec_len)
		{ //If current block is full, make a new block
			dir_entry_block_index = get_available_block();
			if (dir_entry_block_index==-1){
				print_error(-1, name, "A new block is required but all blocks are taken");
				return -1;
			}
			inode->i_block[which_block+1] = dir_entry_block_index;
			inode->i_blocks+=2;
			inode->i_size+=EXT2_BLOCK_SIZE;
			mark_block_used(dir_entry_block_index);
			marker = 0;
			backtrack = EXT2_BLOCK_SIZE;
		}
		else
		{
			backtrack = last_entry->rec_len - last_entry_size;
			marker -= backtrack;
			last_entry->rec_len = last_entry_size;
		}
	}

	new_entry = (dir_entry_t *)(disk + dir_entry_block_index * EXT2_BLOCK_SIZE + marker);
	*new_entry = *entry;
	memcpy(new_entry->name, name, entry->name_len);
	new_entry->rec_len = backtrack;

	inode->i_links_count += 1;

	return 0;
}

int is_fmode_type(unsigned short mode, unsigned short type)
{
	return mode >> 12 == type >> 12 ? 0 : 1;
}

void print_error(int errnum, const char *origin, const char *extra)
{
	if (origin)
	{
		fprintf(stderr, "%s :", origin);
	}
	switch (errnum)
	{
	case ENOENT:
		fprintf(stderr, " No such directory entry. ");
		break;
	case EEXIST:
		fprintf(stderr, " Entry already exists. ");
		break;
	case EISDIR:
		fprintf(stderr, " Expected a file, but got a directory. ");
		break;
	default:
		fprintf(stderr, " unknown error. ");
	}
	if (extra)
	{
		fprintf(stderr, "%s. ", extra);
	}
	fprintf(stderr, "\n");
}