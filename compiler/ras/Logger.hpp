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


namespace TR
{

class Logger
   {

public:

   Logger();

   /**
    * @brief Send a `\0`-terminated string with format specifiers and arguments
    *    to the logger.  The format specifier follows C/C++ conventions.
    */
   virtual int32_t printf(const char *format, ...) = 0;

   /**
    * @brief Send a `\0`-terminated string with format specifiers and arguments
    *    in a `va_list` to the logger.  The format specifier follows C/C++
    *    conventions.
    */
   virtual int32_t vprintf(const char *format, va_list args) = 0;

   /**
    * @brief Send a raw `\0`-terminated string to the logger.
    */
   virtual int32_t prints(const char *string) = 0;

   /**
    * @brief Send a single `char` to the logger.
    */
   virtual int32_t printc(char c) = 0;

   /**
    * @brief Send a single newline to the logger.
    *    This is equivalent to `printc('\n')`.
    */
   virtual int32_t println() = 0;

   /**
    * @brief Returns the current output position indicator for this logger.
    */
   virtual int64_t tell() = 0;

   /**
    * @brief Resets the output position indicator for this logger to its
    *    earliest possible position (effectively rewinding it to the beginning).
    *    Subsequent content will overwrite previously logged content.  This is
    *    useful to implement a circular logger.  A logger must indicate
    *    support for this feature via the `supportsRewinding()` API.
    */
   virtual void rewind() = 0;

   /**
    * @brief Forces the logger to flush any accumulated content to its underlying
    *    media (for example, a file on disk, or a network socket).
    */
   virtual void flush() = 0;

   /**
    * @brief Closes the logger from accepting content.  The logger is also
    *    disabled.  Supplying content to a closed logger is undefined.
    */
   virtual void close() = 0;

   /**
    * @brief Answers whether this logger supports the ability to have its
    *    output position indicator changed to an earlier point.  This is
    *    useful for implementing circular loggers.
    */
   virtual bool supportsRewinding() = 0;

   /**
    * @brief
    *    Answers whether the logger is enabled to accept logging output.
    *    API functions within an implementing logger are not required to check
    *    whether logging is enabled when called (for efficiency reasons).
    *    Logging to a disabled logger is therefore undefined.
    *
    * @details
    *    Logging enablement is a compatibility feature that is deprecated.  It
    *    is a cleaner alternative to the current logging anti-pattern of
    *    checking for the mere presence of a log as the criteria to enable
    *    updates to the log.  Logging should be guarded with some kind of trace
    *    enablement option so the user has complete control over what goes into
    *    a log.  While most writes to a log are guarded in the current code
    *    base, for some updates that check for the mere presence of a log it is
    *    not clear what the appropriate tracing option should be.  These
    *    anti-patterns are refactored to check log enablement via the
    *    `isEnabled()` API, but should be refactored to eliminate this practice
    *    entirely in favour of a tracing option.
    */
   bool isEnabled() { return _enabled; }

   /**
    * @brief Sets the enabled status of the logger
    */
   void setEnabled(bool e) { _enabled = e; }

   /**
    * @brief
    *    Answers `true` when the underlying flushes that a Logger `flush()`
    *    function might do should be skipped. Answers `false` otherwise.
    *
    * @details
    *    There may be a performance advantage to avoiding the actual flush in
    *    some scenarios, providing the consequences of not performing an actual
    *    flush are understood.
    *
    *    This function would typically be called by `flush()` functions within
    *    Logger implementations.
    */
   bool skipFlush() { return _skipFlush; }

   /**
    * @brief Sets the skip flush status
    */
   void setSkipFlush(bool s) { _skipFlush = s; }

private:

   bool _enabled;

   bool _skipFlush;
   };


/**
 * A Logger class that simply consumes its input without outputting anything.
 * This is useful as the default logger that prevents unguarded outputs to
 * the log.
 */
class NullLogger : public Logger
   {

public:

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

private:

   NullLogger() : Logger() {}

   };


/**
 * A Logger class that fatally asserts if any of the logging functions
 * is called.  This is useful for test environments to detect unguarded
 * calls to Logger functions.
 */
class AssertingLogger : public Logger
   {

public:

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

private:

   AssertingLogger() : Logger() {}
   };


/**
 * A Logger class that implements logging using C standard IO functions.
 */
class CStdIOStreamLogger : public Logger
   {

public:

   static CStdIOStreamLogger *create(::FILE *stream);

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

   static CStdIOStreamLogger *Stderr;

   static CStdIOStreamLogger *Stdout;

private:

   CStdIOStreamLogger(::FILE *stream);

   ::FILE *_stream;

   };


/**
 * A Logger class that implements logging using TR IO functions.
 */
class TRIOStreamLogger : public Logger
   {

public:

   static TRIOStreamLogger *create(TR::FILE *stream);

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

private:

   TRIOStreamLogger(TR::FILE *stream);

   TR::FILE *_stream;

   };

}

#endif
