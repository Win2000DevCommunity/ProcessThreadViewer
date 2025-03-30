1. Purpose
    A Windows utility to:

    📌 List processes (name, PID, threads, affinity)

    📌 Inspect threads (TID, status, stack traces)

    ⚠️ Kill processes/threads

2. Compilation Command (MinGW/g++)

    g++ -m32 -o ProcessThreadViewer.exe \
        main.cpp process.cpp thread.cpp ui.cpp symbols.cpp \
        -ldbghelp -lcomctl32 -lkernel32 -lpsapi -luser32 -lgdi32 -ladvapi32 \
        -mwindows -static-libgcc -Wl,-subsystem,windows

    ![image](https://github.com/user-attachments/assets/32137f47-933c-4a7e-8765-a73f3c4525ce)
