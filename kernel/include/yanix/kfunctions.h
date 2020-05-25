#ifndef KFUNCTIONS_H
#define KFUNCTIONS_H


void message(char *message, char code);
void tick_sleep(unsigned int amount);
void sleep(unsigned int milliseconds);
void askKeyboardHook(void (*keyhook)(char key));
void shutdown(int errorcode);

char *int_to_str(unsigned int val);
#endif