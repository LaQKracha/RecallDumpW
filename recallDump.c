#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <shlobj.h>
#define BLUE "\033[94m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define RED "\033[91m"
#define CYAN "\033[96m"
#define MAGENTA "\033[95m"
#define WHITE "\033[97m"
#define ENDC "\033[0m"
#define mkdir _mkdir
#define stat _stat
#else
#include <unistd.h>
#define BLUE "\x1B[34m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define RED "\x1B[31m"
#define CYAN "\x1B[36m"
#define MAGENTA "\x1B[35m"
#define WHITE "\x1B[37m"
#define ENDC "\x1B[0m"
#endif

void display_banner() {
    printf("%sRecallDump%s\n", BLUE, ENDC);
}

void modify_permissions(const char *path) {
#ifdef _WIN32
    char command[512];
    snprintf(command, sizeof(command), "icacls \"%s\" /grant %s:(OI)(CI)F /T /C /Q", path, getenv("USERNAME"));
    system(command);
    printf("%sPermissions modified for path: %s%s\n", GREEN, path, ENDC);
#else
    printf("%sPermission modification not supported on this OS.%s\n", YELLOW, ENDC);
#endif
}

void create_directory(const char *path) {
    if (mkdir(path) == -1) {
        perror(RED "Failed to create directory" ENDC);
    } else {
        printf("%sDirectory created: %s%s\n", GREEN, path, ENDC);
    }
}

void copy_file(const char *source, const char *destination) {
#ifdef _WIN32
    if (!CopyFile(source, destination, FALSE)) {
        printf("%sFailed to copy file from %s to %s. Error: %lu%s\n", RED, source, destination, GetLastError(), ENDC);
    } else {
        printf("%sFile copied from %s to %s%s\n", GREEN, source, destination, ENDC);
    }
#else
    FILE *src = fopen(source, "rb");
    if (!src) {
        perror(RED "Failed to open source file" ENDC);
        return;
    }
    FILE *dst = fopen(destination, "wb");
    if (!dst) {
        perror(RED "Failed to open destination file" ENDC);
        fclose(src);
        return;
    }

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    printf("%sFile copied from %s to %s%s\n", GREEN, source, destination, ENDC);
#endif
}

void copy_directory(const char *source, const char *destination) {
#ifdef _WIN32
    SHFILEOPSTRUCT fileOp = {
        NULL,
        FO_COPY,
        source,
        destination,
        FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_SILENT,
        FALSE,
        NULL,
        NULL
    };
    if (SHFileOperation(&fileOp) != 0) {
        printf("%sFailed to copy directory from %s to %s%s\n", RED, source, destination, ENDC);
    } else {
        printf("%sDirectory copied from %s to %s%s\n", GREEN, source, destination, ENDC);
    }
#else
    printf("%sDirectory copy not supported on this OS.%s\n", YELLOW, ENDC);
#endif
}

void get_current_time_string(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%d-%H-%M", localtime(&now));
}

void process_database(const char *db_path) {
    sqlite3 *db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        printf("%sFailed to open database: %s%s\n", RED, db_path, ENDC);
        return;
    }

    const char *query = "SELECT WindowTitle, TimeStamp, ImageToken FROM WindowCapture";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("%sFailed to prepare SQL statement%s\n", RED, ENDC);
        sqlite3_close(db);
        return;
    }

    printf("%sProcessing database...%s\n", CYAN, ENDC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *window_title = sqlite3_column_text(stmt, 0);
        int timestamp = sqlite3_column_int(stmt, 1);
        const unsigned char *image_token = sqlite3_column_text(stmt, 2);

        printf("%sWindow Title: %s, Timestamp: %d, Image Token: %s%s\n", WHITE, window_title, timestamp, image_token, ENDC);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    printf("%sDatabase processing completed.%s\n", GREEN, ENDC);
}

void rename_images(const char *image_store_path) {
#ifdef _WIN32
    char search_pattern[512];
    snprintf(search_pattern, sizeof(search_pattern), "%s\\*.*", image_store_path);
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(search_pattern, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char old_image_path[512];
                char new_image_path[512];
                snprintf(old_image_path, sizeof(old_image_path), "%s\\%s", image_store_path, findFileData.cFileName);
                snprintf(new_image_path, sizeof(new_image_path), "%s\\%s.jpg", image_store_path, findFileData.cFileName);

                if (!strstr(findFileData.cFileName, ".jpg")) {
                    if (rename(old_image_path, new_image_path) != 0) {
                        perror(RED "Failed to rename image file" ENDC);
                    } else {
                        printf("%sRenamed image file: %s to %s%s\n", GREEN, old_image_path, new_image_path, ENDC);
                    }
                }
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#else
    printf("%sImage renaming not supported on this OS.%s\n", YELLOW, ENDC);
#endif
}

void display_help() {
    printf("%sUsage: RecallDump [options]%s\n", BLUE, ENDC);
    printf("%sOptions:%s\n", CYAN, ENDC);
    printf("%s  -h             Show this help message%s\n", MAGENTA, ENDC);
    printf("%s  -f    Specify the starting date for the extraction (format: YYYY-MM-DD)%s\n", MAGENTA, ENDC);
    printf("%s  -t      Specify the ending date for the extraction (format: YYYY-MM-DD)%s\n", MAGENTA, ENDC);
    printf("%s  -s       Specify a search term to filter the extraction%s\n", MAGENTA, ENDC);
}

int main(int argc, char *argv[]) {
    char from_date[11] = {0};
    char to_date[11] = {0};
    char search_term[100] = {0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            display_help();
            return 0;
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            strncpy(from_date, argv[++i], sizeof(from_date) - 1);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            strncpy(to_date, argv[++i], sizeof(to_date) - 1);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(search_term, argv[++i], sizeof(search_term) - 1);
        }
    }

    display_banner();

    const char *username = getenv("USERNAME");
    char base_path[256];
    snprintf(base_path, sizeof(base_path), "C:\\Users\\%s\\AppData\\Local\\CoreAIPlatform.00\\UKP", username);

    struct stat sb;
    if (stat(base_path, &sb) != 0 || !(sb.st_mode & S_IFDIR)) {
        printf("%sBase path does not exist: %s%s\n", RED, base_path, ENDC);
        return 1;
    }

    modify_permissions(base_path);

    char guid_folder[256];
    snprintf(guid_folder, sizeof(guid_folder), "%s\\GUID_FOLDER", base_path);

    char db_path[256];
    snprintf(db_path, sizeof(db_path), "%s\\ukg.db", guid_folder);

    char image_store_path[256];
    snprintf(image_store_path, sizeof(image_store_path), "%s\\ImageStore", guid_folder);

    if (stat(db_path, &sb) != 0 || stat(image_store_path, &sb) != 0) {
        printf("%sWindows Recall feature not found. Nothing to extract.%s\n", YELLOW, ENDC);
        return 1;
    }

    char proceed[4];
    printf("%sWindows Recall feature found. Do you want to proceed with the extraction? (yes/no): %s", CYAN, ENDC);
    scanf("%3s", proceed);
    if (strcmp(proceed, "yes") != 0) {
        printf("%sExtraction aborted.%s\n", YELLOW, ENDC);
        return 0;
    }

    char timestamp[20];
    get_current_time_string(timestamp, sizeof(timestamp));
    char extraction_folder[256];
    snprintf(extraction_folder, sizeof(extraction_folder), "%s\\%s_Recall_Extraction", _getcwd(NULL, 0), timestamp);
    create_directory(extraction_folder);

    char dest_db_path[256];
    snprintf(dest_db_path, sizeof(dest_db_path), "%s\\ukg.db", extraction_folder);
    copy_file(db_path, dest_db_path);

    char dest_image_store_path[256];
    snprintf(dest_image_store_path, sizeof(dest_image_store_path), "%s\\ImageStore", extraction_folder);
    copy_directory(image_store_path, dest_image_store_path);

    rename_images(dest_image_store_path);

    process_database(dest_db_path);

    printf("\n%sFull extraction folder path: %s%s\n", GREEN, extraction_folder, ENDC);

    return 0;
}
