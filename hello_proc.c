#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Roman Ponomarenko <R.E.P@yandex.ru>");

#define PROC_FILE_NAME "hello_proc_file"

static ssize_t read_proc(struct file *filp, char __user *buffer, size_t size,
		loff_t *offset);
static ssize_t write_proc(struct file *filp, const char __user *buffer,
		size_t size, loff_t *offset);

struct proc_dir_entry *proc_file;
const struct file_operations proc_fops = {
.owner = THIS_MODULE,
.read = read_proc,
.write = write_proc,
};

static char *msg;
static char *msg_iter;

static ssize_t read_proc(struct file *filp, char __user *buffer, size_t size,
		loff_t *offset)
{
	ssize_t bytes_read = 0;

	if (*msg_iter == 0) {
		msg_iter = msg;
		return 0;
	}
	while (size && *msg_iter) {
		put_user(*(msg_iter++), buffer++);
		--size;
		++bytes_read;
	}
	pr_info("[hello_proc] read size %ld from string: %s", size, msg);
	return bytes_read;
}

static ssize_t write_proc(struct file *filp, const char __user *buffer,
		size_t size, loff_t *offset)
{
	ssize_t bytes_write = 0;

	pr_info("[hello_proc] write %ld bytes", size);
	kfree(msg);
	msg = kmalloc(size+1, GFP_KERNEL);
	msg_iter = msg;
	while (size) {
		get_user(*(msg_iter++), buffer++);
		--size;
		++bytes_write;
	}
	if (*(msg_iter-1) != 0)
		*msg_iter = 0;
	msg_iter = msg;
	return bytes_write;
}

static int __init hello_init(void)
{
	proc_file = proc_create(PROC_FILE_NAME, 0666, NULL, &proc_fops);
	if (proc_file == NULL) {
		pr_err("[hello_proc] can't create /proc/%s\n", PROC_FILE_NAME);
		return -ENOMEM;
	}
	pr_info("[hello_proc] init\n");
	msg = kmalloc(15, GFP_KERNEL);
	if (msg == NULL)
		pr_err("[hello_proc] failed to allocate memory\n");
	msg_iter = msg;
	sprintf(msg, "Hello, world!\n");
	return 0;
}

static void __exit hello_exit(void)
{
	kfree(msg);
	proc_remove(proc_file);
	pr_info("[hello_proc] exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
