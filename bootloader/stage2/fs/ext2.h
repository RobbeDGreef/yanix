#ifndef EXT2_H
#define EXT2_H

/**
 * @brief      Loads the kernel file from a ext2 filesystem.
 *
 * @param      kernel_name  The kernel name
 * @param[in]  startlba     The startlba
 * @param[in]  total_lba    The total lba
 * @param[in]  disk         The disk
 *
 * @return     Whether the kernel loaded successfully or not
 */
int load_ext2_kernel(char *kernel_name, unsigned int startlba, unsigned int total_lba, unsigned int disk, unsigned int blocksize_phys);


#endif