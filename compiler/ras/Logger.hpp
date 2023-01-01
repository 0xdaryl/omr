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

#ifndef TR_LOGGER_INCL
#define TR_LOGGER_INCL

#include <stdint.h>
#include "env/FilePointerDecl.hpp"
#include "ras/MeteredFunctions.hpp"

namespace TR
{

class Logger
   {

public:

   virtual int32_t printf(const char *format, ...);

   virtual int32_t prints(const char *string);

   virtual int32_t printc(char c);

   virtual int32_t vprintf(const char *format, va_list args);

   virtual int64_t tell() { return -1; }

   virtual void rewind() {}

   virtual void flush() {}

   virtual void close() {}

   virtual bool supportsRewinding() { return false; }

   };


class StreamLogger : public Logger
   {

public:

/*
   StreamLogger(const char *fileName);

   ~StreamLogger();
*/

   static StreamLogger *create(::FILE *stream);

   virtual int32_t printf(const char *format, ...);

   virtual int32_t prints(const char *string);

   virtual int32_t printc(char c);

   virtual int32_t vprintf(const char *format, va_list args);

   virtual int64_t tell();

   virtual void rewind();

   virtual void flush();

   virtual void close();

   virtual bool supportsRewinding() { return true; }

   static StreamLogger *Stderr;

   static StreamLogger *Stdout;

private:

   StreamLogger(::FILE *stream);

   ::FILE *_stream;

   };

#if 0
class BufferedStreamLogger : public Logger
   {

public:

   static BufferedStreamLogger *create(::FILE *stream, char *buffer, int64_t bufferLength);

   virtual int32_t printf(const char *format, ...);

   virtual int32_t prints(const char *str);

   virtual int32_t printc(char c);

   virtual int32_t vprintf(const char *format, va_list args);

   virtual int64_t tell();

   virtual void rewind();

   virtual void flush();

   virtual void close();

   virtual bool supportsRewinding() { return false; }

private:

   BufferedStreamLogger(::FILE *stream, char *buffer, int64_t bufferLength);

   void flushBuffer();

   int64_t _bufOffset;

   int64_t _bufLength;

   char *_buf;

   char *_bufCursor;

   ::FILE *_stream;

   };
#endif

}

#endif
