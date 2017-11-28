CC=gcc

TARGET=server         				   							 #生成可执行文件名

ROOT_DIR=$(shell pwd)

#注意顺序
LIB= -lpthread  -ldl			 #用到的库名字

INCLUDE_DIR = -I$(ROOT_DIR)

SRC_DIRS=$(ROOT_DIR)								  	 #需要扫描的文件夹

FILES=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))  	 #扫描文件夹下面的.c 文件

OBJS=$(patsubst %.c,%.o,$(FILES))                   		 #将对应的c文件转化成.o 文件


$(TARGET):$(OBJS)
	$(CC)  -o obj/$(TARGET) $(OBJS) $(LIB_DIR) $(LIB)

$(OBJS):%.o:%.c
	$(CC)  $(INCLUDE_DIR) -c $< -o $@  $(LIB_DIR) $(LIB)


clean:
	-$(RM) $(TARGET)
	-$(RM) $(OBJS)
