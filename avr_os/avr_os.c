/*
 * avr_os.c
 *
 * Created: 5/10/2015 9:48:28 PM
 *  Author: eu
 */ 


#include <avr/io.h>
#include "avr_os.h"
#include "os.h"
#include "ram_fs.h"



int main(void)
{
	os_init();
	fs_init("fs_1", 0);
	
	avr_fs *dj;
	fs_open(&dj, "A/salut/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	//fs_item_delete(&dj);
	fs_open(&dj, "A/salut1/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut2/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut3/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut2/insalut2/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	//fs_item_delete(&dj);
	fs_open(&dj, "A/salut3/insalut3/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut3/insalut4/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut3/insalut3/insalut33/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut3/insalut4/insalut340/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut3/insalut4/insalut341/", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_DIR, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/salut3/insalut4/file1", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_FILE, 0, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 50);
	fs_open(&dj, "A/salut3/insalut4/port_str1", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_STRUCT, (unsigned int)&PORTA, AVR_FS_FILE_ATTR_ACCES_WRITABLE, sizeof(PORT_t));
	fs_open(&dj, "A/salut3/insalut4/port_str2", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_STRUCT, (unsigned int)&PORTA, AVR_FS_FILE_ATTR_ACCES_WRITABLE, sizeof(PORT_t));
	fs_open(&dj, "A/salut3/insalut4/port_str3", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_STRUCT, (unsigned int)&PORTA, AVR_FS_FILE_ATTR_ACCES_WRITABLE, sizeof(PORT_t));
	fs_open(&dj, "A/salut3/insalut4/link", FA_CREATE_ALWAYS, AVR_FS_FILE_ATTR_TYPE_LINK, (unsigned int)&PORTB, AVR_FS_FILE_ATTR_ACCES_WRITABLE, 0);
	fs_open(&dj, "A/", FA_OPEN_EXISTING, AVR_FS_FILE_ATTR_TYPE_EMPTY, 0, AVR_FS_FILE_ATTR_ACCES_READONLY, 0);
	fs_recursive_item_delete(&dj);
	while(1)
    {
		asm("wdr");
    }
}