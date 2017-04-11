#define RAW_HEADER "\x64\x9B\xD1\x09"

#define READ8_MAX 4
#define RAW_HEADER_LEN (sizeof(RAW_HEADER) - 1)
#define MAX_FILENAME_LEN 16

struct FATable {
  uint32_t flags;
  uint32_t offset;
  uint32_t length;
  char filename[MAX_FILENAME_LEN];
};

struct Buffer {
  size_t  length;
  size_t  position;
  char    *buffer;
};

struct State {
  char current_byte;
  char prev_byte;
  uint8_t key_pos;
};

const char *DEFAULT_KEY = "32768GLB";
int errsv;
