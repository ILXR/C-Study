#include <bits/stdc++.h>

using namespace std;

int main(int argc, char const *argv[])
{
    char a[] = "Hello World";
    char b[] = "Hello World";

    char *c = "Hello World";
    char *d = "Hello World";

    const char e[] = "Hello World";
    const char *f = "Hello World";

    // 注意，这里的 "a:%x\tb:%x\tc:%x\td:%x\n" 也在 rodata 区域
    printf("a:%x\tb:%x\tc:%x\td:%x\te:%x\tf:%x\n", a, b, c, d, e, f);
    // a:ffffdb54      b:ffffdb60      c:555560b0      d:555560b0      e:ffffdb6c      f:555560b0
    // c == d == f
    // a != b != e
    // 可见， char* 类型指向同样的字面值常量时，采用 COW 机制，指向同一块内存
    // 而字符串数组存储在栈中，不管是不是 const 类型
    return 0;
}
