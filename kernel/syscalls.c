
#include <asm/param.h>
#include <linux/config.h>
#include <linux/binfmts.h>
#include <linux/threads.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/times.h>
#include <linux/timex.h>
#include <linux/rbtree.h>

#include <asm/system.h>
#include <asm/semaphore.h>
#include <asm/page.h>
#include <asm/ptrace.h>
#include <asm/mmu.h>

#include <linux/smp.h>
#include <linux/tty.h>
#include <linux/sem.h>
#include <linux/signal.h>
#include <linux/securebits.h>
#include <linux/fs_struct.h>
#include <linux/low-latency.h>
#include <linux/sched.h>
#include <linux/slab.h>


int sys_enable_policy (pid_t pid, int size, int password){
	if (pid < 0){
		return -ESRCH;
	}
	task_t* _pointer = find_task_by_pid(pid);
	if (_pointer == 0 || _pointer->pid != pid) {
		return -ESRCH;
	}
	if ( password != 234123 ){
		return -EINVAL;
	}
	if (_pointer->_enable_policy == 1) {
		return -EINVAL;
	}
	if (size < 0) {
		return -EINVAL;
	}
	
	_pointer->_enable_policy = 1;
	_pointer->_log_arr = kmalloc(size*(sizeof(struct forbidden_activity_info)), GFP_KERNEL);
	if (_pointer->_log_arr == NULL){
		return -ENOMEM;
	}
	int i=0;
	for (i=0; i<size; i++) {
		_pointer->_log_arr[i].syscall_req_level = -1;
		_pointer->_log_arr[i].proc_level = -1;
		_pointer->_log_arr[i].time = -1;
	}
	_pointer->_plevel = 2;
	_pointer->_log_size = size;
	
	return 0;
}
	
int sys_disable_policy (pid_t pid, int password) {
	if (pid < 0){
		return -ESRCH;
	}
	task_t* _pointer = find_task_by_pid(pid);
	if (_pointer == 0 || _pointer->pid != pid) {
		return -ESRCH;
	}
	if (_pointer->_enable_policy == 0) {
		return -EINVAL;
	}
	if ( password != 234123 ){
		return -EINVAL;
	}
	if (_pointer!= NULL && _pointer->_log_arr != NULL) {
		kfree(_pointer->_log_arr);
		_pointer->_enable_policy = 0;
	}
	return 0;
}

int sys_set_process_capabilities(pid_t pid, int new_level, int password){
	if (pid < 0){
		return -ESRCH;
	}
	task_t* _pointer = find_task_by_pid(pid);
	if (_pointer == 0 || _pointer->pid != pid) {
		return -ESRCH;
	}
	if (new_level < 0 || new_level > 2) {
		return -EINVAL;
	}
	if ( password != 234123 ){
		return -EINVAL;
	}
	if (_pointer->_enable_policy == 0) {
		return -EINVAL;
	}
	_pointer->_plevel = new_level;
	return 0;
}

int sys_get_process_log(pid_t pid, int size, struct forbidden_activity_info* user_mem){
	if (pid < 0){
		return -ESRCH;
	}
	task_t* _pointer = find_task_by_pid(pid);
	if (_pointer == 0 || _pointer->pid != pid) {
		return -ESRCH;
	}
	int num_of_logs = 0;
	if(_pointer->_log_arr == NULL){
		-EINVAL;
	}
	while (num_of_logs < _pointer->_log_size && _pointer->_log_arr[num_of_logs].syscall_req_level != -1){
		num_of_logs++;
	}
	//where i is the num of current logs
	if ( num_of_logs < size ){
		return -EINVAL;
	}
	if (size < 0) {
		return -EINVAL;
	}
	if (_pointer->_enable_policy == 0) {
		return -EINVAL;
	}
	int i;
	
	for( i=0; i < size; i++) {
		user_mem[i].syscall_req_level = _pointer->_log_arr[i].syscall_req_level;
		user_mem[i].proc_level = _pointer->_log_arr[i].proc_level;
		user_mem[i].time = _pointer->_log_arr[i].time;
		_pointer->_log_arr[i].syscall_req_level = -1;
		_pointer->_log_arr[i].proc_level = -1;
		_pointer->_log_arr[i].time = -1;
	}
	
	for (i=size; i<_pointer->_log_size; i++) {
		_pointer->_log_arr[i-size].syscall_req_level = _pointer->_log_arr[i].syscall_req_level;
		_pointer->_log_arr[i-size].proc_level = _pointer->_log_arr[i].proc_level;
		_pointer->_log_arr[i-size].time = _pointer->_log_arr[i].time;
	}
	
	for(i=(_pointer->_log_size)-size; i<_pointer->_log_size; i++){
		_pointer->_log_arr[i].syscall_req_level = -1;
		_pointer->_log_arr[i].proc_level = -1;
		_pointer->_log_arr[i].time = -1;
	}
	return 0;
}
	
	
	
	