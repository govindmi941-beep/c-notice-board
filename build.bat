@echo off
gcc server.c notice.c http.c -o noticeboard.exe -lws2_32
if %errorlevel% neq 0 (
  echo Build failed. Is gcc installed? See README.md
  exit /b 1
)
echo Build OK. Run noticeboard.exe, then open http://localhost:8080
