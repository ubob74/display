#ifndef _ST_IO_H_
#define _ST_IO_H_

struct st_param;
struct vm_fault;

void st_deferred_io_init(struct st_param *);
void st_deferred_io_cleanup(struct st_param *);
vm_fault_t st_deferred_io_mkwrite(struct vm_fault *vmf);

#endif
