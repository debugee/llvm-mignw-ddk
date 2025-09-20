// ioctl.c
#include <windows.h>
#include <stdio.h>

int main() {
    HANDLE h = CreateFileW(L"\\\\.\\DemoDevice", GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (h == INVALID_HANDLE_VALUE) {
        printf("Open device failed: %lu\n", GetLastError());
        return 1;
    }

    char msg[] = "Hello from user!";
    char output[128] = {0};
    DWORD ret = 0;
    BOOL ok = DeviceIoControl(
        h,
        0x00222000, // IOCTL_DEMO_PRINT
        msg, sizeof(msg), // 输入缓冲区和长度
        output, sizeof(output),          // 输出缓冲区
        &ret, NULL
    );
    if (ok)
        printf("Send ok: %s\n", output);
    else
        printf("Send failed: %lu\n", GetLastError());

    CloseHandle(h);
    return 0;
}