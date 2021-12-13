/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cache.h"

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

#define NO+_WRITE_ALLOCATE 0
#define WRITE_ALLOCATE 1


int main(int argc, char **argv) {

	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument
	char* fileString = argv[1];
	ifstream file(fileString); //input file stream
	string line;
	if (!file || !file.good()) {
		// File doesn't exist or some other error
		cerr << "File not found" << endl;
		return 0;
	}

	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

	for (int i = 2; i < 19; i += 2) {
		string s(argv[i]);
		if (s == "--mem-cyc") {
			MemCyc = atoi(argv[i + 1]);
		} else if (s == "--bsize") {
			BSize = atoi(argv[i + 1]);
		} else if (s == "--l1-size") {
			L1Size = atoi(argv[i + 1]);
		} else if (s == "--l2-size") {
			L2Size = atoi(argv[i + 1]);
		} else if (s == "--l1-cyc") {
			L1Cyc = atoi(argv[i + 1]);
		} else if (s == "--l2-cyc") {
			L2Cyc = atoi(argv[i + 1]);
		} else if (s == "--l1-assoc") {
			L1Assoc = atoi(argv[i + 1]);
		} else if (s == "--l2-assoc") {
			L2Assoc = atoi(argv[i + 1]);
		} else if (s == "--wr-alloc") {
			WrAlloc = atoi(argv[i + 1]);
		} else {
			cerr << "Error in arguments" << endl;
			return 0;
		}
	}

	Cache L1(L1Size, BSize, L1Assoc, WrAlloc, L1Cyc, MemCyc);
	Cache L2(L2Size, BSize, L2Assoc, WrAlloc, L2Cyc, MemCyc);
	
	while (getline(file, line)) {

		stringstream ss(line);
		string address;
		char operation = 0; // read (R) or write (W)
		if (!(ss >> operation >> address)) {
			// Operation appears in an Invalid format
			cout << "Command Format error" << endl;
			return 0;
		}

		// DEBUG - remove this line
		cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		cout << ", address (hex)" << cutAddress;

		unsigned long int num = 0;
		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		cout << " (dec) " << num << endl;
		
		if(operation == 'w'){
			if(WrAlloc == WRITE_ALLOCATE){
				if(L1.isBlockInCache(num)){ 							 /* Is block in L1 Cache? */
					block _block1 = L1.getBlockFromAddr(num);
					// _block1.writeToBlock();
					_block1.setBlockDirty(true);
				}
				else{ 
					block _block1 = L1.get_LRU_BlockFromSameLine(num);
					if(_block1 != nullptr){ 							/* Evacuate the block that will be replaced */
						if(_block1.isBlockDirty()){
							L2.updateBlock(_block1); /* if block 1 was dirty, block 2 must become dirty itself*/
						}
						L1.removeBlock(_block1);
					}
					if(L2.isBlockInCache(num)){ 						/* Is block in L2 Cache? */
						block _block2 = L2.getBlockFromAddr(num);
						// _block2.writeToBlock();
						_block2.setBlockDirty(true);
						L1.addBlock(num);
					}
					else{
						block _block2 = L2.get_LRU_BlockFromSameLine(num);
						if(_block2 != nullptr){
							if(_block2.isBlockDirty()){
								/* update memory*/
							}
							L2.removeBlock(_block1);
						}
						L1.addBlock(block(num));
						L2.addBlock(block(num, DIRTY));
					}
				}
			}
			else if(WrAlloc == NO_WRITE_ALLOCATE){
				if(L1.isBlockInCache(num)){ 							 /* Is block in L1 Cache? */
					block _block1 = L1.getBlockFromAddr(num);
					// _block1.writeToBlock();
					_block1.setBlockDirty(true);
				}
				else if(L2.isBlockInCache(num)){
						block _block2 = L2.getBlockFromAddr(num);
						// _block2.writeToBlock();
						_block2.setBlockDirty(true);
				}
				// else write to memory
			}
		if(operation == 'r'){
			if(L1.isBlockInCache(num)){ 							 /* Is block in L1 Cache? */
				// read
			}
			else{ 
				block _block1 = L1.get_LRU_BlockFromSameLine(num);
				if(_block1 != nullptr){ 							/* Evacuate the block that will be replaced */
					if(_block1.isBlockDirty()){
						L2.updateBlock(_block1); /* if block 1 was dirty, block 2 must become dirty itself*/
					}
					L1.removeBlock(_block1);
				}
				if(L2.isBlockInCache(num)){ 						/* Is block in L2 Cache? */
					L1.addBlock(num);
				}
				else{
					block _block2 = L2.get_LRU_BlockFromSameLine(num);
					if(_block2 != nullptr){
						if(_block2.isBlockDirty()){
							/* update memory*/
						}
						L2.removeBlock(_block1);
					}
					L1.addBlock(block(num));
					L2.addBlock(block(num));
				}
			}
		}
/*
		else Search num in L2
			if we found it - write or read and update accoring to the policy
			else go to memory (according to the policy) {
				write or read
				build new block
				bring the block to L2 and L1 
			}
	}
*/

	double L1MissRate;
	double L2MissRate;
	double avgAccTime;

	L1.updateValue(&L1MissRate);
	L2.updateValue(&L2MissRate);


	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
