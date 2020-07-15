CC = arm-none-linux-gnueabi-gcc
#CC = gcc

#debug文件夹的makefile需要最后执行，使用awk排除了debug文件夹，读取剩下的文件夹
SUBDIRS=$(shell ls -l | grep -v 'include' | grep ^d | awk '{if($$9 != "objs") print $$9}')

#记住当前工程的根目录路径
TOP_DIR=$(shell pwd)

OBJS_DIR = objs

BIN = usbVideo



#获取当前目录下的c文件集，放在变量CUR_SOURCE中
CUR_SOURCE=${wildcard *.c}

#将对应的c文件名转为o文件后放在下面的CUR_OBJS变量中
CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}

export CC BIN OBJS_DIR BIN_DIR TOP_DIR

#注意这里的顺序，需要先执行SUBDIRS最后才能是DEBUG
#all:$(SUBDIRS) $(CUR_OBJS) DEBUG

all: $(SUBDIRS) $(CUR_OBJS) LINK 

$(SUBDIRS):ECHO
	make -C $@


ECHO:
	@echo $(SUBDIRS)



#将c文件编译为o文件，并放在指定放置目标文件的目录中即OBJS_DIR
$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(TOP_DIR)/$(OBJS_DIR)/$@  $(INC_PATH)


INC_PATH = -I $(TOP_DIR) \
	-I $(TOP_DIR)/include \
        -I ../libdrm-2.4.101 \
        -I ../libdrm-2.4.101/include/drm

LINK:
	$(CC) $(TOP_DIR)/$(OBJS_DIR)/*.o -o $(BIN) $(LINK-PATH) 


LINK-PATH = -ldrm -L../libdrm-2.4.101/build/


clean:
	@rm $(OBJS_DIR)/*.o 
	@rm $(BIN)
