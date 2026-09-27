# corHash — Hash Tables

A lightweight, high-performance hash table implementation in C with support for custom memory allocators.

- **Version:** 0.1.0
- **Language:** C
- **License:** [Apache License 2.0](LICENSE)

The only dependency is **corAlloc**.

## Where it comes from

corHash is **khash** under the cor prefix, copied, not forked: khash itself is
untouched and keeps serving its own users. `KHashTable` → `CorHashTable`,
`khashItemAdd` → `corHashItemAdd`, and so on for every name; the code is unchanged.

## Features

- **Fast lookups** - O(1) average case with hash code caching to minimize comparisons
- **Power-of-2 optimization** - Uses bitmask instead of modulo when table size is a power of 2
- **Custom allocator support** - Integrates with corAlloc for memory pool allocation
- **User-defined hash and compare functions** - Full control over hashing behavior
- **Minimal dependencies** - Only requires corAlloc library

## API Reference

### Types

```c
// Hash code function - calculates hash from a key name
typedef unsigned int (*CorHashCodeFunction)(const char* name);

// Compare function - returns 0 if name matches the item
typedef int (*CorHashCompareFunction)(const char* name, void* itemP);

// Hash table structure
typedef struct CorHashTable CorHashTable;
```

### Functions

#### corHashTableCreate

```c
CorHashTable* corHashTableCreate(
    CorAlloc*               kaP,              // Memory allocator (NULL for malloc)
    CorHashCodeFunction     hashFunction,     // User hash function
    CorHashCompareFunction  compareFunction,  // User compare function
    int                     slots             // Number of slots (use power of 2 for best performance)
);
```

Creates a new hash table. Returns NULL on allocation failure.

**Performance tip**: Use a power-of-2 value for `slots` (e.g., 64, 128, 256) to enable fast bitmask-based slot calculation instead of modulo.

#### corHashItemAdd

```c
int corHashItemAdd(CorHashTable* hashTableP, const char* itemName, void* itemData);
```

Adds an item to the hash table. Returns 0 on success, -1 on allocation failure.

#### corHashItemLookup

```c
void* corHashItemLookup(CorHashTable* hashTableP, const char* itemName);
```

Looks up an item by name. Returns the item data pointer, or NULL if not found.

#### corHashItemRemove

```c
int corHashItemRemove(CorHashTable* hashTableP, const char* itemName);
```

Removes an item from the hash table. Returns 0 on success, -1 if not found. Note: The item's data is not freed - the caller owns that memory.

#### corHashItemCustomLookup

```c
void* corHashItemCustomLookup(
    CorHashTable*           hashTableP,
    const char*             itemName,
    CorHashCompareFunction  compareFunction
);
```

Lookup using a custom compare function (different from the table's default).

#### corHashItemLookupInAllSlots

```c
void* corHashItemLookupInAllSlots(
    CorHashTable*           hashTableP,
    const char*             itemName,
    CorHashCompareFunction  compareFunction
);
```

Scans all slots looking for a match. Use only when the hash code is unknown or unreliable. This is O(n) and should be avoided in performance-critical paths.

#### corHashRelease

```c
void corHashRelease(CorHashTable* hashTableP);
```

Frees all memory used by the hash table (when using malloc, not corAlloc). Note: Item data is not freed - only the hash table structures.

## Performance Optimizations

The library includes several optimizations for speed:

1. **Hash code caching** - Each item stores its hash code, allowing fast integer comparison before calling the (potentially expensive) compare function
2. **Power-of-2 bitmask** - When table size is a power of 2, uses `hash & mask` instead of `hash % size`
3. **Branch prediction hints** - Uses `__builtin_expect` to optimize common paths
4. **Head insertion** - New items are inserted at the head of collision chains (O(1) insert)

## Building

```bash
make          # the library, and obj/debug/corHashTest - a smoke test, never installed
make clean    # remove build artifacts
```

## Usage Example

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "corHash/corHash.h"

typedef struct {
    char* name;
    int   value;
} MyItem;

unsigned int myHashCode(const char* name)
{
    unsigned int hash = 5381;
    while (*name)
        hash = ((hash << 5) + hash) + *name++;
    return hash;
}

int myCompare(const char* name, void* itemP)
{
    return strcmp(name, ((MyItem*)itemP)->name);
}

int main(void)
{
    // Use 64 slots (power of 2 for best performance)
    CorHashTable* ht = corHashTableCreate(NULL, myHashCode, myCompare, 64);

    // Add items
    MyItem* item = malloc(sizeof(MyItem));
    item->name = "key1";
    item->value = 42;
    corHashItemAdd(ht, item->name, item);

    // Lookup
    MyItem* found = corHashItemLookup(ht, "key1");
    if (found)
        printf("Found: %s = %d\n", found->name, found->value);

    // Remove
    corHashItemRemove(ht, "key1");
    free(item);  // Caller must free item data

    // Cleanup
    corHashRelease(ht);
    return 0;
}
```

## Dependencies

- [corAlloc](https://github.com/SEAMWARE/corAlloc) - Memory pool allocator (optional, can use malloc instead)

## License

[Apache 2.0](LICENSE) &copy; 2019-2026 Ken Zangelin
