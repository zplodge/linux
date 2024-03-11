#include <stdio.h>
#include <string.h>

struct ewma{
	unsigned long internal;
	unsigned long factor;
	unsigned long weight;
};

static inline int ilog2(unsigned long v)
{
	int l = 0;
	while ((1UL << l) < v)
		l++;
	return l;
}

static inline void ewma_init(struct ewma *avg, unsigned long factor, unsigned long weight)
{

	avg->weight = ilog2(weight);
	avg->factor = ilog2(factor);
	avg->internal = 0;
}

static inline struct ewma *ewma_add(struct ewma *avg, unsigned long val)
{
	avg->internal = avg->internal  ?
		(((avg->internal << avg->weight) - avg->internal) +
			(val << avg->factor)) >> avg->weight :
		(val << avg->factor);
	return avg;
}

static inline unsigned long ewma_read(const struct ewma *avg)
{
	return avg->internal >> avg->factor;
}

int main(int argc, char* argv[])
{
	struct ewma avg_data;
	int orig_data[] = {5,10, 18, 19, 12, 15, 14, 13, 11, 10, 9, 8, 10, 15, 18, 19, 20, 21, 22, 25, 30, 15, 13, 10, 5};
	int cnt = sizeof(orig_data)/sizeof(orig_data[0]);
	int i = 0;
	//unsigned long avg_signal = 0;
	
	memset(&avg_data, 0, sizeof(struct ewma));
	ewma_init(&avg_data, 1024, 8);
	for(i=0; i<cnt; i++) {
		ewma_add(&avg_data, orig_data[i]);
		printf("orig:%d, avg:%d \n", orig_data[i], ewma_read(&avg_data));
	}
	return 0;
}