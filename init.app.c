#include "usyscall.h"

long unsigned strlen(const char *str) {
	const char *e = str;
	while (*e) {
		++e;
	}
	return e - str;
}

int os_print(int fd, const char *str) {
	int len = strlen(str);
	return os_write(fd, str, len);
}

int main(int argc, char* argv[]) {
    int wait = 100000000;
    int pid1 = os_fork();
    if(pid1 == 0) {
        int pid2 = os_fork();
        if(pid2 == 0) {
            while(1) {
                os_write(1, "pid_0\n", 6);
                for(int i = 0; i < wait; i++);
            }
        }
        else {
            while(1) {
                os_write(1, "pid_1\n", 6);
                for(int i = 0; i < wait; i++);
            }
        }
    }
    else {
        int pid2 = os_fork();
        if(pid2 == 0) {
            while(1) {
                os_write(1, "pid_2\n", 6);
                for(int i = 0; i < wait; i++);
            }
        }
        else {
            while(1) {
                os_write(1, "pid_3\n", 6);
                for(int i = 0; i < wait; i++);
            }
        }
    }
    os_exit(0);
}
