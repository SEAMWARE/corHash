//
// FILE            corHashTest.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>                                     // printf
#include <string.h>                                    // strcmp
#include <stdlib.h>                                    // malloc

#include "corHash/corHash.h"                           // Hash Table



// -----------------------------------------------------------------------------
//
// HashItem -
//
typedef struct HashItem
{
  char* name;
  char* relation;
} HashItem;



// -----------------------------------------------------------------------------
//
// hashCode -
//
unsigned int hashCode(const char* name)
{
  unsigned int code = 0;

  while (*name != 0)
  {
    code += (unsigned char) *name;
    ++name;
  }

  return code;
}



// -----------------------------------------------------------------------------
//
// compareFunction -
//
int compareFunction(const char* name, void* item)
{
  HashItem* itemP = (HashItem*) item;

  return strcmp(name, itemP->name);
}






// -----------------------------------------------------------------------------
//
// dumpHashTable -
//
void dumpHashTable(CorHashTable* hashTableP)
{
  int slot;
  for (slot = 0; slot < hashTableP->arraySize; slot++)
  {
    CorHashListItem* itemP;
    printf("Slot %03d:  ", slot);
    for (itemP = hashTableP->array[slot]; itemP != NULL; itemP = itemP->next)
    {
      HashItem* hiP = itemP->data;

      printf("%s -> ", hiP->name);
    }
    printf("NULL\n");
  }
}



// -----------------------------------------------------------------------------
//
// main - 
//
int main(int argC, char* argV[])
{
  CorHashTable*  hashTableP  = corHashTableCreate(NULL, hashCode, compareFunction, 8);
  const char*  names[]     = { "Ken", "Malika", "Elliott", "Maria", "Alex", "Gun", "Dan",
                               "Pia", "Jonathan", "Isabelle", "Jocke",
                               "Gunnar", "Evelina", "Hasse", "Lisa", "Stig", "Britt-Marie", "Camilla", "Kenneth", NULL };
  const char*  relations[] = { "Myself", "Wife", "Second Son", "Daughter", "First Son", "Mother", "Father",
                               "Sister", "Niece 2", "Niece 1", "Ex Brother In Lax",
                               "Grandfather", "Grandmother", "Uncle H", "Aunt L", "Uncle S", "Aunt B", "Cousin C", "Cousin K", NULL };
  int ix = 0;

  while (names[ix] != NULL)
  {
    HashItem* itemP = (HashItem*) malloc(sizeof(HashItem));

    itemP->name     = (char*) names[ix];
    itemP->relation = (char*) relations[ix];

    corHashItemAdd(hashTableP, itemP->name, itemP);
    ++ix;
  }

  if (argC < 2)
  {
    printf("Usage: %s <name>|table\n", argV[0]);
    return 1;
  }

  if (strcmp(argV[1], "table") == 0)
  {
    dumpHashTable(hashTableP);
    return 0;
  }

  HashItem* itemP = (HashItem*) corHashItemLookup(hashTableP, argV[1]);

  if (itemP == NULL)
    printf("Can't find '%s' in hash table\n", argV[1]);
  else
    printf("Found '%s', my %s\n", itemP->name, itemP->relation);

  return 0;
}
