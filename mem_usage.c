#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <proc/readproc.h>

#define ARG_MAX_LENGTH 100
#define FILE_NAME "mem_usage.txt"
#define PATH_TO_FILE "subfolder"//"/proc"

// static const char *table_header =
// 	"Name\t|\tPID\t|\tState\t|\tMEM use\t|\tMEM max\t|\tMEM min\t|\n";

/*
 * File table:
 * Name	| PID | State | MEM use | MEM max | MEM min
 */
#define TABLE_HEADER "Name\t|\tPID\t|\tState\t|\tMEM use\t|\tMEM max\t|\tMEM min\t|\n"


struct Mem_usage {
	unsigned long int curr, min, max;
};

static int file_write(proc_t *proc_info, struct Mem_usage *mem)
{
	int err = -1;

	char *pathToFile = calloc(sizeof(PATH_TO_FILE) + sizeof(FILE_NAME), sizeof(char));
	sprintf(pathToFile, "%s/%s", PATH_TO_FILE, FILE_NAME);
	fprintf(stderr, "Full path name: %s\n", pathToFile);

	FILE *fptr = fopen(pathToFile, "a");
	
	static int header = 0;
	if (fptr != NULL) {
		if (header == 0) {
			header = 1;
			fprintf(fptr, TABLE_HEADER);
		}
		fprintf(fptr, "%s\t|\t%d\t|\t%c\t|\t%ld\t|\t%ld\t|\t%ld\t|\tKB\n",
			proc_info->cmd, proc_info->tid, proc_info->state, mem->curr, mem->max, mem->min);
		fclose(fptr);

		err = 0;
	}
	else {
		printf("A file don't created!\n");
	}

	free(pathToFile);

	return err;
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
	}
	
	if (err == 0) {
		err = file_write(&proc_info, mem);
	}
	else {
		printf("%s this process not found!\n", name);
	}

	return err;
}

int main(int argc, char *argv[])
{
	// char name[ARG_MAX_LENGTH];
	char *name = NULL;
	int period = 1;

	if(argc == 1) {
		fprintf(stdout, "Use -n NAME - for process search\n\t-p NUMBER - for setting the period, in seconds\n");
		return 0;
    }

	int opt;
	while ((opt = getopt(argc, argv, "n:p:")) != -1) {
		switch (opt) {
		case 'n':
			if (strlen(optarg) >= ARG_MAX_LENGTH) {
				fprintf(stderr, "Maximum name length is 100 symbols\n");
				return -1;
			}
			else {
				name = optarg;
			}
			break;
		case 'p':
			period = atoi(optarg);
			if (period <= 0) {
				fprintf(stderr, "Minimum value of period equal 1\n");
				return -1;
			}
			break;
		case '?':
			if (optopt == 'n') {
				fprintf(stderr,
					"Enter process name");
			}
			else if (optopt == 'p') {
				fprintf(stderr,
					"Enter period of polling, in seconds");
			}
			else {
				fprintf (stderr,
					"Unknown option %c\n",
					(char)optopt);
			}
			return 1;
			break;
		case ':':
			// todo:
			break;
		default:
			break;
		}
	}

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

