CC = gcc

# lncursesw is used for UTF8 support
CFLAGS = -g -Wall -lncursesw 

BUILD_DIR = build
BIN_DIR = .
SRC_DIR = src

# create build dir
$(shell mkdir -p $(BUILD_DIR))

# target: dependencies
# 	  action

default: ui.o utils.o index.o main.o
	$(CC) $(BUILD_DIR)/ui.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/index.o $(BUILD_DIR)/main.o $(CFLAGS) -o $(BIN_DIR)/complot

utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h
	$(CC) -c $(SRC_DIR)/utils.c -o $(BUILD_DIR)/utils.o

main.o: $(SRC_DIR)/main.c 
	$(CC) -c $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.o

ui.o: $(SRC_DIR)/ui.c $(SRC_DIR)/ui.h
	$(CC) -c $(SRC_DIR)/ui.c -o $(BUILD_DIR)/ui.o

index.o: $(SRC_DIR)/index.c $(SRC_DIR)/index.h
	$(CC) -c $(SRC_DIR)/index.c -o $(BUILD_DIR)/index.o

clean:
	rm $(BUILD_DIR)/*.o

install:
	cp -f complot ~/bin/apps
	chmod +x ~/bin/apps/complot
