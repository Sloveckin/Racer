#!/bin/bash

sudo dmsetup remove_all
sudo losetup -D

truncate -s 32M disk.img
LOOP=$(sudo losetup --find --show disk.img)

SIZE=$(blockdev --getsz $LOOP)

sudo dmsetup create my0 --table "0 $SIZE proxy $LOOP"