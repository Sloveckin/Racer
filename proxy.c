#include <linux/device-mapper.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "proxy"


struct request {
	sector_t begin;
	sector_t end;
	bool is_write;
};

struct request_list {
	struct request_list *prev;
	struct request_list *next;
	struct request data;
};

struct target_info {
	struct dm_dev *dev;
};

static int proxy_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct target_info *target = NULL;
	int error = 0;

	if (argc != 1) {
		ti->error = "Invalid amount of arguments";
		error = -EINVAL;
		goto bad;
	}

	target = kmalloc(sizeof(struct target_info), GFP_KERNEL);
	if (target == NULL) {
		ti->error = "Not enough memory for target_info";
		error = -ENOMEM;
		goto bad;
	}

	int err = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &target->dev);
	if (err) {
		ti->error = "Device lookup failed";
		error = -EINVAL;
		goto bad;
	}

	// Rememer target with information
	ti->private = target;

	printk(KERN_INFO "Device %s successfully mapped\n", argv[0]);

	return 0;

bad:

	if (target)
		kfree(target);

	return error;
}

static int proxy_map(struct dm_target *ti, struct bio *bio)
{
	struct target_info *target = ti->private;
	
	bio_set_dev(bio, target->dev->bdev);

	return DM_MAPIO_REMAPPED;
}

static void proxy_dtr(struct dm_target *ti) 
{
	struct target_info *target = ti->private;
	
	printk(KERN_INFO "Device successfully unmapped\n");
	dm_put_device(ti, target->dev);
	kfree(target);
}

static int proxy_end_io(struct dm_target *ti,
                     struct bio *bio,
                     blk_status_t *error)
{
    pr_info("I/O done: sector=%llu status=%d op=%d\n",
            (unsigned long long)bio->bi_iter.bi_sector,
            *error,
            bio_op(bio));

    return DM_ENDIO_DONE;
}

static struct target_type proxy_target = {
	.name   = "proxy",
	.version = {1, 0, 0},
	.features = DM_TARGET_NOWAIT,
	.module = THIS_MODULE,
	.ctr    = proxy_ctr,
	.dtr    = proxy_dtr,
	.map    = proxy_map,
	.end_io = proxy_end_io,
};
module_dm(proxy);

MODULE_AUTHOR("Slava Rybin");
MODULE_DESCRIPTION(DM_NAME " proxy target that checks data racing");
MODULE_LICENSE("GPL");
