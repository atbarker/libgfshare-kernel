gfsharetest-objs := libgfshare.o lkm_template.o
obj-m += gfsharetest.o

all: maketable
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

maketable: maketable_build
	./gfshare_maketable > libgfshare_tables.h
	rm gfshare_maketable

maketable_build:
	gcc -std=c99 -Wall gfshare_maketable.c -o gfshare_maketable

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm libgfshare_tables.h
