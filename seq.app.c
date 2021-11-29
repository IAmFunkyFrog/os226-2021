#include "usyscall.h"

int atoi(const char *str) {
    int num = 0;

    for(char* str_it = str; *str_it != '\0'; str_it++) {
        if(*str_it >= '0' && *str_it <= '9') num = num * 10 + (*str_it - '0');
        else break;
    }

    return num;
}

int itoa(int v, char *d) {
    int sign = v >= 0 ? 1 : -1;

    int len = 0;
    while(v != 0) {
        d[len++] = (v % 10) + '0';
        v /= 10;
    }
    if(sign < 0) d[len++] = '-';
    d[len] = '\0';

    for(int i = 0; i < len / 2; i++) {
        char tmp = d[i];
        d[i] = d[len - i - 1];
        d[len - i - 1] = tmp;
    }

    return len;
}

int main(int argc, char* argv[]) {
	int n = atoi(argv[1]);
	char buf[32];
	for (int i = 1; i <= n; ++i) {
		int l = itoa(i, buf);
		buf[l] = '\n';
		os_write(1, buf, l + 1);
	}
	os_exit(0);
}
