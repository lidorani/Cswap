#pragma once
#include "Cswap.h"
#include <stdlib.h>
#include <time.h>
#include "randomizedMem.h"

extern unsigned int current_R;
extern unsigned int* historyTable;
extern unsigned char* memory;
extern unsigned int sizeOfHistoryTable;
extern unsigned int previous_R_table_size;
extern unsigned int* previous_R_table;
extern unsigned int head_index_prev_R_table;

int global_Mem_accesses_counter = 0;
int global_Mem_HitCounter=0;
int global_Mem_MissCounter = 0;
int setAccessCountArray[256];
unsigned char setAccessedByOrderArray[4000000];
unsigned int numberOfSetsAccessed = 0;
unsigned int global_lineSwapsCounter = 0;
// input - counter to update : 0 =global_Mem_HitCounter ,global_Mem_MissCounter,
void updateCountersHit_Miss(int counterToUpdate) {
	switch (counterToUpdate) {
	case 0:
		global_Mem_HitCounter++;
		break;
	case 1:
		global_Mem_MissCounter++;
		break;
	}
	
}


void resetAllcountersToZero() {
	global_Mem_accesses_counter = 0;
	global_Mem_HitCounter = 0;
	global_Mem_MissCounter = 0;
	numberOfSetsAccessed = 0;
	global_lineSwapsCounter = 0;
	int i;
	for (i = 0;i < 256; i++) {
		setAccessCountArray[i] = 0;
	}
	for (i = 0; i < 4000000; i++) {
		setAccessedByOrderArray[i] = 0;
	}

}

//genrates a random integer in range 0 to 0xFFFFF (unsigned)
unsigned int generateRandInt(unsigned int lowerBound, unsigned int upperBound) {
	//rand only returns up to RAND_MAX which is up to 0x7FFF
	unsigned int intermitiate_random_number=rand() % (upperBound + 1 - lowerBound) + lowerBound;
	if (upperBound >= RAND_MAX) {
		//draw another number in the range 0-F7 inclusive;
		unsigned int mask = rand() % (0xFF + 1); // number between 0 and 0xFF (inclusive)
		//then shift and add the number to the intermidiate result;
		mask = mask << 12; //shift left 12 times
		intermitiate_random_number = intermitiate_random_number & 0x00000FFF;
		//12 [0:11] lsb bits are from the intermitiate_random_number - and the 8 mbs bits [12:19] are from the mask
		return mask | intermitiate_random_number;
	}
	else {
		return rand() % (upperBound + 1 - lowerBound) + lowerBound;
	}
	
}

//set value is between 0 and 255 inclusive.
//TimeLineArray records accesses in the order they happened
void updateSetAccessCount(unsigned int set) {
	//static int indexOfTimeLIneArray = 0;
	setAccessCountArray[set]++;
	//update timeLine:
	setAccessedByOrderArray[numberOfSetsAccessed] =(unsigned char) set;
	numberOfSetsAccessed++;

}


//Given an "original address" (point of view of programmer)  search for the value in permuted memory and return the matching data;
unsigned char readByteFromMem_withCounters(unsigned int originalAddress) {
	//(1)+(2)calc new address (changes set ,possibly tag?)
	int original_set = getSet(originalAddress);
	unsigned int new_address = permuteAddress(current_R, originalAddress);
	//(3)
	int line_index_curr_mapping = calculateLineIndexInMem(new_address);
	//(4) check if historyTable[line_index] == original set - if so - we found the data - no swap is needed!
	if (historyTable[line_index_curr_mapping] == original_set) {
		//we found the information according to the current mapping! return value:(6)
		//NEW ADDTION:benchmark code:
		updateCountersHit_Miss(0);//hitcounter++;
		updateSetAccessCount(line_index_curr_mapping);
		//old code
		unsigned int diffFromStartOfMem = (new_address)-(unsigned int)memory;
		unsigned char* pointer = &memory[diffFromStartOfMem];
		return (*pointer);
	}
	//If there is no match (4): search the history table for matching original set value:

	unsigned char fastSearchDisable = 0;
	int indexOfLineWhereDataIs = findLineWithSet(originalAddress, fastSearchDisable);
	//benchmark code:
	updateCountersHit_Miss(1);//missCounter++;
	updateSetAccessCount(indexOfLineWhereDataIs); //accessing line where data is
	updateSetAccessCount(line_index_curr_mapping);//also accessing the current mapping line to swap the lines and update mapping
	global_lineSwapsCounter++; //updating counter 
	//(5)We need to “fix” the mapping – we swap the line:
	swapMemoryLines_And_UpdateHistoryTable(indexOfLineWhereDataIs, line_index_curr_mapping);
	//now that the data has been brought to the right place : we can read from RAM:
	unsigned int diffFromStartOfMem = (new_address)-(unsigned int)memory;
	unsigned char* pointer = &memory[diffFromStartOfMem];
	return (*pointer);
}

//assumes the method: setUpEnvironment() and bring_Memory_And_HistoryTableIntoAKnownState() were ran. 
//receives the number of times R should be replaced - also receieves pointers to integers which it will update with the counters it calculated
void runMemoryBenchmark(int numberOfTimesToReplaceR,int * hit_counter,int *miss_counter) {
	//setting up the memory array and the history table
	bring_Memory_And_HistoryTableIntoAKnownState();
	unsigned char* ptr = memory;
	//printMemoryContent();
	//printHistoryTable();
	srand(time(0));
	current_R = generateRandInt(0, (unsigned int)0xFFFFF);
	//printf("current r is =%05X\n", current_R);
	//now we will run a benchmark:
	int i;
	int numberOfAccesses = (12000%4096)+12000; //15,
	//replacing value of of current_r
	//int r_values [10] = {0x8b36f ,0xe84ba ,0x6db02 ,0x83108 ,0x918cc ,0x11139 ,0x8191c ,0x2804a ,0x7a42 ,0x2faa9};
	int r_index=0;
	int r_replacement_interval = 0;
	if (numberOfTimesToReplaceR != 0) {
		 r_replacement_interval = numberOfAccesses / numberOfTimesToReplaceR;
	}
	printf("r_replacement_interval is %d \n", r_replacement_interval);
	for (i = 0; i < numberOfAccesses; i++) {
		//swapping value of r every "r_replacement_interval"
		if (i != 0 && r_replacement_interval!=0 &&i % r_replacement_interval == 0) {
			current_R = generateRandInt(0, (unsigned int)0xFFFFF);
			//printf("current r is =0x%05X . swapped when i %C 4096 =%d\n", current_R, '%', i % 4096);
			//current_R = r_values[r_index%10];//changing the value of current r (for permutation)
			r_index++;
		}
		unsigned char byte = readByteFromMem_withCounters((unsigned int) & ptr[i%4096]);
	}
	int printBenchmarkResult = 0;
	//Results:
	if (printBenchmarkResult) {
		printf("***************************************************************\n");
		printf("Benchmark results : \n");
		printf("Number of memory reads :%d\n", numberOfAccesses);
		printf("Hit count:%d\n", global_Mem_HitCounter);
		printf("Miss count:%d\n",global_Mem_MissCounter);
		printf("Line swap count:%d\n", global_lineSwapsCounter);
		printf("Number of set accesses:%d (larger than num of memory reads because of the line swaps)\n", numberOfSetsAccessed);
		printf("setAccessCountArray : \n");
		for (i = 0; i < 256;i++) {
			//printf("%d:%d\n", i, setAccessCountArray[i]);
			printf("%d\n", setAccessCountArray[i]);
		}
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		printf("--------------------------------------------------------------- \n");
		printf("setAccessedByOrderArray : \n");
		int howManyNumbersInARow = 16;
		for (i = 0; i < numberOfSetsAccessed; i++) {
			//print a line seperator:
			if (i != 0 && i % howManyNumbersInARow == 0) {
				printf("\n");
			}
			//print the sets in the order they were accessed:
			printf("%03d,",setAccessedByOrderArray[i]);
			
		}
		printf("\n--------------------------------------------------------------- \n");
		//printf("History table after benchmark :\n");
		//printHistoryTable();
		//updating variables
		
	}
	*hit_counter = global_Mem_HitCounter;
	*miss_counter = global_Mem_MissCounter;
}


//will run the runMemoryBenchmark multiple times - it will keep track of the hit miss rate vs numOfTimesToReplaceR
//it will print the hit/ miss ratio for each "numOfTimesToReplaceR" value in the range given as input.(inclusive)

void runMemoryBenchmark_MultipleTimes_Measure_Hit_Miss_Rate(int min_numOfTimesToReplaceR, int max_numOfTimesToReplaceR,int intervals) {
	int i;
	//Allocating table with 3 columns (numOfTimesToReplaceR,hit rate ,miss rate)
	int numOfLinesToAlloc = (max_numOfTimesToReplaceR - min_numOfTimesToReplaceR + 1);
	double** hitMissTable = (double**)calloc(numOfLinesToAlloc, sizeof(double*));
	for (i = 0; i < numOfLinesToAlloc; i++) {
		hitMissTable[i] = (double*)calloc(3, sizeof(double));
	}
	int indexInTable = 0, hitRateColumnIndex = 1, missRateColumnIndex=2; //running index for rows in the table
	int currHitCounter = 0, currMissCounter = 0;
	//running the benckmark muktiple times and saving hit/miss results into the array
	for (i = min_numOfTimesToReplaceR; i <= max_numOfTimesToReplaceR; i+=intervals) {
		//reset all global counters to zero : (because we are repeating the test over and over)
		resetAllcountersToZero();
		runMemoryBenchmark(i, &currHitCounter, &currMissCounter);
		//now currHitCounter,currMissCounter have been updated!
		int numberOfAccesses = currHitCounter + currMissCounter;
		double hitRate = (double)currHitCounter /(double) numberOfAccesses;
		double missRate = (double)currMissCounter / (double)numberOfAccesses;
		hitMissTable[indexInTable][0] = i;//i= curr_numOfTimesToReplaceR;
		hitMissTable[indexInTable][hitRateColumnIndex] = hitRate;
		hitMissTable[indexInTable][missRateColumnIndex] = missRate;
		//increment index in table (move to the next row):
		indexInTable++;
	}
	printf("currHitCounter %d , currMissCounter =%d\n", currHitCounter, currMissCounter);
	//now we print the table:
	printf("----------------------------------------------------------\n\n");
	printf("-hitMissTable-\n");
	printf("numOfTimesToReplaceR   \tHitRate   \tMissRate\n");
	for (i = 0; i < numOfLinesToAlloc; i++) {
		printf("%3.0lf \t\t\t %.4lf \t%.4lf\n", hitMissTable[i][0], hitMissTable[i][1], hitMissTable[i][2]);
		if (hitMissTable[i][0] == 0 && hitMissTable[i][1] == 0 && hitMissTable[i][2] == 0) {
			break; //reached an all zero/uninitialized line -we can stop printing 
		}
	}

	//free 2d allocated array.:
	for (i = 0; i < numOfLinesToAlloc; i++) {
		free(hitMissTable[i]);
	}
	free(hitMissTable);

}

void AES_benchmark() {

}

 









