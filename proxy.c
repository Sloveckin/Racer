/// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2003 Jana Saout <jana@saout.de>
 *
 * This file is released under the GPL.
 */

#include <linux/device-mapper.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "zero"

/*
 * Construct a dummy mapping that only returns zeros
 */
static int ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	if (argc != 1) {
		ti->error = "No arguments required";
		return -EINVAL;
	}

	printk(KERN_INFO "Device %s successfully mapped\n", argv[0]);
	return 0;
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
