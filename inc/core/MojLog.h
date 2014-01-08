/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */


#ifndef MOJLOG_H_
#define MOJLOG_H_

#include <stdio.h>
#include <glib.h>
#include "core/MojListEntry.h"

#ifdef MOJ_HAVE_STDARG_H
#	include <stdarg.h>
#endif

#if !defined(USE_PMLOG)
#if defined(MOJ_DEBUG) || defined(MOJ_DEBUG_LOGGING)
#	define MojLogDebug(LOGGER, ...)		(LOGGER).log(MojLogger::LevelDebug, __VA_ARGS__)
#	define MojLogTrace(LOGGER)			MojLogTracer __MOJTRACER(LOGGER, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#else
#	define MojLogDebug(LOGGER, ...)
#	define MojLogTrace(LOGGER)
#endif

#define MojLogInfo(LOGGER, ...) 		(LOGGER).log(MojLogger::LevelInfo, __VA_ARGS__)
#define MojLogNotice(LOGGER, ...)		(LOGGER).log(MojLogger::LevelNotice, __VA_ARGS__)
#define MojLogWarning(LOGGER, ...) 		(LOGGER).log(MojLogger::LevelWarning, __VA_ARGS__)
#define MojLogError(LOGGER, ...) 		(LOGGER).log(MojLogger::LevelError, __VA_ARGS__)
#define MojLogCritical(LOGGER, ...) 	(LOGGER).log(MojLogger::LevelCritical, __VA_ARGS__)

#else  // USE_PMLOG

  #include "PmLogLib.h"
  #define MOJLOG_MESSAGE_MAX 500
  #define MOJLOG_UNIQUE_MAX 30

  inline void mojLogFmtMsg(char *logMsg, char *fmt, ...)
  {
    va_list args;
    va_start (args, fmt);
    vsnprintf(logMsg, MOJLOG_MESSAGE_MAX, fmt, args);
    va_end (args);
  }

  #if defined(MOJ_DEBUG) || defined(MOJ_DEBUG_LOGGING)
    #define MojLogDebug(LOGGER, ...)        { if (G_UNLIKELY((LOGGER).level() <= (LOGGER).LevelDebug)) { \
        char logMsg[MOJLOG_MESSAGE_MAX+1]; \
        mojLogFmtMsg(logMsg, __VA_ARGS__); \
        PmLogDebug((LOGGER).getContext(), "%s", logMsg);}}

    #define MojLogTrace(LOGGER)            MojLogTracer __MOJTRACER(LOGGER, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
  #else
    #define MojLogDebug(LOGGER, ...)
    #define MojLogTrace(LOGGER)
  #endif

  #define MojLogInfo(LOGGER, ...)    { if (G_UNLIKELY((LOGGER).level() <= (LOGGER).LevelInfo)) {\
        char msgId[MOJLOG_UNIQUE_MAX+1]; char logMsg[MOJLOG_MESSAGE_MAX+1]; \
        mojLogFmtMsg(logMsg, __VA_ARGS__); \
        PmLogInfo((LOGGER).getContext(), (LOGGER).fmtUnique(msgId, __FILE__, __LINE__), \
            1, PMLOGKS("FUNC", __func__), "%s", logMsg);}}

  #define MojLogNotice(LOGGER, ...)   { if ((LOGGER).level() <= (LOGGER).LevelNotice) {\
        char msgId[MOJLOG_UNIQUE_MAX+1]; char logMsg[MOJLOG_MESSAGE_MAX+1]; \
        mojLogFmtMsg(logMsg, __VA_ARGS__); \
        PmLogInfo((LOGGER).getContext(), (LOGGER).fmtUnique(msgId, __FILE__, __LINE__), \
            1, PMLOGKS("FUNC", __func__), "%s", logMsg);}}

  #define MojLogWarning(LOGGER, ...) { if ((LOGGER).level() <= (LOGGER).LevelWarning) {\
        char msgId[MOJLOG_UNIQUE_MAX+1]; char logMsg[MOJLOG_MESSAGE_MAX+1]; \
        mojLogFmtMsg(logMsg, __VA_ARGS__); \
        PmLogWarning((LOGGER).getContext(), (LOGGER).fmtUnique(msgId, __FILE__, __LINE__), \
            1, PMLOGKS("FUNC", __func__), "%s", logMsg);}}

  #define MojLogError(LOGGER, ...)    { \
        char msgId[MOJLOG_UNIQUE_MAX+1]; char logMsg[MOJLOG_MESSAGE_MAX+1]; \
        mojLogFmtMsg(logMsg, __VA_ARGS__); \
        PmLogError((LOGGER).getContext(), (LOGGER).fmtUnique(msgId, __FILE__, __LINE__), \
            1, PMLOGKS("FUNC", __func__), "%s", logMsg);}

  #define MojLogCritical(LOGGER, ...) { \
        char msgId[MOJLOG_UNIQUE_MAX+1]; char logMsg[MOJLOG_MESSAGE_MAX+1]; \
        mojLogFmtMsg(logMsg, __VA_ARGS__); \
        PmLogCritical((LOGGER).getContext(), (LOGGER).fmtUnique(msgId, __FILE__, __LINE__), \
            1, PMLOGKS("FUNC", __func__), "%s", logMsg);}


#endif // USE_PMLOG

class MojLogger : private MojNoCopy
{
public:
	enum Level {
		LevelTrace,
		LevelDebug,
		LevelInfo,
		LevelNotice,
		LevelWarning,
		LevelError,
		LevelCritical,
		LevelNone,
		LevelDefault = LevelNotice,
		LevelMax = LevelNone
	};

	MojLogger(const MojChar* name, MojLogEngine* engine = NULL);
	~MojLogger();

	Level level() const { return m_level; }
	const MojChar* name() const { return m_name; }

	void level(Level level) { m_level = level; }
	void log(Level level, const MojChar* format, ...) MOJ_FORMAT_ATTR((printf, 3, 4));
	void vlog(Level level, const MojChar* format, va_list args) MOJ_FORMAT_ATTR((printf, 3, 0));

	static const MojChar* stringFromLevel(Level level);
	static MojErr levelFromString(const MojChar* str, Level& levelOut);

	void *data() const { return m_data; }
	void setData(void *data) { m_data = data; }
#if defined(USE_PMLOG)
    PmLogContext getContext()
    {
        return m_context;
    }
    char * fmtUnique(char *dest, const char *pFile, int32_t lineNbr);
#endif // USE_PMLOG

private:
	friend class MojLogEngine;

#if defined(USE_PMLOG)
	PmLogContext m_context;
#endif // USE_PMLOG
	MojListEntry m_entry;
	MojLogEngine* m_engine;
	const MojChar* m_name;
	Level m_level;
	void *m_data;

	static const MojChar* const s_levelNames[];
};

class MojLogTracer : public MojNoCopy
{
public:
	MojLogTracer(MojLogger& logger, const MojChar* function, const MojChar* file, int line);
	~MojLogTracer();

private:
	static const int IndentSpaces = 2;
	int indentLevel(int inc);

	MojLogger& m_logger;
	const MojChar* m_function;
	int m_level;
};

#endif /* MOJLOG_H_ */
