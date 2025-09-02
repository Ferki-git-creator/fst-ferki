#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>

#define MAX_PATH 4096
#define LARGE_FILE_THRESHOLD (100 * 1024 * 1024)

typedef struct {
    unsigned long long total_size;
    unsigned long long file_count;
    unsigned long long dir_count;
    unsigned long long empty_files;
    unsigned long long non_empty_files;
    unsigned long long empty_dirs;
    unsigned long long non_empty_dirs;
    unsigned long long text_files;
    unsigned long long binary_files;
    unsigned long long script_files;
    unsigned long long large_files;
    unsigned long long min_size;
    unsigned long long max_size;
    unsigned long long total_file_size;
    unsigned long long text_size;
    unsigned long long binary_size;
    unsigned long long sym_links;
    unsigned long long hard_links;
    unsigned long long exec_files;
    unsigned long long recent_files;
    time_t oldest_time;
    time_t newest_time;
    char oldest_file[MAX_PATH];
    char newest_file[MAX_PATH];
    char dir_path[MAX_PATH];
} Stats;

typedef struct {
    int types;
    int size;
    int permissions;
    int dates;
    int links;
    int verbose;
    int human;
    int all;
} Options;

const char *text_extensions[] = {".txt", ".c", ".h", ".cpp", ".hpp", ".java", 
                                ".py", ".sh", ".pl", ".js", ".css", ".html", 
                                ".xml", ".json", ".md", NULL};
const char *script_extensions[] = {".sh", ".py", ".pl", ".rb", ".php", ".js", 
                                  ".lua", NULL};

void init_stats(Stats *stats, const char *path) {
    memset(stats, 0, sizeof(Stats));
    stats->min_size = ULLONG_MAX;
    stats->oldest_time = LONG_MAX;
    stats->newest_time = 0;
    strncpy(stats->dir_path, path, MAX_PATH);
}

int is_text_file(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0;
    for (int i = 0; text_extensions[i]; i++) {
        if (strcasecmp(dot, text_extensions[i]) == 0) return 1;
    }
    return 0;
}

int is_script_file(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0;
    for (int i = 0; script_extensions[i]; i++) {
        if (strcasecmp(dot, script_extensions[i]) == 0) return 1;
    }
    return 0;
}

void human_readable_size(unsigned long long size, char *buf, int human) {
    if (human) {
        const char units[] = "BKMGTPE";
        int unit = 0;
        double dsize = size;
        while (dsize >= 1024 && unit < sizeof(units)-1) {
            dsize /= 1024;
            unit++;
        }
        sprintf(buf, "%.1f%c", dsize, units[unit]);
    } else {
        sprintf(buf, "%llu", size);
    }
}

void process_entry(const char *path, const struct stat *sb, Options *opts, Stats *stats) {
    if (S_ISREG(sb->st_mode)) {
        stats->file_count++;
        stats->total_size += sb->st_size;
        
        if (sb->st_size == 0) stats->empty_files++;
        else stats->non_empty_files++;
        
        if (sb->st_size > LARGE_FILE_THRESHOLD) stats->large_files++;
        
        if (sb->st_size < stats->min_size) stats->min_size = sb->st_size;
        if (sb->st_size > stats->max_size) stats->max_size = sb->st_size;
        
        if (is_text_file(path)) {
            stats->text_files++;
            stats->text_size += sb->st_size;
        } else {
            stats->binary_files++;
            stats->binary_size += sb->st_size;
        }
        
        if (is_script_file(path)) stats->script_files++;
        if (sb->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) stats->exec_files++;
        
        if (sb->st_mtime < stats->oldest_time) {
            stats->oldest_time = sb->st_mtime;
            strncpy(stats->oldest_file, path, MAX_PATH);
        }
        if (sb->st_mtime > stats->newest_time) {
            stats->newest_time = sb->st_mtime;
            strncpy(stats->newest_file, path, MAX_PATH);
        }
    }
    else if (S_ISDIR(sb->st_mode)) {
        stats->dir_count++;
    }
    else if (S_ISLNK(sb->st_mode)) {
        stats->sym_links++;
    }
}

void scan_directory(const char *path, Options *opts, Stats *stats) {
    DIR *dir = opendir(path);
    if (!dir) {
        if (opts->verbose) fprintf(stderr, "Error opening %s: %s\n", path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s/%s", path, entry->d_name);

        struct stat sb;
        if (lstat(full_path, &sb) == -1) {
            if (opts->verbose) fprintf(stderr, "Error stating %s: %s\n", full_path, strerror(errno));
            continue;
        }

        process_entry(full_path, &sb, opts, stats);

        if (S_ISDIR(sb.st_mode)) {
            scan_directory(full_path, opts, stats);
        }
    }
    closedir(dir);
}

void print_stats(Options *opts, Stats *stats) {
    char human_size[64];
    time_t now = time(NULL);
    
    printf("Directory: %s\n\n", stats->dir_path);
    
    printf("General:\n");
    human_readable_size(stats->total_size, human_size, opts->human);
    printf("  Total size: %s\n", human_size);
    printf("  Total files: %llu\n", stats->file_count);
    printf("  Total directories: %llu\n", stats->dir_count);
    printf("  Empty files: %llu\n", stats->empty_files);
    printf("  Non-empty files: %llu\n", stats->non_empty_files);
    printf("  Empty directories: %llu\n", stats->empty_dirs);
    printf("  Non-empty directories: %llu\n", stats->non_empty_dirs);

    if (opts->types || opts->all) {
        printf("\nTypes:\n");
        printf("  Text files: %llu\n", stats->text_files);
        printf("  Binary files: %llu\n", stats->binary_files);
        printf("  Script files: %llu\n", stats->script_files);
        printf("  Large files (>100MB): %llu\n", stats->large_files);
    }

    if (opts->size || opts->all) {
        printf("\nSizes:\n");
        human_readable_size(stats->min_size, human_size, opts->human);
        printf("  Min file size: %s\n", human_size);
        human_readable_size(stats->max_size, human_size, opts->human);
        printf("  Max file size: %s\n", human_size);
        human_readable_size(stats->total_size / (stats->file_count ? stats->file_count : 1), 
                          human_size, opts->human);
        printf("  Avg file size: %s\n", human_size);
    }

    if (opts->dates || opts->all) {
        printf("\nDates:\n");
        printf("  Oldest file: %s", ctime(&stats->oldest_time));
        printf("  Newest file: %s", ctime(&stats->newest_time));
    }

    if (opts->links || opts->all) {
        printf("\nLinks:\n");
        printf("  Symbolic links: %llu\n", stats->sym_links);
        printf("  Hard links: %llu\n", stats->hard_links);
    }
}

int main(int argc, char *argv[]) {
    Options opts = {0};
    Stats stats;
    const char *path = ".";
    
    static struct option long_options[] = {
        {"types", no_argument, 0, 't'},
        {"size", no_argument, 0, 's'},
        {"permissions", no_argument, 0, 'p'},
        {"dates", no_argument, 0, 'd'},
        {"links", no_argument, 0, 'l'},
        {"verbose", no_argument, 0, 'v'},
        {"human", no_argument, 0, 'h'},
        {"all", no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "tspdlvha", long_options, NULL)) != -1) {
        switch (opt) {
            case 't': opts.types = 1; break;
            case 's': opts.size = 1; break;
            case 'p': opts.permissions = 1; break;
            case 'd': opts.dates = 1; break;
            case 'l': opts.links = 1; break;
            case 'v': opts.verbose = 1; break;
            case 'h': opts.human = 1; break;
            case 'a': opts.all = 1; break;
            default: fprintf(stderr, "Usage: %s [directory] [options]\n", argv[0]); exit(EXIT_FAILURE);
        }
    }
    
    if (optind < argc) path = argv[optind];
    
    init_stats(&stats, path);
    scan_directory(path, &opts, &stats);
    print_stats(&opts, &stats);
    
    return 0;
}