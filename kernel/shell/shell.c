#include <stdint.h>
#include <drivers/keyboard/ps2.h>
#include <drivers/video/videoText.h>
#include <lib/function.h>
#include <lib/string/string.h>
#include <gui/gui.h>



#define INPUTMAX 	1024
char  input[INPUTMAX];
uint32_t cursor = 0;

void execute(char *command){
	// break command and arguments up ()
	print((char*)command); print("\n");
}

void shell_key_hook(uint8_t character){	
	if (character == 0xE){
		// backspace
		cursor--;
		input[cursor] = 0;
		lineClear(1);

		
	} else if (character == 0xA){
		// enter
		// execute and clear
		execute(input);
		input[0] = 0;
		cursor   = 0;
	} else {
		print_letter(character);
		input[cursor] = character;
		cursor++;
	}
}

void init_shell(){
	// get keyboard hooks
	register_keyboard_hook(shell_key_hook);
	memset((void*)input, 0, INPUTMAX*sizeof(uint8_t));

}