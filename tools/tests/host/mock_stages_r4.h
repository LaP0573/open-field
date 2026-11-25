// The PC's stand-ins that module/build_r5_memory.cpp takes from this header (OP1F_HOST): the firmware's heap 0 (calloc,
// with a free-space count) and the interrupt masks. Only what the memory unit needs; tools/tests/host/memory_test.cpp.
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static unsigned mock_heap_space = 64u << 20;              // the heap's free bytes
static void* mock_heap_take(int, unsigned bytes) {
    if (bytes > mock_heap_space) return 0;
    mock_heap_space -= bytes;
    return calloc(1, bytes);
}
static int mock_heap_free(int index) { return index == 0 ? (int)mock_heap_space : -1; }
static unsigned mock_mask() { return 0; }
static void mock_unmask(unsigned) {}
