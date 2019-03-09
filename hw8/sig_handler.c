#define __USE_GNU
#include <ucontext.h> 
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <setjmp.h>

jmp_buf jmp;

char* register_strings[23] = {"REG_R8", "REG_R9", "REG_R10", "REG_R11", "REG_R12", "REG_R13", "REG_R14", "REG_R15", "REG_RDI", "REG_RSI", "REG_RBP", "REG_RBX", "REG_RDX", "REG_RAX", "REG_RCX", "REG_RSP", 
								"REG_RIP", "REG_EFL", "RER_CSGSFS", "REG_ERR", "REG_TRAPNO", "REG_OLDMASK", "REG_CR2"}; 

void handler_memory(int signo, siginfo_t* info, void* vcontext) 
{
    if (info->si_signo == SIGSEGV) 
        longjmp(jmp, 1);
    return;
}

int dump(unsigned char* address)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_flags = SA_SIGINFO | SA_NODEFER;
    action.sa_sigaction = handler_memory;
    if (sigaction(SIGSEGV, &action, NULL) < 0) 
    {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
    if (setjmp(jmp) == 0)
        return *address;
    else 
        return -1;
}
void print(unsigned char* address, int fl) 
{
	static int w = 0;
	if (w == 0)
		fprintf(stderr, "%zu: ", (size_t*) address);
	if (fl) 
		fprintf(stderr, "->");
	int rs = dump(address);
	if (rs < 0)
		fprintf(stderr, "xxxx ");
	else 
		fprintf(stderr, "0x%x ", rs);
	w++;
	if (w == 8) 
	{
		fprintf(stderr, "\n");
		w = 0;
	}
}
void handler(int signo, siginfo_t* info, void* vcontext) 
{
	if (info->si_signo != SIGSEGV) 
            return;
	printf("Got SIGSEGV at address %p \n", info->si_addr);
    ucontext_t* context = (ucontext_t*)vcontext;
    printf("Registr dump\n");
    for (int i = 0; i < 32; ++i) 
	{
		print(info->si_addr - 32 + i, 0);			
	}
	print(info->si_addr, 1);
	for (int i = 1; i < 32; ++i) 
	{
		print(info->si_addr + i, 0);			
	}
	for (int i = 0; i < NGREG; i++)
            fprintf(stderr, "%s with the value - 0x%x\n", register_strings[i], 
							(unsigned int) context->uc_mcontext.gregs[i]);
    exit(EXIT_FAILURE);
}

int main() 
{
  	struct sigaction action;
  	memset(&action, 0, sizeof(struct sigaction));
  	action.sa_flags = SA_SIGINFO | SA_NODEFER;
  	action.sa_sigaction = handler;
  	if (sigaction(SIGSEGV, &action, NULL) < 0) 
        {		
  		perror("sigaction SIGSEGV");
  		exit(EXIT_FAILURE);
  	}
  	
	int *mem = (int*)mmap(NULL, sizeof(int) * 30, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    for (int i = 0; i < 30; ++i) 
	{
        mem[i] = i;
    }

    mprotect(mem, sizeof(int) * 30, PROT_READ);

    mem[5] = 12;
  
  	return 0;
}
