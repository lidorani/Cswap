#pragma once
#include "randomizedMem.h"
//struct of 4 Bytes which is a word on MIPS and Xstensa machines.
struct Word
{
	unsigned char data[4] = { 0,0,0,0 };
};


void assignToWordStruct(struct Word* word, unsigned int value) {
	for (int i = 0; i < 4; i++) {
		word->data[i] = value % 256;
		value = value / 256;
	}

}

int WordToInteger(struct Word word) {
	int val = 0;
	for (int i = 3; i >= 0; i--) {
		val = val << 8;//shift right 8 places to make romm for the next byte
		val = val + word.data[i];
	}
	return val;
}

unsigned char getByteAt(struct Word word, int index) {
	return word.data[index];
}

int BlockTointeger(struct Word word) {
	int data = *(int*)&word;
	return data;
}

void printBlock(struct Word block) {
	unsigned char* bytes = block.data;

	printf("block at 0 is : 0x%X . block at 1 is :0x%X . block at 2 is : 0x%X  block at 3 is : 0x%X  (low byte to high) \n", bytes[0], bytes[1], bytes[2], bytes[3]);
}

char printWordInHex(struct Word word) {
	unsigned char* bytes = word.data;
	printf("0x%X%X%X%X", bytes[3], bytes[2], bytes[1], bytes[0]);
	return ' ';
}


///////////////////////////////////////////////////////////////////////////////////////////////
//TODO - get the upper "Size_Of_tag_field_in_address" bits from the integer
unsigned int getTagFrom_32bit_Address(unsigned int address_32_bit) {
	//Size_Of_tag_field_in_address
	int res = (address_32_bit) >> Size_Of_Set_Field;
	//printf("\n1.  res=0x%X", res);
	res = (res) >> Size_Of_offset_field_in_address;
	//printf("\n2.  res=0x%X\n", res);
	return res;
}


void structWordTester(int value) {
	struct Word block;
	block.data[0] = 0;
	printf("size of Word is %d\n", sizeof(Word));
	printf("before assign : block is :\n");
	printBlock(block);
	assignToWordStruct(&block, value);
	printf("after assign : block is :\n");
	printBlock(block);
	printWordInHex(block);
	unsigned int val = WordToInteger(block);
	printf("\nThe value of the word in hex is 0x%X. in decimal %u\n", val, val);

	//int int_address = (int)&block;
	int data = *(int*)&block;
	printf("data = %u , BlockTointeger=%u", data, BlockTointeger(block));
}



//*************accoprding to plan A : ********************************
/*
//TODO - lookup in the - register file for history - based on the tag of orginal address(maybe offset as well??)
//and return the r saved there.
//

int getCurrentRMappingForAddress(int originalAddress) {
	int startAddressForMemory = (int)memory;
	//access regfile -history table at ?

	return 0;
}
//TODO : perform Cswap on the set field of the address - and access memory at the new address and return the byte:
unsigned char readAssumingMappingIsCorrect(int originalAdrress) {
	return 0;
}


//TODO - reorder the (blocks ! )addresses belonging only to the group derived/taken from the tag field of address
void reorderAdrressesInMappingSet(int originalAddress) {
	int tag = getTagFrom_32bit_Address(originalAddress);
/*
	for (all addresses belonging to that "mapping set" in the (1kb?) radomized memory :) {
		swap according to the curr mapping;
	}

}
//TODO update the register file holding the history table - that the mapping for the mapping set is the current R.
//derive mapping set from tag of the address given (on address from that mapping set
void update_historyTable_mapping_Of_addrresses(int originalAddress) {
	int mapping_set_code = getTagFrom_32bit_Address(originalAddress);
	//todo
}


unsigned char readByteFromMem(int originalAddress) {
	int last_r_for_mapping_group = getCurrentRMappingForAddress(originalAddress);
	if (current_R == last_r_for_mapping_group) { //mapping is valid - we can read:
		return readAssumingMappingIsCorrect(originalAddress);
	}
	else { //need to re-order the mappings in that group according to the new R:
		reorderAdrressesInMappingSet( originalAddress);

		update_historyTable_mapping_Of_addrresses(originalAddress);
		//mapping is valid - we can read:
		return  readAssumingMappingIsCorrect(originalAddress);
	}

}

*/