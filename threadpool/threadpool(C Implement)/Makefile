CC=gcc  	   #编译器名字
TARGET=out         #生成可执行文件名

LIB= -lpthread    #用到的库名字

DIRS= .   	#需要扫描的文件夹 

FILES=$(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))  #扫描文件夹下面的.c 文件

OBJS=$(patsubst %.c,%.o,$(FILES))                    #将对应的c文件转化成.o 文件

	
$(TARGET):$(OBJS)
	$(CC)  -o $(TARGET) $(OBJS) $(LIB)
#$(OBJS):%.o:%.c
#	$(CC)  -c -D_7ZIP_ST $< -o $@
	
clean:
	-$(RM) $(TARGET)                                 
	-$(RM) $(OBJS)							




