#include <linux/device-mapper.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "proxy"

struct target_info {
	struct dm_dev *dev;
};

static int ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct target_info *target;
	int error = 0;

	if (argc != 1) {
		ti->error = "Invalid amount of arguments";
		error = -EINVAL;
		goto end;
	}

	target = kmalloc(sizeof(struct target_info*), GFP_DMA);
	if (target == NULL) {
		error = -ENOMEM;
		goto end;
	}

	// Rememer target with information
	ti->private = target;

	printk(KERN_INFO "Device %s successfully mapped\n", argv[0]);

end:
	return error;
}

/*
 * Return zeros only on reads
 */
static int map(struct dm_target *ti, struct bio *bio)
{
	/*switch (bio_op(bio)) {
	case REQ_OP_READ:
		if (bio->bi_opf & REQ_RAHEAD) {
			// readahead of null bytes only wastes buffer cache
			return DM_MAPIO_KILL;
		}
		zero_fill_bio(bio);
		break;
	case REQ_OP_WRITE:
	case REQ_OP_DISCARD:
		///* writes get silently dropped 
		break;
	default:
		return DM_MAPIO_KILL;
	}*/
	

	bio_endio(bio);

	/* accepted bio, don't make new request */
	return DM_MAPIO_SUBMITTED;
}

static struct target_type zero_target = {
	.name   = "proxy",
	.version = {1, 2, 0},
	.features = DM_TARGET_NOWAIT,
	.module = THIS_MODULE,
	.ctr    = ctr,
	.map    = map,
};
module_dm(zero);

MODULE_AUTHOR("Jana Saout <jana@saout.de>");
MODULE_DESCRIPTION(DM_NAME " dummy target returning zeros");
MODULE_LICENSE("GPL");
