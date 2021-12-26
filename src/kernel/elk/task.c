#include "task.h"

__nv thread_t _threads[MAX_THREAD_NUM];

/**  
 * Assigns a slot to a thread. Should be called ONLY at the first system boot.
 * @para.   size: in byte
 */
void __create_thread(int8_t priority, void *data_org, void *data_temp, uint16_t size){
    _threads[priority].priority                 = priority;
    _threads[priority].buffer.buf[0]            = data_org;         // working buffer
    _threads[priority].buffer.buf[1]            = data_temp;        // backup buffer
    _threads[priority].buffer.size              = size;
    _threads[priority].idx_of_first_empty_task  = 0;
}

/** 
 * Init. a new TASK. 
 * @para.   start_used_offset: in byte
 *          end_used_offset: in byte
 */
void __init_task(uint8_t priority, void *task_entry, uint16_t start_used_offset, uint16_t end_used_offset){
    uint16_t temp_index;

    temp_index = _threads[priority].idx_of_first_empty_task;
    _threads[priority].task_array[temp_index].fun_entry = task_entry;
    _threads[priority].task_array[temp_index].task_idx  = temp_index;
    _threads[priority].task_array[temp_index].ck_set.start_used_offset  = start_used_offset;
    _threads[priority].task_array[temp_index].ck_set.end_used_offset    = end_used_offset;
    _threads[priority].idx_of_first_empty_task++;
}
