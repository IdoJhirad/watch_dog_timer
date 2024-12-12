CC:= gcc
CFLAGS:= -ansi -pedantic-errors -Wall -Wextra 
DEBUG:= -g
RELEASE:= -DNDEBUG -o3

SRC_DIR:= /home/ido/git/ds/src
TEST_DIR:= ./test
BIN_DIR_DEBUG:= ./bin/debug
BIN_DIR_RELEASE:= ./bin/release
INCLUDE_DIR:= /home/ido/git/ds/include 


OBJS = $(SRC_DIR)/dvector.c \
	   $(SRC_DIR)/heap.c \
       $(SRC_DIR)/pq_heap.c \
       $(SRC_DIR)/uid.c \
       $(SRC_DIR)/task.c \
       $(SRC_DIR)/scheduler_heap.c

all:
	$(MAKE) wd_shared
	$(MAKE) wd_process
	$(MAKE) wd

wd: client_test.c libwd.so  
	$(CC) $(CFLAGS) $(DEBUG) -o wd_client client_test.c  libwd.so -I ./

wd_process: wd_process.c libwd.so
	$(CC) $(CFLAGS) $(DEBUG) -o wd_process wd_process.c wd_shared.c  libwd.so -I $(INCLUDE_DIR) -I ./

wd_shared:
	$(CC) $(CFLAGS) $(DEBUG) -fPIC -shared -o libwd.so wd.c wd_shared.c wd_process.c $(OBJS) -I $(INCLUDE_DIR) -I ./
	export LD_LIBRARY_PATH=/home/ido/git/projects/watch_dog
