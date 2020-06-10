#ifndef NTFS_READ_H
#define NTFS_READ_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FILE_WORD 0x454C4946
#define END_OF_FILERECORD 0xFFFFFFFF

enum AttributeNames
{
    STANDART_INFOMATION     = 0x10,
    ATTRIBUTE_LIST          = 0X20,
    FILE_NAME               = 0x30,
    VOLUME_VERSION          = 0X40,
    OBJECT_ID               = 0x40,
    SECURITY_DESCRIPTOR     = 0x50,
    VOLUME_NAME             = 0x60,
    VOLUME_INFORMATION      = 0x70,
    DATA                    = 0x80,
    INDEX_ROOT              = 0x90,
    INDEX_ALLOCATION        = 0xA0,
    BITMAP                  = 0xB0,
    SYMBOLIC_LINK           = 0xC0,
    REPARSE_POINT           = 0xC0,
    EA_INFORMATION          = 0xD0,
    EA                      = 0xE0,
    PROPERTY_SET            = 0xF0,
    LOGGED_UTILITY_STREAM   = 0x100,
};

#pragma pack(push,1)

typedef struct  {
    uint8_t     Jump[3];
    char        Name[8];
    uint16_t    BytesPerSector;         // Число байт в одном секторе
    uint8_t     SectorsPerCluster;      // Число секторов в кластере
    uint16_t    ReservedSectors;
    uint8_t     Unused0[5];
    uint8_t     Media;
    uint16_t    Unused1;
    uint16_t    SectorsPerTrack;
    uint16_t    HeadsPerCylinder;
    uint32_t    HiddenSectors;
    uint64_t    Unused2;
    uint64_t    TotalSectors;
    uint64_t    MftStart;               // Начало MTF файла (номер кластера)
    uint64_t    MftMirrorStart;
    uint32_t    ClustersPerFileRecord;
    uint32_t    ClustersPerIndexBlock;
    uint64_t    SerialNumber;
    uint32_t    Checksum;
    uint8_t     Bootloader[426];
    uint16_t    BootSignature;

}BootSector;

typedef struct {
    uint32_t    MagicWord;           //Начало файловой записи
    uint16_t    UpdateSequenceOffset;
    uint16_t    UpdateSequenceWordSize;
    uint64_t    SequnceLogNumber;
    uint16_t    SequenceNumber;
    uint16_t    HardLinkCount;          //Число записей каталога
    uint16_t    FirstAttrOffset;        //Смещение первого аттрибута записи
    uint16_t    Flags;                  //0x01 - Файл; 0x02 - Каталог
    uint32_t    FileRecordRealSize;
    uint32_t    FileRecordAllocateSize;
    uint64_t    FileReference;
    uint16_t    NextAttrId;             // Id следующего аттрибута, начальное значение 0
    uint16_t    Align;
    uint32_t    MFTRecordNumber;

}FileRecordHeader;

typedef struct  {
    uint32_t    AttributeType;
    uint32_t    Length;
    uint8_t     NonResidentFlag;
    uint8_t     NameLength;
    uint16_t    OffsetName;
    uint16_t    Flags;
    uint16_t    AttributeId;

}AttributeHeader;           //Общая часть заголовков резидентного/нерезидентного аттрибутов

typedef struct  {
    AttributeHeader Header;
    uint32_t        AttributeLength;
    uint16_t        AttributeOffset;
    uint8_t         IndexedFlag;
    uint8_t         Unused;

}ResidentAttributeHeader;   //Заголовок резидентного аттрибута

typedef struct  {
    AttributeHeader Header;
    uint64_t        StartingVCN;
    uint64_t        LastVCN;
    uint16_t        DataRunsOffset;
    uint16_t        CompressionUnitSize;
    uint32_t        Padding;
    uint64_t        AllocatedSize;
    uint64_t        RealSize;
    uint64_t        InitializedDataSize;

}NonResidentAttributeHeader; //Заголовок нерезидентного аттрибута

typedef struct  {
    uint8_t     LengthSize  : 4;
    uint8_t     OffsetSize  : 4;
    uint8_t*    Data;
}DataRuns;

typedef struct  {
    uint16_t    GUIDObjectId;
    uint16_t    GUIDBirthVolumeId;
    uint16_t    GUIDBirthObjectId;
    uint16_t    GUIDDomainId;
}ObjectIdBody;

typedef struct  {
    uint64_t    FileReference;
    uint64_t    CreateTime;
    uint64_t    AlteredTime;
    uint64_t    MTime;
    uint64_t    ReadTime;
    uint64_t    AllocatedSize;
    uint64_t    RealSize;
    uint32_t    Flags;
    uint32_t    ReparseAndEAS;
    uint8_t     FilenameLength;
    uint8_t     FilenameNamespace;
    uint16_t*   FilenameData;

}FilenameBody;

typedef struct  {
    uint64_t    FileReference;
    uint16_t    IndexLength;
    uint16_t    StreamLength;
    uint8_t     Flags1;
    uint8_t     Unused2[3];
}IndexEntry;

typedef struct  {
    uint32_t    AttribyteType;
    uint32_t    CollationRule;
    uint32_t    BytesPerIndex;
    uint8_t     ClustersPerIndex;
    uint8_t     Unused[3];
    uint32_t    FirstIndexOffset;
    uint32_t    SizeOfIndexEntry;
    uint32_t    IndexAllocatedSize;
    uint8_t     Flags;
    uint8_t     Unused1[3];

}IndexRootHeader;

typedef struct{ //size 0x2A
    uint32_t    INDXWord;
    uint16_t    UpdataSequenceOffset;
    uint16_t    UpdateSequenceSize;
    uint64_t    LogFileSequenceNumber;
    uint64_t    VCNAllocation;
    uint32_t    FirstIndexEntryOffset;
    uint32_t    IndexEntriesSize;
    uint32_t    IndexEntriesAllocatedSize;
    uint8_t     NonLeafNodeFlag;
    uint8_t     Unused[3];
    uint16_t    UpdateSequence;
}IndexRecordBody;


typedef struct {
    uint64_t    ParentRecordNumber  : 48;
    uint64_t    SequenceNumber      : 16;
    uint32_t    FileRecordNumber;
    uint64_t    SectorNumber;
}FileBufferStructure;
#pragma pack(pop)

void Print_files(uint16_t*, uint16_t );
void Print_root_files(uint16_t*, uint16_t );
uint8_t Ntfs_read_filerecords(FILE* ,uint8_t* , uint8_t* , uint16_t* , uint64_t*);
void Ntfs_read_indexrecords(FILE* ,uint8_t* ,uint16_t* ,uint64_t*);
#endif // NTFS_READ_H
