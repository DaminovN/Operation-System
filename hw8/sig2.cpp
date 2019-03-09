#include <iostream>
#include <iomanip>
#include <ucontext.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <signal.h>

std::string reg_names[] = { // based on sys/ucontext.h source
#if NGREG == 23 // x64
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",
    "RDI",
    "RSI",
    "RBP",
    "RBX",
    "RDX",
    "RAX",
    "RCX",
    "RSP",
    "RIP",
    "EFL",
    "CSGSFS",
    "ERR",
    "TRAPNO",
    "OLDMASK",
    "CR2"
#else   // x32, NGREG == 19
    "GS",
    "FS",
    "ES",
    "DS",
    "EDI",
    "ESI",
    "EBP",
    "ESP",
    "EBX",
    "EDX",
    "ECX",
    "EAX",
    "TRAPNO",
    "ERR",
    "EIP",
    "CS",
    "EFL",
    "UESP",
    "SS"
#endif
};


jmp_buf st;

void catch_mem(int, siginfo_t*, void*)
{
    longjmp(st, 1);
}

int try_memory_access(unsigned char* addr)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = catch_mem;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        std::cerr << "sigaction failed\n";
        return -1;
    }

    if (!setjmp(st)) {
        return *addr;
    }
    else {
        return -1;
    }
}

void out(unsigned char* p, bool it = 0)
{
    static int width = 0;
    if (width == 0) {
        std::cerr << (size_t*)p << ": ";
    }

    if (it) {
        std::cerr << "->";
    }

    int c = try_memory_access(p);
    if (c == -1) {
        std::cerr << "xxxx ";
    }
    else {
        std::cerr << "0x" << std::hex << std::setw(2) << std::setfill('0') << c << " ";
    }

    ++width;
    if (width == 8) {
        width = 0;
        std::cerr << "\n";
    }
}

void dump(unsigned char* addr, int radius)
{
    for (int i = 0; i < radius; ++i) {
        out(addr - radius + i);
    }

    out(addr, 1);

    for (int i = 1; i < radius; ++i) {
        out(addr + i);
    }

    std::cerr << "\n";
}

void sig_info(int sig, siginfo_t* info, void *cont)
{
    ucontext_t *context = (ucontext_t*)cont;

    std::cerr << "Segmentation fault at address " << (size_t*)info->si_addr << "\n";

    dump((unsigned char*)info->si_addr, 32);

    std::cerr << "General purpose register values:\n";

    for (int i = 0; i < NGREG; ++i) {
        std::cerr << std::setw(9) << std::left << std::setfill(' ') << reg_names[i] + ": " << "0x" << std::hex << std::setfill('0') << context->uc_mcontext.gregs[i] << "\n";
    }

    exit(1);
}


int main()
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sa.sa_sigaction = sig_info;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        std::cerr << "sigaction failed\n";
        return 1;
    }

    char *mem = ()(mmap(NULL, 4096, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
   	mem[0] = 'x';

    return 0;
}