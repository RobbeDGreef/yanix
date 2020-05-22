#include <stdint.h>
#include <stddef.h>
#include <mm/heap.h>

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
int memcmp(const void* apointer, const void* bpointer, size_t size)
{
	const uint8_t* a = (const uint8_t*) apointer;
	const uint8_t* b = (const uint8_t*) bpointer;
	for (size_t i = 0; i < size; i++){
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}


/**
 * @brief      Copies memory from one location to another
 *
 * @param      destpointer    Pointer to the destination memory block
 * @param[in]  sourcepointer  Pointer to the source memory block
 * @param[in]  size           The amount of bytes to copy
 *
 * @return     returns pointer to the destination memory block
 */
void *memcpy(void* restrict destpointer, const void* restrict sourcepointer, size_t size)
{
	
	// this is an optimized version of memcpy made for 32 bit words

	size_t dwords  = size/4;
    size_t n_bytes = size%4;

    uint32_t *destWord   = (uint32_t*) destpointer;
    uint32_t *sourceWord = (uint32_t*) sourcepointer;

    uint8_t *dest8   = ((uint8_t*)destpointer) + dwords*4;
    uint8_t *source8 = ((uint8_t*)sourcepointer) + dwords*4;

    size_t i;
    for (i = 0; i < dwords; i++){
        destWord[i] = sourceWord[i];
    }

    for (i = 0; i < n_bytes; i++){
        dest8[i] = source8[i];
    }
    return destpointer;
}

/**
 * @brief      moves memory from one location to another
 *
 * @param      destpointer    Pointer to the destination memory block
 * @param[in]  sourcepointer  Pointer to the source memory block
 * @param[in]  size           The amount of bytes to copy
 *
 * @return     returns pointer to the destination memory block
 */
void *memmove(void* destpointer, const void* sourcepointer, size_t size)
{
	uint8_t* dst = (uint8_t*) destpointer;
	const uint8_t* src = (uint8_t*) sourcepointer;
	if (dst < src) {
		for(size_t i = 0; i < size; i++){
			dst[i] = src[i];
		}
	} else {
		for(size_t i = size; i != 0; i--){
			dst[i-1] = src[i-1];
		}
	}
	return destpointer;
}


/**
 * @brief      Sets all bytes in a memory block to given value
 *
 * @param      destpointer  Pointer to the destination memory block
 * @param[in]  value        The value to set the bytes to
 * @param[in]  size         The amount of bytes to copy
 *
 * @return     returns pointer to the destination memory block
 */
void *memset(void* destpointer, int value, size_t size)
{
	/* @todo: optimize this function better than this hack */
	if (value == 0)
	{
		uint32_t *dst = (uint32_t*) destpointer;
		for (size_t i = 0; i < size / sizeof(uint32_t); i++)
		{
			dst[i] = 0;
		}
		return destpointer;
	}
	uint8_t* dst = (uint8_t*)destpointer;
	for (size_t i = 0; i < size; i++){
		dst[i] = (uint8_t) value;
	}
	return destpointer;
}


/**
 * @brief      Gets the length of a c sytle string (\0 terminated)
 *
 * @param[in]  str   The string
 *
 * @return     The length of the string
 */
size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len] != 0)
		len++;
	return len;
}


/**
 * @brief      Inverts a given c style string
 *
 * @param      str   The string to reverse
 */
void reverse(char* str)
{
	int i = 0;
	int j = strlen(str)-1;
	char b;
	for (; i < j; i++, j--){
		b = str[i];
		str[i] = str[j];
		str[j] = b;
	}
}


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
size_t append(char* str, char character)
{
	size_t l = strlen(str);
	str[l] = character;
	l++;
	str[l] = 0;
	return l;
}


/**
 * @brief      Removes given amount of bytes from end of a string
 *
 * @param      str     The string
 * @param[in]  length  The amount of bytes to remove
 *
 * @return     The new length of the string
 */
int remove(char* str, size_t length)
{
	size_t l = strlen(str);
	if (((int)l - (int)length) < 0){
		return -1;
	}
	l -= length;
	str[l] = 0;
	return l;
}


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
int insert(char* str, size_t location, char character)
{
	size_t l = strlen(str);
	if (l < location){
		return -1;
	}
	for (size_t i = l; i != location; i--){
		str[i+1] = str[i];
	}
	str[location] = character;
	return l+1;
}

/**
 * @brief      Displays a given integer in hexadecimal format
 *
 * @param[in]  integer  The integer to display
 * @param      str      The buffer to add the character to
 */
void hex_to_ascii(int integer, char* str)
{
	str[0] = '\0';
    append(str, '0'); 
    append(str, 'x');
    
    char noZeros = 1;
    signed int tmp;

    for (int i = 28; i > 0; i -= 4){
        tmp = (integer >> i) & 0xF;
        
        if ((tmp == 0) && (noZeros != 0)){
            continue;
        } 
        
        if (tmp >= 0xA){
            noZeros = 0;
            append(str, tmp - 0xA + 'A');
        }
        else{
            noZeros = 0;
            append(str, tmp + '0');
        }
    }
    tmp = integer & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'A');
    else append(str, tmp + '0');
}

void hex_to_ascii_no_Ox(int integer, char* str)
{
	str[0] = '\0';
    //char noZeros = 1;
    signed int tmp;

    for (int i = 28; i > 0; i -= 4){
        tmp = (integer >> i) & 0xF;
        if (tmp >= 0xA){
      //      noZeros = 0;
            append(str, tmp - 0xA + 'a');
        } else{
        //    noZeros = 0;
            append(str, tmp + '0');
        }
    }
    tmp = integer & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
    else append(str, tmp + '0');

}


/**
 * @brief      Searches for the first match.
 *
 * @param      str        The string
 * @param[in]  character  The character to search for
 *
 * @return     The location of the character (returns -1 when character is not found)
 */
int find(char* str, char character)
{
	size_t l = strlen(str);
	for (size_t i = 0; i < l; i++){
		if (str[i] == character){
			return i;
		}
	}
	return -1;
}


/**
 * @brief      Displays given integer in decimal format
 *
 * @param[in]  n     The integer
 * @param      str   The buffer
 */
void int_to_string(int n, char* str)
{
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n/=10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';
    reverse(str);
}

int strcmp(const char *x, const char *y)
{
	while (*x)
	{
		if (*x != *y)
			break;

		x++;
		y++;
	}

	return *(const unsigned char *) x - *(const unsigned char *) y;
}

char *strdup_user(const char *str)
{
	int len = strlen(str) + 1;
	char *newstr = kmalloc_user(len);
	memcpy(newstr, str, len);
	return newstr;
}