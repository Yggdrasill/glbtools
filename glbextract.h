#define RAW_HEADER "\x64\x9B\xD1\x09"

#define RAW_HEADER_LEN (sizeof(RAW_HEADER) - 1)
#define MAX_FILENAME_LEN 16

struct FATable {
  uint32_t flags;
  uint32_t offset;
  uint32_t length;
  char filename[MAX_FILENAME_LEN];
};

const uint8_t DEFAULT_KEY = "32768GLB";
