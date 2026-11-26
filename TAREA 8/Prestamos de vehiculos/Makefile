CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude -Ithird_party/cJSON
GUI_LDFLAGS = -lcomctl32 -lgdi32 -mwindows -lm
TEST_LDFLAGS = -lm

CORE_SRCS = src/linked_list.c src/loan_service.c src/json_store.c src/report.c third_party/cJSON/cJSON.c
GUI_SRCS = src/main.c src/gui.c $(CORE_SRCS)
TEST_SRCS = tests/test_core.c $(CORE_SRCS)

all: prestamos.exe

prestamos.exe: $(GUI_SRCS)
	$(CC) $(CFLAGS) -o $@ $(GUI_SRCS) $(GUI_LDFLAGS)

test: test_core.exe
	./test_core.exe

test_core.exe: $(TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRCS) $(TEST_LDFLAGS)

clean:
	if exist prestamos.exe del /Q prestamos.exe
	if exist test_core.exe del /Q test_core.exe

.PHONY: all test clean
