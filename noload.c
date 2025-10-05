#define pr_fmt(fmt) "%s:%s():%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/sched/sysctl.h>

#define DEFAULT_TIMEOUT_SECS 60

static unsigned int nthreads = 10000;
module_param(nthreads, uint, 0444);
MODULE_PARM_DESC(nthreads, "Number of threads to create (default: 10000)");

struct task_struct **kthreads;

static unsigned long timeout_secs(void) {
#if IS_ENABLED(CONFIG_DETECT_HUNG_TASK)
    extern unsigned long sysctl_hung_task_timeout_secs __attribute__((weak));
    if (&sysctl_hung_task_timeout_secs)
        return sysctl_hung_task_timeout_secs
                   ? sysctl_hung_task_timeout_secs / 2 + 1
                   : CONFIG_DEFAULT_HUNG_TASK_TIMEOUT;
#endif

    return DEFAULT_TIMEOUT_SECS;
}

static int kthread_fn(void *unused) {
    while (!kthread_should_stop())
        schedule_timeout_uninterruptible(timeout_secs() * HZ);

    return 0;
}

static int __init noload_init(void) {
    int i, ret;

    if (nthreads == 0) {
        pr_err("nthreads must be greater than 0.\n");
        return -EINVAL;
    }

    kthreads = vmalloc_array(nthreads, sizeof(struct task_struct *));
    if (!kthreads)
        return -ENOMEM;

    for (i = 0; i < nthreads; i++) {
        struct task_struct *k = kthread_run(kthread_fn, NULL, "noload/%07d", i);
        if (IS_ERR(k)) {
            ret = PTR_ERR(k);
            goto err;
        }
        kthreads[i] = k;
    }

    return 0;

err:
    for (int j = 0; j < i; j++)
        if (kthreads[j] != NULL)
            kthread_stop(kthreads[j]);

    vfree(kthreads);
    return ret;
}

static void __exit noload_exit(void) {
    for (int i = 0; i < nthreads; i++)
        if (kthreads[i] != NULL)
            kthread_stop(kthreads[i]);

    vfree(kthreads);
    return;
}

module_init(noload_init);
module_exit(noload_exit);

MODULE_LICENSE("GPL");
