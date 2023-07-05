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

#ifndef OMR_CODE_METADATA_MANAGER_INCL
#define OMR_CODE_METADATA_MANAGER_INCL

/*
 * The following #define(s) and typedef(s) must appear before any #includes in this file
 */
#ifndef OMR_CODE_METADATA_MANAGER_CONNECTOR
#define OMR_CODE_METADATA_MANAGER_CONNECTOR
namespace OMR { class CodeMetaDataManager; }
namespace OMR { typedef OMR::CodeMetaDataManager CodeMetaDataManagerConnector; }
#endif

#include <stdint.h>
#include "env/TRMemory.hpp"
#include "infra/Annotations.hpp"
#include "j9nongenerated.h"

namespace OMR { typedef ::J9JITHashTable MetaDataHashTable; }
namespace TR { class CodeCache; }
namespace TR { class CodeMetaDataManager; }
namespace TR { class CodeMetaData; }

namespace OMR
{

/**
 * Manages metadata about code produced by the compiler.
 *
 * This class is built around a singleton, that can be initialized and verified
 * by calling \c create().
 *
 * The class assumes that each code cache is registered with the
 * \c TR::CodeMetaDataManager using \c addCodeCache().
 *
 * The \c TR::CodeMetaDataManager only manages pointers.  It takes no ownership
 * of the \c TR::CodeMetaData pointers provided to it.
 */
class OMR_EXTENSIBLE CodeMetaDataManager
   {

public:

   TR_PERSISTENT_ALLOC(TR_Memory::CodeMetaData);

   inline TR::CodeMetaDataManager *self();

   static TR::CodeMetaDataManager *codeMetaDataManager() { return _codeMetaDataManager; }

   /**
    * @brief Factor method to create singleton \c TR::CodeMetaDataManager
    *
    * @return true if a new \c TR::CodeMetaDataManager was created; false otherwise
    */
   static bool create();

   /**
    * @brief
    *    For a given method's \c TR::CodeMetaData, finds the appropriate
    *    hashtable in the AVL tree and inserts the new meta data.
    *
    * @note
    *    This function does not check to verify that a metadata's given range
    *    is not already occupied by an existing metadata.  This is because metadata
    *    ranges represent the virtual memory actually occupied by compiled code.
    *    Thus, if two metadata ranges overlap, then two pieces of compiled code
    *    co-exist on top of each other, which is inherently incorrect.  If the
    *    possiblity occurs that two metadata may legitimately have range values that
    *    overlap, checks will need to be added.
    *
    * @note
    *    It is important that the range for the metadata be > 0.  Inserting a zero
    *    width range will fail because lookups will fail.
    *
    * @param[in] metaData : the \c TR::CodeMetaData to insert
    *
    * @return Returns true if successful; false otherwise
    */
   bool insertMetaData(TR::CodeMetaData *metaData);

   /**
    * @brief
    *    Determines if the metadata manager contains a reference to a particular
    *    CodeMetaData.
    *
    * @note
    *    Just because the metadata manager returns a metadata for a particular
    *    startPC does not mean that it is the same metadata being searched for.
    *    The method could have been recompiled, and then subsequently unloaded and
    *    another method compiled to the same location before the recompilation-based
    *    reclamation could occur.  This will result in the metadata manager containing
    *    a different CodeMetaData for a given startPC.
    *
    * @param[in] metadata: the metadata to search for
    *
    * @return true if the same metadata is found for the metadata's startPC; false otherwise
    */
   bool containsMetaData(TR::CodeMetaData *metaData);

   /**
    * @brief Remove the given metadata from the metadata manager if it is found therein.
    *
    * @param[in] metaData : the \c TR::CodeMetaData for the code to remove from
    *    the metadata.
    *
    * @return true if the metadata is successfully removed; false otherwise
    */
   bool removeMetaData(TR::CodeMetaData *metaData);

   /**
    * @brief
    *    Attempts to find the metadata associated with a given code address
    *
    * @note
    *    \c findMetaDataForPC() does not locally acquire the JIT metadata monitor.
    *    Users must first manually acquire the monitor or go through another function
    *    that does.
    *
    * @param[in] pc : the PC for which we require the metadata
    *
    * @return the \c TR::CodeMetaData if found; NULL otherwise
    */
   TR::CodeMetaData *findMetaDataForPC(uintptr_t pc);

   /**
    * @brief
    *    Register the given code cache with the metadata manager.
    *    Whenever a new codecache is allocated, register it with this metadata
    *    manager so that subsequent insertion and query is aware of the contents.
    *
    * @param[in] codeCache : the \c TR::CodeCache to register
    *
    * @return the \c OMR::MetaDataHashTable created for the new code cache
    */
   OMR::MetaDataHashTable *addCodeCache(TR::CodeCache *codeCache);

   protected:

   /**
    * @brief Initializes the translation metadata manager's members.
    *
    * The non-cache members are pointers to types instantiated externally.  The
    * constructor simply copies the pointers into the object's private members.
    */
   CodeMetaDataManager();

   /**
    * @brief Inserts a metadata into the metadata manager causing it to represent a
    * given memory range.
    *
    * @note
    *    This method expects to be called via another method in the metadata
    *    manager and thus does not acquire the metadata manager's monitor.
    *
    * @param[in] metadata : the \c TR::CodeMetaData which will represent the given
    *    memory range.
    * @param[in] startPC : the beginning of the memory range to represent
    * @param[in] endPC : the end of the memory range to represent
    *
    * @return true if the metadata was successfully inserted; false otherwise
    */
   bool insertRange(TR::CodeMetaData *metaData, uintptr_t startPC, uintptr_t endPC);

   /**
    * @brief Removes a metadata from the metadata manager
    *
    * @note
    *    This method expects to be called via another method in the metadata
    *    manager and thus does not acquire the metadata manager's monitor.
    *
    * @param[in] metadata : the \c TR::CodeMetaData to remove
    * @param[in] startPC : the beginning of the memory range the metadata represents
    * @param[in] endPC : the end of the memory range the metadata represents
    *
    * @return true if the metadata was successfully removed; false otherwise
    */
   bool removeRange(TR::CodeMetaData *metaData, uintptr_t startPC, uintptr_t endPC);

   /**
    * @brief
    *    Determines if the current metadataManager query is using the same
    *    metadata as the previous query, and if not, searches for and retrieves the
    *    new metadata's code cache's hash table.
    *
    * @note
    *    This method expects to be called via another method in the metadata
    *    manager and thus does not acquire the metadata manager's monitor.
    *
    * @param[in] currentPC : the PC we are currently inquiring about
    */
   void updateCache(uintptr_t currentPC);

   TR::CodeMetaData *findMetaDataInHash(
      OMR::MetaDataHashTable *table,
      uintptr_t searchValue);

   uintptr_t insertMetaDataRangeInHash(
      OMR::MetaDataHashTable *table,
      TR::CodeMetaData *dataToInsert,
      uintptr_t startPC,
      uintptr_t endPC);

   TR::CodeMetaData **insertMetaDataArrayInHash(
      OMR::MetaDataHashTable *table,
      TR::CodeMetaData **array,
      TR::CodeMetaData *dataToInsert,
      uintptr_t startPC);

   TR::CodeMetaData **allocateMethodStoreInHash(OMR::MetaDataHashTable *table);

   uintptr_t removeMetaDataRangeFromHash(
      OMR::MetaDataHashTable *table,
      TR::CodeMetaData *dataToRemove,
      uintptr_t startPC,
      uintptr_t endPC);

   TR::CodeMetaData **removeMetaDataArrayFromHash(
      TR::CodeMetaData **array,
      TR::CodeMetaData *dataToRemove);

   OMR::MetaDataHashTable *allocateCodeMetaDataHash(
      uintptr_t start,
      uintptr_t end);

   J9AVLTree *allocateMetaDataAVL();

   // Singleton: Protected to allow manipulation of singleton pointer
   // in test cases.
   static TR::CodeMetaDataManager *_codeMetaDataManager;

   J9AVLTree *_metaDataAVL;

   private:

   mutable uintptr_t _cachedPC;
   mutable OMR::MetaDataHashTable *_cachedHashTable;
   mutable TR::CodeMetaData *_retrievedMetaDataCache;

   };


extern "C"
{
intptr_t avl_jit_metadata_insertionCompare(J9AVLTree *tree, OMR::MetaDataHashTable *insertNode, OMR::MetaDataHashTable *walkNode);

intptr_t avl_jit_metadata_searchCompare(J9AVLTree *tree, uintptr_t searchValue, OMR::MetaDataHashTable *walkNode);
}

}

#endif
