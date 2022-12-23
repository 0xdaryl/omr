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
TR::Logger::vprintf(const char *format, va_list args)
   {
   return ::vfprintf(stdout, format, args);
   }

/*
 * -----------------------------------------------------------------------------
 * StreamLogger
 * -----------------------------------------------------------------------------
 */
TR::StreamLogger::StreamLogger(::FILE *fd)
   {
   _fd = fd;
   }

TR::StreamLogger *TR::StreamLogger::create(::FILE *fd)
   {
   return new TR::StreamLogger(fd);
   }

int32_t
TR::StreamLogger::printf(const char *format, ...)
   {
   va_list args;
   va_start(args, format);
   int32_t length = ::vfprintf(_fd, format, args);
   va_end(args);
   return length;
   }

int32_t
TR::StreamLogger::prints(const char *str)
   {
   ::fputs(str, _fd);
   return 0;
   }

int32_t
TR::StreamLogger::printc(char c)
   {
   ::fputc(c, _fd);
   return 0;
   }

int32_t
TR::StreamLogger::vprintf(const char *format, va_list args)
   {
   return ::vfprintf(_fd, format, args);
   }

int64_t
TR::StreamLogger::tell()
   {
   return ::ftell(_fd);
   }

void
TR::StreamLogger::flush()
   {
   ::fflush(_fd);
   }

void
TR::StreamLogger::rewind()
   {
   ::fseek(_fd, 0, SEEK_SET);
   }

void
TR::StreamLogger::close()
   {
   }

TR::StreamLogger *TR::StreamLogger::Stderr = TR::StreamLogger::create(stderr);

TR::StreamLogger *TR::StreamLogger::Stdout = TR::StreamLogger::create(stdout);

/*
 * -----------------------------------------------------------------------------
 * BufferedStreamLogger
 * -----------------------------------------------------------------------------
 */

TR::BufferedStreamLogger::BufferedStreamLogger(::FILE *fd, char *buffer,  int64_t bufferLength) :
      _bufOffset(0),
      _bufLength(bufferLength),
      _buf(buffer),
      _bufCursor(buffer),
      _fd(fd)
   {
   }

TR::BufferedStreamLogger *TR::BufferedStreamLogger::create(::FILE *fd, char *buffer, int64_t bufferLength)
   {
   return new TR::BufferedStreamLogger(fd, buffer, bufferLength);
   }


void
TR::BufferedStreamLogger::flushBuffer()
   {
   ssize_t bytesWritten = ::write(_fd, _buf, _bufOffset);

   if (bytesWritten != -1)
      {
      _bufOffset = 0;
      _bufCursor = _buf;
      }
   else
      {
      // unable to flush
      fprintf(stderr, "Unable to flush buffer : _buf=%p, _bufOffset=%d, _bufLength=%d\n", _buf, _bufOffset, _bufLength);
      }
   }


int32_t
TR::BufferedStreamLogger::prints(char *str)
   {
   size_t len = strlen(str);

   if (_bufOffset + len > _bufLength)
      {
      flushBuffer();
      }

   memcpy(_bufCursor, str, len);

   _bufCursor += len;
   _bufOffset += len;
   return 0;
   }


int32_t
TR::BufferedStreamLogger::printc(char c)
   {
   // Space for a single character?
   //
   if (_bufCursor + 1 > _bufLength)
      {
      flushBuffer();
      }

   *_bufCursor++ = c;
   _bufOffset++;
   return 0;
   }


int32_t
TR::BufferedStreamLogger::printf(const char *format, ...)
   {
   va_list args;
   va_start(args, format);
   int32_t n = vprintf(format, args);
   va_end(args);
   return 0;
   }

int32_t
TR::BufferedStreamLogger::vprintf(const char *format, va_list args)
   {
   va_list argsCopy;
   va_copy(argsCopy, args)

   int32_t n = ::vsnprintf(_bufCursor, _bufLength-_bufOffset, format, args);

   if (n >= (_bufLength-_bufOffset))
      {
      // The string was not written completely.  Flush the buffer and try again.
      //
      flushBuffer();
      n = ::vsnprintf(_bufCursor, _bufLength-_bufOffset, format, argsCopy);

      if (n >= (_bufLength-_bufOffset))
         {
         // Still could not write string to buffer.  Consider it truncated.
         //
         n = _bufLength - _bufOffset;
         }
      }

   va_copy_end(argsCopy);

   _bufCursor += n;
   _bufOffset += n;
   return 0;
   }

void
TR::BufferedStreamLogger::flush()
   {
   flushBuffer();
   ::fflush(_fd);
   }

void
TR::BufferedStreamLogger::close()
   {
   flushBuffer();
   }
