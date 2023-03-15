#include <stdio.h>

int main() {
    
    long long id = 1234567890;
    int code = (int)(id / 'b' / 'c' / 'd' % 'a' << 1);
    printf("code = %x\n", code);

    return 0;
}
