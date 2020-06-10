#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ntfs_read.h>

#define DUMP_PATH "d:\\Dump flash\\dev2_lba0_3100000.bin"

int main()
{

    uint8_t *file_buffer = NULL;
    uint16_t file_data_buffer[4096];
    uint8_t boot_sector[1024];
    uint64_t lcn_root_buffer[1024];
    uint16_t root_filename_buffer[4096];
    uint64_t * ptr = NULL;
    file_buffer = malloc(0x400);
    FILE *f;
    if((f = fopen(DUMP_PATH,"rb")) == NULL)
    {
        printf("Fail to open file");
        return 0xF;

    }
    Ntfs_read_filerecords(f,boot_sector,file_buffer,file_data_buffer,lcn_root_buffer);
    Ntfs_read_indexrecords(f,file_buffer,root_filename_buffer,lcn_root_buffer);
    Ntfs_read_filerecords(f,boot_sector,file_buffer,file_data_buffer,lcn_root_buffer);
    Ntfs_read_indexrecords(f,file_buffer,root_filename_buffer,lcn_root_buffer);
    Ntfs_read_filerecords(f,boot_sector,file_buffer,file_data_buffer,lcn_root_buffer);
    Ntfs_read_indexrecords(f,file_buffer,root_filename_buffer,lcn_root_buffer);
    Print_root_files(root_filename_buffer,sizeof(root_filename_buffer) / 2); //Информация только о имени файлов/директорий в корне
    Print_files(file_data_buffer,sizeof(file_data_buffer));

    fclose(f);
    free(file_buffer);

    return 0;
}
