#include <stdlib.h>
#include <getopt.h>
#include "core.h"
#include "log.h"
#include "damon.h"

int arg_operation(char *operation)
{
	pr_debug("operation: %s\n", operation);
	return damon_write_operation(operation);
}

int arg_pid(int pid)
{
	pr_debug("target_pid: %d\n", pid);
	return damon_write_pid(pid);
}

int arg_exec(int argc, char *argv[])
{
	int opt;
	char *operation = NULL;
	int target_pid = -1;
	int ret = 0;

	static struct option long_options[] = {
		{"operation", required_argument, 0, 'o'},
		{"target_pid", required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "o:p:", long_options, NULL)) != -1) {
		switch (opt) {
			case 'o':
				operation = optarg;
				break;
			case 'p':
				target_pid = atoi(optarg);
				break;
			default:
				pr_err("Usage: %s --operation=vaddr --target_pid=PID\n", argv[0]);
				exit(1);
		}
	}

	if (operation)
		ret = arg_operation(operation);
	if (ret)
		goto err;

	if (target_pid > 0)
		ret = arg_pid(target_pid);
	if (ret)
		goto err;

	return 0;
err:
	return -1;
}
