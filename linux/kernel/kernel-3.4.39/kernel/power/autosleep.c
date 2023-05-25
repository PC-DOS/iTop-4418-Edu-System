/*
 * kernel/power/autosleep.c
 *
 * Opportunistic sleep support.
 *
 * Copyright (C) 2012 Rafael J. Wysocki <rjw@sisk.pl>
 */

#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/pm_wakeup.h>

/* add by cym 20160616 for wifi suspend */
#ifdef CONFIG_MTK_COMBO_MT66XX
#include <linux/fs.h>
#include <linux/uaccess.h>
#endif
/* end add */

#include "power.h"

static suspend_state_t autosleep_state;
static struct workqueue_struct *autosleep_wq;
/*
 * Note: it is only safe to mutex_lock(&autosleep_lock) if a wakeup_source
 * is active, otherwise a deadlock with try_to_suspend() is possible.
 * Alternatively mutex_lock_interruptible() can be used.  This will then fail
 * if an auto_sleep cycle tries to freeze processes.
 */
static DEFINE_MUTEX(autosleep_lock);
static struct wakeup_source *autosleep_ws;

/* add by cym 20160616 for wifi suspend */
#ifdef CONFIG_MTK_COMBO_MT66XX
void wifi_ctrl(int type)
{
        struct file *fp, *fp1;
        mm_segment_t fs;
        loff_t pos = 0;

        fp = filp_open("/dev/wmtWifi", O_RDWR | O_CREAT, 0644);
        if (IS_ERR(fp)) {
                printk("create file error\n");
                return -1;
        }

        fs = get_fs();

        set_fs(KERNEL_DS);

        pos = 0;
        if(1 == type)
	{
		vfs_write(fp, "1", 1, &pos);
	}
        else if(0 == type)
	{
		vfs_write(fp, "0", 1, &pos);
	}
	else if(2 == type)
	{
		fp1 = filp_open("/sys/power/wake_unlock", O_RDWR | O_CREAT, 0644);
                if (IS_ERR(fp)) {
                        printk("create file wake_unlock error\n");
                        return -1;
                }

                pos = 0;

                vfs_write(fp1, "PowerManagerService.WakeLocks", 29, &pos);

                filp_close(fp1, NULL);
	}

        filp_close(fp, NULL);

        set_fs(fs);
}
#endif
/* end add */

static void try_to_suspend(struct work_struct *work)
{
	/* add by cym 20160616 for wifi suspend */
#ifdef CONFIG_MTK_COMBO_MT66XX
	int error;
#endif
/* end add */

	unsigned int initial_count, final_count;

	if (!pm_get_wakeup_count(&initial_count, true))
		goto out;

	mutex_lock(&autosleep_lock);

	if (!pm_save_wakeup_count(initial_count)) {
		mutex_unlock(&autosleep_lock);
		goto out;
	}

	if (autosleep_state == PM_SUSPEND_ON) {
		mutex_unlock(&autosleep_lock);
		return;
	}
	if (autosleep_state >= PM_SUSPEND_MAX)
		hibernate();
	else
/* modify by cym 20160616 for wifi suspend */
#if 0
		pm_suspend(autosleep_state);
#else
#ifdef CONFIG_MTK_COMBO_MT66XX
	{
		wifi_ctrl(0);
		error = pm_suspend(autosleep_state);
	}
#endif
#endif
/* end add */

	mutex_unlock(&autosleep_lock);

	if (!pm_get_wakeup_count(&final_count, false))
		goto out;

	/*
	 * If the wakeup occured for an unknown reason, wait to prevent the
	 * system from trying to suspend and waking up in a tight loop.
	 */
	if (final_count == initial_count)
		schedule_timeout_uninterruptible(HZ / 2);

 out:
	queue_up_suspend_work();

/* add by cym 20160616 for wifi suspend */
#ifdef CONFIG_MTK_COMBO_MT66XX
	if(!error)
                wifi_ctrl(1);
	else if(-16 == error)
	{
		//printk("**************************fun:%s error = %d\n", __FUNCTION__, error);
		wifi_ctrl(2);
	}
#endif
/* end add */
}

static DECLARE_WORK(suspend_work, try_to_suspend);

void queue_up_suspend_work(void)
{
	if (!work_pending(&suspend_work) && autosleep_state > PM_SUSPEND_ON)
		queue_work(autosleep_wq, &suspend_work);
}

suspend_state_t pm_autosleep_state(void)
{
	return autosleep_state;
}

int pm_autosleep_lock(void)
{
	return mutex_lock_interruptible(&autosleep_lock);
}

void pm_autosleep_unlock(void)
{
	mutex_unlock(&autosleep_lock);
}

int pm_autosleep_set_state(suspend_state_t state)
{

#ifndef CONFIG_HIBERNATION
	if (state >= PM_SUSPEND_MAX)
		return -EINVAL;
#endif

	__pm_stay_awake(autosleep_ws);

	mutex_lock(&autosleep_lock);

	autosleep_state = state;

	__pm_relax(autosleep_ws);

	if (state > PM_SUSPEND_ON) {
		pm_wakep_autosleep_enabled(true);
		queue_up_suspend_work();
	} else {
		pm_wakep_autosleep_enabled(false);
	}

	mutex_unlock(&autosleep_lock);
	return 0;
}

int __init pm_autosleep_init(void)
{
	autosleep_ws = wakeup_source_register("autosleep");
	if (!autosleep_ws)
		return -ENOMEM;

	autosleep_wq = alloc_ordered_workqueue("autosleep", 0);
	if (autosleep_wq)
		return 0;

	wakeup_source_unregister(autosleep_ws);
	return -ENOMEM;
}
