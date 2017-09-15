//20150560 CSE Kim Dongwon
#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

void help();

typedef struct{
	unsigned tag;
	int valid;
	int LRU;
}line;

int main(int argc, char** argv)
{
	int s, E, b;
	int S = 1;
	int B = 1;
	int i;
	int opt;

	int stage = 0;

	char id;
	unsigned address;
	int size;

	unsigned tag;
	int  set;

	int isHit = 0;
	int full = 0;
	int temp;

	int hits = 0; 
	int misses = 0;
	int evictions = 0;

	FILE* trace_file;
	line* cache;
	line* current_cache;

	while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
		switch(opt){
			case 'h':
				help();
				break;
			case 'v':
				break;
			case 's':
				s = atoi(optarg);
				for(i = 0; i < s ; i++){
					S = 2*S;
				}
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				for(i = 0; i < b ; i++){
					B = 2*B;
				}
				break;
			case 't':
				trace_file = fopen(optarg, "r");
				printf("%s\n",optarg);
				if(trace_file == NULL)
					printf("Error : file open\n");
				break;
		}
	}

	cache = malloc(sizeof(line)*E*S);
	for(i = 0; i < E*S; i++){
		cache[i].valid = 0;
	}

	while(fscanf(trace_file, " %c %x,%d", &id, &address, &size) > 0){
		printf("ssibal\n");
		if(id == 'I')
			continue;
		stage++;
		temp = 1;
		isHit = 0;
		full = 1;
		for(i = 0; i < s+b; i++){
			temp = 2*temp;
		}
		tag = address/temp;
		printf("tag: %d\n", tag);
		set = address%temp;
		temp = 1;
		for(i = 0; i < b; i++){
			temp = 2*temp;
		}
		set = set/temp;
		current_cache = &cache[E * set];

		temp = 0;
		for(i = 0; i < E; i++){
			if((current_cache + i)->valid == 1){
				if((current_cache + i)->valid == 1 && (current_cache + i)->tag == tag){
					isHit = 1;
					temp = i;
					hits++;
					break;
				}
			}
		}

		if(isHit == 0){
			misses++;
			for(i = 0; i < E; i++){
				if(i == 0)
					temp = i;
				if((current_cache + i)->valid == 0){
					temp = i;
					full = 0;
					break;
				}
				else if((current_cache + i)->LRU < (current_cache + temp)->LRU)
					temp = i;
			}
			if(full == 1){
				evictions++;
			}
		}
		if(id == 'M')
			hits++;

		(current_cache + temp)->tag = tag;
		(current_cache + temp)->LRU = stage;
		(current_cache + temp)->valid = 1;
	}
	fclose(trace_file);
	free(cache);
    printSummary(hits, misses, evictions);
    return 0;
}
void help(){
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n" );
	return;
}
