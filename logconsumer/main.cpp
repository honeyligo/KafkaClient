#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ustd/ustd_string.h"
#include "ustd/ustd_timer.h"
#include "ustd/ustd_system.h"
#include "Consumer.h"

enum command_type_t
{
	STOP_,
	RELOAD_,
	NULL_
};

static command_type_t get_command_type(const char *cmd)
{
	if (strcmp(cmd, "-stop") == 0)
	{
		return STOP_;
	}
	else if (strcmp(cmd, "-reload") == 0)
	{
		return RELOAD_;
	}

	return NULL_;
}

static void kill_process(const std::vector<int>& pids)
{
	ustd::Consumer::getInstance()->save(true);
	for (size_t i = 0; i < pids.size(); ++i)
	{
		if (getpid() != pids[i])
		{
			kill(pids[i], SIGKILL);
		}
	}
}

int main(int argc, char* argv[])
{	
	if (argc > 1)
	{
		std::vector<int> pids;
		std::string process_name = argv[0];
		process_name = process_name.substr(process_name.find_last_of("/") + 1);
		ustd::system::get_process_id(process_name.c_str(), pids);

		switch (get_command_type(argv[1]))
		{
		case command_type_t::RELOAD_:
			{
				kill_process(pids);
				break;
			}

		case command_type_t::STOP_:
			{
				kill_process(pids);
				return 0;
			}

		default:
			{
				printf("logconsumer: invalid option: \"%s\".\n", argv[1]);
				return 0;
			}
		}
	}

	pid_t pid = fork();
	if (pid != 0)
	{
		exit(0);
	}

	setsid();

	if (!ustd::Consumer::getInstance()->start())
	{
		printf("logconsumer server start error!\n");
		return 0;
	}

	ustd::Consumer::getInstance()->wait();

	return 0;
}
