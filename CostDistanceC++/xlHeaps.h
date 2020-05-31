/**
 * Author		:Xingong Li
 * Purpose	:A hash table indexed min-heap. Currently, the main use of the classes is for 
 *					calculating cost distance and sea level rise inundation height. A max-heap 
 *					can also be added later.
 */

#ifndef XLHEAPS_H_
#define XLHEAPS_H_

#include <vector>
#include <exception> // general exception
#include <stdexcept> // out_of_range
#include <iostream>
#include <cassert>

#include "xlHashTable.h"

using namespace std;

// macros for navigation in the binary tree stored in the vector
#define PARENT(pos) ((pos-1)>>1) 	// equivalent to floor(pos/2)
#define LEFT(pos)   ((pos<<1)+1) // equivalent to pos*2 + 1
#define RIGHT(pos)  ((pos<<1)+2) // equivalent to pos*2 + 2

/// EXCEPTION
class HeapEmptyException : public out_of_range{
	public:
	HeapEmptyException(const string &message) : out_of_range(message) {;}
};
class InvalidKeyIncreaseException : public out_of_range{
	public:
	InvalidKeyIncreaseException(const string &message) : out_of_range(message) {;}
};
class InvalidIndexException : public out_of_range{
	public:
	InvalidIndexException(const string &message) : out_of_range(message) {;}	
};

class xlHeapException : public out_of_range{
	public:
	xlHeapException(const string &message) : out_of_range(message) {;}
};


template <class HeapKey> //HeapKey can be "double" or a class
class xlMinHeap{

private:
	//
	class HeapElement {
		public:
			HeapKey hKey; // heap key
			long		idxKey; // index key for the heap key
			int			backIdx; // back index to the heap key

			//HeapElement() {hKey=NULL: idxKey=0; backIdx=-1;}
			// constructor
			HeapElement(HeapKey hKey, long idxKey, int backIdx) {
				HeapElement::hKey=hKey; 
				HeapElement::idxKey=idxKey;
				HeapElement::backIdx=backIdx;
			}
	};

private:
	//The heap is implemented using a vector and the root is assumed to be at end of the vector
	//The vector stores the pointers to the objects to save time when vector grows or shrinks.
	//See Lippman’s comments on page 259 in C++ Primer?
	vector<HeapElement*> vec; 

	//hashtable length, should be set the number of columns in a raster layer
	long mHashTableLength;
	xlHashTable<HeapElement>* ht;
	
public:
	/// Simple constructor with default hash table length
	xlMinHeap(){
		mHashTableLength = 1024;
		ht=new xlHashTable<HeapElement>(mHashTableLength);
	}
	
	/// constructor with user specified hash table length
	xlMinHeap( long hashTableLength ){
		mHashTableLength=hashTableLength;
		ht=new xlHashTable<HeapElement>(mHashTableLength);
	}
	
	//destructor
	~xlMinHeap() {
		//delete the elements in the vector
		for (int i=0; i < vec.size(); i++) {
			HeapElement* he = vec[i];
			delete he;
		}
		//delete the hash table
		delete ht;
	}

	/// returns the size of the heap
	int size(){ 
		return vec.size();
	}
	
	/// check for emptyness
	bool empty(){
		return vec.empty();
	}

	/// return a copy of heap key and its indexed key pair from the heap by its index in the underlying vector 
	// This function is designed for quickly access all the heap keys and their indexed keys (for updating all the elements)
	// When update the heap key, we have to use the index key, as each update will change the index in the underlying vector!!
	pair<HeapKey,long> getKeys(int idx) throw(xlHeapException){
		// idx is the index of the vector!
		if( vec.empty() )
			throw new xlHeapException("Empty heap, no heap key and index key are available!");
		if (idx<0 || idx>=vec.size())
			throw new xlHeapException("Index is out of heap range!");
		else 
			return make_pair(vec[idx]->hKey,vec[idx]->idxKey);
	}

	/// return a copy of the heap key at an indexed key
	HeapKey getHeapKey(long idxKey) {
		// find the heap element through the hash table (this is the reason of using a hash table here)
		HeapElement* he=ht->find(idxKey);
		if (he==NULL) 
			// no heap key can be found at the indexed key
			return NULL;
		else
			return he->hKey;
	}
	
	//// This is a bad idea as the index of a heap key in the vector changes when we update its heap key.
	//// So, we can only use the index key to up date the heap key!!!
	///// update a heap key by its index in the underlying vector
	///// this function is used to update all the heap keys in the vector
	//void updateHeapKey(int idx, HeapKey hKey) throw(xlHeapException){
	//	// idx is the index of the vector!
	//	if( vec.empty() )
	//		throw new xlHeapException("Empty heap, can not update the heap key!");
	//	if (idx<0 || idx>=vec.size())
	//		throw new xlHeapException("Index is out of heap range!");
	//	else {
	//			// new key is smaller than existing heap key, move up the new key
	//			if (hKey<vec[idx]->hKey) decreaseKey(idx, hKey);
	//			// new key is larger than the existing key, move down the new key
	//			if (hKey>vec[idx]->hKey) {
	//				vec[idx]->hKey=hKey;
	//				minHeapify(idx);
	//			}
	//	}
	//	return;
	//}	

	/// update a heap key through its indexed key 
	void updateHeapKey(long idxKey, HeapKey hKey) throw(xlHeapException){
		// find the heap element through hash table 
		HeapElement* he=ht->find(idxKey);
		if (he==NULL) {
			throw new xlHeapException("The key doesn't exist, can not update!");
		}
		else { 
			// new key is smaller than existing heap key, move up the new key
			if (hKey<he->hKey) decreaseKey(he->backIdx, hKey);
			// new key is larger than the existing key, move down the new key
			if (hKey>he->hKey) {
				he->hKey=hKey;
				minHeapify(he->backIdx);
			}
		}
	}
		
	/// pushes a new heap key or decrease an existing heap key
	/// The existing heap key is got updated only when the new key is smaller than the existing key.
	/// This is the difference between update() and push()
	void push(HeapKey hKey, long idxKey){
		// find the heap element through hash table (this is the reason of using a hash table here)
		HeapElement* he=ht->find(idxKey);

		if (he==NULL) { // this is a new heap key
			//get back index, i.e., the last position in the vector
			//the back index is used to quickly get the position of the heap key in the vector
			int backIdx=vec.size();

			//create a heap element
			HeapElement *he1=new HeapElement(hKey,idxKey,backIdx);
			
			//push the element into the back of the vector
			vec.push_back(he1);

			//trickles the element up to its proper position
			decreaseKey(backIdx, hKey);

			// add the heap key to the hash table
			ht->insert(idxKey,he1);
		}
		else { // the heap key is already in the heap
			if (hKey < he->hKey) {
				decreaseKey(he->backIdx, hKey);
			}
		}
	}
	
	/// return a copy of the heap key and its index key at the top of the heap
	pair<HeapKey,long> top() throw(HeapEmptyException){
		if( vec.empty() )
			throw new HeapEmptyException("Empty heap!");
		else {
			HeapElement* he=vec[0];
			return make_pair(he->hKey,he->idxKey);
		}
	}

	/// extract the top element of the heap
	pair<HeapKey,long> pop() throw(HeapEmptyException){
		if( vec.size() < 1 ) //a.k.a. heap.empty()
			throw new HeapEmptyException("Heap underflow");
		
		// get the top element
		HeapElement *he=vec[0];
		long idxKey=he->idxKey;
		HeapKey hKey=he->hKey;

		// overwrite top with last element
		vec[0] = vec.back();
		vec[0]->backIdx=0; //change its back index
		
		// remove the last element
		vec.pop_back();
		
		// start heapify from root
		minHeapify(0);
		
		// release the memory to the heap element
		delete he;	

		// delete the heap element from the hash table
		ht->remove(idxKey); 

		return make_pair(hKey,idxKey);
	}
	
	/// print heap element and their back index (debug purposes)
	void print() {
		// print vector content
		cout << "Key Objects:"<<endl;
		for ( int i=0; i < vec.size(); i++)
			cout << " " << vec[i]->hKey << "(" << vec[i]->backIdx << ")" <<endl;
		cout << endl;
		
		//print hash table content
		ht->print();

		return;
	}
	
private:
	
	void minHeapify(int curIdx){
		// Converts a semiheap rooted at Root (i.e., index 0) into a min-heap
		// Recursively trickle the item at Root down to 
		// its proper position by swapping it with its smaller child, if that child is smaller then the item
		// If the item is a leaf, nothing needs to be done

		unsigned int leftIdx = LEFT( curIdx );
		unsigned int rightIdx = RIGHT( curIdx );
		
		// decide if and where to swap, left or right, then swap
		// current is the best choice (defalut)
		int smallerIdx;
		
		// is left a better choice? (exists an invalid placed smaller value on the left side)
		if( leftIdx < vec.size() && vec[leftIdx]->hKey < vec[curIdx]->hKey )
			smallerIdx = leftIdx;
		else
			smallerIdx = curIdx;
		
		// is right a better choice? (exists an invalid placed smaller value on the right side)
		if( rightIdx < vec.size() && vec[rightIdx]->hKey < vec[smallerIdx]->hKey )
			smallerIdx = rightIdx;
	
		// a better choice exists?
		if( smallerIdx != curIdx ){
			// swap elements
			swap( curIdx, smallerIdx );
			
			// recursively call this function on alterated subtree
			minHeapify( smallerIdx );
		}
	}

	/// propagates the correctness (in heap sense) down from a vertex curIdx
	void decreaseKey( int curIdx, HeapKey hKey ) throw(InvalidKeyIncreaseException){
		// check if the given key update is actually an increase
		if( hKey > vec[curIdx]->hKey )
			throw new InvalidKeyIncreaseException("In a min-heap only decrease in key updates are legal");

		// update value with current key
		vec[curIdx]->hKey = hKey;
				
		// traverse the tree up making necessary swaps
		int parentIdx = PARENT(curIdx);
		while( curIdx > 0 ){
			if( vec[ parentIdx ]->hKey > vec[ curIdx ]->hKey ){
				// make swap
				swap( curIdx, parentIdx );
				// move up
				curIdx = parentIdx;
				parentIdx = PARENT(curIdx);
			} else {
				break;
			}					
		}
	}

	/// swap the content of two elements in position pos1 and pos2
	void swap(int pos1, int pos2){
		assert( !vec.empty() );
		assert( pos1>=0 && pos1<(int)vec.size() );
		assert( pos2>=0 && pos2<(int)vec.size() );
		
		// swap the back index
		vec[pos1]->backIdx = pos2;
		vec[pos2]->backIdx = pos1;

		// swap the element
		HeapElement *he;
		he = vec[pos1];
		vec[pos1] = vec[pos2];
		vec[pos2] = he;
	}

};

#endif /*XLHEAPS_H_*/