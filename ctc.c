
#include <windows.h>

typedef __int64 s64;
#define Minimum(a,b) ((a) < (b) ? (a) : (b))

#pragma function(memcpy)
void *memcpy(void *dest_init, void *src_init, size_t n) {
    unsigned char *dest = dest_init;
    unsigned char *src = src_init;
    while (n--) {
        *dest++ = *src++;
    }
    return dest;
}

typedef wchar_t wchar ;

#define WriteLiteralToConsole(h, s, b) WriteFile(h, s, sizeof(s)-1, b, 0)

int mainCRTStartup(void) {
    int wargc;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    
    if (wargc > 1) {
        HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdout_handle != INVALID_HANDLE_VALUE) {
            DWORD bytes_written;
            WriteLiteralToConsole(stdout_handle,
                                  "usage: ctc.exe\r\n"
                                  "    reads from stdin and copies to clipboard\r\n",
                                  &bytes_written);
        }
        return 0;
    }
    
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD page_size = si.dwPageSize;
    
    s64 size_used = 0;
    char *buffer = VirtualAlloc(0, 1ULL<<35, MEM_RESERVE, PAGE_READWRITE);
    
    s64 size_committed = page_size;
    
    VirtualAlloc(buffer, size_committed, MEM_COMMIT, PAGE_READWRITE);
    
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    
    DWORD bytes_read;
    DWORD bytes_to_read;
    BOOL rc;
    do {
        bytes_to_read = Minimum(size_committed - size_used, 0xffffffff);
        rc = ReadFile(stdin_handle, buffer + size_used, bytes_to_read, &bytes_read, 0);
        size_used += bytes_read;
        if (size_committed == size_used) {
            size_committed *= 2;
            VirtualAlloc(buffer, size_committed, MEM_COMMIT, PAGE_READWRITE);
        }
    } while (rc == TRUE && bytes_read == bytes_to_read);
    
    buffer[size_used++]=0;
    
    char *clipboard_buffer = GlobalAlloc(GPTR, size_used);
    if (clipboard_buffer) {
        memcpy(clipboard_buffer, buffer, size_used);
        
        OpenClipboard(0);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, clipboard_buffer);
    } else {
        return -1;
    }
    return 0;
}