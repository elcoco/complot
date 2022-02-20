CC = gcc

# lncursesw is used for UTF8 support
CFLAGS = -g -Wall -lncursesw -lm -lpthread

BUILD_DIR = build
BIN_DIR = .
SRC_DIR = src

# create build dir
$(shell mkdir -p $(BUILD_DIR))

# target: dependencies
# 	  action

default: line.o read_thread.o json_parser.o perr.o csv.o yaxis.o plot.o ui.o utils.o index.o main.o
	$(CC) $(BUILD_DIR)/line.o $(BUILD_DIR)/yaxis.o $(BUILD_DIR)/read_thread.o $(BUILD_DIR)/json_parser.o $(BUILD_DIR)/perr.o $(BUILD_DIR)/csv.o $(BUILD_DIR)/plot.o $(BUILD_DIR)/ui.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/index.o $(BUILD_DIR)/main.o $(CFLAGS) -o $(BIN_DIR)/complot

utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h
	$(CC) -c $(SRC_DIR)/utils.c $(CFLAGS) -o $(BUILD_DIR)/utils.o

line.o: $(SRC_DIR)/line.c $(SRC_DIR)/line.h
	$(CC) -c $(SRC_DIR)/line.c $(CFLAGS) -o $(BUILD_DIR)/line.o

csv.o: $(SRC_DIR)/csv.c $(SRC_DIR)/csv.h
	$(CC) -c $(SRC_DIR)/csv.c $(CFLAGS) -o $(BUILD_DIR)/csv.o

read_thread.o: $(SRC_DIR)/read_thread.c $(SRC_DIR)/read_thread.h
	$(CC) -c $(SRC_DIR)/read_thread.c $(CFLAGS) -o $(BUILD_DIR)/read_thread.o

plot.o: $(SRC_DIR)/plot.c $(SRC_DIR)/plot.h
	$(CC) -c $(SRC_DIR)/plot.c $(CFLAGS) -o $(BUILD_DIR)/plot.o

yaxis.o: $(SRC_DIR)/yaxis.c $(SRC_DIR)/yaxis.h
	$(CC) -c $(SRC_DIR)/yaxis.c $(CFLAGS) -o $(BUILD_DIR)/yaxis.o

main.o: $(SRC_DIR)/main.c 
	$(CC) -c $(SRC_DIR)/main.c $(CFLAGS) -o $(BUILD_DIR)/main.o

ui.o: $(SRC_DIR)/ui.c $(SRC_DIR)/ui.h
	$(CC) -c $(SRC_DIR)/ui.c $(CFLAGS) -o $(BUILD_DIR)/ui.o

index.o: $(SRC_DIR)/index.c $(SRC_DIR)/index.h
	$(CC) -c $(SRC_DIR)/index.c $(CFLAGS) -o $(BUILD_DIR)/index.o

perr.o: $(SRC_DIR)/json/perr.c $(SRC_DIR)/json/perr.h
	$(CC) -c $(SRC_DIR)/json/perr.c $(CFLAGS) -o $(BUILD_DIR)/perr.o

json_parser.o: $(SRC_DIR)/json/json_parser.c $(SRC_DIR)/json/json_parser.h $(SRC_DIR)/json/perr.h
	$(CC) -c $(SRC_DIR)/json/json_parser.c $(CFLAGS) -o $(BUILD_DIR)/json_parser.o

clean:
	rm $(BUILD_DIR)/*.o

install:
	cp -f complot ~/bin/apps
	chmod +x ~/bin/apps/complot
