#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <proc/readproc.h>

#define ARG_MAX_LENGTH 100
#define FILE_NAME "mem_usage.txt"

struct Mem_usage {
	unsigned long int curr, min, max;
};

static int file_write(proc_t *proc_info, struct Mem_usage *mem)
{
	/*
	 * File table:
	 * Name	| PID | State | MEM use | MEM max | MEM min
	*/
	int fd = open("FILE.csv", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

	static int header = 0;
	if (header == 0) {
		header = 1;
		printf("Name\t|\tPID\t|\tState\t|\tMEM use\t|\tMEM max\t|\tMEM min\t|\n");
	}
	printf("%s\t|\t%d\t|\t  %c\t|\t%ld\t|\t%ld\t|\t%ld\t|\tKB\n",
		proc_info->cmd, proc_info->tid, proc_info->state, mem->curr, mem->max, mem->min);
	if (fd != -1) {
		close(fd);
	}
	return 0;
}

static int process_handler(char *name, struct Mem_usage *mem)
{
	int err = -1;

	PROCTAB *proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
	proc_t proc_info;
	memset(&proc_info, 0, sizeof(proc_info));

	while (readproc(proc, &proc_info) != NULL) {
		if (strcmp(proc_info.cmd, name) == 0) {
			mem->curr = (proc_info.resident * 4096) / 1024;
			if (mem->curr < mem->min) {
				mem->min = mem->curr;
			}
			if (mem->curr > mem->max) {
				mem->max = mem->curr;
			}
			err = 0;
			break;
		}
		// printf("%d,\t%ld\t", proc_info.ppid, proc_info.resident);
		// printf("%lld,\t%lld\n", proc_info.utime, proc_info.stime);
	}
	
	if (err == 0) {
		file_write(&proc_info, mem);
	}

	return err;
}

int main(int argc, char *argv[])
{
	// char name[ARG_MAX_LENGTH];
	char *name = NULL;
	int period = 1;

	if(argc == 1) {
		printf("Add arguments\n");
		return 0;
    }

	int opt;
	while ((opt = getopt(argc, argv, "n:p:")) != -1) {
		switch (opt) {
		case 'n':
			if (strlen(optarg) >= ARG_MAX_LENGTH) {
				printf("Maximum name length is 100 symbols\n");
				return 0;
			}
			else {
				name = optarg;
				// strcpy(name, optarg);
			}
			break;
		case 'p':
			period = atoi(optarg);
			if (period <= 0) {
				printf("Minimum period = 1\n");
				return 0;
			}
			// period = atoi(optarg);
			break;
		case '?':
			if (optopt == 'n') {
				printf("Enter process name\n");
				// fprintf(stderr,
				// 	"Enter process name");
			}
			else if (optopt == 'p') {
				printf("Enter period of polling, in Seconds\n");
				// fprintf(stderr,
				// 	"Enter period of polling, in Seconds");
			}
			// else {
			// 	fprintf (stderr,
			// 		"Unknown option\n",
			// 		optopt);
			// }
			return 1;
			break;
		case ':':
			// todo:
			break;
		default:
			// todo:
			break;
		}
	}

	// printf("Options: -n is %s\t-p is %d\n", name, period);
	
	struct Mem_usage mem_usage = {.curr = 0, .max = 0};
	memset(&mem_usage.min, 0xFF, sizeof(mem_usage.min));
	
	while (1) {
		int ret = process_handler(name, &mem_usage);
		if (ret != 0) {
			return 1;
		}
		sleep(period);
	}

	return 0;
}

