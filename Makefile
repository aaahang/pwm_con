
KERN_DIR = /home/stoicus/imx6ull/bsp/100ask_imx6ull-sdk/Linux-4.9.88
INCDIR := /home/stoicus/imx6ull/source/tem_con/get_data
TARGET := pwm_con
obj-m = $(TARGET).o
EXTRA_CFLAGS += -I$(INCDIR)
$(TARGET)-objs := con_drv.o

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	 $(CROSS_COMPILE)gcc -o con_test con_test.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f con_test

