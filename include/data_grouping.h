#ifndef DATA_GROUPING_H
#define DATA_GROUPING_H

typedef enum {
    VFD,
    VFT
} KeyType;

void group_data_by_type(char** assigned_files, KeyType key_type);

#endif // DATA_GROUPING_H
