#ifndef __USTD_SYSTEM_H__
#define __USTD_SYSTEM_H__

#include <vector>
#include <string>
#include <string.h>
#include "ustd_string.h"

namespace ustd
{
namespace system
{

static size_t get_process_id(const char *name, std::vector<int> &pids)
{
	char cmd[128];
	sprintf(cmd, "ps -ef | grep %s", name);

	FILE *fp = popen(cmd, "r");
	if (fp != NULL)
	{
		char buffer[1024];
		while (fgets(buffer, 1024, fp) != NULL)
		{
			if (strstr(buffer, "grep") != NULL)
			{
				continue;
			}

			char *pid = NULL;
			if (pid = strtok(buffer, " "))
			{
				if (pid = strtok(NULL, " "))
				{
					pids.push_back(strtol(pid, NULL, 10));
				}
			}
		}

		pclose(fp);
	}

	return pids.size();
}

}
}

#endif
