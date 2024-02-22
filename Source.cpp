#include "Cswap.h"
#include "tests.h"
#include "randomizedMem.h"
#include <malloc.h>
//#include <math.h>
#include "Word.h"
#include "Benchmarks.h"



//#define memory_Array_Size_In_Bytes 4096
//defining  environment 
//Seed:
unsigned int previous_R_table_size = 8;
unsigned int* previous_R_table;
unsigned int head_index_prev_R_table;
unsigned int current_R = 0;
//register file:
unsigned int* historyTable; //saves "original sets" for memory lines (size is in integers ,it is the array's length)
unsigned int sizeOfHistoryTable;
//
unsigned char* memory;
unsigned char* Allocated_Mem_To_Free;


int testFindLineWithSet();
int test_lidor_permute_test();
int testclearBits();
int testCswap_20bit_r_8bit_set_for_injectivity();
unsigned char* findLegalStartingAddress(unsigned char* AllocatedMemory, int sizeOfAllocatedMem);
char testPermuteAddress(unsigned int r);

//allocate arrays:
void setUpEnvironment() {
    initializePRNG();
    //Allocating memory for the environment arrays.
    int memSize = (int)Memory_Size_In_Bytes;
    int bytesToAllocate = 2 * memSize;
    Allocated_Mem_To_Free = (unsigned char*)  calloc( (bytesToAllocate) , sizeof(unsigned char));
    memory = findLegalStartingAddress(Allocated_Mem_To_Free, bytesToAllocate);
    //now in the allocated memory we will look for a suitable memory address:
    // offset =0 , set =0 . -> bits [0:11] (inclusive) sould be zero :

    
    //We need to calculate regFileSize= ( R_size_in_bits*( Memory_Size_In_Bytes / cache_line_size_in_bytes)  )/8
    int sizeOfsetField = Size_Of_Set_Field;
    int regFileSize = (memSize / pow(2, Size_Of_offset_field_in_address)); //num of lines in memory arr = memsize_in_bytes/line len
    sizeOfHistoryTable = regFileSize;
    printf("setUpEnvironment():regFileSize = %d bytes. \n", regFileSize);
    //Allocating memory for the environment arrays.
    historyTable = (unsigned int*)calloc((regFileSize), sizeof(unsigned int));
    previous_R_table = (unsigned int*)calloc(previous_R_table_size, sizeof(int));
    //If allocations failed : we exit the program 
    if (historyTable == 0 || previous_R_table == 0 || memory == 0) {
        printf("memory allocation failed");
        exit(19);
    }

}

unsigned char* findLegalStartingAddress(unsigned char* AllocatedMemory,int sizeOfAllocatedMem) {
    int i;
    unsigned char* pointer = AllocatedMemory;
    unsigned int offset = getOffset((unsigned int)AllocatedMemory);
    //changing the offset to be 0:
    while (getOffset((unsigned int)pointer) != 0) {
        pointer++;
    }
    //now the starting offset is 0 - Now  we need to change the set to 0
    for (i = 0; i < sizeOfAllocatedMem; i=i+16) {
        if (getSet((unsigned int)pointer) == 0) {
            printf("The original address was 0x%X - the pointer returned from *findLegalStartingAddress* is 0x%X \n", (unsigned int)AllocatedMemory, (unsigned int)pointer);
            return pointer;
        }
        pointer += 16;
    }
    
    //printf("\n\n\n\n\n\n\n\n");
    return 0; //error code - did not find a legal address
}



void freeEnvironment() {
    //free(memory);
    free(Allocated_Mem_To_Free);
    free(historyTable);
    free(previous_R_table);
}



void testForCswap() {
    unsigned char x = 0x1;
    x = insertBitAtIndex(x, 2, 1);

    unsigned short int r = 0xFFBB;//0xAABB;
    unsigned char r_low_byte = 0xBB;//0xAA;
    unsigned char r_high_byte = 0xFF; // 0xFF;
    unsigned char set = 0x10;
    printf("r is =0x%X | 0x%X\n", r_high_byte, r_low_byte);
    printf("set is : %x \n", set);
    //printf("r[8-11] : %d \n", 1111);
    unsigned char res = func(r, set);
    printf("res after bit shuffling 0x%X  ", res);
    printByte(res);
    printf("\n\n******************************\n");
    //for(int i=7;i>=0;i--){
    //    printf("%u",bitAtIndex(res,i));
    //}
    //printf("0x%x",CSwap(0x0,1));
}

void testForRandomizedMem() {
    printf("\n\n ****** Test test For Randomized Mem  ********\n ");
    permuteAddress_tester();
    int address_32bit = 0xABCDEFFF; //offset=F,set=FF,tag=abcd
    printf("if address_32bit = 0x%X - then getTagFrom_32bit_Address() = 0x%X \n", address_32bit, getTagFrom_32bit_Address(address_32bit));
    setUpEnvironment();
    //tests:
    int memSize = Memory_Size_In_Bytes;
    int lastAddr = (int)memory + (memSize - 1);
    printf("start addr of memory arr is : 0x%X and last addr is : 0x%X ,difference is:%d\n ", (int)memory, lastAddr, lastAddr - (int)memory);
    memory[0] = 1;
    memory[15] = 0xAA;
    memory[1023] = 0;
    printMemoryContent();
    printf("\ncontent is memory[0] = %d ,memory[1023]=%d , and &memory[1023] is %d \n", memory[0], memory[1023], (int)&memory[1023]);
    //test getSet:
    address_32bit = 0xBBBBB43A; // tag = BBBBB , set= 43 , offset= A
    int setFromTest = getSet(address_32bit);
    printf("getSet for 0xBBBBB43A is %X \n", setFromTest);
    //Test printPreviousRTable() ,changeRValue(),findLineWithSet():
    changeRValue(55);
    changeRValue(29);
    changeRValue(31);
    /*printPreviousRTable();*/
    //bring history table into known state then print it:
    //printHistoryTable();

    printf("\n\n\n\n--------------------------------------------\n");
    printf("--------------------------------Correctness tests:--------------------------------\n");
    int bool_testFindLineWithSet = testFindLineWithSet();
    printf("testFindLineWithSet res is: %s\n", bool_testFindLineWithSet ? "Success!" : "**Test Failed**");
    //test_lidor_permute_test();
    int bool_testclearBits =testclearBits();
    printf("testclearBits res is :%s\n", bool_testclearBits ? "Success!" : "**Test Failed**");
     int testRes=testCswap_20bit_r_8bit_set_for_injectivity();
     printf("testCswap_20bit_r_8bit_set_for_injectivity res is: %s\n", testRes ? "Success!": "**Test Failed**");
    
     int mutiple_R_TestRes =test_lidor_permute_test();
     printf("test_lidor_permute_test res is: %s\n", mutiple_R_TestRes ? "Success!" : "**Test Failed**");
     
     //Testing test.h tests : mainly the reading of all of the memory :
     int readingWithoutPremutation_TestRes= testReadingWithoutPremutation();
     printf("testReadingWithoutPremutation res is: %s\n", readingWithoutPremutation_TestRes ? "Success!" : "**Test Failed**");
     
     
     int readingWithPermutation_testRes = testReadingWithPermutation(0x00001);
     printf("\n");
     printf("readingWithPermutation_testRes res is :%s\n", readingWithPermutation_testRes ? "Success!" : "**Test Failed**");
    
     
     int testReadingWithPerm_Multi_R_Test_Res = testReadingWithPermutation_Multiple_R(0x0FF);
     printf("\n");
     printf("readingWithPermutation_Multi_R_testRes res is :%s\n", testReadingWithPerm_Multi_R_Test_Res ? "Success!" : "**Test Failed**");
     
     printf("\n--------------------------------End of tests--------------------------------\n");
    //We need to free the allocated memory in the end:
    freeEnvironment();
}

//Input: current r
//Output: -
//Function description: This function tests if the function "permuteAddress" is invertible, by iterating over addresses of the lines and
//                      checking if the permuted address covers the whole range.
char testPermuteAddress(unsigned int r) {
    //Firstly - we will bring the history table into a known state
    //We need to test :
    //1.Iterate over all addresses in memory area (or just start of each line ) and check with prints or with an array that there are no duplicates!)
    //2. check to see that there are no duplicates /missing values for line numbers.

    //creating array to hold values, after running the test it should be filled with ones
    int memSize = Memory_Size_In_Bytes;
    int rows = (memSize / pow(2, Size_Of_offset_field_in_address)); //num of lines in memory arr = memsize_in_bytes/line len
    int* arr = (int*)calloc(rows, sizeof(int));

    if (arr == NULL) {
        printf("ERROR: \"testPermuteAddress\" failed due to memory allocating issues\n");
        return 0;
    }

    /*for (int i = 0; i < rows; i++) {
        arr[i] = 0;
    }*/

    unsigned int start_address = (unsigned int)memory;
    unsigned int address, new_address;
    int line;
    for (int i = 0; i < rows; i++) {
        address = start_address + i * 16;
        new_address = permuteAddress(r, address);
        #ifdef debugPrints
        printf("original address = 0x%X, new_address = 0x%X ", address, new_address);
        unsigned int firstTag = (unsigned int)address & 0xFFFFF000;
        int secondTag = (unsigned int)new_address & 0xFFFFF000;
        printf(".firstTag is 0x%X,secondTag is 0x%X .they are %s\n", firstTag, secondTag, firstTag==secondTag?"equal":"** Different!!! **");
        #endif
        line = calculateLineIndexInMem(new_address);
        arr[line] += 1;
    }
    //print the counter arr (All array cells should be set to 1).
    /*for (int i = 0; i < rows; i++) {
        printf("%d",arr[i]);
    }
    printf("\n");*/
    //printf("r=%x\n", r);
    /*
    if (r % (0x800) == 0) {
        //printf("\n");
        for (int i = 0; i < 128; i++) {
          //  printf("%d", arr[i]);
        }
        //printf("\n");
    }
    
    if (r % (0x800) == 0) {
        //printf("\n----\n");
    }
    */

    int count0 = 0, count1 = 0, count2 = 0, other = 0;
    for (int i = 0; i < rows; i++) {
        switch (arr[i]) {
        case 0: count0++;
            break;
        case 1: count1++;
            break;
        default: other++;
        }
    }

    free(arr);

    printf("for r=0x%X: false=%d, right=%d\n", r, count0 + other, count1);
    if (count0 == 0)
        return 1;
    else return 0;
    /*
    //validity check
    bool isValid = true;
    for (int i = 0; i < 128; i++) {
        printf("i=%d\n", i);
        if (arr[i] != 1) {
            printf("Error in function \"permuteAddress\" for r=%X\n", r);
            isValid = false;
            break;
        }
    }
    if (isValid == true) {
        printf("test completed succesfully **for r=%X**, function \"permuteAddress\" is invertable",r);
    }
    */
}
//Testing if all memory addresses have a correspnding line number [0-255] - if one set is not found - then test fails
int testFindLineWithSet() {
    int bool_TestRes = 1; //true - if all set values are found in the history table
    printf("****testing the FindLineWithSet****\n");
    //set history table to have all of the set values
    int isShuffled = 0;
    changeHistoryTable_test(isShuffled);
    //printHistoryTable();
    int i = 0;
    int mem_size_in_bytes = Memory_Size_In_Bytes;
    int address = (int)&memory[0];
    for (i = 0; i < mem_size_in_bytes; i = i + 16) {
        int lineIndex = calculateLineIndexInMem(address);
        char fastSearchDisable = 0;
        //printf("for address %X ,(line %03d ,set=%02X) :findLineWithSet is :", address, lineIndex, getSet(address));
        int findLineWithSet_Output = findLineWithSet(address, fastSearchDisable);
        //printf("% 03u \n", findLineWithSet_Output);
        if (findLineWithSet_Output == -1) {
            return 0;//false-test failed - as the function returned an error code -1 -> did not find the set in the table
        }
        address += 16;
    }
    //findLineWithSet();
    return 1;//test passed
}





int test_lidor_permute_test() {
    unsigned int test_address = (int)memory;
    int memSize = Memory_Size_In_Bytes;
    /*
    printf("\n\n******TESTING CSWAP****\n");
    unsigned int r = 0x111F1;
    printf("test adddres is 0x%X , r for the test is 0x%X ", test_address,r);
    printf("set is: %X\n", getSet(test_address));
    unsigned char res =Cswap_20bit_r_8bit_set(r, getSet(test_address));
    printf("res of Cswap is 0x%X\n", res);
    printf("\n******TESTING permuteAddress****\n");
    r = 0x10101;
    printf("current address is: 0x%X    \nset is: %X    \nr equals %X \n\n", test_address, getSet(test_address), r);
    unsigned int permutedAddress = permuteAddress(r, test_address);
    printf("Bounds are: 0x%X, 0x%X ,difference is %d \n", (int)memory, (unsigned int)memory + (unsigned int)memSize, (unsigned int)memory + (unsigned int)memSize- (unsigned int)memory);
    printf("$$ permutedAddress res is 0x%X $$\n\n", permutedAddress);
    */

    printf("*************test_lidor_permute_test(): TESTING FUNCTION: permuteAddress (if it's invertible)************************\n");
    int count = 0, i = 0;
    unsigned int r = 0x0000000;
    int bool_prev_test_res = 0;
    unsigned int numberOfRValuesToTest = 0x0000FFFF;
    printf("Testing R range from 0 to 0x%X :\n", numberOfRValuesToTest);
    for (i=0; i <= numberOfRValuesToTest; i++){
        int bool_test_res = testPermuteAddress(r);
        if (bool_test_res){
            count++;
        }
        else {
            printf("PROBLEM WITH R=%x, \"permuteAddress\" is not bijective!\n", r);
            return 0;//false = test failed
        }

        if (i == 0 || bool_prev_test_res != bool_test_res) {
            printf("-> change in test res: curr R 0x%X, prev R:0x%X ,test res changed to %s\n", r, r - 1, bool_test_res ? "true" : "false");
            printf("-------------------------------------------------------------------------------------\n");
        }
        bool_prev_test_res = bool_test_res;
        r++;
    }
    if (count == i) {
        printf("\nPerfect! all %d tests have passed succesfully\n", count);
        return 1; //true = test passed.
    }
    else {
        return 0;//false = test failed
    }
    
    //unsigned int r = 0x111F0;
    //testPermuteAddress(r);
}


//Test to make sure that clearing the bits at range [4:11] (inclusive) works.
//returns 1 if test passes - 0 otherwise.
int testclearBits() {
    int i = 0;
    int r = 0;
    unsigned  int address = (unsigned int)memory;
    int bool_check = 0;
    printf("\n******** test:testclearBits **********\n ");
    for (i = 0; i < 256; i++) {
        unsigned int afterClearing = clearBits(address, 11, 4);
        if ((afterClearing & 0x00000FF0)==0) {
            bool_check = 1;
        }
        else {
            bool_check = 0;
            return 0;//test failed.
        }

        //printf("address= 0x%X after clearBits = 0x%X .check : %s\n", address, afterClearing, bool_check?"true":"false");
        address += 16;
    }
    return 1; //true -test passed!
}

//This method is not needed - EARASE LATER!
//injectivity = one to one - had had ercki . 
int testCswap_20bit_r_8bit_set_for_injectivity() {
    unsigned int r = 0xE84BA;
    int i = 0;
    unsigned  int address = (unsigned int)memory;
    int boolMutipleTestRes = 1;
    printf("\n **** test : testCswap_20bit_r_8bit_set_for_injectivity -only printing errors*****\n");
    for (i = 0; i < 1; i++) {
        int booRes = testPermuteAddress(r);
        if (booRes == 0) {
            printf("r=0x%X . injectivity is %s \n", r, booRes ? "true" : "false");
            return 0;
        }
        r++;
    }
    return boolMutipleTestRes;

}





int main()
{
    printf("****Running tests****\n");
    //structWordTester(0xabcdeffc);
    //testForRandomizedMem();
    setUpEnvironment();
    //testCswap_20bit_r_8bit_set_for_injectivity();
    int hitCounter = 0, missCounter = 0;
    //runMemoryBenchmark(10,&hitCounter,&missCounter);
    int intervals = 10;
    runMemoryBenchmark_MultipleTimes_Measure_Hit_Miss_Rate(10, 9000, intervals);
    //printf("%d , 0x%X", RAND_MAX, RAND_MAX);
    freeEnvironment();
    printf("\n**********Tests ended  *********\n\n\n");
    return 0;
}