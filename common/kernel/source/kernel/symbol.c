#define _KERNEL

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>

#include <sys/lock.h>
#include <sys/mutex.h>

#include <ps4/kernel.h>

static Ps4KernelCache *cache;
static struct mtx *ps4KernelSymbolLookUpMutex; // FIXME: Use RW lock or RM locks

#define PS4_KERNEL_CACHE_SYMBOL_LOOKUP "ps4.kernel.symbol.lookup"
#define PS4_KERNEL_CACHE_SYMBOL_LOOKUP_CACHE PS4_KERNEL_CACHE_SYMBOL_LOOKUP ".cache"
#define PS4_KERNEL_CACHE_SYMBOL_LOOKUP_MUTEX PS4_KERNEL_CACHE_SYMBOL_LOOKUP ".mutex"

extern sdkVersion;

int ps4KernelSymbolLookUp(const char *str, void **value)
{
	int r;

	if(str == NULL)
		return PS4_ERROR_ARGUMENT_PRIMARY_MISSING;
	if(value == NULL)
		return PS4_ERROR_ARGUMENT_OUT_MISSING;

	/*if(sdkVersion > 0x01760001)	
	{
		void *v = ps4KernelDlSym((char *)str);
		if(v == NULL)
			return PS4_ERROR_KERNEL_SYMBOL_LOOKUP_NOT_FOUND;
		*value=v;
	}
	else
	{*/
	
	if(cache == NULL)
	{
		struct mtx *giant = ps4KernelDlSym("Giant");
		

		mtx_lock(giant);
		r = ps4KernelCacheGlobalGet(PS4_KERNEL_CACHE_SYMBOL_LOOKUP_CACHE, (void **)&cache);
		if(r != PS4_OK)
		{
			r = ps4KernelCacheCreate(&cache);
			if(r != PS4_OK)
			{
				mtx_unlock(giant);
				return r;
			}
			ps4KernelCacheGlobalSet(PS4_KERNEL_CACHE_SYMBOL_LOOKUP_CACHE, cache);

			ps4KernelMemoryAllocateData((void **)&ps4KernelSymbolLookUpMutex, sizeof(struct mtx));
			if(ps4KernelSymbolLookUpMutex == NULL)
			{
				ps4KernelCacheDestroy(cache);
				mtx_unlock(giant);
				return PS4_ERROR_KERNEL_OUT_OF_MEMORY;
			}
			mtx_init(ps4KernelSymbolLookUpMutex, "ps4KernelSymbolLookUpMutex", NULL, MTX_DEF | MTX_RECURSE);
			ps4KernelCacheGlobalSet(PS4_KERNEL_CACHE_SYMBOL_LOOKUP_MUTEX, ps4KernelSymbolLookUpMutex);
		}
		else
			r = ps4KernelCacheGlobalGet(PS4_KERNEL_CACHE_SYMBOL_LOOKUP_MUTEX, (void **)&ps4KernelSymbolLookUpMutex);
		mtx_unlock(giant);
		
	}
    
	mtx_lock(ps4KernelSymbolLookUpMutex);
	r = ps4KernelCacheGet(cache, str, value);
	if(r != PS4_OK)
	{
		void *v = ps4KernelDlSym((char *)str);
		ps4KernelCacheSet(cache, str, v);
		*value = v;
		if(v == NULL)
			return PS4_ERROR_KERNEL_SYMBOL_LOOKUP_NOT_FOUND;
	}
	mtx_unlock(ps4KernelSymbolLookUpMutex);
	//}
	return PS4_OK;
	
}
