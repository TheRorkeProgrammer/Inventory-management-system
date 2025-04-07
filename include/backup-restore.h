#ifndef BACKUP_RESTORE_H
#define BACKUP_RESTORE_H

#include "inventory.h"
#include "ui.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define BACKUP_DIR "backups"
#define BACKUP_LOG "backups.log"
#define SAFETY_BACKUP "safety_restore.bak"
#define RETENTION_CONFIG_FILE "retention.cfg"
#define ROTATION_LOG "rotation.log"
#define MAX_BACKUPS 100

#ifdef _WIN32
// Windows doesn't have scandir, use this implementation
int scandir(const char* dirp, struct dirent*** namelist,
    int (*filter)(const struct dirent*),
    int (*compar)(const struct dirent**, const struct dirent**));
#endif

/*Structure for backup rotation*/
typedef struct {
    int retain_count; // Keeping last N backups
    int retain_days; // Deleting older than X days
} RetentionPolicy;

/*Structure for backup entry logging*/
typedef struct {
    char path[256];
    char filename[256];
    size_t file_size;
    time_t timestamp;
    bool is_valid;
    char checksum[33];
} BackupEntry;

extern BackupEntry backup_list[MAX_BACKUPS];
extern int backup_count;

/*===== Backup creation operations =====*/
bool create_backup();
void list_backups();
bool validate_backup(const char* backup_path);

/*===== Backup restoration operations ======*/
bool restore_backup(const char* backup_path);
bool validate_backup_integrity(const char* path);
char* create_safety_backup();
bool restore_from_safety();

/*===== Backup rotation operations ======*/
/*Configuration management*/
RetentionPolicy load_retention_config();
bool validate_retention_policy(const RetentionPolicy* policy);
/*Backup age calculation*/
time_t get_backup_timestamp(const char* filename);
int calculate_backup_age_days(time_t backup_time);
/*Backup listing and sorting*/
BackupEntry* list_backups_sorted(int* count);
/*Rotation logic*/
void enforce_retention_policy(const RetentionPolicy* policy);
void delete_old_backups_by_count(int max_count);
void delete_old_backups_by_age(int max_days);
/*Safe deletion and logging*/
void delete_backup_safely(const char* path);
void log_rotation_action(const char* path, const char* reason);

void save_retention_policy(RetentionPolicy policy);


#endif // !BACKUP_RESTORE_H
