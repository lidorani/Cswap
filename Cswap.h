#pragma once
//#define debugPrints 1
#ifdef debugPrints
#endif
#include <stdio.h>
void printByte(unsigned char byte);
unsigned char CSwap(unsigned char input, unsigned char r_i);
//indexOfbit is 0 to 7 (inclusive).returns the bit at index indexOfbit
unsigned char bitAtIndex(int input, int indexOfbit) {
    //  printf(" input is :%x input>>indexOfbit is %X \n",input,input>>indexOfbit);
    return (input >> indexOfbit) & 0x00000001;

}

unsigned char insertBitAtIndex(unsigned char number, int index, unsigned char bitToInsert) {
    unsigned int mask = 0;
    if (bitToInsert == 0) {
        mask = (0xFE << index) | (0xFF >> (8 - index)); // padding zeros with zeroes when shifting left
        number = number & mask; //zeroing the the bit at the index
    }
    else {
        mask = (0x1 << index);
        number = number | mask;//setting the bit at the index to 1
    }
    return number;
}





unsigned char func(unsigned  int r, unsigned char set) {
    unsigned char newSet = 0;
    //swapping the two halves
    unsigned char r_lowerHalf = r % 256;
    unsigned char r_upperHalf = r / 256;
    unsigned char XorRes = set ^ r_lowerHalf;
    if (r % (0x800) == 0) {
        //printf("\tfunc: r = 0x%05X ,r_lowerHalf= 0x%02X, r_upperHalf= 0x%02X", r, r_lowerHalf, r_upperHalf);
        //printf(" |   xor res is : 0x%02X\n", XorRes);
         //printf("xor res :0x%02X,", XorRes);
        //printf("\n"); 
    }
    /*printf("xor res is : 0x%X\n", XorRes);
    printByte(XorRes);
    printf("\n");*/

    //now we can swap the order of bits:
    unsigned char intermidiateRes = 0;
    unsigned char first_block = CSwap(bitAtIndex(XorRes, 0) | bitAtIndex(XorRes, 4) << 1, bitAtIndex(r, 8));
    intermidiateRes = (first_block >> 1) << 4 | (first_block & 1) << 0;
    unsigned char second_block = CSwap(bitAtIndex(XorRes, 1) | bitAtIndex(XorRes, 5) << 1, bitAtIndex(r, 9));
    intermidiateRes = intermidiateRes | (second_block >> 1) << 5 | (second_block & 1) << 1;
    unsigned char third_block = CSwap(bitAtIndex(XorRes, 2) | bitAtIndex(XorRes, 6) << 1, bitAtIndex(r, 10));
    intermidiateRes = intermidiateRes | (third_block >> 1) << 6 | (third_block & 1) << 2;
    unsigned char fourth_block = CSwap(bitAtIndex(XorRes, 3) | bitAtIndex(XorRes, 7) << 1, bitAtIndex(r, 11));
    intermidiateRes = intermidiateRes | (fourth_block >> 1) << 7 | (fourth_block & 1) << 3;
    // printf("first_block=%X,second_block=%X,third_block=%X,fourth_block=%X\n",first_block,second_block,third_block,fourth_block);
     //printf("after shift first_block=%X,second_block=%X,third_block=%X,fourth_block=%X\n",first_block,second_block<<2,third_block<<4,fourth_block<<6);
    ////////////////////
    //printf("intermidiateRes is %X \n ", intermidiateRes);

    //Now we can do the second stage - swapping inside the 2 halves (4 bits swapping -the same as the figure in the paper)
    //oPERATING NOW ON "intermidiateRes " FROM PREV STAGE.
    unsigned char stage_2_res = intermidiateRes;
    //lower half swapping
    first_block = CSwap(bitAtIndex(intermidiateRes, 0) | bitAtIndex(intermidiateRes, 2) << 1, bitAtIndex(r, 12));
    //stage_2_res= stage_2_res | ( (first_block >> 1) << 2 | (first_block & 1) << 0  ) ;
    stage_2_res = insertBitAtIndex(stage_2_res, 0, first_block & 1);
    stage_2_res = insertBitAtIndex(stage_2_res, 2, first_block >> 1);
    // printf("stage_2_res after swapping index 0 and 2  is %X \n ", stage_2_res);
    second_block = CSwap(bitAtIndex(intermidiateRes, 1) | bitAtIndex(intermidiateRes, 3) << 1, bitAtIndex(r, 13));
    //stage_2_res=stage_2_res | ((second_block >> 1) << 3 | (second_block & 1) << 1);
    stage_2_res = insertBitAtIndex(stage_2_res, 1, second_block & 1);
    stage_2_res = insertBitAtIndex(stage_2_res, 3, second_block >> 1);
    //upper half swapping
    third_block = CSwap(bitAtIndex(intermidiateRes, 4) | bitAtIndex(intermidiateRes, 6) << 1, bitAtIndex(r, 14));
    //stage_2_res = stage_2_res | ((first_block >> 1) << 6 | (first_block & 1) << 4);
    stage_2_res = insertBitAtIndex(stage_2_res, 4, third_block & 1);
    stage_2_res = insertBitAtIndex(stage_2_res, 6, third_block >> 1);
    fourth_block = CSwap(bitAtIndex(intermidiateRes, 5) | bitAtIndex(intermidiateRes, 7) << 1, bitAtIndex(r, 15));
    //stage_2_res = stage_2_res | ((first_block >> 1) << 7 | (first_block & 1) << 5);
    stage_2_res = insertBitAtIndex(stage_2_res, 5, fourth_block & 1);
    stage_2_res = insertBitAtIndex(stage_2_res, 7, fourth_block >> 1);
    //printf("stage_2_res at the end = %X \n", stage_2_res);

    //third stage - swapping pairs of 2 bits (0 and 1 ,2 and 3 etc..)
    //QUESTION : DO WE NEED A BIGGER R? WE NEED FOUR MORE BITS FOR THE LAST STAGE:
    unsigned char stage_3_res = stage_2_res;
    intermidiateRes = stage_2_res;
    first_block = CSwap(bitAtIndex(intermidiateRes, 0) | bitAtIndex(intermidiateRes, 1) << 1, bitAtIndex(r, 16));
    stage_3_res = insertBitAtIndex(stage_3_res, 0, first_block & 1);
    stage_3_res = insertBitAtIndex(stage_3_res, 1, first_block >> 1);
    //
    second_block = CSwap(bitAtIndex(intermidiateRes, 2) | bitAtIndex(intermidiateRes, 3) << 1, bitAtIndex(r, 17));
    stage_3_res = insertBitAtIndex(stage_3_res, 2, second_block & 1);
    stage_3_res = insertBitAtIndex(stage_3_res, 3, second_block >> 1);
    //
    third_block = CSwap(bitAtIndex(intermidiateRes, 4) | bitAtIndex(intermidiateRes, 5) << 1, bitAtIndex(r, 18));
    stage_3_res = insertBitAtIndex(stage_3_res, 4, third_block & 1);
    stage_3_res = insertBitAtIndex(stage_3_res, 5, third_block >> 1);
    //
    fourth_block = CSwap(bitAtIndex(intermidiateRes, 6) | bitAtIndex(intermidiateRes, 7) << 1, bitAtIndex(r, 19));
    stage_3_res = insertBitAtIndex(stage_3_res, 6, fourth_block & 1);
    stage_3_res = insertBitAtIndex(stage_3_res, 7, fourth_block >> 1);

    intermidiateRes = stage_3_res;
    if (r % (0x800) == 0) {
       // printf("\ncswap res on set 0x%02X  is : 0x%02X ",set,intermidiateRes);
    }
    return intermidiateRes;
}

//Cswap with 8 bit set and 20 bit r 
unsigned char Cswap_20bit_r_8bit_set(unsigned int r, unsigned char set) {
   return func(r, set);
}




//recieves 2 bit input and a r (decides whether to swap or not)
unsigned char CSwap(unsigned char input, unsigned char r_i) {
    // printf("CSwap : input is : %X r_i is : %d \n", input, r_i);
    if (r_i == 0)
        return input; //no change
    else {//r_i==1 => swap!
       // printf("(input>>1) =%x\n" , (input>>1));
        //printf("(input&0x01)<<1 =%x\n" ,(input&0x01)<<1);
        return   (input >> 1) | ((input & 0x01) << 1);


    }

}


void printByte(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%u", bitAtIndex(byte, i));
    }
}

