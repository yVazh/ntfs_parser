#include "ntfs_read.h"

#include <string.h>
#include <stdbool.h>

void Print_root_files(uint16_t *dataBuffer, uint16_t sizeOfDataBuffer)
{
    uint32_t tempIndex = 0;
    for(uint32_t i = 0; i < sizeOfDataBuffer; i++)
    {

        switch(dataBuffer[i])
        {
            case 0x3F:
            {
                for(uint32_t k = tempIndex; k < i ; k++)
                {
                    printf("%c",dataBuffer[k]);
                }
                if(i - tempIndex > 0) printf("\n");
                tempIndex = i + 1;
                break;
            }
            default:break;
        }
    }
    printf("End of root\n\n\n");
}

void Print_files(uint16_t *databuffer, uint16_t sizeOfDataBuffer)
{
    uint32_t tempIndex = 0;
    for(uint32_t i = 0; i < sizeOfDataBuffer; i++)
    {

        switch(databuffer[i])
        {
            case 0x3F:
            {
                FileBufferStructure* fileBufferStructure = (FileBufferStructure*)&databuffer[i+1];
                for(uint32_t k = tempIndex; k < i ; k++)
                {
                    printf("%c",databuffer[k]);
                }
                printf("\tSectorNumber: %d\n",fileBufferStructure->SectorNumber);
                tempIndex = i + 11;
                break;
            }

        }
    }
    printf("End of parsing\n");
}

uint8_t Ntfs_read_filerecords(FILE* f,uint8_t* boot_sector, uint8_t* file_buffer, uint16_t* file_data_buffer, uint64_t* lcn_root_buffer)
{
    uint64_t fileRecordNumber = 0;
    uint64_t file_buffer_offset     = 0;
    uint64_t data_buffer_offset     = 0;
    uint64_t lcn_file_number        = 0;
    bool indexRootFlag = false;
    bool indexAllocationFlag = false;

    fseek(f,65536,SEEK_SET);
    fread(boot_sector,sizeof(uint8_t),1024,f);
    BootSector* bootSector = (BootSector*)(boot_sector);
    fseek(f,0x40010000 + 0x0 * 0x400,SEEK_SET); // Начало MFT файла (bootSector->BytesPerSector * bootSector->SectorsPerCluster * bootSector->MftStart)
    uint64_t filerecordSector = (((bootSector->MftStart + 0x10) << 12) / 0x200); //Стартовый сектор MFT зоны
    while(true)
    {
        uint32_t deltaOffset = 0;
        fread(file_buffer,sizeof(uint8_t),1024,f);
        FileRecordHeader* fileRecord = (FileRecordHeader *) (file_buffer);
        if(fileRecord->MagicWord == FILE_WORD)
        {
//            if(!(fileRecord->Flags & 0x01))
//            {
//                continue;
//            }
            if(fileRecord->MFTRecordNumber != 0x05)
            {
                continue;
            }
            else
            {
                file_buffer_offset = fileRecord->FirstAttrOffset;
                while(file_buffer_offset != 1024)
                {
                    AttributeHeader* attrHead = (AttributeHeader* )(&file_buffer[file_buffer_offset]);

                    switch(attrHead->AttributeType)
                    {
                        case FILE_NAME: //0x30
                        {
                            ResidentAttributeHeader* residAttrHead =  (ResidentAttributeHeader*)(&file_buffer[file_buffer_offset]);
                            FilenameBody* filenameBody = (FilenameBody*)(&file_buffer[file_buffer_offset + (sizeof(ResidentAttributeHeader) + 2 * residAttrHead->Header.NameLength)]);
                            break;
                        }
                        case INDEX_ROOT: //0x90
                        {
                            indexRootFlag = true;
                            ResidentAttributeHeader* residAttrHead = (ResidentAttributeHeader*)(&file_buffer[file_buffer_offset]);
                            deltaOffset = residAttrHead->AttributeOffset + sizeof(IndexRootHeader);
                            IndexRootHeader* indexRootHeader = (IndexRootHeader*)(&file_buffer[file_buffer_offset + residAttrHead->AttributeOffset]);

                            do
                            {
                                IndexEntry*  indexEntry   = (IndexEntry*)(&(file_buffer[file_buffer_offset + deltaOffset]));
                                FilenameBody* filenameBody = (FilenameBody*)(&file_buffer[file_buffer_offset + deltaOffset + sizeof(IndexEntry)]);
//                                root_filename_buffer
                                deltaOffset += indexEntry->IndexLength;

                            }while(deltaOffset < residAttrHead->AttributeLength);

                            break;
                        }
                        case INDEX_ALLOCATION: //0xA0
                        {
                            indexAllocationFlag = true;
                            NonResidentAttributeHeader* nonresidAttrHead = (NonResidentAttributeHeader*)(&file_buffer[file_buffer_offset]);
                            DataRuns* dataRuns = (DataRuns*)(&file_buffer[file_buffer_offset + nonresidAttrHead->DataRunsOffset]);
                            uint16_t offset = 0;
                            do
                            {
                                memcpy(&(lcn_root_buffer[lcn_file_number++]),&(dataRuns->Data),(dataRuns->LengthSize));
                                memcpy(&(lcn_root_buffer[lcn_file_number++]),&(file_buffer[file_buffer_offset + nonresidAttrHead->DataRunsOffset + sizeof(uint8_t) + offset + dataRuns->LengthSize]),(dataRuns->OffsetSize));
                                offset += dataRuns->LengthSize + dataRuns->OffsetSize + 1;
                                dataRuns = (DataRuns*)(&file_buffer[file_buffer_offset + nonresidAttrHead->DataRunsOffset + offset]);
                            }while((dataRuns->LengthSize != 0x0)&&(dataRuns->OffsetSize != 0x0));
                            break;
                        }
                        case STANDART_INFOMATION: //0x10
                        {
                            break;
                        }
                        case OBJECT_ID: //0x40
                        {
                            break;
                        }
                        case DATA: //0x80
                        {
                            if(!(attrHead->NonResidentFlag & 0x1))
                            {
                                ResidentAttributeHeader* residAttrHead = (ResidentAttributeHeader*)(&file_buffer[file_buffer_offset]);
                            }
                            else
                            {
                                NonResidentAttributeHeader* nonresidAttrHead = (NonResidentAttributeHeader*)(&file_buffer[file_buffer_offset]);
                            }
                            break;
                        }

                        default:
                        {

                            break;
                        }


                    }
                    if (attrHead->AttributeType == END_OF_FILERECORD)
                    {
                        file_buffer_offset = 1024;
                    }
                    else
                    {
                        file_buffer_offset += attrHead->Length;
                    }
                }
                break;
            }
        }
    }
    free(file_buffer);
    return 0x0;
}

void Ntfs_read_indexrecords(FILE* f,uint8_t* file_buffer,uint16_t* root_filename_buffer, uint64_t* lcn_root_buffer)
{
    uint64_t index_filename_offset = 0;
    uint64_t root_filename_buffer_index = 0;
    uint64_t indexRecordNumber = 0;
    uint64_t temp_lcn = 0x10;
    uint32_t lcn_index = 1;
    while(lcn_root_buffer[lcn_index] != 0)
    {
        file_buffer = malloc(0x1000 * lcn_root_buffer[lcn_index - 1]);
        fseek(f,(lcn_root_buffer[lcn_index] + temp_lcn) << 12,SEEK_SET);
        fread(file_buffer,sizeof(uint8_t),0x1000 * lcn_root_buffer[lcn_index - 1],f);

        IndexRecordBody* indexRecordBody = (IndexRecordBody*)(file_buffer);
        index_filename_offset = indexRecordBody->FirstIndexEntryOffset;

        do
        {
            IndexEntry*     indexEntry     = (IndexEntry*)(&file_buffer[index_filename_offset + 0x18]);
            if(indexEntry->Flags1 & 0x2)
                break;
            FilenameBody* filenameBody = (FilenameBody*)(&file_buffer[index_filename_offset + 0x18 + sizeof(IndexEntry)]);
            if(filenameBody->FilenameNamespace != 0x2)
            {
                    memcpy(&(root_filename_buffer[root_filename_buffer_index]),&(filenameBody->FilenameData),2 * filenameBody->FilenameLength);
                    root_filename_buffer_index += filenameBody->FilenameLength;
                    root_filename_buffer[root_filename_buffer_index++] = 0x3F;
            }
            index_filename_offset += indexEntry->IndexLength;
            indexRecordNumber++;

        }while(true);
        temp_lcn += lcn_root_buffer[lcn_index];
        lcn_index += 2;
        free(file_buffer);
    }
}
