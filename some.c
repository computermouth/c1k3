

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

int main(){
	
	while (1){
		
		struct timespec t;
		clock_gettime(CLOCK_BOOTTIME, &t);
		int32_t ms = t.tv_sec * 1000 + t.tv_nsec / 1000000;
		printf("ms: %d -- %f\n", ms, sinf(ms * .00033));
		
	}
	
	return 0;
}