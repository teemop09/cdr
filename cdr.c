#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

BOOL is_unc_path(LPCWSTR path) {
    if (wcsncmp(path, L"\\\\?\\UNC\\", 8) == 0) return TRUE;
    if (wcsncmp(path, L"\\\\", 2) == 0 && wcsncmp(path, L"\\\\?\\", 4) != 0) return TRUE;
    return FALSE;
}

void print_error(const wchar_t* msg) {
    int size = WideCharToMultiByte(CP_ACP, 0, msg, -1, NULL, 0, NULL, NULL);
    char* mb_msg = (char*)malloc(size);
    WideCharToMultiByte(CP_ACP, 0, msg, -1, mb_msg, size, NULL, NULL);
    fprintf(stderr, "Error: %s\n", mb_msg);
    free(mb_msg);
    exit(1);
}

int main() {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argv == NULL) {
        fprintf(stderr, "Error: Failed to parse command line\n");
        return 1;
    }

    if (argc == 1) {
        DWORD len = GetCurrentDirectoryW(0, NULL);
        wchar_t* current_dir = (wchar_t*)malloc(len * sizeof(wchar_t));
        GetCurrentDirectoryW(len, current_dir);
        int size = WideCharToMultiByte(CP_ACP, 0, current_dir, -1, NULL, 0, NULL, NULL);
        char* mb_dir = (char*)malloc(size);
        WideCharToMultiByte(CP_ACP, 0, current_dir, -1, mb_dir, size, NULL, NULL);
        printf("%s\n", mb_dir);
        free(mb_dir);
        free(current_dir);
        LocalFree(argv);
        return 0;
    }

    if (argc == 2 && (wcscmp(argv[1], L"-h") == 0 || wcscmp(argv[1], L"--help") == 0)) {
        printf("Usage: cdr [path]\n");
        printf("       cdr -h | --help\n");
        printf("\n");
        printf("Resolves symlinks/junctions to their target directory, then changes to that directory.\n");
        printf("If no path is given, prints the current working directory.\n");
        printf("Note: UNC paths (e.g., \\\\server\\share) are not supported.\n");
        LocalFree(argv);
        return 0;
    }

    if (argc != 2) {
        fprintf(stderr, "Error: Invalid arguments. Use -h for help.\n");
        LocalFree(argv);
        return 1;
    }

    LPCWSTR input_path = argv[1];
    wchar_t normalized_path[MAX_PATH];
    DWORD normalized_len = GetFullPathNameW(input_path, MAX_PATH, normalized_path, NULL);
    if (normalized_len == 0 || normalized_len > MAX_PATH) {
        print_error(L"Failed to normalize path");
    }

    if (is_unc_path(normalized_path)) {
        print_error(L"UNC paths are not supported");
    }

    DWORD attrs = GetFileAttributesW(normalized_path);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        print_error(L"Path not found");
    }
    if (!(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        print_error(L"Not a directory");
    }

    HANDLE hFile = CreateFileW(
        normalized_path,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        print_error(L"Failed to open path");
    }

    wchar_t resolved_path[MAX_PATH];
    DWORD resolved_len = GetFinalPathNameByHandleW(hFile, resolved_path, MAX_PATH, VOLUME_NAME_DOS);
    CloseHandle(hFile);
    if (resolved_len == 0 || resolved_len > MAX_PATH) {
        print_error(L"Failed to resolve path");
    }

    if (is_unc_path(resolved_path)) {
        print_error(L"Resolved path is a UNC path, which is not supported");
    }

    wchar_t* final_path = resolved_path;
    if (wcsncmp(resolved_path, L"\\\\?\\", 4) == 0) {
        final_path = resolved_path + 4;
    }

    int size = WideCharToMultiByte(CP_ACP, 0, final_path, -1, NULL, 0, NULL, NULL);
    char* mb_final_path = (char*)malloc(size);
    WideCharToMultiByte(CP_ACP, 0, final_path, -1, mb_final_path, size, NULL, NULL);
    printf("%s\n", mb_final_path);
    free(mb_final_path);

    LocalFree(argv);
    return 0;
}
