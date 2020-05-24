#ifndef STRING_H
#define STRING_H

//#include <sys/cdefs.h>
#include <stddef.h>

// standard library

/**
 * @brief      Compairs memory from two given locations
 *
 * @param[in]  apointer  The first pointer to memory
 * @param[in]  bpointer  The second pointer to memory
 * @param[in]  size      The amount of bytes to compair
 *
 * @return     returns 0 when the two memory blocks are the same,
 * 			   -1 when the first diferent byte is smaller in the first pointer than the byte in the second pointer
 * 			   1 when the first diferent byte is larger in the first pointer than the byte in the second pointer
 */
int    memcmp(const void*, const void*, size_t);

/**
 * @brief      Copies memory from one location to another
 *
 * @param      destpointer    Pointer to the destination memory block
 * @param[in]  sourcepointer  Pointer to the source memory block
 * @param[in]  size           The amount of bytes to copy
 *
 * @return     returns pointer to the destination memory block
 */
void*  memcpy(void* __restrict, const void* __restrict, size_t);

/**
 * @brief      moves memory from one location to another
 *
 * @param      destpointer    Pointer to the destination memory block
 * @param[in]  sourcepointer  Pointer to the source memory block
 * @param[in]  size           The amount of bytes to copy
 *
 * @return     returns pointer to the destination memory block
 */
void*  memmove(void*, const void*, size_t);

/**
 * @brief      Sets all bytes in a memory block to given value
 *
 * @param      destpointer  Pointer to the destination memory block
 * @param[in]  value        The value to set the bytes to
 * @param[in]  size         The amount of bytes to copy
 *
 * @return     returns pointer to the destination memory block
 */
void*  memset(void*, int, size_t);

/**
 * @brief      Gets the length of a c sytle string (\0 terminated)
 *
 * @param[in]  str   The string
 *
 * @return     The length of the string
 */
size_t strlen(const char*);

// extra functions


/**
 * @brief      Inverts a given c style string
 *
 * @param      str   The string to reverse
 */
void reverse(char*);

/**
 * @brief      Appends character after c style string 
 * 
 * @note 	   The string should still have free memory at the end (1 byte)
 *
 * @param      str        The string
 * @param[in]  character  The character to append
 *
 * @return     The new length of the string 
 */
size_t append(char*, char);

/**
 * @brief      Removes given amount of bytes from end of a string
 *
 * @param      str     The string
 * @param[in]  length  The amount of bytes to remove
 *
 * @return     The new length of the string
 */

int remove(char*, size_t);

/**
 * @brief      Inserts a character at given location
 * 
 * @note       The string should have free memory at the end (1 byte)
 *
 * @param      str        The string 
 * @param[in]  location   The location to insert
 * @param[in]  character  The character to insert
 *
 * @return     The new length of the string
 */
int insert(char*, size_t, char);

/**
 * @brief      Displays a given integer in hexadecimal format
 *
 * @param[in]  integer  The integer to display
 * @param      str      The buffer to add the character to
 */
void hex_to_ascii(int, char*);
void hex_to_ascii_no_Ox(int integer, char* str);

/**
 * @brief      Searches for the first match.
 *
 * @param      str        The string
 * @param[in]  character  The character to search for
 *
 * @return     The location of the character (returns -1 when character is not found)
 */
int find(char*);

/**
 * @brief      Displays given integer in decimal format
 *
 * @param[in]  n     The integer
 * @param      str   The buffer
 */
void int_to_string(int, char*);

int strcmp(const char *x, const char *y);
char *strdup_user(const char *str);
char *strdup(const char *str);
char *strchr(const char *str, char c);
char *strdup_s(const char *mem, int len);
char *strchr_r(const char *str, char c);

#endif