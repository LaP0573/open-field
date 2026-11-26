// The PC's stand-ins for module/build_r8_options.cpp (OP1F_HOST): the logger, a model of the disk (files by their
// paths), and the settings' side: the rows added, the shared seven lines a draw fills, the chosen row and the list
// of the rows' screens. The test defines the cards unit's and the folders unit's names.
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

static std::map<std::string, std::string> mock_disk;
static bool mock_verbose = getenv("VERBOSE") != 0;
static int mock_lines;
static void mock_log(int level, const char*, int, const char* format, ...) {
    ++mock_lines;
    if (!mock_verbose) return;
    va_list a; va_start(a, format); fprintf(stderr, "[%d] ", level); vfprintf(stderr, format, a); fputc('\n', stderr); va_end(a);
}
static int mock_size(const char* p) { auto it = mock_disk.find(p); return it == mock_disk.end() ? -1 : (int)it->second.size(); }
struct MockHandle { std::string path; size_t at; };
static std::vector<MockHandle> mock_handles;
static int mock_open(const char* p, const char*) { if (!mock_disk.count(p)) return -1; mock_handles.push_back({p, 0}); return (int)mock_handles.size() - 1; }
static int mock_read(void* b, unsigned size, unsigned count, int h) {
    MockHandle& m = mock_handles[h]; const std::string& s = mock_disk[m.path];
    size_t n = size * count; if (n > s.size() - m.at) n = s.size() - m.at;
    memcpy(b, s.data() + m.at, n); m.at += n; return (int)n;
}
static void mock_close(int) {}
static void* mock_new(unsigned bytes) { return calloc(1, bytes); }
static void mock_screen_make(void* screen, const char*) { memset(screen, 0, 0x2c); }
static std::string mock_name[7], mock_value[7];
static unsigned mock_draws;
static void mock_screen_draw(void*, void*) { ++mock_draws; }
static void mock_line_name(void*, unsigned line, const char* t) { if (line < 7) mock_name[line] = t; }
static void mock_line_value(void*, unsigned line, const char* t) { if (line < 7) mock_value[line] = t; }
static void mock_top(void*, unsigned) {}
static void mock_chosen(void*) {}
static void mock_red(void*, unsigned, unsigned) {}
static std::vector<void*> mock_rows;                    // the controllers added, in order
static void mock_add(void*, void* controller) { mock_rows.push_back(controller); }
static unsigned mock_settings_chosen;                   // the chosen row: an index into the list below
static unsigned mock_settings_list[3];                  // a vector: +4 first, +8 end
static unsigned mock_screens[8];                        // the rows' screens
#define STOCK_LOG mock_log
#define STOCK_FILE_SIZE mock_size
#define STOCK_OPEN mock_open
#define STOCK_READ mock_read
#define STOCK_CLOSE mock_close
#define STOCK_NEW mock_new
#define STOCK_SCREEN_MAKE mock_screen_make
#define STOCK_SCREEN_DRAW mock_screen_draw
#define STOCK_LINE_NAME mock_line_name
#define STOCK_LINE_VALUE mock_line_value
#define STOCK_KNOB_TOP mock_top
#define STOCK_CHOSEN_KEYBOARD mock_chosen
#define STOCK_CHOSEN_SYSTEM mock_chosen
#define STOCK_CHOSEN_MIDI mock_chosen
#define STOCK_RED_KEYBOARD mock_red
#define STOCK_RED_SYSTEM mock_red
#define STOCK_RED_MIDI mock_red
#define LAST_STOCK_ROW 0x6006u
#define TABLE_KEYBOARD 0x6001u
#define TABLE_SYSTEM 0x6002u
#define TABLE_MIDI 0x6003u
#define SETTINGS_CHOSEN ((unsigned)(unsigned long)&mock_settings_chosen)
#define SETTINGS_LIST ((unsigned)(unsigned long)mock_settings_list)
#define STOCK_ADD(owner, controller) mock_add(owner, controller)
#define BARRIER() ((void)0)
