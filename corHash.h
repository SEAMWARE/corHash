//
// FILE            corHash.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef CORHASH_CORHASH_H_
#define CORHASH_CORHASH_H_

#include "corAlloc/corAlloc.h"                         // corAlloc



// -----------------------------------------------------------------------------
//
// CorHashCodeFunction - user defined function that calculates the hash code
//
typedef unsigned int (*CorHashCodeFunction)(const char* name);



// -----------------------------------------------------------------------------
//
// CorHashCompareFunction - 
//
typedef int (*CorHashCompareFunction)(const char* name, void* itemP);



// -----------------------------------------------------------------------------
//
//  CorHashListItem -
//
typedef	struct CorHashListItem
{
  void*                  data;
  struct CorHashListItem*  next;
  unsigned int           hashCode;   // Cached hash code for fast comparison
} CorHashListItem;



// -----------------------------------------------------------------------------
//
// CorHashTable -
//
typedef struct CorHashTable
{
  CorHashListItem**     array;
  int                   arraySize;
  unsigned int          mask;             // For power-of-2 sizes: arraySize-1, else 0
  CorHashCodeFunction   hashCodeFunction;
  CorHashCompareFunction  compareFunction;
  CorAlloc*             kallocP;
} CorHashTable;



// -----------------------------------------------------------------------------
//
// corHashTableCreate
//
extern CorHashTable* corHashTableCreate(CorAlloc* kaP, CorHashCodeFunction hashFunction, CorHashCompareFunction compareFunction, int slots);



// -----------------------------------------------------------------------------
//
// corHashItemAdd
//
extern int corHashItemAdd(CorHashTable* hashTable, const char* itemName, void* itemData);



// -----------------------------------------------------------------------------
//
// corHashItemLookup
//
extern void* corHashItemLookup(CorHashTable* hashTable, const char* itemName);



// -----------------------------------------------------------------------------
//
// corHashItemRemove - remove an item from the hash table
//
extern int corHashItemRemove(CorHashTable* hashTable, const char* itemName);



// -----------------------------------------------------------------------------
//
// corHashItemCustomLookup
//
extern void* corHashItemCustomLookup(CorHashTable* hashTableP, const char* itemName, CorHashCompareFunction compareFunction);



// -----------------------------------------------------------------------------
//
// corHashItemLookupInAllSlots - temporal function ...
//
extern void* corHashItemLookupInAllSlots(CorHashTable* hashTableP, const char* itemName, CorHashCompareFunction compareFunction);



// -----------------------------------------------------------------------------
//
// corHashRelease
//
extern void corHashRelease(CorHashTable* hashTableP);

#endif  // CORHASH_CORHASH_H_
