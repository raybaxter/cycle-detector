#if defined(DEBUG)
void print_corner();
#define debug_print(...) fprintf(stderr,__VA_ARGS__)

void print_corner() 
{
	int i,j;
	int size = 6;
	int i_start =  0;
	int i_end = i_start + size;
	int j_start =  0;
	int j_end = j_start + size;
	for(i=i_start;i<i_end;i++)
		{
			for(j=j_start;j<j_end;j++) 
				debug_print("%1x ", is_ancestor(i,j));
			debug_print("\n");
		}
	debug_print("\n");
}

#else
#define debug_print(...)
#endif
