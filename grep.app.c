#include "usyscall.h"

char *strstr(const char *str, const char *substr) {
    for(int i = 0; str[i] != '\0'; i++) {
        int sub_len = 0;
        while(substr[sub_len] != '\0' && *(str + i + sub_len) != '\0') {
            if(substr[sub_len] == *(str + i + sub_len)) sub_len++;
            else break;
        }
        if(substr[sub_len] == '\0') return str + i;
    }
    return 0;
}

void *memchr(const void *str, int c, long unsigned n) {
    char* casted_str = (char*) str;
    for(int i = 0; i < n; i++) {
        if(casted_str[i] == c) return (void*)(casted_str + i);
    }
    return 0;
}

void *memmove(void *dst, const void *src, long unsigned n) {
    char* casted_dst = (char*)dst;
    char* casted_src = (char*)src;
    for(int i = 0; i < n; i++) {
        if(dst >= src) casted_dst[n - i - 1] = casted_src[n - i - 1];
        else casted_dst[i] = casted_src[i];
    }
}

int main(int argc, char* argv[]) {
	char buf[5];
	int len;
	int o = 0;
	while (0 < (len = os_read(0, buf + o, sizeof(buf) - o - 1))) {
		len += o;
		char *p = buf;
		char *p2 = memchr(p, '\n', len);
		while (p2) {
			*p2 = '\0';
			if (strstr(p, argv[1])) {
				os_write(1, p, p2 - p);
				os_write(1, "\n", 1);
			}
			p = p2 + 1;
			p2 = memchr(p, '\n', len - (p - buf));
		}
		o = len - (p - buf);
		memmove(buf, p, o);
	}

	os_exit(0);
}
