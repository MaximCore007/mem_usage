#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <proc/readproc.h>

#define ARG_MAX_LENGTH 100

static int get_process(char *name)
{
	int err = -1;
	
	PROCTAB *proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
	
	proc_t proc_info;
	memset(&proc_info, 0, sizeof(proc_info));

	printf("Name\tPID\tState\tCurr. MEM\n");
	while (readproc(proc, &proc_info) != NULL) {
		if (strcmp(proc_info.cmd, name) == 0) {
			printf("%s\t%d\t%c\t%.2f MB\n",
				proc_info.cmd, proc_info.tid, proc_info.state, (float)((float)proc_info.resident * 4096.f) / (1024.f * 1000.f));
			err = 0;
			break;
		}
		// printf("%d,\t%ld\t", proc_info.ppid, proc_info.resident);
		// printf("%lld,\t%lld\n", proc_info.utime, proc_info.stime);
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
	
	while (1) {
		int ret = get_process(name);
		if (ret != 0) {
			return 1;
		}
		sleep(period);
	}

	return 0;
}

