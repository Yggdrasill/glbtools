#define RAW_HEADER "\x64\x9B\xD1\x09"

#define READ8_MAX         4
#define RAW_HEADER_LEN    (sizeof(RAW_HEADER) - 1)
#define MAX_FILENAME_LEN  16
#define FAT_SIZE          28
#define MAX_FILES         4096

#define HELP_TEXT "[ARGS...] [FILE]\n" \
                  "-h\tprint this help text\n" \
                  "-l\tprint list of files in archive\n" \
                  "-x\textract all files in archive\n" \
                  "-e\textract specific comma-separated files in archive"

struct FATable {
  uint32_t extract;
  uint32_t flags;
  uint32_t offset;
  uint32_t length;
  char filename[MAX_FILENAME_LEN];
};

struct State {
  char current_byte;
  char prev_byte;
  uint8_t key_pos;
};

struct Tokens {
  char *table[MAX_FILES];
  size_t ntokens;
};

enum ARGS {
  ARGS_LIST = 0x01,
  ARGS_EXTA = 0x02,
  ARGS_EXTS = 0x04
};

const char *DEFAULT_KEY = "32768GLB";
