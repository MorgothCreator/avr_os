# avr_os V0.1

# The project has been moved here https://git.morgothdisk.com

8bit avr atmega and atxmega OS.

This OS is intended to work with 8 bit AVR microcontrollers like Atmega and ATxmega.

At this moment contain a minimal RAMFS with:

	1) fs_init - Can create a new partition with a given name, 
		this will help in case of untrusted applications.

	2) fs_open - Can fallow a given path, can create directoryes, 
		files, structures and links.

	3) fs_find_item - find an item with a given name on current directory.

	4) fs_recursive_item_delete - will delete all subdirectorys, files, 
		structures and links into a given directory.

	5) fs_item_register - will register a directory, file, 
		structure or link with a given name into current directory.


A minimal function to change the tasks.
