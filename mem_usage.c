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
/*
 * File table:
 * Name	| PID | State | MEM use | MEM max | MEM min
 */
#define TABLE_HEADER "Name\t|\tPID\t|\tState\t|\tMEM use\t|\tMEM max\t|\tMEM min\t|\n"

struct Mem_usage {
	unsigned long int curr, min, max;
};

/*
 * USER DATA END
*/
#define PROCFS_MAX_SIZE 1024U

static struct proc_dir_entry *procentry;

static char procfs_buffer[PROCFS_MAX_SIZE];
static int procfs_buff_size = 0;

static int my_open(struct inode *sp_inode, struct file *sp_file)
{
	printk(KERN_INFO "/proc/%s called open\n", FILE_NAME);

	return 0;
}

static ssize_t my_read(struct file *sp_file, char __user *buffer, size_t size, loff_t *offset)
{
	printk(KERN_INFO "/proc/%s called raed\n", FILE_NAME);

 	if (offset > 0) {
		/* we have finished to read, return 0 */
		return 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_buffer, procfs_buff_size);
		return procfs_buff_size;
	}
		// fprintf(fptr, "%s\t|\t%d\t|\t%c\t|\t%ld\t|\t%ld\t|\t%ld\t|\tKB\n",
		// 	proc_info->cmd, proc_info->tid, proc_info->state, mem->curr, mem->max, mem->min);
	// copy_to_user(buffer, message, len);

	return 0;
}

static ssize_t my_write(struct file* file,const char __user *buffer,size_t count,loff_t *f_pos)
{
	printk("/proc/%s called write\n", FILE_NAME);

	/* get buffer size */
	procfs_buff_size = count;
	if (procfs_buff_size > PROCFS_MAX_SIZE ) {
		procfs_buff_size = PROCFS_MAX_SIZE;
	}
	
	/* write data to the buffer */
	if (copy_from_user(procfs_buffer, buffer, procfs_buff_size) ) {
		return -EFAULT;
	}
	
	return procfs_buff_size;
}

static int my_close(struct inode *sp_node, struct file *sp_file)
{
	printk("proc called release\n");

	return 0;
}

static struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.release = my_close
};

static int __init mem_usage_init(void)
{
	procentry = proc_create(ENTRY_NAME, PERMS, NULL, NULL);
	if(procentry == NULL) {
		printk(KERN_ALERT "ERROR: could not initialize /proc/%s", FILE_NAME);
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;	
	}

	printk(KERN_INFO "/proc/%s file created\n", ENTRY_NAME);
	return 0;
}

static void __exit mem_usage_exit(void)
{
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_INFO "/proc/%s removed\n", ENTRY_NAME);
}

module_init(mem_usage_init);
module_exit(mem_usage_exit);
