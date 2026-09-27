//
// FILE            corHash.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdlib.h>                                    // malloc, NULL
#include <string.h>                                    // memset

#include "kalloc/kaAlloc.h"                            // kaAlloc

#include "corHash/corHash.h"                           // Own interface


// Branch prediction hints for hot paths
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)



// -----------------------------------------------------------------------------
//
// corHashTableCreate -
//
CorHashTable* corHashTableCreate(KAlloc* kaP, CorHashCodeFunction hashFunction, CorHashCompareFunction compareFunction, int slots)
{
  CorHashTable* hashTableP = (CorHashTable*) ((kaP != NULL)? kaAlloc(kaP, sizeof(CorHashTable)) : malloc(sizeof(CorHashTable)));

  if (unlikely(hashTableP == NULL))
    return NULL;

  hashTableP->array = (CorHashListItem**) ((kaP != NULL)? kaAlloc(kaP, slots * sizeof(CorHashListItem*)) : malloc(slots * sizeof(CorHashListItem*)));
  if (unlikely(hashTableP->array == NULL))
  {
    if (kaP == NULL)
      free(hashTableP);
    return NULL;
  }

  // Initialize all slots to NULL
  memset(hashTableP->array, 0, slots * sizeof(CorHashListItem*));

  hashTableP->arraySize        = slots;
  hashTableP->hashCodeFunction = hashFunction;
  hashTableP->compareFunction  = compareFunction;
  hashTableP->kallocP          = kaP;

  // Check if arraySize is power of 2 for fast modulo using bitmask
  hashTableP->mask = ((slots & (slots - 1)) == 0) ? (slots - 1) : 0;

  return hashTableP;
}



// -----------------------------------------------------------------------------
//
// corHashItemAdd
//
int corHashItemAdd(CorHashTable* hashTableP, const char* itemName, void* itemData)
{
  CorHashListItem*  itemP;
  unsigned int    hashCode;
  unsigned int    slot;

  if (hashTableP->kallocP != NULL)
    itemP = (CorHashListItem*) kaAlloc(hashTableP->kallocP, sizeof(CorHashListItem));
  else
    itemP = (CorHashListItem*) malloc(sizeof(CorHashListItem));

  if (unlikely(itemP == NULL))
    return -1;

  itemP->data = itemData;

  // Compute hash code once and store it for fast comparison during lookup
  hashCode = hashTableP->hashCodeFunction(itemName);
  itemP->hashCode = hashCode;

  // Get slot using bitmask if power-of-2, otherwise modulo
  slot = (hashTableP->mask != 0) ? (hashCode & hashTableP->mask) : (hashCode % hashTableP->arraySize);

  // Insert the item in its slot (as the first item of the list)
  itemP->next = hashTableP->array[slot];
  hashTableP->array[slot] = itemP;

  return 0;
}



// -----------------------------------------------------------------------------
//
// corHashItemLookup
//
void* corHashItemLookup(CorHashTable* hashTableP, const char* itemName)
{
  unsigned int    hashCode;
  unsigned int    slot;
  CorHashListItem*  itemP;

  // Compute hash code once
  hashCode = hashTableP->hashCodeFunction(itemName);

  // Get slot using bitmask if power-of-2, otherwise modulo
  slot = (hashTableP->mask != 0) ? (hashCode & hashTableP->mask) : (hashCode % hashTableP->arraySize);

  for (itemP = hashTableP->array[slot]; itemP != NULL; itemP = itemP->next)
  {
    // Fast path: compare hash codes first (integer comparison is faster than string comparison)
    if (likely(itemP->hashCode == hashCode))
    {
      if (hashTableP->compareFunction(itemName, itemP->data) == 0)
        return itemP->data;
    }
  }

  return NULL;
}



// -----------------------------------------------------------------------------
//
// corHashItemCustomLookup
//
void* corHashItemCustomLookup(CorHashTable* hashTableP, const char* itemName, CorHashCompareFunction compareFunction)
{
  unsigned int    hashCode;
  unsigned int    slot;
  CorHashListItem*  itemP;

  // Compute hash code once
  hashCode = hashTableP->hashCodeFunction(itemName);

  // Get slot using bitmask if power-of-2, otherwise modulo
  slot = (hashTableP->mask != 0) ? (hashCode & hashTableP->mask) : (hashCode % hashTableP->arraySize);

  for (itemP = hashTableP->array[slot]; itemP != NULL; itemP = itemP->next)
  {
    // Fast path: compare hash codes first
    if (likely(itemP->hashCode == hashCode))
    {
      if (compareFunction(itemName, itemP->data) == 0)
        return itemP->data;
    }
  }

  return NULL;
}



// -----------------------------------------------------------------------------
//
// corHashItemLookupInAllSlots - scans all slots (use only when hash code is unknown)
//
void* corHashItemLookupInAllSlots(CorHashTable* hashTableP, const char* itemName, CorHashCompareFunction compareFunction)
{
  int             slot;
  CorHashListItem*  itemP;

  for (slot = 0; slot < hashTableP->arraySize; ++slot)
  {
    for (itemP = hashTableP->array[slot]; itemP != NULL; itemP = itemP->next)
    {
      if (compareFunction(itemName, itemP->data) == 0)
        return itemP->data;
    }
  }

  return NULL;
}



// -----------------------------------------------------------------------------
//
// corHashItemRemove - remove an item from the hash table
//
// Returns:
//   0  - item removed successfully
//   -1 - item not found
//
int corHashItemRemove(CorHashTable* hashTableP, const char* itemName)
{
  unsigned int    hashCode;
  unsigned int    slot;
  CorHashListItem*  itemP;
  CorHashListItem*  prevP = NULL;

  // Compute hash code once
  hashCode = hashTableP->hashCodeFunction(itemName);

  // Get slot using bitmask if power-of-2, otherwise modulo
  slot = (hashTableP->mask != 0) ? (hashCode & hashTableP->mask) : (hashCode % hashTableP->arraySize);

  for (itemP = hashTableP->array[slot]; itemP != NULL; prevP = itemP, itemP = itemP->next)
  {
    // Fast path: compare hash codes first
    if (likely(itemP->hashCode == hashCode))
    {
      if (hashTableP->compareFunction(itemName, itemP->data) == 0)
      {
        // Found it - unlink from list
        if (prevP == NULL)
          hashTableP->array[slot] = itemP->next;
        else
          prevP->next = itemP->next;

        // Free the list node (not the data - caller owns that)
        if (hashTableP->kallocP == NULL)
          free(itemP);

        return 0;
      }
    }
  }

  return -1;
}



// -----------------------------------------------------------------------------
//
// corHashRelease - free all memory used by the hash table
//
void corHashRelease(CorHashTable* hashTableP)
{
  if (hashTableP->kallocP == NULL)
  {
    int ix;

    for (ix = 0; ix < hashTableP->arraySize; ix++)
    {
      CorHashListItem* itemP = hashTableP->array[ix];

      while (itemP != NULL)
      {
        CorHashListItem* nextP = itemP->next;  // Save next before freeing
        free(itemP);
        itemP = nextP;
      }
    }

    free(hashTableP->array);
    free(hashTableP);
  }
}
