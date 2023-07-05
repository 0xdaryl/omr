/*******************************************************************************
 * Copyright IBM Corp. and others 2000
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#include "runtime/CodeMetaDataManager.hpp"

#include <stdint.h>
#include <string.h>
#include "avl_api.h"
#include "env/TRMemory.hpp"
#include "infra/Assert.hpp"
#include "j9nongenerated.h"
#include "runtime/CodeCache.hpp"
#include "runtime/CodeCacheMemorySegment.hpp"
#include "runtime/CodeMetaData.hpp"
#include "runtime/CodeMetaDataManager.hpp"
#include "runtime/CodeMetaDataManager_inlines.hpp"

#if !defined(TR_TARGET_POWER) || !defined(__clang__)
#include "AtomicSupport.hpp"
#endif

namespace OMR
{

TR::CodeMetaDataManager *CodeMetaDataManager::_codeMetaDataManager = NULL;


CodeMetaDataManager::CodeMetaDataManager() :
   _cachedPC(0),
   _cachedHashTable(NULL),
   _retrievedMetaDataCache(NULL)
   {
   _metaDataAVL = self()->allocateMetaDataAVL();
   }

bool
CodeMetaDataManager::create()
   {
   bool initSuccess = false;
   if (_codeMetaDataManager)
      {
      initSuccess = true;
      }
   else
      {
      //TR::Monitor *monitor = TR::Monitor::create("JIT-CodeMetaDataManagerMonitor");
      //if (monitor)
         {
         _codeMetaDataManager = new (PERSISTENT_NEW) TR::CodeMetaDataManager();
         if (_codeMetaDataManager)
            initSuccess = true;
         }
      }

   return initSuccess;
   }

// DMDM: jit_allocate_artifacts
// First allocation
//
J9AVLTree *
CodeMetaDataManager::allocateMetaDataAVL()
   {
   J9AVLTree *metaDataAVLTree =
      (J9AVLTree *) TR_Memory::jitPersistentAlloc(sizeof(J9AVLTree), TR_Memory::CodeMetaDataAVL);

   if (!metaDataAVLTree)
      return NULL;

   metaDataAVLTree->insertionComparator = (intptr_t (*)(J9AVLTree *, J9AVLTreeNode *, J9AVLTreeNode *))OMR::avl_jit_metadata_insertionCompare;
   metaDataAVLTree->searchComparator = (intptr_t (*)(J9AVLTree *, uintptr_t, J9AVLTreeNode *))OMR::avl_jit_metadata_searchCompare;
   metaDataAVLTree->genericActionHook = NULL;
   metaDataAVLTree->flags = 0;
   metaDataAVLTree->rootNode = 0;

   // Use the OMR AVL structure but TR will manage its own memory
   //
   metaDataAVLTree->portLibrary = NULL;

   return metaDataAVLTree;
   }

bool
CodeMetaDataManager::insertMetaData(TR::CodeMetaData *metaData)
   {
   TR_ASSERT(metaData, "metaData must not be NULL");
   TR_ASSERT(metaData->getCodeAllocSize() > 0, "code address range must be > 0");

   //OMR::CriticalSection insertingMetaData(_monitor);

   return self()->insertRange(metaData, metaData->getCodeAllocStart(), metaData->getCodeAllocEnd());
   }

bool
CodeMetaDataManager::containsMetaData(TR::CodeMetaData *metaData)
   {
   // OMR::CriticalSection searchingMetaData(_monitor);
   return (metaData && metaData == self()->findMetaDataForPC(metaData->getCodeAllocStart()));
   }

bool
CodeMetaDataManager::removeMetaData(TR::CodeMetaData *metaData)
   {
   TR_ASSERT(metaData, "metaData must not be NULL");
   //OMR::CriticalSection removingMetaData(_monitor);

   bool removeSuccess = false;
   if (self()->containsMetaData(metaData))
      {
      removeSuccess = self()->removeRange(metaData, metaData->getCodeAllocStart(), metaData->getCodeAllocEnd());
      }

   _retrievedMetaDataCache = NULL;

   return removeSuccess;
   }

TR::CodeMetaData *
CodeMetaDataManager::findMetaDataForPC(uintptr_t pc)
   {
   TR_ASSERT(pc != 0, "attempting to query existing MetaData for a NULL PC");

   self()->updateCache(pc);
   if (!_retrievedMetaDataCache)
      {
      if (_cachedHashTable)
         {
         _retrievedMetaDataCache = self()->findMetaDataInHash(_cachedHashTable, pc);
         }
      }

   return _retrievedMetaDataCache;
   }

// protected
bool
CodeMetaDataManager::insertRange(
      TR::CodeMetaData *metaData,
      uintptr_t startPC,
      uintptr_t endPC)
   {
   bool insertSuccess = false;
   self()->updateCache(metaData->getCodeAllocStart());
   if (_cachedHashTable)
      {
      insertSuccess = (self()->insertMetaDataRangeInHash(_cachedHashTable, metaData, startPC, endPC) == 0);
      }

   return insertSuccess;
   }

// protected
bool
CodeMetaDataManager::removeRange(
      TR::CodeMetaData *metaData,
      uintptr_t startPC,
      uintptr_t endPC)
   {
   bool removeSuccess = false;
   self()->updateCache(metaData->getCodeAllocStart());
   if (_cachedHashTable)
      {
      removeSuccess = (self()->removeMetaDataRangeFromHash(_cachedHashTable, metaData, startPC, endPC) == 0);
      }

   return removeSuccess;
   }

// protected
void
CodeMetaDataManager::updateCache(uintptr_t currentPC)
   {
   TR_ASSERT(currentPC > 0, "Attempting to find a code cache's metaData hash table for a NULL PC.");

   if (currentPC != _cachedPC)
      {
      _retrievedMetaDataCache = NULL;
      _cachedPC = currentPC;
      _cachedHashTable =
         static_cast<OMR::MetaDataHashTable *>(static_cast<void *>(avl_search(_metaDataAVL, currentPC) ) );

      TR_ASSERT(_cachedHashTable, "Either we lost a code cache or we attempted to find a hash table for a non-code cache startPC: Searched for %p", currentPC);
      }
   }

#undef LOW_BIT_SET
#undef SET_LOW_BIT
#undef REMOVE_LOW_BIT
#undef DETERMINE_BUCKET
#undef BUCKET_SIZE
#undef METHOD_STORE_SIZE

#define LOW_BIT_SET(value) ((uintptr_t) (value) & (uintptr_t) 1)
#define SET_LOW_BIT(value) ((uintptr_t) (value) | (uintptr_t) 1)
#define REMOVE_LOW_BIT(value) ((uintptr_t) (value) & (uintptr_t) -2)
#define DETERMINE_BUCKET(value, start, buckets) (((((uintptr_t)(value) - (uintptr_t)(start)) >> (uintptr_t) 9) * (uintptr_t)sizeof(uintptr_t)) + (uintptr_t)(buckets))
#define BUCKET_SIZE 512
#define METHOD_STORE_SIZE 256

// protected

TR::CodeMetaData *
CodeMetaDataManager::findMetaDataInHash(
      OMR::MetaDataHashTable *table,
      uintptr_t searchValue)
   {
   TR::CodeMetaData *entry, **bucket;

   if (searchValue >= table->start && searchValue < table->end)
      {
      // The search value is in this hash table
      //
      bucket = (TR::CodeMetaData **)DETERMINE_BUCKET(searchValue, table->start, table->buckets);

      if (*bucket)
         {
         // The bucket for this search value is not empty
         //
         if (LOW_BIT_SET(*bucket))
            {
            // The bucket consists of a single low-tagged TR::CodeMetaData pointer
            //
            entry = *bucket;
            }
         else
            {
            // The bucket consists of an array of TR::CodeMetaData pointers,
            // the last of which is low-tagged.

            // Search all but the last entry in the array
            //
            bucket = (TR::CodeMetaData **)(*bucket);
            for ( ; ; bucket++)
               {
               entry = *bucket;

               if (LOW_BIT_SET(entry))
                  break;

               if (searchValue >= entry->getCodeAllocStart() && searchValue < entry->getCodeAllocEnd())
                  return entry;
               }
            }

         // Search the last (or only) entry in the bucket, which is low-tagged
         //
         entry = (TR::CodeMetaData *)REMOVE_LOW_BIT(entry);

         if (searchValue >= entry->getCodeAllocStart() && searchValue < entry->getCodeAllocEnd())
            return entry;
         }
      }

   return NULL;
   }

// protected
// DMDM: hash_jit_artifact_insert_range
uintptr_t
CodeMetaDataManager::insertMetaDataRangeInHash(
      OMR::MetaDataHashTable *table,
      TR::CodeMetaData *dataToInsert,
      uintptr_t startPC,
      uintptr_t endPC)
   {
   TR::CodeMetaData **index;
   TR::CodeMetaData **endIndex;
   TR::CodeMetaData **temp;

   if ((startPC < table->start) || (endPC > table->end))
      {
      return 1;
      }

   // Don't insert zero sized ranges.
   if (startPC == endPC)
      {
      return 1;
      }

   index = (TR::CodeMetaData **) DETERMINE_BUCKET(startPC, table->start, table->buckets);
   endIndex = (TR::CodeMetaData **) DETERMINE_BUCKET(endPC, table->start, table->buckets);

   do
      {
      if (*index)
         {
         temp = self()->insertMetaDataArrayInHash(table, (TR::CodeMetaData**) *index, dataToInsert, startPC);
         if (!temp)
            {
            return 2;
            }

#if !defined(TR_TARGET_POWER) || !defined(__clang__)
         VM_AtomicSupport::writeBarrier();
#endif
         *index = (TR::CodeMetaData *) temp;
         }
      else
         {
#if !defined(TR_TARGET_POWER) || !defined(__clang__)
         VM_AtomicSupport::writeBarrier();
#endif
         *index = (TR::CodeMetaData *) SET_LOW_BIT(dataToInsert);
         }

      } while (++index <= endIndex);

   return 0;
   }

// protected
// DMDM: hash_jit_artifact_array_insert
TR::CodeMetaData **
CodeMetaDataManager::insertMetaDataArrayInHash(
      OMR::MetaDataHashTable *table,
      TR::CodeMetaData **array,
      TR::CodeMetaData *dataToInsert,
      uintptr_t startPC)
   {
   TR::CodeMetaData **returnVal = array;

   // If the array pointer is tagged, then it's not a chain, it's a single
   // tagged metadata.
   //
   if (LOW_BIT_SET(array))
      {
      // There is a single tagged entry in the bucket, not a chain.  In this case, we will
      // always be allocating a new chain from the current allocate of the method store.
      // We'll need 2 entries (one for the new entry and one for the existing tagged entry
      // which will also terminate the chain.

      // This comparison is safe since currentAllocate and methodStoreEnd will
      // always be pointing into the same allocated block.
      //
      if ((table->currentAllocate + 2) > table->methodStoreEnd)
         {
         if (self()->allocateMethodStoreInHash(table) == NULL)
            {
            return NULL;
            }
         }

      returnVal = (TR::CodeMetaData **) table->currentAllocate;
      table->currentAllocate += 2;
      returnVal[0] = (TR::CodeMetaData *)dataToInsert;
      returnVal[1] = (TR::CodeMetaData *)array;
      }
   else
      {
      // There is already a a chain, so we need to either add to the end of it
      // if there is free space, or copy the chain and add to the end of the copy.
      //
      TR::CodeMetaData **newElement = array;

      do
         {
         ++newElement;
         }
      while (!LOW_BIT_SET(*(newElement-1)));

      // If there is a NULL at the newElement position, then we can just add there.
      // It is safe to check *newElement even when the method store is full because
      // the method store always has an extra non-NULL entry on the end.
      //
      if (*newElement == NULL)
         {
         /** Adding to the end of an existing chain with a free slot after it.  To avoid twizzling
          * bits, copy the existing chain end forward one slot, and place the new entry in the
          * old end slot.  A write barrier must be issued before storing the new element, both
          * to ensure the metadata is fully visible before it can be seen in the array, and to
          * make the new chain end visible.
          */

         *newElement = *(newElement - 1);
#if !defined(TR_TARGET_POWER) || !defined(__clang__)
         VM_AtomicSupport::writeBarrier();
#endif
         *(newElement - 1) = dataToInsert;

         // Increase the next allocation point only if the new element is not
         // being stored into freed space.
         //
         if (newElement == (TR::CodeMetaData **) table->currentAllocate)
            {
            table->currentAllocate += 1;
            }
         }
      else
         {
         uintptr_t chainLength = newElement - array;

         /** Not enough space to add to the end of the existing chain, so it must be copied
          * and extended in free space.  There may be enough free space in the current
          * method store.  Once space is found, copy the chain into it with the new entry at
          * the beginning (to avoid twizzling tag bits).  There's no need for a write barrier
          * here since the new chain is not visible to anyone yet, and the caller of this
          * function issues a write barrier before updating the bucket pointer.
          */

         // This comparison is safe since currentAllocate and methodStoreEnd will
         // always be pointing into the same allocated block.
         //
         if ((table->currentAllocate + chainLength + 1) > table->methodStoreEnd)
            {
            if (self()->allocateMethodStoreInHash(table) == NULL)
               {
               return NULL;
               }
            }

         returnVal = (TR::CodeMetaData**) table->currentAllocate;
         table->currentAllocate += (chainLength + 1);
         returnVal[0] = dataToInsert;
         memcpy(returnVal + 1, array, chainLength * sizeof(uintptr_t));  /* safe to memcpy since the new array is not yet visible */
         }
      }

   return returnVal;
   }

// protected
// hash_jit_allocate_method_store
TR::CodeMetaData **
CodeMetaDataManager::allocateMethodStoreInHash(OMR::MetaDataHashTable *table)
   {
   // Add 1 slot for link, 1 slot for terminator
   //
   uintptr_t size = (METHOD_STORE_SIZE + 2) * sizeof(uintptr_t);
   uintptr_t *newStore =
      (uintptr_t *) TR_Memory::jitPersistentAlloc(size, TR_Memory::CodeMetaDataAVL);

   if (newStore != NULL)
      {
      memset(newStore, 0, size);

      *newStore = (uintptr_t) table->methodStoreStart;  // Link to the old store

      table->methodStoreStart = (uintptr_t *) newStore;
      table->methodStoreEnd = (uintptr_t *) (newStore + METHOD_STORE_SIZE + 1);
      table->currentAllocate = (uintptr_t *) (newStore + 1);

      // Ensure that the method store is terminated with a non-NULL entry
      //
      *(table->methodStoreEnd) = (uintptr_t) 0xBAAD076D;
      }

   return (TR::CodeMetaData **) newStore;
   }

// DMDM hash_jit_artifact_remove_range
uintptr_t
CodeMetaDataManager::removeMetaDataRangeFromHash(
      OMR::MetaDataHashTable *table,
      TR::CodeMetaData *dataToRemove,
      uintptr_t startPC,
      uintptr_t endPC)
   {
   TR::CodeMetaData **index;
   TR::CodeMetaData **endIndex;
   TR::CodeMetaData *temp;

   if ((startPC < table->start) || (endPC > table->end))
      return (uintptr_t) 1;

   index = (TR::CodeMetaData **) DETERMINE_BUCKET(startPC, table->start, table->buckets);
   endIndex = (TR::CodeMetaData **) DETERMINE_BUCKET(endPC, table->start, table->buckets);

   do
      {
      if (LOW_BIT_SET(*index))
         {
         if ((TR::CodeMetaData *)REMOVE_LOW_BIT(*index) == dataToRemove)
            *index = 0;
         else
            return (uintptr_t) 1;
         }
      else if (*index)
         {
         temp = (TR::CodeMetaData *) (self()->removeMetaDataArrayFromHash((TR::CodeMetaData**) *index, dataToRemove));
         if (!temp)
            return (uintptr_t) 1;
         else if (temp == (TR::CodeMetaData *) 1)
            return (uintptr_t) 2;
         else
            *index = temp;
         }
      else
         return (uintptr_t) 1;

      } while (++index <= endIndex);

   return (uintptr_t) 0;
   }

// DMDM: hash_jit_artifact_array_remove
TR::CodeMetaData **
CodeMetaDataManager::removeMetaDataArrayFromHash(
      TR::CodeMetaData **array,
      TR::CodeMetaData *dataToRemove)
   {
   uintptr_t count = 0;
   uintptr_t size;
   uintptr_t removeSpot = 0;
   TR::CodeMetaData **index = array;

   while (!LOW_BIT_SET(*index))                  /* search for dataToRemove in the array */
      {
      ++count;
      if (*index == dataToRemove)
         removeSpot = count;
      ++index;
      }

   if ((TR::CodeMetaData*) REMOVE_LOW_BIT(*index) == dataToRemove)
      {
      *index=0;                              /* dataToRemove is last pointer in the array. */
      --index;
      *index = (TR::CodeMetaData*)SET_LOW_BIT(*index);
      }
   else if(removeSpot)
      {
      size = (count-removeSpot+1)*sizeof(uintptr_t);      /* dataToRemove is in middle (or start) of the array. */
      memmove((void*)(array+removeSpot-1),(void*)(array+removeSpot),size);
      *index = 0;
      }
   else
      {
      return (TR::CodeMetaData**) 1;               /* We did not find dataToRemove in array */
      }

   /* tidy the case of the array having contracted to becoming a single element - it can be recycled. */
   if (LOW_BIT_SET(*array))
      {
      // NULL unused slots to allow re-use, and to simplify debugging
      //
      TR::CodeMetaData ** rc = (TR::CodeMetaData**) *array;    /* Only one pointer left.  Just return the one pointer */
      *array = NULL;
      return rc;
      }

   return array;
   }

// Call when creating a new code cache

OMR::MetaDataHashTable *
CodeMetaDataManager::addCodeCache(
      TR::CodeCache *codeCache)
   {
   TR_ASSERT(codeCache->segment(), "missing code cache segment");

   OMR::MetaDataHashTable *newTable = self()->allocateCodeMetaDataHash(
         (uintptr_t) (codeCache->segment()->segmentBase()),
         (uintptr_t) (codeCache->segment()->segmentTop()) );

   if (newTable)
      {
      avl_insert(_metaDataAVL, (J9AVLTreeNode *) newTable);
      }

   return newTable;
   }

// protected, secondary
OMR::MetaDataHashTable *
CodeMetaDataManager::allocateCodeMetaDataHash(uintptr_t start, uintptr_t end)
   {
   uintptr_t size;
   OMR::MetaDataHashTable *table =
      (OMR::MetaDataHashTable *) TR_Memory::jitPersistentAlloc(sizeof(OMR::MetaDataHashTable), TR_Memory::CodeMetaDataAVL);

   if (table == NULL)
      {
      return NULL;
      }

   memset(table, 0, sizeof(*table));
   table->start = start;
   table->end = end;

   size = DETERMINE_BUCKET(end, start, 0) + sizeof(uintptr_t);
   table->buckets = (uintptr_t *) TR_Memory::jitPersistentAlloc(size, TR_Memory::CodeMetaDataAVL);

   if (table->buckets == NULL)
      {
      TR_Memory::jitPersistentFree(table);
      return NULL;
      }

   memset(table->buckets, 0, size);

   if (self()->allocateMethodStoreInHash(table) == NULL)
      {
      TR_Memory::jitPersistentFree(table->buckets);
      TR_Memory::jitPersistentFree(table);
      return NULL;
      }

   return table;
   }

extern "C"
{

intptr_t
avl_jit_metadata_insertionCompare(J9AVLTree *tree, OMR::MetaDataHashTable *insertNode, OMR::MetaDataHashTable *walkNode)
   {
   if (walkNode->start > insertNode->start)
      {
      return 1;
      }
   else if (walkNode->start < insertNode->start)
      {
      return -1;
      }

   return 0;
   }

intptr_t
avl_jit_metadata_searchCompare(J9AVLTree *tree, uintptr_t searchValue, OMR::MetaDataHashTable *walkNode)
   {
   if (searchValue >= walkNode->end)
      return -1;

   if (searchValue < walkNode->start)
      return 1;

   return 0;
   }

} // extern "C"

}
