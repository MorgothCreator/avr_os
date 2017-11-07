/*
 * ram_fs.c
 *
 * Created: 5/11/2015 1:17:23 PM
 *  Author: eu
 */ 

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "ram_fs.h"
#include "avr_os.h"

volatile bool ram_fs_lock = false;
volatile unsigned int ram_fs_modifyed = 0;
avr_fs *ram_fs_root[RAM_FS_DRIVES_MAX] = {NULL};
bool system_permision_request = false;





/*-----------------------------------------------------------------------*/
/* Create a file/directory in current directory                          */
/*-----------------------------------------------------------------------*/

FRESULT fs_item_register (
	avr_fs **dj,			/* Pointer to the directory object. */
	const char *path,		/* Pointer to the file name. */
	unsigned char attr,		/* Attribute for new entry. */
	unsigned int addr,		/* If is a link or a structure here is the address. */
	unsigned int size		/* If is a file, this contain the initial size of the file, if is a link this contain the item address. */
)
{
	if(strlen(path) > RAM_FS_FILENAME_MAX_LEN)
		return FR_INVALID_NAME;
	if(ram_fs_lock)
		return FR_LOCKED;
	ram_fs_lock = true;/* Lock the ram_fs, to avoid interference with another modification done by another thread. */
	avr_fs *new_dir = (avr_fs *)addr;/* Write the linked item address, if is a link, this will be used for new entry. */
	if((attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_LINK && 
		(attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_STRUCT)/* If is a link or a structure no new memory is allocated. */
	{
		if((attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_DIR || (attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR)
		{
			new_dir = calloc(1, sizeof(avr_fs));/* Allocate memory for new directory. */
		} 
		else
		{
			new_dir = calloc(1, size);/* Allocate memory for new file. */
		}
	}
	if(!new_dir)
	{
		ram_fs_lock = false;/* Unlock the ram_fs */
		return FR_NOT_ENOUGH_CORE;
	}
	avr_fs *actual_dir = *dj;
	/* Search an old allocated entry that now is empty */
	avr_fs *cur_dj_search_entry = actual_dir + 1;/* Skip write actual directory address to the first entry of current directory, because point to parent directory. */
	bool empty_entry_discovered = false;
	for (; cur_dj_search_entry < (actual_dir->file_size / sizeof(avr_fs)) + actual_dir; cur_dj_search_entry += 1)/* Check if is a old entry allocated but empty. */
	{
		if((cur_dj_search_entry->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_EMPTY)
		{
			empty_entry_discovered = true;
			break;
		}
	}/* If no empty already allocated entry is not found, relocate the current directory with the new size and actualize the references to actual directory*/
	if(!empty_entry_discovered)
	{
		avr_fs *actual_dir_back = actual_dir;
		actual_dir = realloc(actual_dir, actual_dir->file_size + sizeof(avr_fs));/* Reallocate memory for current directory, with one entry more memory.*/
#if (RAM_FS_DEBUG == 1)
		if(actual_dir_back != actual_dir)
			memset(actual_dir_back, 0, actual_dir->file_size);
#endif
		if(!actual_dir)
		{
			free(new_dir);
			ram_fs_lock = false;/* Unlock the ram_fs */
			return FR_NOT_ENOUGH_CORE;
		}
		unsigned char search_drive = 0;
		for(; search_drive < RAM_FS_DRIVES_MAX; search_drive++)
		{
			if(*dj == ram_fs_root[search_drive])
			{
				ram_fs_root[search_drive] = actual_dir;/* The root directory has been relocated, actualize the global root directory address. */
				actual_dir->data = (unsigned char *)actual_dir;/* If is root, need to actualize the root directory address, because the first entry will point to himself. */
			}
		}
		avr_fs *parent_entry = (avr_fs *)(actual_dir->parent_entry_offset + actual_dir->data);/* Calculate parent entry address, if is root directory the parent will be himself. */
		ram_fs_modifyed++;/* This is a semaphore that will modify the value every time the ram_fs will be modified. */
		parent_entry->data = (unsigned char *)actual_dir;/* Write the new address of actual directory to parent entry. */
		avr_fs *cur_dj = actual_dir + 1;/* Skip write actual dir address to the first entry of current directory, because point to parent directory. */
		for (; cur_dj < (actual_dir->file_size / sizeof(avr_fs)) + actual_dir; cur_dj += 1)/* Write the new address of actual directory to all children directory's of current directory. */
		{
			unsigned char file_attr = cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm;
			if(file_attr != AVR_FS_FILE_ATTR_TYPE_FILE && file_attr != AVR_FS_FILE_ATTR_TYPE_STRUCT)
			{
				avr_fs *pointed_dj = (avr_fs *)cur_dj->data;
				pointed_dj->data = (unsigned char *)actual_dir;
			}
		
		}
	}
	if((attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_DIR || 
		(attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR)
	{
		new_dir->data = (unsigned char *)actual_dir;/* Write on new directory the address of actual directory. */
		new_dir->parent_entry_offset = actual_dir->file_size;/* Write on new directory the offset address of parent entry. */
		new_dir->file_size = sizeof(avr_fs);/* Write on new directory the size of new directory. */
		new_dir->file_attr = AVR_FS_FILE_ATTR_TYPE_DIR;/* Write on new directory attribute of new directory. */
#if (RAM_FS_DEBUG == 1)
		memset(new_dir->name, 0, RAM_FS_FILENAME_MAX_LEN);
#endif
		strcpy(new_dir->name, "..");/* Write on new directory name of parent directory. */
	}
	avr_fs *new_entry = (avr_fs *)actual_dir + (actual_dir->file_size / sizeof(avr_fs));/* Calculate the position of new entry on current directory. */
	if(empty_entry_discovered)
	{/* In case of an already allocated entry that is empty. */
		new_entry = cur_dj_search_entry;/* New entry address is the first empty discovered entry. */
	}
	else
	{/* In case of A new entry has been allocated. */
		actual_dir->file_size = actual_dir->file_size + sizeof(avr_fs);/* Actualize the size of current directory. */
	}
#if (RAM_FS_DEBUG == 1)
	memset(new_entry->name, 0, RAM_FS_FILENAME_MAX_LEN);
#endif
	strncpy(new_entry->name, path, RAM_FS_FILENAME_MAX_LEN);/* Write the name of new directory. */
	new_entry->file_attr = attr;/* Write attribute of new directory. */
	new_entry->data = (unsigned char *)new_dir;/* Write the name of new item. */
	if((attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_DIR || 
		(attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR || 
			(attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_LINK)
	{
		new_entry->file_size = 0;/* This is a directory or a link, the size is 0. */
	} 
	else 
	{
		new_entry->file_size = size;/* This is a file, write the size. */
	}
	*dj = new_entry;/* Return new created entry address */
	ram_fs_lock = false;/* Unlock the ram_fs */
	return FR_OK;
}





/*-----------------------------------------------------------------------*/
/* Delete a pointed file/directory                                       */
/*-----------------------------------------------------------------------*/

FRESULT fs_item_delete (
avr_fs **dj		/* Pointer to the directory/file object. */
)
{
	avr_fs *entry_to_delete = *dj;
	if(((entry_to_delete->file_attr & AVR_FS_FILE_ATTR_ACCES_gm) == AVR_FS_FILE_ATTR_ACCES_READONLY ||
		(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_ACCES_gm) == AVR_FS_FILE_ATTR_ACCES_SYSTEM ||
			(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR) && 
				system_permision_request == false)
	{
		return FR_WRITE_PROTECTED;
	}
	//if((entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_ROOT)
	//{
	//	return FR_DENIED;/* This is a root directory and cannot be deleted */
	//}
	avr_fs *file_to_delete = (avr_fs *)entry_to_delete->data;
	if((entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_DIR ||
		(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR)
	{
		/* This is a directory, delete current directory and pointed entry */
		avr_fs *cur_dj_search_entry = file_to_delete + 1;/* Skip first entry of actual directory. */
		for (; cur_dj_search_entry < (file_to_delete->file_size / sizeof(avr_fs)) + file_to_delete; cur_dj_search_entry += 1)/* Search if directory is not empty */
		{
			if((cur_dj_search_entry->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_EMPTY)
			{
				return FR_DIR_NOT_EMPTY;
			}
		}
	}
	//avr_fs *entry_to_delete = (avr_fs *)file_to_delete->data + (file_to_delete->parent_entry_offset / sizeof(avr_fs));
	if((entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_DIR ||
		(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR ||
			(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_FILE ||
				(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_STRUCT_FILE ||
					(entry_to_delete->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_ROOT)
	{
#if (RAM_FS_DEBUG == 1)
		memset(file_to_delete, 0, file_to_delete->file_size);
#endif
		free(file_to_delete);
	}
#if (RAM_FS_DEBUG == 1)
	memset(entry_to_delete, 0, sizeof(avr_fs));
#else
	entry_to_delete->file_attr &= ~AVR_FS_FILE_ATTR_TYPE_gm;
	entry_to_delete->data = 0;
#endif
	return FR_OK;
}





/*-----------------------------------------------------------------------*/
/* Delete a pointed file/directory                                       */
/*-----------------------------------------------------------------------*/

FRESULT fs_recursive_item_delete (
avr_fs **dj		/* Pointer to the directory/file object. */
)
{
	FRESULT res = FR_OK;
	avr_fs *tmp_dj = *dj;
	/* This is a directory, delete current directory and pointed entry */
	avr_fs *cur_dj_search_entry = *dj + 1;/* Skip first entry of actual directory. */
	for (; cur_dj_search_entry < (tmp_dj->file_size / sizeof(avr_fs)) + tmp_dj; cur_dj_search_entry += 1)/* Search if directory is not empty */
	{
		res = fs_item_delete(&cur_dj_search_entry);
		if(res != FR_OK)
		{
			avr_fs *_cur_dj_search_entry = (avr_fs *)cur_dj_search_entry->data;
			res = fs_recursive_item_delete(&_cur_dj_search_entry);
			if(res != FR_OK)
			{
				return res;
			}
			res = fs_item_delete(&cur_dj_search_entry);
			if(res != FR_OK)
			{
				return res;
			}
		}
	}
	return res;
}





/*-----------------------------------------------------------------------*/
/* Find a item entry name                                                */
/*-----------------------------------------------------------------------*/

FRESULT fs_item_find (
	avr_fs **dj,			/* Pointer to the directory object. */
	const char *path		/* Pointer to the file name. */
)
{
	avr_fs *cur_dj = *dj;
	avr_fs *ymp_cur_dj = *dj;
	for (;(avr_fs *)cur_dj < (ymp_cur_dj->file_size/sizeof(avr_fs)) + ymp_cur_dj; cur_dj += 1)
	{
		if(strncmp(path, ((avr_fs *)cur_dj)->name, RAM_FS_FILENAME_MAX_LEN) == 0 && (((avr_fs *)cur_dj)->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_EMPTY)
		{
			*dj = (avr_fs *)cur_dj;
			return FR_OK;
		}
	}
	return FR_NO_PATH;
}




/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

FRESULT fs_follow_path (	/* FR_OK(0): successful, !=0: error code. */
	avr_fs **dj,			/* Directory object to return last directory and found object. */
	const char *path,		/* Full-path string to find a file or directory. */
	unsigned char mode,		/* Access mode and file open mode flags. */
	unsigned char type,		/* If is a file, structure or fs structure here is the type. . */
	unsigned int addr,		/* If is a link here is the address. */
	unsigned char permision,/* here is the permision. */
	unsigned int filesize	/* If is a file, structure or fs structure here is the size. */
)
{
	FRESULT res = FR_OK;
	char *segment_start = malloc(strlen(path) + 1);
	char *segment_end;
	char *dir_back = segment_start;
	strcpy(segment_start, path);
#if (RAM_FS_DEBUG == 1)
	int size_of_tmp_str = strlen(dir_back);
#endif
	avr_fs *cur_dj = *dj;

	for (;;) {
		segment_end = strpbrk (segment_start, "/\\"); /* Find end of path segment. */
		if(!segment_end)
		{
			if(strlen(segment_start))
			{
				/* Find file */
				res = fs_item_find(&cur_dj, segment_start);
				if(res == FR_OK)
				{
					*dj = cur_dj;
				}
				else
				{
					if(mode == FA_CREATE_ALWAYS || mode == FA_OPEN_ALWAYS)
					{/* Verify if current directory is write protected */
						if(((cur_dj->file_attr & AVR_FS_FILE_ATTR_ACCES_gm) == AVR_FS_FILE_ATTR_ACCES_READONLY ||
							(cur_dj->file_attr & AVR_FS_FILE_ATTR_ACCES_gm) == AVR_FS_FILE_ATTR_ACCES_SYSTEM) &&
								system_permision_request == false)
						{
							res = FR_WRITE_PROTECTED;
						}
						else
						{
							if((cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_DIR &&
								(cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR)/* Verify if current entry is a directory entry */
							{
								if(dir_back) 
								{
#if (RAM_FS_DEBUG == 1)
									memset(dir_back, 0, size_of_tmp_str);
#endif
									free(dir_back);
								}
								return FR_NO_PATH;
							}
							//cur_dj = (avr_fs *)cur_dj->data;
							res = fs_item_register(&cur_dj, segment_start, (type & AVR_FS_FILE_ATTR_TYPE_gm) | (permision & AVR_FS_FILE_ATTR_ACCES_gm), addr, filesize);/* Create a non directory entry or file. */
							*dj = cur_dj;
						}
					}
				}
				if(dir_back)
				{
#if (RAM_FS_DEBUG == 1)
					memset(dir_back, 0, size_of_tmp_str);
#endif
					free(dir_back);
				}
				return res;
			} 
			else
			{
				if(dir_back)
				{
#if (RAM_FS_DEBUG == 1)
					memset(dir_back, 0, size_of_tmp_str);
#endif
					free(dir_back);
				}
				//*dj = cur_dj;
				*dj = (avr_fs *)cur_dj->data;
				return FR_OK;
			}
		}
		if((cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_FILE ||
			(cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_STRUCT ||
				(cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_FS)/* If a fs is available, this will be removed. */
		{
			if(dir_back)
			{
#if (RAM_FS_DEBUG == 1)
				memset(dir_back, 0, size_of_tmp_str);
#endif
				free(dir_back);
			}
			return FR_NO_PATH;
		}
		*segment_end = 0;
		if(!strcmp(segment_start, "..") && (cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) == AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR)
		{
			if(dir_back)
			{
#if (RAM_FS_DEBUG == 1)
				memset(dir_back, 0, size_of_tmp_str);
#endif
				free(dir_back);
			}
			*dj = cur_dj;
			return FR_NO_PATH;
		}
		/* Find directory */
		res = fs_item_find(&cur_dj, segment_start);
		if(res != FR_OK)
		{
			res = FR_NO_PATH;
			if(mode == FA_CREATE_ALWAYS || mode == FA_OPEN_ALWAYS)
			{
				if(((cur_dj->file_attr & AVR_FS_FILE_ATTR_ACCES_gm) == AVR_FS_FILE_ATTR_ACCES_READONLY ||
					(cur_dj->file_attr & AVR_FS_FILE_ATTR_ACCES_gm) == AVR_FS_FILE_ATTR_ACCES_SYSTEM) && 
						system_permision_request == false)
				{
					res = FR_WRITE_PROTECTED;
				}
				else
				{
					if((cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_DIR &&
						(cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_NOT_RET_DIR &&
							(cur_dj->file_attr & AVR_FS_FILE_ATTR_TYPE_gm) != AVR_FS_FILE_ATTR_TYPE_ROOT)/* If is not a directory or a root can't navigate from here. */
					{
						if(dir_back)
						{
#if (RAM_FS_DEBUG == 1)
							memset(dir_back, 0, size_of_tmp_str);
#endif
							free(dir_back);
						}
						return FR_NO_PATH;
					}
					res = fs_item_register(&cur_dj, segment_start, AVR_FS_FILE_ATTR_TYPE_DIR | (permision & AVR_FS_FILE_ATTR_ACCES_gm), addr, filesize);/* Create a directory. */
				}
			}
			if(res != FR_OK)
			{
				if(dir_back)
				{
#if (RAM_FS_DEBUG == 1)
					memset(dir_back, 0, size_of_tmp_str);
#endif
					free(dir_back);
				}
				return res;
			}
		}
		segment_start = segment_end + 1;
		if(!cur_dj->data)
		{
			if(dir_back)
			{
#if (RAM_FS_DEBUG == 1)
				memset(dir_back, 0, size_of_tmp_str);
#endif
				free(dir_back);
			}
			return FR_INVALID_OBJECT;
		}
		if(strlen(segment_start))
		{
			cur_dj = (avr_fs *)cur_dj->data;
		}
	}
	if(dir_back)
	{
#if (RAM_FS_DEBUG == 1)
		memset(dir_back, 0, size_of_tmp_str);
#endif
		free(dir_back);
	}
	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Get fs ptr                                                            */
/*-----------------------------------------------------------------------*/

avr_fs *fs_ptr_get (
const char *path	/* Pointer to the file name. */
)
{
	if(path[0] >= 'A' && path[0] <= 'Z' && (path[1] == '/' || path[1] == '\\') )
	{
		return ram_fs_root[path[0] - 'A'];
	}
	else
	{
		return NULL;
	}
}



/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT fs_open (
	avr_fs **fp,		/* Pointer to the blank file object. */
	const char *path,	/* Pointer to the file name. */
	unsigned char mode,	/* Access mode and file open mode flags. */
	unsigned char type,		/* If is a file, structure or fs structure here is the type. . */
	unsigned int addr,		/* If is a link here is the address. */
	unsigned char permision,/* here is the permision. */
	unsigned int filesize	/* If is a file, structure or fs structure here is the size. */
)
{
	mode &= FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW;
	if(path[0] >= 'A' && path[0] <= 'Z' && (path[1] == '/' || path[1] == '\\') )
	{
		*fp = ram_fs_root[path[0] - 'A'];
		path+=2;/* Strip it and start from the root directory */
	}
	return fs_follow_path(fp, path, mode, type, addr, permision, filesize);	/* Follow the file path. */
}





/*-----------------------------------------------------------------------*/
/* Delete file/directory with given path                                 */
/*-----------------------------------------------------------------------*/

FRESULT fs_delete (
	const char *path	/* Pointer to the file name. */
)
{
	avr_fs *dj;
	FRESULT res1 = FR_OK, res2 = FR_OK;

	res1 = fs_open(&dj, "A/", FA_OPEN_EXISTING, AVR_FS_FILE_ATTR_TYPE_EMPTY, 0, AVR_FS_FILE_ATTR_ACCES_READONLY, 0);
	/* if an error has occurred navigate to given path, return the error. */
	if(res1 != FR_OK)
	{
		return res1;
	}
	res1 = fs_recursive_item_delete(&dj);
	res2 = fs_item_delete(&dj);
	/* If given path has been deleted successfully return the recursive delete result */
	/* If given path delete has been denied signify that is the root path and cannot be deleted, return  the recursive delete result */
	if(res2 == FR_DENIED || res2 == FR_OK)
	{
		return res1;
	}
	/* if an error has occurred when try to delete given path, return that error. */
	return res2;
	
}


/*-----------------------------------------------------------------------*/
/* Fs initialize                                                 */
/*-----------------------------------------------------------------------*/

FRESULT fs_init(char *fs_name, int drive)
{
	if(ram_fs_root[drive])
	{
		return FR_MKFS_ABORTED;
	}
	avr_fs *ram_fs = calloc(1, sizeof(avr_fs));
	if (!ram_fs)
	{
		return FR_NOT_ENOUGH_CORE;
	} 
	ram_fs->data = (unsigned char *)ram_fs;
	ram_fs->parent_entry_offset = 0;
	ram_fs->file_attr = AVR_FS_FILE_ATTR_TYPE_ROOT;
	ram_fs->file_size = sizeof(avr_fs);
#if (RAM_FS_DEBUG == 1)
	memset(ram_fs->name, 0, RAM_FS_FILENAME_MAX_LEN);
#endif
	strcpy(ram_fs->name, fs_name);/* Write on new directory name of parent directory. */
	ram_fs_root[drive] = ram_fs;
	return FR_OK;
}
