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

#define trprintf(cond, log, fmt, ...) \
   do { \
      if (cond) log->printf(fmt, ##__VA_ARGS__); \
   } while (0)

#define trprints(cond, log, str) \
   do { \
      if (cond) log->prints(str); \
   } while (0)

#define trprintc(cond, log, c) \
   do { \
      if (cond) log->printc(c); \
   } while (0)


#define trprintln(cond, log) \
   do { \
      if (cond) log->println(); \
   } while (0)


namespace TR
{

class Logger
   {

public:

   Logger() :
      _enabled(false) {}

   virtual int32_t printf(const char *format, ...) = 0;

   virtual int32_t prints(const char *string) = 0;

   virtual int32_t printc(char c) = 0;

   virtual int32_t println() = 0;

   virtual int32_t vprintf(const char *format, va_list args) = 0;

   virtual int64_t tell() = 0;

   virtual void rewind() = 0;

   virtual void flush() = 0;

   virtual void close() = 0;

   virtual bool supportsRewinding() = 0;

   bool isEnabled() { return _enabled; }
   void setEnabled(bool e) { _enabled = e; }

private:

   bool _enabled;
   };


/**
 * A Logger class that simply consumes its input without outputting anything.
 * This is useful as the default logger that prevents unguarded outputs to
 * the log.
 */
class NullLogger : public Logger
   {

   NullLogger() : Logger() {}

   static NullLogger *create();

   virtual int32_t printf(const char *format, ...) { return 0; }

   virtual int32_t prints(const char *string) { return 0; }

   virtual int32_t printc(char c) { return 0; }

   virtual int32_t println() { return 0; }

   virtual int32_t vprintf(const char *format, va_list args) { return 0; }

   virtual int64_t tell() { return -1; }

   virtual void rewind() { }

   virtual void flush() { }

   virtual void close() { }

   virtual bool supportsRewinding() { return false; }

   };


/**
 * A Logger class that fatally asserts if any of the logging functions
 * is called.  This is useful for test environments to detect unguarded
 * calls to Logger functions.
 */
class AssertingLogger : public Logger
   {

   AssertingLogger() : Logger() {}

   static AssertingLogger *create();

   virtual int32_t printf(const char *format, ...);

   virtual int32_t prints(const char *string);

   virtual int32_t printc(char c);

   virtual int32_t println();

   virtual int32_t vprintf(const char *format, va_list args);

   virtual int64_t tell();

   virtual void rewind();

   virtual void flush();

   virtual void close();

   virtual bool supportsRewinding() { return false; }

   };


class StreamLogger : public Logger
   {

public:

   static StreamLogger *create(::FILE *stream);

   static StreamLogger *create(TR::FILE *stream);

   virtual int32_t printf(const char *format, ...);

   virtual int32_t prints(const char *string);

   virtual int32_t printc(char c);

   virtual int32_t println();

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

}

#endif
