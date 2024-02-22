#pragma once

#include "Cswap.h"
#include <stdlib.h>
#include <time.h>
#include "randomizedMem.h"


//#define debugPrints 1
#ifdef debugPrints
#endif
/* define  
What do we want to do ?
1. define the size of the randomized access memory area : (1kb? ,4kb?)
2.


*/

//#define Memory_Size_In_Bytes 4096; //1kb- size of randomized area/region.
//#define  Register_File_Size 256 //TBD - in bytes-keeps track of mappings - not used currently
////////////////////////////////////////////////////////////////////////////////
//#define Size_Of_tag_field_in_address 20
//#define Size_Of_offset_field_in_address 4
//#define Size_Of_Set_Field 8 ;//in bits -will be 4 or 8 - only the size that we are permuting!
//To declare a global variable in a header file, you can use the extern keyword.
extern unsigned int current_R;
extern unsigned int* historyTable;
extern unsigned char* memory;
extern unsigned int sizeOfHistoryTable;
extern unsigned int previous_R_table_size;
extern unsigned int* previous_R_table;
extern unsigned int head_index_prev_R_table;


//_____________________________________________________________________________________________________
//Prints the history table(for "original sets" for each memory line)
void printHistoryTable();
void bring_Memory_And_HistoryTableIntoAKnownState();
int testReadingAllBytes();


int twoInPower(int num);

void TestReadByteFromMem() {

}

//bring the memory and the history table into a known state in which each "original line" is mapped to itself 
int testReadingWithoutPremutation() {
	current_R = 0;
    int bool_disable_shuffle = 0;
    printf("\n------------------TEST:testReadingWithoutPremutation :------------------\n");
    //bring history table into a known state:
    //changeHistoryTable_test(bool_disable_shuffle);
    bring_Memory_And_HistoryTableIntoAKnownState();
    //printMemoryContent();
    //printHistoryTable();
    printf("\n*******************testing*****************\n ");
    int boolTestRes=testReadingAllBytes();
    return boolTestRes;

}

//After we have brought the memory and hotry table into the known state - we want to read the values from it
//USING THE READ METHOD : //test assumes that the memory array has already been brought into a known state.
int testReadingAllBytes() {
    printf("-----------------------------------------------------------------\n");
    printf("**testReadingAllBytes** : (current_r= 0x%X) \n", current_R);
    int i;
    int offsetSize = Size_Of_offset_field_in_address;
    //int lineLenInBytes = pow(2, offsetSize);
    unsigned char expectedValue = 0;
    unsigned int currOriginalAddress = (unsigned int)memory;
    int memSize = Memory_Size_In_Bytes;
    for (i = 0; i < memSize; i++) {
        if (i > 0 && i % 16 == 0) {
            expectedValue++;
            printf("\n");

        }
        //read a single byte from mem :
        unsigned char byte = readByteFromMem(currOriginalAddress + i);
        printf("0x%02X,", byte);
        if (byte != expectedValue) {
            return 0;//false -test failed
        }

    }
    printf("\n");
    return 1;//TRUE - the test passed and values were read succefully

}



//Test:
//bring the memory and the history table into a known state in which each "original line" is mapped to itself
//change two lines - according to some mapping . 
//change the whole memory to fit to a legal value of r (to the mapping that is created by some legal R value =7FF)
//change history table as well and test:
//reading from original address works . AND there is a hit ! - meaning that when we search for the original set in the history table we are able to find it 
//return 1 if test passes - 0 otherwise.
int testReadingWithPermutation(int r) {
    printf("--------------------------------------------------------------------\n");
    printf("NOW TRYING TO CHANGE R VALUE TO 0X%04X \n", r);

    bring_Memory_And_HistoryTableIntoAKnownState();
    //change current r (r value used in permutation inside readByteFromMem()): 
    current_R = r;
    int readAllBytes_testRes= testReadingAllBytes();
    if (readAllBytes_testRes == 0) {
        return 0;//false = test failed!
    }
    //printf("\nAfter reading all bytes in memory we get :\n ");
    //printMemoryContent();
    //printHistoryTable();
    return readAllBytes_testRes;


}

//Test testReadingWithPermutation many times with different R values.
//returns 1 if test passes - 0 otherwise.
//will iterate over r value between 0 and the input max value of r (inclusive!).
int testReadingWithPermutation_Multiple_R(int maxValOfR) {
    int r;
    for (r = 0; r <= maxValOfR; r++) {
        int currTestRes= testReadingWithPermutation(r);
        if (currTestRes == 0) {//test failed
            printf("### Test testReadingWithPermutation failed at r =0x%04X ### \n ", r);
            //printf("\nAfter reading all bytes in memory we get :\n ");
            //printMemoryContent();
            //printHistoryTable();
            return 0;
        }
        
    }
    //all tests passed-> return true:
    return 1;
}


//tester for the getOffset() method :
void test_getOffset(int address,int numOfLinesInMem) {
    int i, j;
    for ( i = 0; i < numOfLinesInMem; i++) {
        for (j = 0; j <16 ; j++) {
            printf("%d,", getOffset(address + j));
        }
        printf("\n");
        address += 16;
    }
}


//helper method :raises two by the power of num -> (res = (2^exponent)  ) 
int twoInPower(int exponent) {
    if (exponent == 0)
        return 1;
    int res = 2;
    res = res << (exponent -1);
    return res;
}

//will put the set of the corresponding line into the history table;
void bring_Memory_And_HistoryTableIntoAKnownState() {
    int memSize = Memory_Size_In_Bytes;
    int offsetSize = Size_Of_offset_field_in_address;
    int  LineLenInMem = twoInPower(offsetSize);
    int numberOfLinesInMem = memSize / LineLenInMem;
    int i;
    int indexInMemory = 0;
    for (i = 0; i < numberOfLinesInMem; i++) {
        int valueToWrite = i;//valueToWrite to the whole line
        int j;
        for (j = 0; j < LineLenInMem; j++) { // write valueToWrite 16 times
            memory[indexInMemory] = valueToWrite;
            indexInMemory++;
        }
    }
    //now we will update the history table so that for each line index i=0...127 we will store the original set
    unsigned int currentAddress = (unsigned int)&memory[0];
    unsigned int lastAddr = currentAddress + memSize - 1;
    for (i = 0; i < sizeOfHistoryTable && (currentAddress<= lastAddr); i++) {
        int currSetForLine_i = getSet(currentAddress);
        historyTable[i] = currSetForLine_i;
        currentAddress += LineLenInMem;
    }
    
}

//This Test is not in use!
//Only for tests - do not use for setup
//if bool_enable_shuffle - the line indexes will be randomly shuffled - if not line[i]=i - not shuffling at all
void changeHistoryTable_test(int bool_enable_shuffle) {
    //I will write arbitrary values to the history table:
    int i;
    //filling the array with numbers 
    for (i = 0; i < sizeOfHistoryTable; i++) {
        historyTable[i] = i;
    }

    if (bool_enable_shuffle) {
        //now we shuffle using Fisher-Yates algorithm:
        for (int i = sizeOfHistoryTable - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = historyTable[i];
            historyTable[i] = historyTable[j];
            historyTable[j] = temp;
        }
    }
    
    /* unsigned int counter[128] = {0};
     for (i = 0; i < 128; i++) {
         counter[historyTable[i]]++;
     }
     for (i = 0; i < sizeOfHistoryTable; i++) {
         printf("%d count is %d \n", i, counter[i]);
     }
     */
    //printHistoryTable();
}









///________________________ LIDOR  _____________________________________
//Test:
//bring the memory and the history table into a known state in which each "original line" is mapped to itself
//change curr_R value and see the changes 
//and swap history table as well.
//change history table as well and test:
//reading from original address works .also see if we are getting a swap between the two lines that you swapped
void testReading_secondTest() {

}

