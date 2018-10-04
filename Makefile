obj-m += lkm_template.o
lkm_template-objs := libgfshare.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

maketable: maketable_build
	./gfshare_maketable > libgfshare_tables.h
	rm gfshare_maketable

maketable_build:
	gcc -std=c99 -Wall gfshare_maketable.c -o gfshare_maketable

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
