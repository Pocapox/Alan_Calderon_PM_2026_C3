@echo off
echo ========================================================
echo   Compilando Calculadora de Costo de Viaje por KM (C)
echo ========================================================

gcc -O2 -Wall -mwindows main.c vehicle.c dlg_vehicle.c -o CalculadoraCostoViaje.exe -lcomctl32 -lgdi32 -luser32 -lkernel32

if %errorlevel% neq 0 (
    echo [ERROR] Ocurrio un error durante la compilacion.
    pause
    exit /b %errorlevel%
)

echo [OK] Compilacion completada con exito!
echo Archivo generado: CalculadoraCostoViaje.exe
echo.
echo Presione cualquier tecla para iniciar el programa...
pause >nul
start CalculadoraCostoViaje.exe
