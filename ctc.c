
#include <windows.h>

typedef __int64 s64;

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
    
    s64 n = 0;
    char *buffer = VirtualAlloc(0, 1ULL<<35, MEM_RESERVE, PAGE_READWRITE);
    
    s64 size = page_size;
    
    VirtualAlloc(buffer, size, MEM_COMMIT, PAGE_READWRITE);
    
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    
    DWORD bytes_read;
    BOOL rc;
    do {
        DWORD bytes_to_read = size - n;
        rc = ReadFile(stdin_handle, buffer + n, bytes_to_read, &bytes_read, 0);
        if (bytes_read == bytes_to_read) {
            n = size;
            size *= 2;
            VirtualAlloc(buffer + n, size, MEM_COMMIT, PAGE_READWRITE);
        } else {
            n += bytes_read;
            buffer[n++] = 0;
        }
    } while (rc == TRUE && bytes_read == page_size);
    
    char *clipboard_buffer = GlobalAlloc(GPTR, n);
    if (clipboard_buffer) {
        memcpy(clipboard_buffer, buffer, n);
        
        OpenClipboard(0);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, clipboard_buffer);
    } else {
        return -1;
    }
    return 0;
}