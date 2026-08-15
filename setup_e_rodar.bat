@echo off
title AimLab TriggerBot - Setup
echo ============================================
echo   AimLab TriggerBot v3  -  Setup
echo ============================================
echo.

python --version >nul 2>&1
if errorlevel 1 (
    echo  ERRO: Python nao encontrado.
    echo  Instale em https://python.org
    echo  Marque "Add Python to PATH" na instalacao!
    pause & exit /b 1
)

echo  Instalando dependencias...
python -m pip install --upgrade pip -q
python -m pip install -r requirements.txt -q

echo.
echo  ============================================
echo   Tudo pronto! Iniciando o bot...
echo  ============================================
echo.
python trigger.py
pause
