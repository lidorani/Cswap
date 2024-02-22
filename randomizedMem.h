#pragma once
#include "tests.h"
#include "Cswap.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

//#define debugPrints 1
#ifdef debugPrints
#endif
/*
What do we want to do ? 
1. define the size of the randomized access memory area : (1kb? ,4kb?)
2. 


*/

#define Memory_Size_In_Bytes  4096  //4kb- size of randomized area/region.
//#define  Register_File_Size 128 //TBD - in bytes-keeps track of mappings - not used currently
////////////////////////////////////////////////////////////////////////////////
#define Size_Of_tag_field_in_address 20
#define Size_Of_offset_field_in_address 4
#define Size_Of_Set_Field 8 ;//in bits -will be 4 or 8 - only the size that we are premuting!
//To declare a global variable in a header file, you can use the extern keyword.
extern unsigned int current_R ;
extern unsigned int* historyTable;
extern unsigned char* memory;
extern unsigned int sizeOfHistoryTable;
extern unsigned int previous_R_table_size;
extern unsigned int* previous_R_table;
extern unsigned int head_index_prev_R_table;



unsigned char bool_has_R_changed=0;

int calculateLineIndexInMem(int new_address);
int getSet(int originalAddress);
//Prints the content of the memory arr in format:
// line ### :address 0xXXXXXXXX : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
void printMemoryContent() {
	int i = 0, j = 0;
	int startAddrForMemory = (int)memory;
	int memSize = Memory_Size_In_Bytes;
	printf("Memory arr:\n");
	for (i = 0; i < memSize; i++) {
		if (i % 16 == 0) {
			int lineIndex = calculateLineIndexInMem((int)&memory[i]);
			printf("\nline %03d: 0x%08X :", lineIndex,(int)&memory[i]);
		}
		unsigned char data = memory[i]; //read singe byte
		printf(" 0x%02X", data); // print a single byte value

	}
	printf("\n");
}



//////////////////////////OPTION B////////////////////////////////////////////
/////////////////////////////////////R table methods ////////////////////////
//Prints the "previous_R_table":
void printPreviousRTable() {
	int i = 0;
	printf("--previous R table is:\n  [current R is  0x%X (%u)] \n ",current_R,current_R);
	for (i = 0; i < previous_R_table_size; i++) {
		printf("\t%d : 0x%X (%u) \n", i, previous_R_table[i], previous_R_table[i]);
	}
	printf("--\n");
}

//Prints the history table(for "original sets" for each memory line)
void printHistoryTable() {
	int i = 0;
	printf("--History table is:\n\tline             original set\n");
	for (i = 0; i < sizeOfHistoryTable; i++) {
		printf("\tline index %03d : 0x%02X (%u) \n", i, historyTable[i], historyTable[i]);
	}
}





void initializePRNG() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	srand((int)&ts.tv_nsec);
}



//This method changes the value of "current_R" to the newR and puts the old R value in the previous_R_table:
//TODO - need to improve - change to a stack.- perhaps 
void changeRValue(unsigned int newR) {
	//choose a random index to store the 
	int sizeOf_previous_R_table = previous_R_table_size;
	int randomIndex =rand()% sizeOf_previous_R_table; //gives as an integer between 0 and (previous_R_table_size-1)
	previous_R_table[randomIndex] = current_R;
	current_R = newR;
}

//Receives an integer and two indices . returns the num after clearing bits in range num[upperBit_index:lowerBit_index] (inclusive)
//to zero:
unsigned int clearBits(unsigned int num, int upperBit_index, int lowerBit_index) {
	unsigned int mask = (1 << (upperBit_index - lowerBit_index + 1)) - 1;
	mask = ~(mask << lowerBit_index);
	return num & mask;
}


//int Cswap()
//TODO : calculates the new address according to R value and original address
// the result needs to be inside memory array bounds! test this!!!!
//Assumption: "originalAddress" was checked to be inside array memory bounds before it was sent to this method
unsigned int permuteAddress(int r, unsigned int originalAddress) {
	//printf("\n\n*******in permuteAdress**************\n");
	//printf("r=%X, original address=%X\n", r, originalAddress);

	// Cswap() on set value? make sure to keep within array bounds . use cyclic calculation (modulo and add base addr??)
	unsigned char original_set = getSet(originalAddress);
	//unsigned char new_set = 0xEE;
	unsigned char new_set = Cswap_20bit_r_8bit_set(r, original_set);
	if (r % 0x800 == 0) {
		//printing the new_set
		//printf("0x%02X,",new_set);
	}
	
	#ifdef debugPrints
	printf("after Cswap_20bit_r_8bit_set: %X\n", new_set);
	printf("in permute address: original set=0x%X.after cswap :new_set is: 0x%X \n", original_set, new_set);
	#endif

	//printf("in permute address: original set=0x%X.after cswap :new_set is: 0x%X \n", original_set, new_set);
	//unsigned char new_set= Cswap_20bit_r_8bit_set(r, original_set);
	//creating the new address:
	int offset_size = Size_Of_offset_field_in_address;
	int setSize = Size_Of_Set_Field;
	int upperRange = setSize + offset_size - 1;
	unsigned int newAddress = clearBits(originalAddress, upperRange, offset_size); //tested [V]
	#ifdef debugPrints
	printf("newAddress after clearBits = %X \n", newAddress);
	#endif
	//inserting the new_set into the address :
	newAddress = newAddress | (new_set << (offset_size ));
	#ifdef debugPrints
	printf("before bound check newAddress (+new_set) = %X   (new_set=%X))\n", newAddress, new_set);
	#endif
	
	//now we need to keep the new address within array bounds:
	//calculate bounds:
	unsigned int arrayLowerBound = (unsigned int)memory;
	unsigned int arrayUpperBound = arrayLowerBound + (unsigned int)Memory_Size_In_Bytes;

	//New code from lidor: not tested yet!
	unsigned int memory_size = (int)Memory_Size_In_Bytes;

	/*
	if (newAddress < arrayLowerBound || newAddress >= arrayUpperBound)
		printf("OUT OF RANGE\n");
	int bool_isOutOfBounds = 0;
	if (newAddress < arrayLowerBound) {
		bool_isOutOfBounds = 1;
		//cylic add from the end of the array
		unsigned int delta = arrayLowerBound - newAddress;
		newAddress = arrayUpperBound - (delta % memory_size);
		//if(r%(0x800)==0)
		//		printf("\t   address TOO LOW!new address is 0x%X . r is 0x%X\n", newAddress,r);
	}
	else if (newAddress >= arrayUpperBound) {
		bool_isOutOfBounds = 1;
		//cylic add from the bottom/start of the array
		unsigned int delta = newAddress - arrayUpperBound;
		newAddress = arrayLowerBound + (delta % memory_size);
		//if (r % 0x800 == 0)
		//		printf("\t\t\t   address TOO HIGH! new address is 0x%X .  r is 0x%X\n", newAddress,r);
	}
	*/

	//printf("in permute address: address =0x%X . new_set is: 0x%X  ->new address 0x%X .corrected =%s\n", originalAddress, new_set, newAddress,bool_isOutOfBounds?"true":"false");
	return newAddress;
}

void permuteAddress_tester() {
	unsigned int addr= 0xFFFFFFFF;
	int res= permuteAddress(current_R, addr);
	printf("on addr =0x%X permuteAddress =0x%X\n", addr,res);

}

//recieves 32 bit address - returns the offset set taken from offset field bits: 
unsigned int getOffset(unsigned int address) {
	
	unsigned int offset = (address << Size_Of_tag_field_in_address)<<Size_Of_Set_Field; 
	offset = (offset >> Size_Of_tag_field_in_address )>>Size_Of_Set_Field;
	return offset;
}


//recieves 32 bit address - returns the set taken from set field bits: 
int getSet(int originalAddress) {
	//removing tag bits by shifting left and then right:
	unsigned int set = originalAddress << Size_Of_tag_field_in_address;
	set= set >> Size_Of_tag_field_in_address;
	//printf("removing tag bits :set = %X \n", set);
	//removing offset bits :
	set = set >> Size_Of_offset_field_in_address; // removes offset bits by shifting right:
	//printf("removing offset bits :set = %X \n", set);
	return set;
}


//recieves an address within the array - needs to calculate from it the line index in memory arry (line len is based on offset size -16 bytes)
//return index of the line in the array
//need to test
int calculateLineIndexInMem(int new_address) {
	int startAddr =(int) memory;
	int res = (new_address - startAddr) / 16;
	return res;
}

//returns the index of the line that has a matching "original_set" value , 
//use previous R table to search before searching linearly the whole history arr:
//flag:enableFastSearch =1 -> fast search is turned in.enableFastSearch =0 ->then fast search is off
int findLineWithSet(int originalAddress,unsigned char enableFastSearch) {
	int original_set = getSet(originalAddress);
	int lineIndexFound = 0;
	int i = 0;
	//TODO : STEP 1 :search using permuteAddress on old R values in the "previous_R_table": 
	//THIS LINE NEEDS TO CHANGE TO USE MODULO AND THE "index_head_r_table" global variable.
	for (i = 0; i < previous_R_table_size&& enableFastSearch; i++) {
		// (1)take old R value from previous_R_table:
		int prev_R = previous_R_table[i];
		//(2)calculate permuation based on old R value:
		int address_from_old_r =permuteAddress(prev_R, originalAddress);
		int line_index_from_old_mapping = calculateLineIndexInMem(address_from_old_r);
		//(3)check if we find the line where the data is based on the old value of R: 
		if (historyTable[line_index_from_old_mapping] == original_set) {
			//we found a match!
			return line_index_from_old_mapping;
		}
	}

	//STEP 2 : IF THERE IS NO MATCH USING PREVIOUS R VALUES (BECAUSE R_TABLE_SIZE IS LIMITED) 
	//SEARCH THE WHOLE HISTORY TABLE(FOR LINES->ORIGINAL SETS FOR ) WITH A LOOP - O(n) :
	for (i = 0; i < sizeOfHistoryTable; i++) {
		if (historyTable[i] == original_set) {
			return i;
		}
	}
	printf("** Error at findLinewithSet - we didnt find a matching line for the original set %d !!! \n", original_set);
	return -1;//ilgel line index as we didnt find the data
}
//Recieves two line indexes - calculate the strating addresses in memory array for the two lines and swaps the data of the two lines 
//adn updates history table!!!!
void swapMemoryLines_And_UpdateHistoryTable(int line_1, int line_2) {
	unsigned char* startAddr_1;
	unsigned char* startAddr_2;
	//for(...){  swap the data  }
	int i;
	//
	int  sizeOfOffsetField = Size_Of_offset_field_in_address;
	int numOfBytesInLine = pow(2, sizeOfOffsetField);
	//int byteOffset1, byteOffset2;
	//byteOffset1 = numOfBytesInLine * line1;
	//calculating start address for both lines :
	startAddr_1 = memory + (numOfBytesInLine * line_1);
	startAddr_2 = memory + (numOfBytesInLine * line_2);
	//printf("\ninside swapMemoryLines_And_UpdateHistoryTable\n");
	//printf("startAddr_1 0x%X and startAddr_2 0x%X \n",startAddr_1, startAddr_2);
	//then: swap all bytes in both 
	for (i = 0; i < numOfBytesInLine; i++) {
		char temp = startAddr_1[i];
		startAddr_1[i] = startAddr_2[i];
		startAddr_2[i] = temp;

	}
	//swap (the contents of historyTable[line_1], historyTable[line_1]) 
	//update history table:
	int temp = historyTable[line_1];
	historyTable[line_1] = historyTable[line_2];
	historyTable[line_2] = temp;
	
}

//Given an "original address" (point of view of programmer)  search for the value in permuted memory and return the matching data;
unsigned char readByteFromMem(unsigned int originalAddress) {
	//printf("readByteFromMem: originalAddress =0x%X\n", originalAddress);
	//(1)+(2)calc new address (changes set ,possibly tag?)
	int original_set = getSet(originalAddress);
	unsigned int new_address = permuteAddress(current_R, originalAddress);
	//printf("new_address =0x%X\n", new_address);
	//(3)
	int line_index_curr_mapping = calculateLineIndexInMem(new_address);
	//(4) check if historyTable[line_index] == original set - if so - we found the data - no swap is needed!
	if (historyTable[line_index_curr_mapping] == original_set) {
		//we found the information acccrding to the current mapping! return value::(6)
		//unsigned char* pointer = (unsigned char*)new_address;
		unsigned int diffFromStartOfMem = (new_address)-(unsigned int)memory;
		unsigned char* pointer = &memory[diffFromStartOfMem] ;
		//printf("inside if: pointer =0x%X\n", pointer);
		//printf("data at address 0x%X is : 0x%X", (unsigned int)pointer, *pointer);
		//printf("Test : memory array is at : 0x%X . and byte at memory[0] is: 0x%X ", memory, memory[0]);
		return (*pointer);
	}
	//If there is no match (4): search the history table for matching original set value:
	unsigned char fastSearchDisable = 0;
	int indexOfLineWhereDataIs = findLineWithSet(originalAddress, fastSearchDisable);
	//(5)We need to “fix” the mapping – we swap the line:
	swapMemoryLines_And_UpdateHistoryTable(indexOfLineWhereDataIs, line_index_curr_mapping);
	//now that the data has been brought to the right place : we can read from RAM:
	unsigned int diffFromStartOfMem = (new_address)-(unsigned int)memory;
	unsigned char* pointer = &memory[diffFromStartOfMem];
	return (*pointer);
}


//ToDO:
//permuteAddress - lidor - check carefully and write tests!
//findLinewithSet - Noam
//swapMemoryLines_And_UpdateHistoryTable() - Noam or lidor whoever finishes

//31.05:
/* 
finish //permuteAddress - Noam - check carefully and write tests!
write test for findLinewithSet(after finishing permute address
write the flow for memory access :
firstly we take the original address and apply permute address permuteAddress()
permuteAddress(int r, unsigned int originalAddress)
then check if history table original set matches the "original set in the history table)
if it is - return the byte - otherwise :
invoke find findLinewithSet()
then use the int index to create the address 

*/

//
// Lidor - finish permute address
//Noam -write tests for permute address and findLineWithSet.

//TODO : 07.06.2023:
//Implement the declaration of the memory area as random access and update the history table before we start to use
//permuteAddress and findLineWithSet:
//

//Tests:
