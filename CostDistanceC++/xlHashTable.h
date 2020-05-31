/**
 * Author		:Xingong Li
 * Date			:May 10, 2011
 * Purpose	:The hash table class used in a hash table indexed min-heap. 
 */

#ifndef XLHASHTABLE_H_
#define XLHASHTABLE_H_

#include <exception> // general exception
#include <stdexcept> // out_of_range
#include <iostream>

using namespace std;

/// EXCEPTION
class HashTableDeleteException : public out_of_range{
	public:
	HashTableDeleteException(const string &message) : out_of_range(message) {;}
};

//
// A hash table class which stores the key and the pointer to the indexed 
// object. The indexed key must be long but the object can be any kind.
//

template <class idxObject> //indexed object as template parameter
// can also use: template<typename idxObject>
class xlHashTable {

private:
	// hash node is defined as a nested class
	class htNode {
		public:
			long idxKey; // key for the indexed object
			idxObject* ref; //pointer to the indexed objects
			htNode* next; //pointer to next node which has the same hash value
			
			// constructor
			htNode () {idxKey=0;ref=NULL; next=NULL ;}
	};

	long htSize; //size of the hash table
	htNode** htTable; //array of pointer to htNode of size of 0..htSize-1

public:

	// constructor with default hash table length
	xlHashTable() {
		htSize=1024; // default hash table length
		htTable=new htNode*[htSize];
		for ( int i=0; i < htSize; i++) { //initialize the table as NULL
			htTable[i]=NULL;
		}
	}

	// constructor with a user specified hash table length
	xlHashTable(long htLength) {
		htSize=htLength;
		htTable=new htNode*[htSize];
		for ( int i=0; i < htSize; i++) { //initialize the table as NULL
			htTable[i]=NULL;
		}
	}

	// destructor to release the memory used by the hash table
	~xlHashTable() {
		for ( int i=0; i < htSize; i++) {
			htNode* p = htTable[i];
			while (p!=NULL) {
				htNode* t=p;
				p = p->next;
				delete t;
			}
			htTable[i]=NULL;
		}
		delete[] htTable;
	}

	// calculate the hash value of the indexed key
	long hash(long idxKey) {
		return (idxKey % htSize); // hash function
	}

	// insert the key and the pointer to the indexed object into the hash table
	void insert(long idxKey, idxObject* e) {
		// get the location of the key in the hash table  
		long bucket = hash(idxKey);

		// create the new hash node
		htNode* p = new htNode;
		p->idxKey = idxKey;
		p->ref = e;

		// insert the node in the table table
		// insert node at beginning of list
		htNode* p0 = htTable[bucket];
		htTable[bucket] = p;
		p->next = p0;
	}

	// delete the key and the pointer to the indexed object from the hash table
	void remove(long idxKey) throw(HashTableDeleteException){

		// find the key from the table
		long bucket=hash(idxKey);
		htNode* p0 =NULL;
		htNode* p = htTable[bucket];

		while (p!=NULL) {
			if (p->idxKey == idxKey) 
				break;
			else {
				p0 = p;
				p = p->next;
			}
		}

		if (p==NULL) {
			//cout << "Exception: key " << idxKey << " is NOT found in the hash table!!!" << endl;
			// key not found in the table, throw an exception!!
			throw new HashTableDeleteException("Item not found in the hash table!");
		}
		else {
			//p points to the node to be deleted and p0 points to the previous node
			if (p0!=NULL) 
				// not the first node
				p0->next=p->next;
			else
				// p points to the first node
				htTable[bucket]=p->next;

			//delete the node
			delete p;
		}
	}
	
	// find whether the key is in the hash table
	idxObject* find(long idxKey) {
		//  find the node containing the key
		htNode* p = htTable[hash(idxKey)];
		while (p!=NULL) {
			if (p->idxKey == idxKey) 
				break;
			else
				p = p->next;
		}

		if (p!=NULL)
			return p->ref;
		else
			return NULL;
	}

	// print the hash table for debugging purpose
	void print() {
		cout << "Hash Table:"<<endl;
		for ( int i=0; i < htSize; i++) {
			htNode* p = htTable[i];
			cout << i << ": ";
			while (p!=NULL) {
				cout << p->idxKey << "; ";
				p = p->next;
			}
			cout << endl;
		}
		cout << endl;
	}
};

#endif /*XLHASHTABLE_H_*/