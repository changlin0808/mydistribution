
CC=gcc
CPPFLAGS=-I ./inc/
CFLAGS=-Wall 
LIBS=-lfcgi -lhiredis
VPATH=./src
#找到当前目录下所有的.c文件
#src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
#obj = $(patsubst %.c, %.o, $(src))



target=upload data 

ALL:$(target)
data:echo.o make_log.o cJSON.o 
	$(CC) $^ -o $@ $(LIBS)
upload:upload_cgi.o util_cgi.o make_log.o cJSON.o
	$(CC) $^ -o $@ $(LIBS)
#$(obj):%.o:%.c
#	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(LIBS)
#$(target):$(obj)
#	$(CC) $^ -o $@ $(LIBS)
%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(LIBS)

open:
	spawn-fcgi -a 127.0.0.1 -p 8085 -f ./data
	spawn-fcgi -a 127.0.0.1 -p 8083 -f ./upload

#clean指令

clean:
	rm -rf  $(target) *.o

distclean:
	-rm -rf $(obj) $(target)

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
