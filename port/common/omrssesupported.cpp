/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2017, 2017
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <stdint.h>

#if defined(WIN32)
#include <intrin.h>
#include <xmmintrin.h>
#include <excpt.h>
#elif defined(LINUX) || defined(OSX)
#include <xmmintrin.h>
#include <signal.h>
#include <setjmp.h>
#endif

#include "omrcomp.h"

#if defined(LINUX) || defined(OSX)

sigjmp_buf sseEnv;

/**
 * \fn static void handleSIGILLForSTMXCSR(int signal)
 *
 * \brief The Linux signal handler guarding STMXCSR instruction.  If called, the
 * instruction caused a #UD which indicates that SSE extensions are not supported
 * on this operating system.
 *
 * \param signal The signal for which this handler was invoked
 *
 */
static void
handleSIGILLForSTMXCSR(int signal) {
   siglongjmp(sseEnv, 1);
}
#endif


/**
 * \fn BOOLEAN targetOSSupportsSSEExtensions(BOOLEAN testFail)
 *
 * \brief Determine whether the target OS supports SSE extensions on an x86 processor.
 * Support for SSE extensions implies support for SSE, SSE2, SSE3, SSSE3, and SSE4.
 *
 * An OS that supports SSE extensions must provide support for saving and restoring
 * SSE state using the FXSAVE and FXRSTOR instructions.  An OS indicates this support
 * by setting CR4.OSFXSR[bit 9] = 1.  It must also disable x87 emulation by setting
 * CR0.EM[bit 2] = 0, and the CPUID instruction must report support for at least SSE.
 *
 * A user-space process can test for these by executing the STMXCSR instruction
 * to store the contents of the MXCSR register to a 32-bit memory location.  A
 * processor will raise a #UD in all modes if any of the above requirements do not
 * hold.
 *
 * Furthermore, an OS is required to provide an exception handler for unmasked SIMD
 * floating point exceptions.  An OS indicates this support by setting
 * CR4.OSXMMEXCPT[bit 10] = 1.  However, there is not a straightforward way of
 * determining this support in user-space other than executing instructions to
 * generate individual unmasked SIMD exceptions and testing for #UD.
 *
 * The code below will only confirm SSE extension support if all SIMD exceptions
 * are masked.  If any SIMD exceptions are unmasked then SSE extensions will be
 * considered unsupported.  This restriction should be sufficient for all known
 * operating systems.
 *
 * On Linux and OSX this implementation is NOT thread safe because the sseEnv
 * sigjmp_buf cannot be shared simultaneously among multiple threads.
 *
 * \param testFail When TRUE, force the function to fail by issuing a #UD as the
 * STMXCSR instruction would if OS support for SSE extensions was not configured.
 * Useful for testing.
 *
 * \return TRUE if the OS supports SSE extensions; FALSE otherwise.
 *
 */

BOOLEAN
targetOSSupportsSSEExtensions(BOOLEAN testFail = FALSE) {

   typedef enum {
      OS_SSE_Unknown,
      OS_SSE_Supported,
      OS_SSE_NotSupported
   } SSESupportStates;

   SSESupportStates osSupportsSSE = OS_SSE_Unknown;

#if defined(WIN32)
   __try {

      if (testFail) {
         /* Force a #UD exception */
         __ud2();
      }

      uint32_t mxcsr = _mm_getcsr();

      /*
       * Check that all SIMD exceptions are masked.  If not, SSE extension
       * support is considered incomplete by this implementation.
       */
      if ((mxcsr & _MM_MASK_MASK) == _MM_MASK_MASK) {
         osSupportsSSE = OS_SSE_Supported;
      } else {
         osSupportsSSE = OS_SSE_NotSupported;
      }
   }
   __except(EXCEPTION_EXECUTE_HANDLER) {
      osSupportsSSE = OS_SSE_NotSupported;
   }

#elif defined(LINUX) || defined(OSX)

   /**
    * Install a SIGILL signal handler to catch any #UD from the STMXCSR instruction.
    */
   struct sigaction origAction;

   struct sigaction stmxcsrAction = {{0}};
   stmxcsrAction.sa_handler = handleSIGILLForSTMXCSR;
   stmxcsrAction.sa_flags = 0;
   sigemptyset(&stmxcsrAction.sa_mask);

   sigaction(SIGILL, &stmxcsrAction, &origAction);

   if (sigsetjmp(sseEnv, 1)) {
      osSupportsSSE = OS_SSE_NotSupported;
   } else {
      if (testFail) {
         /**
          * Force a #UD exception.  Emit the instruction rather than explicitly
          * raising a SIGILL to better mimic the behaviour of the STMXCSR if it
          * fails.
          */
         __asm__ volatile ("ud2");
      }

      uint32_t mxcsr = _mm_getcsr();

      /*
       * Check that all SIMD exceptions are masked.  If not, SSE extension
       * support is considered incomplete by this implementation.
       */
      if ((mxcsr & _MM_MASK_MASK) == _MM_MASK_MASK) {
         osSupportsSSE = OS_SSE_Supported;
      } else {
         osSupportsSSE = OS_SSE_NotSupported;
      }
   }

   sigaction(SIGILL, &origAction, NULL);

#endif

   return (osSupportsSSE == OS_SSE_Supported) ? TRUE : FALSE;
}


#include <stdio.h>
int
main() {

  BOOLEAN b;

  for (int i=0; i<1024; i++) {
     b = targetOSSupportsSSEExtensions();
     printf("sse=%d\n", b);
     b = targetOSSupportsSSEExtensions(TRUE);
     printf("sse=%d\n", b);
     b = targetOSSupportsSSEExtensions(TRUE);
     printf("sse=%d\n", b);
  }

  return 0;
}
