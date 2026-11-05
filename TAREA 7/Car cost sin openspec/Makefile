CC = gcc
CFLAGS = -O2 -Wall -mwindows
LIBS = -lcomctl32 -lgdi32 -luser32 -lkernel32
TARGET = CalculadoraCostoViaje.exe
SRCS = main.c vehicle.c dlg_vehicle.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	del /Q $(TARGET) *.o *.res 2>nul || true

run: $(TARGET)
	start $(TARGET)
