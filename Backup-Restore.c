#include "include/backup-restore.h"
#include "include/ui.h"
#include "include/inventory.h"
#include <stdio.h>
#include <stdlib.h>


/*=== Directory Management ===*/
/*Function to check if backup directory exists*/
static void ensure_backup_dir_exists() {
    /*Creating backup directory if it doesn't exist*/
    struct stat st = { 0 };
    if (stat(BACKUP_DIR, &st) == -1) {
        mkdir(BACKUP_DIR);
    }
}

/*Function to copy backup file*/
bool copy_file(const char* src, const char* dest) {
    /*Opening files*/
    FILE* src_fptr = fopen(src, "rb");
    FILE* dest_fptr = fopen(dest, "wb");

    /*Checking if files are opened*/
    if (!src_fptr || !dest_fptr) {
        if (src_fptr) {
            fclose(src_fptr);
        }
        if (dest_fptr) {
            fclose(dest_fptr);
        }
        return false;
    }

    /*If successfull*/
    bool success = true;
    unsigned char buffer[4096];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), src_fptr)) > 0) {
        if (fwrite(buffer, 1, bytes, dest_fptr) != bytes) {
            success = false;
            break;
        }
    }

    fclose(src_fptr);
    fclose(dest_fptr);
    return success;
}

/*Function to generate backup filename*/
char* generate_backup_filename() {
    /*generating timestamp*/
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    /*Filename*/
    static char filename[256];
    snprintf(filename, sizeof(filename),
        "%s/inventory_%04d%02d%02d_%02d%02d%02d.bak",
        BACKUP_DIR,
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec
    );
    return filename;
}

/*=== Validation and logging ===*/
/*Function to validate backup*/
bool validate_backup(const char* backup_path) {
    /*Opening files*/
    FILE* orig = fopen(FILENAME, "rb");
    FILE* backup = fopen(backup_path, "rb");

    if (!orig || !backup) {
        if (orig) {
            fclose(orig);
        }
        if (backup) {
            fclose(backup);
        }
        return false;
    }

    /*Size validation*/
    fseek(orig, 0, SEEK_END);
    fseek(backup, 0, SEEK_END);
    long orig_size = ftell(orig);
    long backup_size = ftell(backup);

    fclose(orig);
    fclose(backup);

    return (orig_size == backup_size);
}

/*Function to get file size*/
static long get_file_size(const char* path) {
    /*Opening file*/
    FILE* fptr = fopen(path, "rb");
    if (!fptr) {
        return -1;
    }

    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    fclose(fptr);
    return size;
}

/*Function to log backup activity to transaction log*/
void log_backup(const char* backup_path, bool success) {
    /*Opening file*/
    FILE* log_fptr = fopen(BACKUP_LOG, "a");
    if (!log_fptr) {
        return;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    /*Logging backup operations activity */
    fprintf(log_fptr, "%04d-%02d-%02d %02d:%02d:%02d,%s,%s, %ld\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        backup_path,
        success ? "SUCCESS" : "FAILED",
        get_file_size(FILENAME)
    );

    fclose(log_fptr);
}


/*=== Backup creation operations ===*/
/*Function to create backup file*/
bool create_backup() {
    ensure_backup_dir_exists();
    const char* backup_path = generate_backup_filename();
    RetentionPolicy auto_policy = load_retention_config();

    /*Copying main inventory file*/
    if (!copy_file(FILENAME, backup_path)) {
        log_rotation_action(backup_path, "Backup creation failed");
        return false;
    }

    /*Validating the backup*/
    if (!validate_backup(backup_path)) {
        log_rotation_action(backup_path, "Validation failed");
        remove(backup_path);
        return false;
    }

    /*Logging successful backup*/
    log_backup(backup_path, true);

    /*Applying retention policy after successful backup*/
    enforce_retention_policy(&auto_policy);

    return true;
}


/*===== Backup restoration operations =====*/
/*Comparing backups based on the timestamps*/
int compare_backup_entries(const void* a, const void* b) {
    BackupEntry* entry_a = (BackupEntry*)a;
    BackupEntry* entry_b = (BackupEntry*)b;
    return entry_a->timestamp - entry_b->timestamp;
}

/*Listing available backups*/
void list_backups() {
    DIR* dir;
    struct dirent* ent;
    BackupEntry backups[100];
    int count = 0;

    if ((dir = opendir(BACKUP_DIR)) != NULL) {
        while ((ent = readdir(dir)) != NULL && count < 100) {
            if (strstr(ent->d_name, ".bak")) {
                // Manual timestamp parsing
                struct tm tm = { 0 };
                char* ptr = ent->d_name;

                // Skip "inventory_" prefix
                if (strncmp(ptr, "inventory_", 10) != 0) continue;
                ptr += 10;

                // Parse YYYYMMDD_HHMMSS format
                if (sscanf(ptr, "%4d%2d%2d_%2d%2d%2d",
                    &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                    &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {

                    tm.tm_year -= 1900;  // tm_year starts from 1900
                    tm.tm_mon -= 1;     // tm_mon is 0-11
                    backups[count].timestamp = mktime(&tm);

                    strncpy(backups[count].path, ent->d_name, 255);
                    count++;
                }
            }
        }
        closedir(dir);

        // Rest of the function remains the same
        qsort(backups, count, sizeof(BackupEntry), compare_backup_entries);
        printf("\nAvailable Backups:\n");
        for (int i = 0; i < count; i++) {
            char date[20];
            strftime(date, 20, "%Y-%m-%d %H:%M:%S", localtime(&backups[i].timestamp));
            printf("%2d. %s\n", i + 1, date);
        }
    }
    else {
        printf("\nNo backups found!\n");
    }
}

/*Creating safety backup system*/
char* create_safety_backup() {
    static char safety_path[256];
    snprintf(safety_path, sizeof(safety_path), "%s/%s", BACKUP_DIR, SAFETY_BACKUP);

    if (copy_file(FILENAME, safety_path)) {
        return safety_path;
    }
    return NULL;
}

bool restore_from_safety() {
    char safety_path[256];
    snprintf(safety_path, sizeof(safety_path), "%s/%s", BACKUP_DIR, SAFETY_BACKUP);

    if (access(safety_path, F_OK) == 0) {
        return copy_file(safety_path, FILENAME);
    }
    return false;
}

/*Core restoration logic*/
bool restore_backup(const char* backup_path) {
    /*Creating safety net*/
    char* safety = create_safety_backup();
    if (!safety) {
        printf("Failed to create safety backup!\n");
        return false;
    }

    /*Performing restoration*/
    if (!copy_file(backup_path, FILENAME)) {
        printf("Restoration failed! Reverting to safety backup.\n");
        restore_from_safety();
        remove(safety);
        return false;
    }

    /*Validating restored file*/
    if (!validate_backup_integrity(FILENAME)) {
        printf("Validation failed! Reverting...\n");
        remove(safety);
        return false;
    }

    /*Cleaning up*/
    remove(safety);
    return true;
}

/*Validating backup integrity*/
bool validate_backup_integrity(const char* path) {
    /*Opening file*/
    FILE* fptr = fopen(path, "rb");
    if (!fptr) {
        return false;
    }
    /*Validating structure size*/
    Product p;
    while (fread(&p, sizeof(Product), 1, fptr)) {
        if (p.id <= 0 || p.price < 0 || p.quantity < 0) {
            fclose(fptr);
            return false;
        }
    }
    fclose(fptr);
    return true;
}

/*===== Backup rotation operations ======*/
/*Parsing timestamp from filename*/
time_t get_backup_timestamp(const char* filename) {
    struct tm tm = { 0 };
    int year, month, day, hour, min, sec;

    /*Parsing filename format*/
    if (sscanf(filename, "inventory_%4d%2d%2d_%2d%2d%2d.bak",
        &year, &month, &day, &hour, &min, &sec) == 6) {
        return (time_t)-1;
    }

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;

    return mktime(&tm);
}

/*Sorting backups by scanning backup directory*/
BackupEntry* list_backups_sorted(int* count) {
    /*opening diractory*/
    DIR* dir;
    struct dirent* ent;
    static BackupEntry backups[MAX_BACKUPS];
    *count = 0;

    /*Scanning directory*/
    if ((dir = opendir(BACKUP_DIR)) != NULL) {
        while ((ent = readdir(dir)) != NULL && *count < MAX_BACKUPS) {
            if (strstr(ent->d_name, ".bak")) {
                time_t t = get_backup_timestamp(ent->d_name);
                if (t != (time_t)-1) {
                    strncpy(backups[*count].path, ent->d_name, 255);
                    backups[*count].timestamp = t;
                    (*count)++;
                }
            }
        }
        closedir(dir);

        /*Sorting oldest first*/
        qsort(backups, *count, sizeof(BackupEntry), compare_backup_entries);
    }

    return backups;
}

/*Readng config file for retention*/
RetentionPolicy load_retention_config() {
    RetentionPolicy policy = { .retain_count = 7, .retain_days = 30 };
    /*Opening config file*/
    FILE* fptr = fopen(RETENTION_CONFIG_FILE, "r");

    if (fptr) {
        char line[256];
        while (fgets(line, sizeof(line), fptr)) {
            if (sscanf(line, "retain_count=%d", &policy.retain_count) == 1) {
                continue;
            }
            if (sscanf(line, "retain_days=%d", &policy.retain_days) == 1) {
                continue;
            }
        }
        fclose(fptr);
    }

    return policy;
}

/*Deleting old backups by count*/
void delete_old_backups_by_count(int max_count) {
    int total = 0;
    BackupEntry* backups = list_backups_sorted(&total);

    for (int i = 0; i < total - max_count; i++) {
        char full_path[289];
        snprintf(full_path, sizeof(full_path), "%s / %s", BACKUP_DIR, backups[i].path);
        delete_backup_safely(full_path);
    }
}

/*Deleting old backups by age*/
void delete_old_backups_by_age(int max_days) {
    time_t now = time(NULL);
    int total = 0;
    BackupEntry* backups = list_backups_sorted(&total);

    for (int i = 0; i < total; i++) {
        double age_days = difftime(now, backups[i].timestamp) / (60 * 60 * 24);

        if (age_days > max_days) {
            char full_path[278];
            snprintf(full_path, sizeof(full_path), "%s / %s", BACKUP_DIR, backups[i].path);
            delete_backup_safely(full_path);
        }
    }
}

/*Enforcing retention policies*/
void enforce_retention_policy(const RetentionPolicy* policy) {

    /*Deleting old backups by count*/
    if (policy->retain_count > 0) {
        delete_old_backups_by_count(policy->retain_count);
    }

    /*Deleting old backup by age*/
    if (policy->retain_days > 0) {
        delete_old_backups_by_age(policy->retain_days);
    }
}

/*Utility functions*/
/*Deleting backup safely*/
void delete_backup_safely(const char* path) {
    if (remove(path) == 0) {
        log_rotation_action(path, "Deleted by retention policy");
    }
    else {
        char error[256];
        snprintf(error, sizeof(error), "Deletion failed: %s", strerror(errno));
        log_rotation_action(path, error);
    }
}

/*Getting current timestamp of backup file*/
static const char* get_current_timestamp() {
    static char buffer[20];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);

    return buffer;
}

/*auditing activity to log file*/
void log_rotation_action(const char* path, const char* reason) {
    /*Opening log file*/
    FILE* fptrlog = fopen(ROTATION_LOG, "a");
    if (fptrlog) {
        fprintf(fptrlog, "[%s] %s: %s\n", get_current_timestamp(), path, reason);
        fclose(fptrlog);
    }
}

/*Saving retention policy*/
void save_retention_policy(RetentionPolicy policy) {
    FILE* fptr = fopen(RETENTION_CONFIG_FILE, "w");
    if (fptr) {
        fprintf(fptr, "retain_count=%d\n", policy.retain_count);
        fprintf(fptr, "retain_days=%d\n", policy.retain_days);
        fclose(fptr);
    }
}