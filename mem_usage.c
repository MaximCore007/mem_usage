#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maxim");
MODULE_DESCRIPTION("Program for anlayze memory usage");
MODULE_VERSION("0.01");

#define ENTRY_NAME "Memory usage"
#define PERMS 0644
#define PARENT NULL
/*
 * USER DATA BEGIN
*/
#define FILE_NAME "mem_usage.txt"
#define PATH_TO_FILE "subfolder"//"/proc"

#define MY_DATA_SIZE 1000U
static char message[MY_DATA_SIZE];

/*
 * File table:
 * Name	| PID | State | MEM use | MEM max | MEM min
 */
#define TABLE_HEADER "Name\t|\tPID\t|\tState\t|\tMEM use\t|\tMEM max\t|\tMEM min\t|\n"

/*
 * USER DATA END
*/
static struct file_operations myfops = {

};

static int my_open(struct node *sp_inode, struct file *sp_file)
{
	strcpy(message, "proc called open!\n");
	
	char *message = kzalloc(sizeof(char) * MY_DATA_SIZE, GFP_KERNEL);

	if(message == NULL) {
		printk("ERROR, don't get memory!\n");
		return -ENOMEM;
	}

	strcpy(message, "Hello World!\n");

	return 0;
}

static ssize_t my_read(struct file *sp_file, char __user *buffer, size_t size, loff_t *offset)
{
	printk("proc called raed\n");

	int len = strlen(message);

	read_p = !read_p;
	if (read_p) {
		return 0;
	}

	copy_to_user(buffer, message, len);

	return len;
}

static ssize_t my_write(struct file* file,const char __user *buffer,size_t count,loff_t *f_pos)
{
	printk("proc called write\n");

	return 0;
}

static int my_close(struct inode *sp_node, struct file *sp_file)
{
	printk("proc called release\n");
	kfree(message);

	return 0;
}

static int __init mem_usage_init(void)
{
	myfops.owner = THIS_MODULE;
	myfops.open = my_open;
	myfops.read = my_read;
	myfops.write = my_write;
	myfops.release = my_close;

	struct proc_dir_entry *entry;
	entry = proc_create(ENTRY_NAME, PERMS, NULL, &myfops);
	if(!entry){
		printk("ERROR! proc_create");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;	
	}

	printk(KERN_INFO "/proc/%s file create successfully!\n", ENTRY_NAME);
	return 0;
}

static void __exit mem_usage_exit(void)
{
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_INFO "removing /proc/%s.\n", ENTRY_NAME);
}

module_init(mem_usage_init);
module_exit(mem_usage_exit);

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

