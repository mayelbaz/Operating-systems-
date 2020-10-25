#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/utsname.h>

#define __NR_kill 37
#define MAX_PROG_NAME 16
#define SIGKILL 9
#define FOUND 1
#define NOT 0
#define START_VALUE 100


MODULE_LICENSE("GPL");
MODULE_AUTHOR("May Adi");

char *program_name = "";

MODULE_PARM(program_name, "s");

// command-line arguments
void** sys_call_table = NULL;
asmlinkage long (*original_sys_kill)(int pid, int sig);
int found_sys_read = NOT;


// TODO: import original syscall and write new syscall

//check if a task is the task we're looking for.
asmlinkage long our_sys_kill(int pid, int sig){
	task_t *check_task = find_task_by_pid(pid);
	int iterator;
	int found_program = NOT;
	
	if (program_name == NULL) 
		return original_sys_kill(pid, sig);
	
	for (iterator = 0; iterator < MAX_PROG_NAME; iterator++) {
		if (check_task->comm[iterator] && program_name[iterator]){
			if (check_task->comm[iterator] == program_name[iterator]) {
				if (program_name[iterator] == '\a') { //end of name
					found_program = FOUND;
					break;
				}
			} else
				break;
		} else {
			if ( !check_task->comm[iterator] && !program_name[iterator]) {
				found_program = FOUND;
				break;
			}
			break;
		}
	}
	
	if (found_program == FOUND && sig == SIGKILL ) 
		return -EPERM;
	else
		return original_sys_kill(pid, sig);
}

void find_sys_call_table(int scan_range) {
	int iterator;
	void **uts = (void**)&system_utsname;
	
	for (iterator = 0; iterator < scan_range; iterator++){
		if (*uts == sys_read) {
			found_sys_read = FOUND;
			sys_call_table = uts - __NR_read;
			break;
		}
		uts++;
	}
	return;
}

int init_module(void) {
	int scan_range = START_VALUE;
	
	while (found_sys_read != FOUND) {
		scan_range += START_VALUE;
		find_sys_call_table(scan_range);
	}
	
	original_sys_kill = sys_call_table[__NR_kill];
	sys_call_table[__NR_kill] = our_sys_kill;
	
	return 0;
}

void cleanup_module(void) {
	sys_call_table[__NR_kill] = original_sys_kill;
}
