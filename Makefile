PWD := $(shell pwd)

ccflags-y += -I$(src)/include/

gfsharetest-objs := lkm_template.o libgfshare.o
obj-m += gfsharetest.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

maketable: maketable_build
	./gfshare_maketable > libgfshare_tables.h
	rm gfshare_maketable

maketable_build:
	gcc -std=c90 -Wall gfshare_maketable.c -o gfshare_maketable

test:
	sudo insmod gfsharetest.ko
	sleep 3
	sudo rmmod gfsharetest.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	#rm libgfshare_tables.h
