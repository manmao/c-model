CC=g++	       #编译器名字

TARGET=out         				   		#生成可执行文件名

ROOT_DIR=$(shell pwd)

#注意顺序
LIB= -lstdc++ -lpthread -ldl			 #用到的库名字

INCLUDE_DIR=-I./timer

SRC_DIRS=.	./timer				  	 #需要扫描的文件夹

FILES=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))  	 #扫描文件夹下面的.c 文件

OBJS=$(patsubst %.cpp,%.o,$(FILES))                   		 #将对应的c文件转化成.o 文件


$(TARGET):$(OBJS)
	$(CC) -g  -o $(TARGET) $(OBJS) $(LIB) $(INCLUDE_DIR)

$(OBJS):%.o:%.cpp
	$(CC) -DLINUX2  -g -c $< -o $@ $(LIB) $(INCLUDE_DIR)


clean:
	-$(RM) $(TARGET)
	-$(RM) $(OBJS)
