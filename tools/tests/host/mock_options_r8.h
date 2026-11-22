// The PC's stand-ins for module/build_r8_options.cpp (OP1F_HOST): the logger, a model of the disk (files by their
// paths), the frame's buffer. The test defines the cards unit's and the folders unit's names.
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

static std::map<std::string, std::string> mock_disk;
static unsigned short mock_frame[480 * 256];
static bool mock_verbose = getenv("VERBOSE") != 0;
static void mock_log(int level, const char*, int, const char* format, ...) {
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
#define STOCK_LOG mock_log
#define STOCK_FILE_SIZE mock_size
#define STOCK_OPEN mock_open
#define STOCK_READ mock_read
#define STOCK_CLOSE mock_close
#define RD32(a) ((void)(a), (unsigned)(unsigned long)mock_frame)
#define BARRIER() ((void)0)
