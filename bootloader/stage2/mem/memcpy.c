#include <types.h>

/**
 * @brief      Non optimized memcpy
 *
 * @param      destptr  The destptr
 * @param[in]  srcptr   The srcptr
 * @param[in]  size     The size
 *
 * @return     { description_of_the_return_value }
 */
void *memcpy(void *destptr, const void *srcptr, size_t size)
{
	unsigned char *dst 		 = (unsigned char *) destptr;
	const unsigned char *src = (const unsigned char *) srcptr;

	for (size_t i = 0; i < size; i++)
	{
		dst[i] = src[i];
	}
	return destptr;
}
