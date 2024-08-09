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
#include "env/IO.hpp"
#include "infra/Assert.hpp"
#include "ras/Logger.hpp"


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
 * StreamLogger
 * -----------------------------------------------------------------------------
 */
TR::StreamLogger::StreamLogger(::FILE *stream)
   {
   _stream = stream;
   }

TR::StreamLogger *TR::StreamLogger::create(::FILE *stream)
   {
   return new TR::StreamLogger(stream);
   }

TR::StreamLogger *TR::StreamLogger::create(TR::FILE *stream)
   {
   return new TR::StreamLogger(reinterpret_cast<::FILE *>(stream));
   }

int32_t
TR::StreamLogger::printf(const char *format, ...)
   {
   va_list args;
   va_start(args, format);
   int32_t length = ::vfprintf(_stream, format, args);
   va_end(args);
   return length;
   }

int32_t
TR::StreamLogger::prints(const char *str)
   {
   ::fputs(str, _stream);
   return 0;
   }

int32_t
TR::StreamLogger::printc(char c)
   {
   ::fputc(c, _stream);
   return 0;
   }

int32_t
TR::StreamLogger::println()
   {
   ::fputc('\n', _stream);
   return 0;
   }

int32_t
TR::StreamLogger::vprintf(const char *format, va_list args)
   {
   return ::vfprintf(_stream, format, args);
   }

int64_t
TR::StreamLogger::tell()
   {
   return ::ftell(_stream);
   }

void
TR::StreamLogger::flush()
   {
   ::fflush(_stream);
   }

void
TR::StreamLogger::rewind()
   {
   ::fseek(_stream, 0, SEEK_SET);
   }

void
TR::StreamLogger::close()
   {
   }

TR::StreamLogger *TR::StreamLogger::Stderr = TR::StreamLogger::create(stderr);

TR::StreamLogger *TR::StreamLogger::Stdout = TR::StreamLogger::create(stdout);

