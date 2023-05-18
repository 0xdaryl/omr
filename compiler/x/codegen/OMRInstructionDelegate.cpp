/*******************************************************************************
 * Copyright IBM Corp. and others 2019
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
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include "codegen/InstructionDelegate.hpp"
#include "codegen/X86Instruction.hpp"

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86ImmInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86ImmSnippetInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86ImmSymInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86RegImmInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86RegImmSymInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86RegRegImmInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86MemImmInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86MemImmSymInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86MemRegImmInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::X86RegMemImmInstruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::AMD64RegImm64Instruction *instr, uint8_t *cursor)
   {
   }

void
OMR::X86::InstructionDelegate::createMetaDataForCodeAddress(TR::AMD64RegImm64SymInstruction *instr, uint8_t *cursor)
   {
   TR::CodeGenerator *cg = instr->cg();
   TR::Compilation *comp = cg->comp();
   TR::Node *instrNode = instr->getNode();
   TR::SymbolReference *instrSymRef = instr->getSymbolReference();

   if (instrSymRef->getSymbol()->isLabel())
      {
      // Assumes a 64-bit absolute relocation (i.e., not relative).
      //
      cg->addRelocation(new (cg->trHeapMemory()) TR::LabelAbsoluteRelocation(cursor, instrSymRef->getSymbol()->castToLabelSymbol()));

      switch (instr->getReloKind())
         {
         case TR_AbsoluteMethodAddress:
            cg->addExternalRelocation(
               TR::ExternalRelocation::create(
                  cursor,
                  NULL,
                  TR_AbsoluteMethodAddress,
                  cg),
               __FILE__,
               __LINE__,
               instrNode);
            break;

         default:
            break;
         }
      }
   else
      {
      switch (instr->getReloKind())
         {
         case TR_DataAddress:
            {
            if (cg->needRelocationsForStatics())
               {
               cg->addExternalRelocation(
                  TR::ExternalRelocation::create(
                     cursor,
                     (uint8_t *)instrSymRef,
                     (uint8_t *)instrNode ? (uint8_t *)(intptr_t) instrNode->getInlinedSiteIndex() : (uint8_t *)-1,
                     (TR_ExternalRelocationTargetKind) instr->getReloKind(),
                     cg),
                  __FILE__,
                  __LINE__,
                  instrNode);
               }

            break;
            }

         case TR_NativeMethodAbsolute:
            {
            if (comp->getOption(TR_EmitRelocatableELFFile))
               {
               TR_ResolvedMethod *target = instrSymRef->getSymbol()->castToResolvedMethodSymbol()->getResolvedMethod();
               cg->addStaticRelocation(
                  TR::StaticRelocation(
                     cursor,
                     target->externalName(cg->trMemory()),
                     TR::StaticRelocationSize::word64,
                     TR::StaticRelocationType::Absolute));
               }

            break;
            }

         case TR_DebugCounter:
            {
            if (cg->needRelocationsForStatics())
               {
               TR::DebugCounterBase *counter = comp->getCounterFromStaticAddress(instrSymRef);
               if (counter == NULL)
                  {
                  comp->failCompilation<TR::CompilationException>("Could not generate relocation for debug counter for a TR::AMD64RegImm64SymInstruction\n");
                  }

               TR::DebugCounter::generateRelocation(comp, cursor, instrNode, counter);
               }

            break;
            }

         default:
            break;
         }
      }
   }

