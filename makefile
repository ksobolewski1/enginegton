CC=gcc
CFLAGS:=-Iinc -Wall -DMAIN     
LDFLAGS:=-lm

BUILD_DIR=./build
OBJ_DIR=./build/obj
SRC_DIR=src
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

TEST_DIR=./test
TEST_NAME=BASE 
TESTSRC:=$(wildcard $(TEST_DIR)/src/*.c)
TESTOBJ:=$(patsubst $(TEST_DIR)/src/%.c,$(OBJ_DIR)/%.o,$(TESTSRC))

DATA=$(SRC_DIR)/data.c

TARGET=$(BUILD_DIR)/eng


default: $(TARGET)   


$(DATA): $(BUILD_DIR)/precomp.c inc/data.h
	gcc -Iinc $(BUILD_DIR)/precomp.c -o $(BUILD_DIR)/precomp
	$(BUILD_DIR)/precomp $(CURDIR)/$(DATA)		


$(TARGET): $(OBJ) | $(DATA)   
	gcc -o $(TARGET) $(OBJ) $(LDFLAGS) 


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c  
	$(CC) -c $(CFLAGS) $< -o $@	


$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@


test: CFLAGS=-Iinc -Itest/inc -g -Wall -DTEST 
test: TARGET=$(BUILD_DIR)/test

test: $(TARGET) 
	$(TARGET) $(CURDIR) $(TEST_NAME)


clean:
	rm $(TARGET) ; rm $(OBJ) 

