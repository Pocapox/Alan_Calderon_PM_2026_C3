CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
TARGET = carcost.exe
GUI_TARGET = carcost-gui.exe
TEST_TARGET = tests/run_tests.exe

COMMON_SRC = src/vehicle.c src/storage.c src/calculator.c src/input.c src/parse.c src/utf8.c
APP_SRC = src/main.c $(COMMON_SRC)
TEST_SRC = tests/run_tests.c $(COMMON_SRC)
HEADERS = src/vehicle.h src/storage.h src/calculator.h src/input.h src/parse.h src/utf8.h

GUI_SRC = src/gui_main.c src/vehicle.c src/storage.c src/calculator.c src/parse.c src/utf8.c
GUI_RES = src/gui.res.o
GUI_LIBS = -mwindows -municode -lcomctl32 -luxtheme -lgdi32 -luser32

.PHONY: all test clean gui

all: $(TARGET)

$(TARGET): $(APP_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(APP_SRC)

$(GUI_RES): src/gui.rc src/gui.manifest
	windres -i src/gui.rc -o $(GUI_RES)

$(GUI_TARGET): $(GUI_SRC) $(GUI_RES) $(HEADERS) src/no-manifest.specs
	$(CC) $(CFLAGS) -specs src/no-manifest.specs -municode -mwindows -o $(GUI_TARGET) $(GUI_SRC) $(GUI_RES) $(GUI_LIBS)

gui: $(GUI_TARGET)

$(TEST_TARGET): $(TEST_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -Isrc -o $(TEST_TARGET) $(TEST_SRC)

test: $(TEST_TARGET)
	$(TEST_TARGET)

clean:
	-cmd /C "if exist $(TARGET) del /Q $(TARGET)"
	-cmd /C "if exist $(GUI_TARGET) del /Q $(GUI_TARGET)"
	-cmd /C "if exist src\gui.res.o del /Q src\gui.res.o"
	-cmd /C "if exist tests\run_tests.exe del /Q tests\run_tests.exe"
	-cmd /C "if exist test_vehicles.dat del /Q test_vehicles.dat"
	-cmd /C "if exist test_vehicles.dat.tmp del /Q test_vehicles.dat.tmp"
