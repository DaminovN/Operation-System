#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

struct {
    bool has_inum;
    char inum[32];

    bool has_name;
    char name[1024];

    bool has_size;
    int size_cmp;
    off_t size;

    bool has_nlinks;
    nlink_t nlinks;

    bool has_path;
    char path[256];
} arguments;

void initialize() 
{
    arguments.has_inum = false;
    arguments.has_name = false;
    arguments.has_size = false;
    arguments.has_nlinks = false;
    arguments.has_path = false;
}

bool check_if_ok(struct dirent *file, char *path) 
{
    struct stat cur_file_stats;

    if (stat(path, &cur_file_stats) < 0)
        {
	perror("Could not open stat");
	return false;
	}

    if (arguments.has_inum)
    {
        char str[64];
        sprintf(str, "%lu", cur_file_stats.st_ino);

        if (strcmp(arguments.inum, (str)) != 0)
            return false;
    }
    if (arguments.has_name && strcmp(arguments.name, file->d_name) != 0)
    	return false;
    
    if (arguments.has_size) 
    {
        if (arguments.size_cmp == 0) 
        {
            if (arguments.size != cur_file_stats.st_size) 
                return false;
        } 
        else if (cur_file_stats.st_size * arguments.size_cmp < arguments.size * arguments.size_cmp)
                return false;
    }
    if (arguments.has_nlinks && arguments.nlinks != cur_file_stats.st_nlink) 
    	return false;

    if (arguments.has_path) 
    {

        char *a[] = {arguments.path, path, NULL};

        int status;
        pid_t pid = fork();
        if (pid == 0) 
        {
            if (execve(arguments.path, a, NULL) < 0) 
            {
                printf("error\n");
                perror("execve failed");
                status = -1;
            }
        } 
        else if (pid > 0) 
        {
            waitpid(pid, &status, 0);
        } 
        else 
        {
            perror("fork failed");
        }

    }
    return true;
}

void recuresively_enter(DIR *current_dir, char *path) 
{
    if (current_dir == NULL)
    {
	perror("Current dir is NULL");
	return;
    }

    struct dirent *file_or_dir;
    while ((file_or_dir = readdir(current_dir)) != NULL) 
    {

        if (strcmp(file_or_dir->d_name, ".") == 0 || strcmp(file_or_dir->d_name, "..") == 0) // Current/Parent directory
            continue;

        char cur_path[256];
        strcpy(cur_path, path);
        strcat(cur_path, "/");
        strcat(cur_path, file_or_dir->d_name);

	    if (file_or_dir->d_type == DT_REG)
        {
            if (check_if_ok(file_or_dir, cur_path))
                    printf("%s\n", cur_path);
        } 
        else if (file_or_dir->d_type == DT_DIR)
        {
            DIR* dir = opendir(cur_path);
            if (dir == NULL)
            {
                printf("Unable to open directory: %s\n", cur_path);
                break;
            }
            recuresively_enter(dir, cur_path);
        } 
        else 
        {
            printf("%c Unknown format found\n", file_or_dir->d_type);
        }
    }
    closedir(current_dir);
}

int main(int argc, char *argv[]) 
{
    initialize();
    for (int i = 2; i < argc - 1; i += 2) 
    {
        if (strcmp(argv[i], "-inum") == 0) 
        {
            arguments.has_inum = true;
            strcpy(arguments.inum, argv[i + 1]);
        } 
        else if (strcmp(argv[i], "-name") == 0) 
        {
            arguments.has_name = true;
            strcpy(arguments.name, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-size") == 0) 
        {
            arguments.has_size = true;
            if (argv[i + 1][0] == '-')
            {
                arguments.size_cmp = 1;
            } 
            else if (argv[i + 1][0] == '=') 
            {
                arguments.size_cmp = 0;
            } 
            else if (argv[i + 1][0] == '+') 
            {
                arguments.size_cmp = -1;
            } 
            else 
            {
                perror("Wrong format of -size, expected: \"[-=+]%d\" was not found");
            }
            arguments.size = atol(argv[i + 1] + 1);
        } 
        else if (strcmp(argv[i], "-nlinks") == 0) 
        {
            arguments.has_nlinks = true;
            arguments.nlinks = atoi(argv[i + 1]);
        } 
        else if (strcmp(argv[i], "-exec") == 0) 
        {
            arguments.has_path = true;
            strcpy(arguments.path, argv[i + 1]);
        }
        else 
        {
        	char str[50];
        	sprintf(str, "Unknown argument number %d", i + 1);
            perror(str);
		return 0;
        }
    }
    recuresively_enter(opendir(argv[1]), argv[1]);
    return 0;
}
