obj-m += mem_usage.o

SRCS := $(shell pwd)

all: modules

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules

clean:
	make -C $(KERNEL_SRC) M=$(SRC) clean