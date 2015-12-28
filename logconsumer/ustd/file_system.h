#ifndef __USTD_FILE_SYSTEM_H__
#define __USTD_FILE_SYSTEM_H__

#include <vector>
#include <string>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

namespace ustd
{
namespace file_system
{

static size_t get_files(const char *path, std::vector<std::string> &files, const char *pattern = NULL)
{
    files.clear();
    char buffer[4096] = {0x00};

    DIR *pdir = opendir(path);
    if (pdir != NULL)
    {
        dirent *ent = NULL;

        while ((ent = readdir(pdir)) != NULL)
        {
            if (!(ent->d_type & DT_DIR))
            {
                if (pattern != NULL)
                {
                    if (0 == fnmatch(pattern, ent->d_name, FNM_PATHNAME | FNM_PERIOD))
                    {
                        sprintf(buffer, "%s/%s", path, ent->d_name);
                        files.push_back(buffer);
                    }
                }
                else
                {
                    sprintf(buffer, "%s/%s", path, ent->d_name);
                    files.push_back(buffer);
                }
            }
        }

        closedir(pdir);
    }   

    return (files.size());

}

static size_t get_all_files(const char *path, std::vector<std::string> &files, const char *pattern = NULL)
{
    char buffer[4096] = {0x00};
    DIR *pdir = opendir(path);

    if (pdir != NULL)
    {
        dirent *ent = NULL;
        while ((ent = readdir(pdir)) != NULL)
        {
            if (ent->d_type & DT_DIR)
            {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    sprintf(buffer, "%s/%s", path, ent->d_name);
                    get_all_files(buffer, files, pattern);
                }
            }
            else
            {
                if (pattern != NULL)
                {
                    if (0 == fnmatch(pattern, ent->d_name, FNM_PATHNAME | FNM_PERIOD))
                    {
                        sprintf(buffer, "%s/%s", path, ent->d_name);
                        files.push_back(buffer);
                    }
                }
                else
                {
                    sprintf(buffer, "%s/%s", path, ent->d_name);
                    files.push_back(buffer);
                }

            }
        }

        closedir(pdir);
    }

    return (files.size());
}

static size_t get_dirs(const char *path, std::vector<std::string> &dirs)
{
	char buffer[4096] = {0x00};
	DIR *pdir = opendir(path);

	if (pdir != NULL)
	{
		dirent *ent = NULL;
		while ((ent = readdir(pdir)) != NULL)
		{
			if (ent->d_type & DT_DIR)
			{
				if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
				{
					sprintf(buffer, "%s/%s", path, ent->d_name);
					dirs.push_back(buffer);
				}
			}
		}

		closedir(pdir);
	}

	return dirs.size();
}

static size_t get_all_dirs(const char *path, std::vector<std::string> &dirs)
{
    char buffer[4096] = {0x00};
    DIR *pdir = opendir(path);

    if (pdir != NULL)
    {
        dirent *ent = NULL;
        while ((ent = readdir(pdir)) != NULL)
        {
            if (ent->d_type & DT_DIR)
            {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    sprintf(buffer, "%s/%s", path, ent->d_name);
                    dirs.push_back(buffer);
                    get_all_dirs(buffer, dirs);
                }
            }
        }

        closedir(pdir);
    }

    return dirs.size();
}

static bool dir_exists(const char *path)
{
	DIR *pdir = opendir(path);
	if (pdir != NULL)
	{
		closedir(pdir);
		return true;
	}

	return false;
}

static bool file_exists(const char *path)
{
	if (0 == access(path, F_OK) && !dir_exists(path))
	{
		return true;
	}

	return false;
}

static std::string get_module_path()
{
	char buffer[4096];
	if (readlink("/proc/self/exe", buffer, 4096) <= 0)
	{
		return "";
	}

	char *path_end = strrchr(buffer, '/');
	if (NULL == path_end)
	{
		return "";
	}

	*path_end = '\0';

	return (buffer);
}

}
}

#endif
