dd oflag=direct if=/dev/urandom of=/dev/mapper/my0 \

bs=32k count=1 seek=4 &

dd oflag=direct if=/dev/urandom of=/dev/mapper/my0 \

bs=8k count=1 seek=17