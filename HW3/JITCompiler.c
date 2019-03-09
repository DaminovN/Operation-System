#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

const size_t SIZE = 1024;

void* allocate_executable_memory(size_t size) 
{
    void* ptr = mmap(0, size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    mprotect(ptr, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (ptr == (void*) -1) 
    {
        perror("mmap");
        return NULL;
    }
    return ptr;
}
void insert_code(unsigned char* mpointer) 
{
    unsigned char code[] = {
/*            0x55, 0x48, 0x89, 0xe5, 0x89, 0x7d, 0xfc, 0x8b, 0x7d, 0xfc, 0xc1, 0xe7, 0x01, 0x89, 0xf8, 0x5d, 0xc3  Multiply (MultiplyByTwo) */
/*            0x55, 0x48, 0x89, 0xe5, 0x48, 0x89, 0x7d, 0xf8, 0x48, 0x8b, 0x7d, 0xf8, 0x48, 0x83, 0xc7, 0x03, 0x48, 0x89, 0xf8, 0x5d, 0xc3  AddThree */
            0x55, 0x48, 0x89, 0xe5, 0x48, 0x89, 0x7d, 0xf8, 0x48, 0x8b, 0x7d, 0xf8, 0x48, 0x83, 0xc7, 0x04, 0x48, 0x89,
            0xf8, 0x5d, 0xc3 // Add (AddFour)
    };
    memcpy(mpointer, code, sizeof(code));
}
void modification(unsigned char* mpointer, int position, unsigned char* modification) 
{
    memcpy(mpointer + position, modification, sizeof(*modification));
}
int main() 
{
    void* mpointer = allocate_executable_memory(SIZE);
    insert_code(mpointer);
    int x;
    scanf("%d", &x);
    unsigned char y;
    y = (unsigned char) x;
    modification(mpointer, 15, &y);

    int (*fpointer)(int) = mpointer;
    int ans = fpointer(2);

    printf("%d\n", ans);
    munmap(mpointer, SIZE);
    return 0;
}
