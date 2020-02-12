@echo off
rmdir /s /q Debug >nul 2>&1
rmdir /s /q obj >nul 2>&1
rmdir /s /q ipch >nul 2>&1
rmdir /s /q .vs >nul 2>&1

del *.user >nul 2>&1
del *.db >nul 2>&1
