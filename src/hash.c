/******************************************************************************
 * $Id$
 * Project: Vapor Chess
 * Purpose: Keep track of positions we've seen before.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#include "hash.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


hash_buckets *HashTable = NULL;
uint64 IndexMask = 0;
uint64 nHashEntries = 0;


/******************************************************************************
 * const void *initHash(uint64 Size);
 * PARAMETERS
 *    Size - Size in bytes of the HashTable.
 * DESCRIPTION
 *    Initial setup of the hash. HashTable must not have been previously setup
 *    and Size must be a power of two greater than or equal to
 *    sizeof(hash_buckets).
 * RETURN VALUE
 *    Returns a pointer to the hash table, or NULL if hash table creation
 *    failed, (including if the hash table already exists).
 */
const void *initHash(uint64 Size)
{
  uint64 nEntries = Size/sizeof(hash_buckets);
  uint64 Mask = nEntries-1;

  // printf("sizeof(hash_entry) = %u\n", (unsigned)sizeof(hash_entry));
  assert(sizeof(hash_entry) == 16);

  if (HashTable) {
    return NULL;
  }

  while (nEntries & (nEntries-1)) {
    nEntries = nEntries & (nEntries-1);
  }
  if (!nEntries) {
    return NULL;
  }

  HashTable = calloc(nEntries, sizeof(hash_buckets));
  if (HashTable) {
    nHashEntries = nEntries;
    IndexMask = Mask;
    memset(HashTable, 0, nEntries*sizeof(hash_buckets));
  }

  return HashTable;
}

/******************************************************************************
 * void freeHash(void);
 * DESCRIPTION
 *    Safely frees the memory allocated to the HashTable, if any.
 * RETURN VALUE
 *    Does not return a value.
 */
void freeHash(void)
{
  if (HashTable) {
    free(HashTable);
    HashTable = NULL;
    nHashEntries = 0;
    IndexMask = 0;
  }
}

/* end of file */
