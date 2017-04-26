#ifndef GLB_CONST_H
#define GLB_CONST_H

#define RAW_HEADER "\x64\x9B\xD1\x09"

#define READ8_MAX         4
#define RAW_HEADER_LEN    (sizeof(RAW_HEADER) - 1)
#define MAX_FILES         4096
#define MAX_FILENAME_LEN  16
#define FAT_SIZE          28

enum ARGS {
  ARGS_LIST = 0x01,
  ARGS_EXTA = 0x02,
  ARGS_EXTS = 0x04
};

#endif
