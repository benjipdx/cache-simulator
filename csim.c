#include "cachelab.h"

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

/* Benjamin Reichert
CS201 3/4/14 Karavanic

References: https://github.com/fangjian601/csapp/blob/master/cachelab/csim.c

*/

#define HIT 1
#define MISS 2
#define MISS_N_HIT 3
#define MISS_EVICTION_HIT 4
#define MISS_EVICTION 5

typedef struct
{
	int valid;
	int lru;
	unsigned int tag;
}Line; 

typedef struct
{
	Line * lines;
}Set;

typedef struct
{
	int setNum; //number of sets, S
	int lineNum; //number of lines
	Set * sets; //pointer to sets

}Cache;


int hits = 0;
int misses = 0;
int evictions = 0;
int hflag = 0; // display help
int vflag = 0; // Optional verbose ﬂag that displays trace info
int b = 0;
int e = 0;
int s = 0; 
//filename
char * t = NULL;
int setMask = 0;
int tagMask = 0;

int get_tag_bits(int address, int s, int b){
	int bitmask = 0;
	bitmask = 0x7FFFFFFF >> (31 - s - b);
	address = address >> (s + b);
	return (bitmask & address); //return the tag
}

int get_set_bits(int address, int s, int b){
	int bitmask = 0;
	bitmask = 0x7FFFFFFF >> (31 - s);
	address = address >> b;
	return (bitmask & address); //return set bits
}

int update_lru(Cache * cache, int setbits, int cacheline){
	for (int i = 0; i < cache -> lineNum; ++i){ //the least recently used cache element is represented by a 1, while the most recently used has 0's
		if(1 == cache -> sets[setbits].lines[i].valid && cache -> sets[setbits].lines[i].lru > cache -> sets[setbits].lines[cacheline].lru){
			--cache -> sets[setbits].lines[i].lru; //decrement all the other lru's back to 0
		}
	}
	
	cache -> sets[setbits].lines[cacheline].lru = cache -> lineNum; //the cache line we're looking at gets the lru updated to hold the value of the lineNum at that point. 
	return 0;

}

void print(Cache * cache){

	printf("Printing a cache for you.\n");

	for(int i = 0; i < cache-> setNum; i++){
		printf("Set # %d\n", i);
		for(int j = 0; j < cache -> lineNum; ++j){
			printf("Line # %d\n", j);
			printf("Valid: %d  LRU: %d  TAG: %d\n", cache -> sets[i].lines[j].valid, cache -> sets[i].lines[j].lru, cache -> sets[i].lines[j].tag);
		}

	}
}

int linescan(Cache * cache, int s, int E, int b, int addr, char id, int size){
	
	int setbits = get_set_bits(addr, s, b);
	int tagbits = get_tag_bits(addr, s, b);
	for(int i = 0; i < cache -> lineNum; ++i){
		if(cache -> sets[setbits].lines[i].valid == 1 && cache -> sets[setbits].lines[i].tag == tagbits){ //if valid and tag matches
		//we hit that imediately
			if( id == 'M' ) {
				++hits;
				++hits;
			}
			else{
				++hits;
			}
			update_lru(cache, setbits, i);
			return HIT;
		}
	}

	//didnt hit so we count a miss
	++misses;

	for(int i = 0; i < cache -> lineNum; ++i){
		if(cache -> sets[setbits].lines[i].valid == 0){ //not a valid bit. so we miss
			//empty line, so we dont evict, lucky day
			cache -> sets[setbits].lines[i].tag = tagbits;
			cache -> sets[setbits].lines[i].valid = 1; //now since we missed, and put some crap there it is valid
			update_lru(cache, setbits, i);
			if(id == 'M'){
				++hits;
				return MISS_N_HIT;
				
			}
			else{
				return MISS;
			}
		}
	}

	//need to kick you out, evicting you. sorry.
	
	++evictions;
	for(int i = 0; i < cache -> lineNum; ++i){
		if(cache -> sets[setbits].lines[i].lru == 1){
			//found a victim
			cache -> sets[setbits].lines[i].valid = 1;
			cache -> sets[setbits].lines[i].tag = tagbits;
			update_lru(cache, setbits, i);
			if(id == 'M'){
				++hits;
				return MISS_EVICTION_HIT;
			}
			else{
				return MISS_EVICTION;
			}
		}
	}

	return 0;

}


int main(int argc, char ** argv)
{
	//Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>

int c; //for holding case

	//getopt
	opterr = 0;
	
	while((c = getopt(argc, argv, "hvs:E:b:t:")) != -1){
		switch(c){
			case 'h':
				hflag = 1;
				break;
			case 'v':
				vflag = 1;
				break;
			case 'b': 
				b = atoi(optarg);
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				e = atoi(optarg);
				break;
			case 't':
				t = optarg;
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				return 1;
			default:
				abort(); //didnt get anything, abort the prog
		}
	}
	//debug
	printf("hflag: %d vflag: %d svalue: %d evalue: %d tvalue: %s bvalue: %d\n", hflag, vflag, s, e, t, b); 

	//Make me a cache, I know you want to

	Cache * cache = NULL;

	//INITILIZE THE CACHE///////////////////////////////////////

	cache = (Cache *)malloc(sizeof(Cache));
	cache -> setNum = (2 << s);
	cache -> lineNum = e;
	cache -> sets = (Set *)malloc(cache -> setNum * sizeof(Set)); //malloc the set size


	for(int i = 0; i < cache -> setNum; ++i){
		cache -> sets[i].lines = malloc(cache -> lineNum * sizeof(Line)); //allocate each cache line
		for(int j = 0; j < cache -> lineNum; ++j){ //initiliaze data to 0 in each line
			cache -> sets[i].lines[j].valid = 0;
			cache -> sets[i].lines[j].lru = 0;
			cache -> sets[i].lines[j].tag = 0;
		}
	}

	//debug
	//print(cache);


	///////////////////file io//////////////////////////////
	FILE * tracefile; //pointer to file obj

	tracefile = fopen (t,"r"); //open file for reading 
	char id;
	unsigned addr;
	int size;

	//so we can read lines
	while (fscanf(tracefile, " %c %x,%d", &id, &addr, &size) > 0){
	//this is where the "simulation" happens
		if(id != 0x49){ //if the id is a I, ignore that line, loop again and read another line
			//do the things!

			int cachelinestatus = linescan(cache, s, e, b, addr, id, size);
			
			switch(cachelinestatus){
				case HIT:
					printf("%c, %x,%d hit\n", id, addr, size);
					break;
				case MISS_N_HIT:
					printf("%c, %x,%d miss and hit\n", id, addr, size);
					break;
				case MISS_EVICTION_HIT:
					printf("%c, %x,%d miss eviction hit", id, addr, size);
					break;
				case MISS_EVICTION:
					printf("%c, %x,%d miss eviction\n", id, addr, size);
					break;
				case MISS:
					printf("%c, %x,%d miss\n", id, addr, size);
					break;
			}
		}
	
	}

	//close dem filez
	fclose(tracefile);
	
//	print(cache);

	free(cache); //free dem memories
	printSummary(hits, misses, evictions); //print the summary
	
    return 0;
}
