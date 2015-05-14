/*
 * ram_fs.h
 *
 * Created: 5/11/2015 1:17:33 PM
 *  Author: eu
 */ 

#include "avr_os.h"

#ifndef RAM_FS_H_
#define RAM_FS_H_

#ifndef RAM_FS_FILENAME_MAX_LEN
#define RAM_FS_FILENAME_MAX_LEN	9
#endif

#ifndef RAM_FS_DRIVES_MAX
#define RAM_FS_DRIVES_MAX	8
#endif

/* File function return code (FRESULT) */

typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) Working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */
	FR_INVALID_PARAMETER,	/* (19) Given parameter is invalid */
	FR_DIR_NOT_EMPTY		/* (20) The directory is not empty */
} FRESULT;

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#define FA__ERROR			0x80


#define AVR_FS_FILE_ATTR_TYPE_gp			0
#define AVR_FS_FILE_ATTR_TYPE_gm			(0x0F << AVR_FS_FILE_ATTR_TYPE_gp)

#define AVR_FS_FILE_ATTR_TYPE_EMPTY			(0 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry is empty */
#define AVR_FS_FILE_ATTR_TYPE_FILE			(1 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry points to a regular file */
#define AVR_FS_FILE_ATTR_TYPE_STRUCT		(2 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry points to a structure file, that is already allocated (like a link but with size) */
#define AVR_FS_FILE_ATTR_TYPE_STRUCT_FILE	(3 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry points to a structure file , hat will be created*/
#define AVR_FS_FILE_ATTR_TYPE_FS			(4 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry point to a file system structure */
#define AVR_FS_FILE_ATTR_TYPE_DIR			(8 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry point to a directory */
#define AVR_FS_FILE_ATTR_TYPE_LINK			(9 << AVR_FS_FILE_ATTR_TYPE_gp) /* The entry is a link to another entry */

#define AVR_FS_FILE_ATTR_ACCES_gp			4
#define AVR_FS_FILE_ATTR_ACCES_gm			(0x03 << AVR_FS_FILE_ATTR_ACCES_gp)

#define AVR_FS_FILE_ATTR_ACCES_WRITABLE		(0 << AVR_FS_FILE_ATTR_ACCES_gp) /* The entry is empty */
#define AVR_FS_FILE_ATTR_ACCES_READONLY		(1 << AVR_FS_FILE_ATTR_ACCES_gp) /* The entry is empty */
#define AVR_FS_FILE_ATTR_ACCES_EXECUTABLE	(2 << AVR_FS_FILE_ATTR_ACCES_gp) /* The entry is empty */
#define AVR_FS_FILE_ATTR_ACCES_SYSTEM		(3 << AVR_FS_FILE_ATTR_ACCES_gp) /* The entry is empty */


typedef struct  
{
	unsigned int parent_entry_offset;
	unsigned int file_size;
	unsigned char *data;
	unsigned char file_attr;
	char name[RAM_FS_FILENAME_MAX_LEN];
}avr_fs;

FRESULT fs_item_register (
	avr_fs **dj,			/* Pointer to the directory object. */
	const char *path,		/* Pointer to the file name. */
	unsigned char attr,		/* Attribute for new entry. */
	unsigned int addr,		/* If is a link or a structure here is the address. */
	unsigned int size		/* If is a file, this contain the initial size of the file, if is a link this contain the item address. */
);
FRESULT fs_item_delete (
	avr_fs **dj		/* Pointer to the directory/file object. */
);
FRESULT fs_recursive_item_delete (
	avr_fs **dj		/* Pointer to the directory/file object. */
);
FRESULT fs_item_find (
	avr_fs **dj,			/* Pointer to the directory object. */
	const char *path		/* Pointer to the file name. */
);
FRESULT fs_follow_path (	/* FR_OK(0): successful, !=0: error code. */
	avr_fs **dj,			/* Directory object to return last directory and found object. */
	const char *path,		/* Full-path string to find a file or directory. */
	unsigned char mode,		/* Access mode and file open mode flags. */
	unsigned char type,		/* If is a file, structure or fs structure here is the type. . */
	unsigned int addr,		/* If is a link here is the address. */
	unsigned char permision,/* here is the permision. */
	unsigned int filesize	/* If is a file, structure or fs structure here is the size. */
);
FRESULT fs_open (
	avr_fs **fp,		/* Pointer to the blank file object. */
	const char *path,	/* Pointer to the file name. */
	unsigned char mode,	/* Access mode and file open mode flags. */
	unsigned char type,		/* If is a file, structure or fs structure here is the type. . */
	unsigned int addr,		/* If is a link here is the address. */
	unsigned char permision,/* here is the permision. */
	unsigned int filesize	/* If is a file, structure or fs structure here is the size. */
);
FRESULT fs_init(
	char *fs_name, /* File system name. */
	int drive/* Drive nr to mount. */
);

extern avr_fs *ram_fs_root[];

#endif /* RAM_FS_H_ */