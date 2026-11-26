// The PC's stand-ins for module/build_r7_tapes.cpp (OP1F_HOST): the logger, a model of the unit's disk (files and
// folders by their paths), the disk mode's word. The test defines the cards unit's and the folders unit's names.
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

static std::map<std::string, std::string> mock_disk;   // a file's path -> its bytes
static std::set<std::string> mock_dirs;                // a folder's path, without the slash behind
static unsigned mock_disk_open;                        // the disk mode's word
static int mock_mounted = -1;                          // the mounted tape, 0 to 7, -1 for none
static int mock_mounted_tape() { return mock_mounted; }
static int mock_lines;
static bool mock_verbose = getenv("VERBOSE") != 0;

static void mock_log(int level, const char*, int, const char* format, ...) {
    ++mock_lines;
    if (!mock_verbose) return;
    va_list a; va_start(a, format); fprintf(stderr, "[%d] ", level); vfprintf(stderr, format, a); fputc('\n', stderr); va_end(a);
}
static std::string mock_bare(const char* p) { std::string s(p); while (!s.empty() && s.back() == '/') s.pop_back(); return s; }
static int mock_size(const char* p) { auto it = mock_disk.find(p); return it == mock_disk.end() ? -1 : (int)it->second.size(); }
static int mock_remove(const char* p) { return mock_disk.erase(p) ? 1 : 0; }
static int mock_make_folders(const char* p) {
    std::string s = mock_bare(p);
    for (size_t at = s.find('/'); ; at = s.find('/', at + 1)) { mock_dirs.insert(s.substr(0, at)); if (at == std::string::npos) break; }
    return 1;
}
static int mock_is_folder(const char* p) { return mock_dirs.count(mock_bare(p)) ? 1 : 0; }
struct MockDir { std::vector<std::string> names; size_t at; char entry[11 + 64]; };
static void* mock_dir_open(const char* p) {
    std::string dir = mock_bare(p);
    if (!mock_dirs.count(dir)) return 0;
    MockDir* d = new MockDir(); d->at = 0;
    std::string in = dir + "/";
    for (auto& f : mock_disk) if (f.first.compare(0, in.size(), in) == 0 && f.first.find('/', in.size()) == std::string::npos) d->names.push_back(f.first.substr(in.size()));
    for (auto& f : mock_dirs) if (f.compare(0, in.size(), in) == 0 && f.find('/', in.size()) == std::string::npos) d->names.push_back(f.substr(in.size()));
    return d;
}
static void* mock_dir_next(void* h) {
    MockDir* d = (MockDir*)h;
    if (d->at == d->names.size()) return 0;
    strcpy(d->entry + 11, d->names[d->at++].c_str());
    return d->entry;
}
static int mock_dir_close(void* h) { delete (MockDir*)h; return 0; }
struct MockHandle { std::string path; size_t at; bool open; };
static std::vector<MockHandle> mock_handles;
static int mock_open(const char* p, const char* mode) {
    if (mode[0] == 'r' && !mock_disk.count(p)) return -1;
    if (mode[0] == 'w') mock_disk[p] = "";
    mock_handles.push_back({p, 0, true});
    return (int)mock_handles.size() - 1;
}
static int mock_read(void* b, unsigned size, unsigned count, int h) {
    MockHandle& m = mock_handles[h]; const std::string& s = mock_disk[m.path];
    size_t n = size * count; if (n > s.size() - m.at) n = s.size() - m.at;
    memcpy(b, s.data() + m.at, n); m.at += n; return (int)n;
}
static int mock_write(const void* d, unsigned size, unsigned count, int h) { mock_disk[mock_handles[h].path].append((const char*)d, size * count); return (int)count; }
static void mock_close(int h) { mock_handles[h].open = false; }
static int mock_rename(const char* from, const char* to) {
    auto it = mock_disk.find(from); if (it == mock_disk.end()) return 0;
    std::string bytes = it->second; mock_disk.erase(it); mock_disk[to] = bytes; return 1;
}
#define STOCK_LOG mock_log
#define STOCK_FILE_SIZE mock_size
#define STOCK_FILE_REMOVE mock_remove
#define STOCK_MAKE_FOLDERS mock_make_folders
#define STOCK_IS_FOLDER mock_is_folder
#define STOCK_DIR_OPEN mock_dir_open
#define STOCK_DIR_NEXT mock_dir_next
#define STOCK_DIR_CLOSE mock_dir_close
#define STOCK_OPEN mock_open
#define STOCK_READ mock_read
#define STOCK_WRITE mock_write
#define STOCK_CLOSE mock_close
#define STOCK_RENAME mock_rename
#define STOCK_MOUNTED_TAPE mock_mounted_tape
#define RD32(a) ((void)(a), mock_disk_open)
