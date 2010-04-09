#include "resource.h"

#include <hash_map>

// Use this one if you're using atomized strings. It's faster.
struct atomStr_hashTraits
{
	static const size_t bucket_size = 1024; // expected max # of buckets (not a limit, but will slow down)
	static const size_t min_buckets = 128; // should be plenty of room for a ld48 game :)

	atomStr_hashTraits() {}
	bool operator() ( const char *s1, const char *s2) const
	{
		return s1<s2;
	}

	size_t operator()( const char *skey) const
	{
		// string is atomized, so it's a good hash for itself
		return reinterpret_cast<size_t>(skey);
	}
};

typedef stdext::hash_map<const char *, void*, atomStr_hashTraits> Resource_hash;
static Resource_hash g_resHash;

void *ResourcePtr_get( const char *atomized_name, 
					ResourcePtrFunc *resfunc )
{
	void *val = g_resHash[ atomized_name ];
	if (!val)
	{
		// call the user-supplied function to load the 
		// resource
		val = (*resfunc)(atomized_name);	

		// store it, if successful
		if (val)
		{
			g_resHash[ atomized_name ] = val;
		}
	}
	return val;
}

unsigned int Resource_get( const char *atomized_name, 
						   ResourceFunc *resfunc )
{
	// TODO
	return 0;
}