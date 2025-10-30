CC=gcc
CFLAGS:=-Iinc -Wall -DMAIN     
LDFLAGS:=-lm


BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
SRC_DIR=src
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))


DATA=$(SRC_DIR)/data.c
DATA_OBJ=$(OBJ_DIR)/data.o


ifeq ($(wildcard $(DATA)),)
    OBJ += $(DATA_OBJ)
endif


TARGET=$(BUILD_DIR)/eng


default: $(TARGET)


$(DATA): $(BUILD_DIR)/precomp.c inc/data.h
	gcc -Iinc $(BUILD_DIR)/precomp.c -o $(BUILD_DIR)/precomp
	$(BUILD_DIR)/precomp $(CURDIR)/$(DATA)	


$(TARGET): $(DATA) $(OBJ)  
	gcc -o $(TARGET) $(OBJ) $(LDFLAGS) 


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c  
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@	


test: CFLAGS=-Iinc -Itest/inc -g -Wall -DTEST 


test: TARGET=$(BUILD_DIR)/test


test: $(TARGET) 


clean:
	rm -f $(TARGET) ; rm -rf $(OBJ_DIR) ; rm -f $(BUILD_DIR)/precomp

clean-test:
	rm -f $(BUILD_DIR)/test ; rm -rf $(OBJ_DIR)
