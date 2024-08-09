/*******************************************************************************
 * Copyright (c) 2023, 2023 IBM Corp. and others
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
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "env/FrontEnd.hpp"
#include "env/IO.hpp"
#include "infra/Assert.hpp"
#include "ras/Logger.hpp"

TR::Logger::Logger()
   {
   _enabled = false;

   static char *envSkipLoggerFlushes = feGetEnv("TR_skipLoggerFlushes");
   _skipFlush = envSkipLoggerFlushes ? true : false;
   }

int32_t
TR::Logger::printf(const char *format, ...)
   {
   va_list args;
   va_start(args, format);
   int32_t length = ::vfprintf(stdout, format, args);
   va_end(args);
   return length;
   }

int32_t
TR::Logger::prints(const char *str)
   {
   fputs(str, stdout);
   return 0;
   }

int32_t
TR::Logger::printc(char c)
   {
   fputc(c, stdout);
   return 0;
   }

int32_t
TR::Logger::println()
   {
   fputc('\n', stdout);
   return 0;
   }

int32_t
TR::Logger::vprintf(const char *format, va_list args)
   {
   return ::vfprintf(stdout, format, args);
   }

/*
 * -----------------------------------------------------------------------------
 * NullLogger
 * -----------------------------------------------------------------------------
 */

TR::NullLogger *TR::NullLogger::create()
   {
   return new TR::NullLogger();
   }

/*
 * -----------------------------------------------------------------------------
 * AssertingLogger
 * -----------------------------------------------------------------------------
 */

TR::AssertingLogger *TR::AssertingLogger::create()
   {
   return new TR::AssertingLogger();
   }

int32_t
TR::AssertingLogger::printf(const char *format, ...)
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger printf");
   }

int32_t
TR::AssertingLogger::prints(const char *string)
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger prints");
   }

int32_t
TR::AssertingLogger::printc(char c)
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger printc");
   }

int32_t
TR::AssertingLogger::println()
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger println");
   }

int32_t
TR::AssertingLogger::vprintf(const char *format, va_list args)
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger vprintf");
   }

int64_t
TR::AssertingLogger::tell()
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger tell");
   }

void
TR::AssertingLogger::rewind()
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger rewind");
   }

void
TR::AssertingLogger::flush()
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger flush");
   }

void
TR::AssertingLogger::close()
   {
   TR_ASSERT_FATAL(false, "Unexpected Logger close");
   }

/*
 * -----------------------------------------------------------------------------
 * CStdIOStreamLogger
 * -----------------------------------------------------------------------------
 */
TR::CStdIOStreamLogger::CStdIOStreamLogger(::FILE *stream)
   {
   _stream = stream;
   }

TR::CStdIOStreamLogger *TR::CStdIOStreamLogger::create(::FILE *stream)
   {
   return new TR::CStdIOStreamLogger(stream);
   }

int32_t
TR::CStdIOStreamLogger::printf(const char *format, ...)
   {
   va_list args;
   va_start(args, format);
   int32_t length = ::vfprintf(_stream, format, args);
   va_end(args);
   return length;
   }

int32_t
TR::CStdIOStreamLogger::prints(const char *str)
   {
   ::fputs(str, _stream);
   return 0;
   }

int32_t
TR::CStdIOStreamLogger::printc(char c)
   {
   ::fputc(c, _stream);
   return 0;
   }

int32_t
TR::CStdIOStreamLogger::println()
   {
   ::fputc('\n', _stream);
   return 0;
   }

int32_t
TR::CStdIOStreamLogger::vprintf(const char *format, va_list args)
   {
   return ::vfprintf(_stream, format, args);
   }

int64_t
TR::CStdIOStreamLogger::tell()
   {
   return ::ftell(_stream);
   }

void
TR::CStdIOStreamLogger::flush()
   {
   if (!skipFlush())
      ::fflush(_stream);
   }

void
TR::CStdIOStreamLogger::rewind()
   {
   ::fseek(_stream, 0, SEEK_SET);
   }

void
TR::CStdIOStreamLogger::close()
   {
   // Do not close the stream as the Logger is simply a wrapper around it.
   // Just disable the Logger.
   //
   setEnabled(false);
   }

TR::CStdIOStreamLogger *TR::CStdIOStreamLogger::Stderr = TR::CStdIOStreamLogger::create(stderr);

TR::CStdIOStreamLogger *TR::CStdIOStreamLogger::Stdout = TR::CStdIOStreamLogger::create(stdout);

/*
 * -----------------------------------------------------------------------------
 * TRIOStreamLogger
 * -----------------------------------------------------------------------------
 */
TR::TRIOStreamLogger::TRIOStreamLogger(TR::FILE *stream)
   {
   _stream = stream;
   }

TR::TRIOStreamLogger *TR::TRIOStreamLogger::create(TR::FILE *stream)
   {
   return new TR::TRIOStreamLogger(stream);
   }

int32_t
TR::TRIOStreamLogger::printf(const char *format, ...)
   {
   va_list args;
   va_start(args, format);
   int32_t length = TR::IO::vfprintf(_stream, format, args);
   va_end(args);
   return length;
   }

int32_t
TR::TRIOStreamLogger::prints(const char *str)
   {
   TR::IO::fprintf(_stream, "%s", str);
   return 0;
   }

int32_t
TR::TRIOStreamLogger::printc(char c)
   {
   TR::IO::fprintf(_stream, "%c", c);
   return 0;
   }

int32_t
TR::TRIOStreamLogger::println()
   {
   TR::IO::fprintf(_stream, "\n");
   return 0;
   }

int32_t
TR::TRIOStreamLogger::vprintf(const char *format, va_list args)
   {
   return TR::IO::vfprintf(_stream, format, args);
   }

int64_t
TR::TRIOStreamLogger::tell()
   {
   return TR::IO::ftell(_stream);
   }

void
TR::TRIOStreamLogger::flush()
   {
   if (!skipFlush())
      TR::IO::fflush(_stream);
   }

void
TR::TRIOStreamLogger::rewind()
   {
   TR::IO::fseek(_stream, 0, SEEK_SET);
   }

void
TR::TRIOStreamLogger::close()
   {
   // Do not close the stream as the Logger is simply a wrapper around it.
   // Just disable the Logger.
   //
   setEnabled(false);
   }

