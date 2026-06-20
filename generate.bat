@echo off
@echo "------------------------- clean -------------------------"
rmdir /s /q build
@echo "------------------------- generate -------------------------"
cmake -S . -B build/
echo "------------------------- build -------------------------"
cmake --build build --config Debug
echo "------------------------- run -------------------------"
.\build\bin\Debug\null_engine.exe