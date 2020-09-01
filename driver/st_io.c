#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <linux/rmap.h>
#include <linux/pagemap.h>

#include "st.h"
#include "st_io.h"

static void st_deferred_io_work(struct work_struct *work)
{
	struct st_param *param = container_of(work, struct st_param,
			deferred_work.work);
	struct st_deferred_io *stdefio = param->stdefio;

	mutex_lock(&stdefio->lock);
	stdefio->deferred_io(param);
	mutex_unlock(&stdefio->lock);
}

vm_fault_t st_deferred_io_mkwrite(struct vm_fault *vmf)
{
	struct page *page = vmf->page;
	struct st_param *st_param = vmf->vma->vm_private_data;
	struct st_deferred_io *stdefio = st_param->stdefio;
	bool ret;

	pr_err("%s: enter\n", __func__);

	file_update_time(vmf->vma->vm_file);

	mutex_lock(&stdefio->lock);
	lock_page(page);
	page_mkclean(page);
	mutex_unlock(&stdefio->lock);

	ret = schedule_delayed_work(&st_param->deferred_work, stdefio->delay);
	if (!ret)
		pr_debug("%s: can't schedule the work %lX\n",
			__func__, (unsigned long)&st_param->deferred_work);
	return VM_FAULT_LOCKED;
}
EXPORT_SYMBOL(st_deferred_io_mkwrite);

void st_deferred_io_init(struct st_param *param)
{
	struct st_deferred_io *stdefio = param->stdefio;
	mutex_init(&stdefio->lock);
	INIT_DELAYED_WORK(&param->deferred_work, st_deferred_io_work);
}
EXPORT_SYMBOL(st_deferred_io_init);

void st_deferred_io_cleanup(struct st_param *param)
{
	cancel_delayed_work_sync(&param->deferred_work);
}
EXPORT_SYMBOL(st_deferred_io_cleanup);
