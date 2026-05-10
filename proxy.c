#include <linux/device-mapper.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "proxy"


struct request {
	sector_t begin;
	sector_t end;
	bool is_write;

	struct list_head list;
};

struct target_info {
	struct dm_dev *dev;
	struct list_head list;
	spinlock_t lock;
};

struct bio_context {
	struct request *node;
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

	INIT_LIST_HEAD(&target->list);

	int err = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &target->dev);
	if (err) {
		ti->error = "Device lookup failed";
		error = -EINVAL;
		goto bad;
	}

	// Rememer target with information
	ti->private = target;

	// Init spinlock
	spin_lock_init(&target->lock);

	printk(KERN_INFO "Device %s successfully mapped\n", argv[0]);

	return 0;

bad:

	if (target)
		kfree(target);

	return error;
}

// Optimization... because this function is called in loop :-)
static inline bool intersection(struct request *req1, struct request *req2)
{
	if (req1->is_write == false && req2->is_write == false)	
		return false;

	return !(req1->end <= req2->begin || req2->end <= req1->begin);
}

static int proxy_map(struct dm_target *ti, struct bio *bio)
{
	struct target_info *target = ti->private;

	sector_t begin = bio->bi_iter.bi_sector;
	sector_t end = begin + bio_sectors(bio);

	struct request *req = kmalloc(sizeof(struct request), GFP_KERNEL);
	if (req == NULL) {
		ti->error = "Not enough memory";
		return DM_MAPIO_KILL;
	}
	
	req->begin = begin;
	req->end = end;
	req->is_write = op_is_write(bio->bi_opf);

	INIT_LIST_HEAD(&req->list);

	unsigned long flags;
	spin_lock_irqsave(&target->lock, flags);

	struct request *cur;
	list_for_each_entry(cur, &target->list, list) {
		if (intersection(cur, req)) {
			printk(KERN_WARNING "Data racing");
		}
	}

	ti->per_io_data_size = sizeof(struct bio_context);
	struct bio_context *ctx = dm_per_bio_data(bio, ti->per_io_data_size);
	ctx->node = req;

	list_add(&req->list, &target->list);
	ctx->node = req;

	spin_unlock_irqrestore(&target->lock, flags);

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
	struct target_info *target = ti->private;
	struct bio_context *ctx = dm_per_bio_data(bio, ti->per_io_data_size);

	unsigned long flags;
	spin_lock_irqsave(&target->lock, flags);

	list_del(&ctx->node->list);

	spin_unlock_irqrestore(&target->lock, flags);

	kfree(ctx->node);

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
