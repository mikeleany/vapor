/******************************************************************************
 * $Id$
 * Project: Vapor Chess
 * Purpose: Keep track of positions we've seen before.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__HASH_H
#define VAPOR__HASH_H

#include "vapor.h"
#include "chess.h"

#include <stdlib.h>

typedef enum hash_bound {
  upperbound,
  exactscore,
  lowerbound,
} hash_bound;

typedef struct hash_entry {
  zobrist ZKey;     // 8 bytes
  uint8 Bound;      // 1 byte
  uint8 Depth;      // 1 byte
  uint16 When;      // 2 bytes
  int16 Score;      // 2 bytes
  hashmove Move;    // 2 bytes
} hash_entry;       // 16 bytes
#define NUM_BUCKETS 4
typedef hash_entry hash_buckets[NUM_BUCKETS];
extern hash_buckets *HashTable;
extern uint64 IndexMask;
extern uint64 nHashEntries;


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
const void *initHash(uint64 Size);

/******************************************************************************
 * void freeHash(void);
 * DESCRIPTION
 *    Safely frees the memory allocated to the HashTable.
 * RETURN VALUE
 *    Does not return a value.
 */
void freeHash(void);

/******************************************************************************
 * static inline const hash_entry *hashLookup(zobrist ZKey);
 * PARAMETERS
 *    ZKey - Zobrist key used to fine hash score.
 * DESCRIPTION
 *    Find the hash entry with the given Zobrist key, if any.
 * RETURN VALUE
 *    Returns a pointer to the hash entry, or NULL if no such entry exists.
 */
static inline const hash_entry *hashLookup(zobrist ZKey)
{
  const uint64 Index = (uint64) (ZKey & IndexMask);
  int Bucket;

  if (!HashTable) {
    return NULL;
  }

  for (Bucket = 0; Bucket < NUM_BUCKETS; Bucket++) {
    if (HashTable[Index][Bucket].ZKey == ZKey && HashTable[Index][Bucket].When) {
      return &HashTable[Index][Bucket];
    }
  }

  // not found
  return NULL;
}

/******************************************************************************
 * static inline const hash_entry *saveToHash(const hash_entry *HashEntry);
 * PARAMETERS
 *    HashEntry - The hash entry to store.
 * DESCRIPTION
 *    Stores and entry in the transposition table (hash).
 * RETURN VALUE
 *    Returns pointer to the saved entry in the hash. Returns NULL if the store
 *    failed.
 */
static inline const hash_entry *saveToHash(const hash_entry *HashEntry)
{
  const uint64 Index = (uint64) (HashEntry->ZKey & IndexMask);
  int CurDraft;
  int Draft;
  int Bucket = 0;

  if (!HashTable || !HashEntry) {
    return NULL;
  }

  Draft = HashTable[Index][0].Depth + HashTable[Index][0].When;
  for (int i = 1; i < NUM_BUCKETS; i++) {
    if (HashTable[Index][i].ZKey == HashEntry->ZKey) {
      Bucket = i;
      break;
    }
    CurDraft = (HashTable[Index][i].Depth + HashTable[Index][i].When);
    if (CurDraft > Draft) {
      Draft = CurDraft;
      Bucket = i;
    }
  }

  HashTable[Index][Bucket] = *HashEntry;
  return &HashTable[Index][Bucket];
}

#endif // #ifndef VAPOR__HASH_H

/* end of file */
