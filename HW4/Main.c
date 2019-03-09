#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

extern int f1();

extern int f2();

int main() {
    int n1, n2, n3;

    n1 = f1();
    n2 = f2();

    void *library_handler;
    int (*f3)();

    library_handler = dlopen("./libSecondDynamic.so",
                             RTLD_LAZY);
    if (!library_handler) {
        fprintf(stderr, "dlopen() error: %s\n", dlerror());
        exit(1);
    };

    f3 = dlsym(library_handler, "f3");

    n3 = (*f3)();

    printf("f1() = %d\n", n1);
    printf("f2() = %d\n", n2);
    printf("f3() = %d\n", n3);

    dlclose(library_handler);
    return 0;
}
