#include <stddef.h>
#include <sys/types.h>
#define PS4_KERNEL_ELF_RANGE_ADDRESS (char *)0xffffffff80000000
enum{ PS4_KERNEL_ELF_RANGE_SIZE = 0x02000000 };
#define PS4_KERNEL_ELF_FIXED_ADDRESS (char *)0xffffffff80700000
//enum{ PS4_KERNELEL_ELF_SIZE = 0x00EAC180 };
enum{ PS4_KERNEL_ELF_PAGE_SIZE = 16 * 1024 }; // no dependencies

extern uint32_t sdkVersion;

void *ps4KernelFindClosestPageStart(uint64_t addr)
{

	int remainder = addr % 0x4000;
	if(remainder == 0)
	{
		return addr;
	}
	else
	{
		return addr + 0x4000 - remainder;
	}
}

void *ps4KernelSeekElfAddress()
{
	static char *addr = NULL;
	
	char *m;
	int i;
	unsigned char elfMagic[] = {0x7f, 'E', 'L', 'F', 0x02, 0x01, 0x01, 0x09, 0x00};
	const size_t magicSize = sizeof(elfMagic) / sizeof(*elfMagic);
	
	if(sdkVersion>0x01760001)
	{
		uint64_t rip;
		{
			unsigned long __edx;
			unsigned long __eax;
			__asm__ ("rdmsr" : "=d"(__edx), "=a"(__eax) : "c"(0xC0000082));
			rip = (((uint64_t)__edx) << 32) | (uint64_t)__eax;
		}
	
		char *startaddr = ps4KernelFindClosestPageStart(rip);
	
		for(char *m = startaddr;m > startaddr - PS4_KERNEL_ELF_RANGE_SIZE;m -= PS4_KERNEL_ELF_PAGE_SIZE)
		{
			for(i = 0; i < magicSize && m[i] == elfMagic[i]; ++i);
			if(i == magicSize)
			{
				addr = m;
				return m;
			}
		}
		return NULL;
	}
	else
	{
		m = PS4_KERNEL_ELF_FIXED_ADDRESS;
		for(i = 0; i < magicSize && m[i] == elfMagic[i]; ++i);
			if(i == magicSize)
				return m;
			for(m = PS4_KERNEL_ELF_RANGE_ADDRESS;
				m < PS4_KERNEL_ELF_RANGE_ADDRESS + PS4_KERNEL_ELF_RANGE_SIZE;
				m += PS4_KERNEL_ELF_PAGE_SIZE)
			{
				for(i = 0; i < magicSize && m[i] == elfMagic[i]; ++i);
				if(i == magicSize)
					return m;
			}

		return NULL;
	}
	
}
