#ifndef _SYLAR_LOG_H
#define _SYLAR_LOG_H

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace sylar {

class Logger;

// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(const char* file, int32_t line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time);

    const char* getFile() const {
        return m_file;
    }
    int32_t getLine() const {
        return m_line;
    }
    uint32_t getElapse() const {
        return m_elapse;
    }
    uint32_t getThreadId() const {
        return m_threadId;
    }
    uint32_t getFiberId() const {
        return m_fiberId;
    }
    uint64_t getTime() const {
        return m_time;
    }
    const std::string getContent() {
        return m_ss.str();
    }
    const std::string& getThreadName() const {
        return m_threadName;
    }

private:
    const char* m_file = nullptr;  // 文件名
    int32_t     m_line = 0;        // 行号
    uint32_t    m_elapse = 0;    // 程序启动开始到现在的毫秒数
    uint32_t    m_threadId = 0;  // 线程id
    uint32_t    m_fiberId = 0;   // 协程id
    uint64_t    m_time;          // 时间戳
    std::string m_threadName;    // 线程名称
    std::stringstream m_ss;      // 日志内容流
};

// 日志级别
class LogLevel {
public:
    enum Level { DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };

    static const char* ToString(LogLevel::Level level);
};

// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);

    std::string format(std::shared_ptr<Logger> logger,
                       LogLevel::Level level, LogEvent::ptr event);

    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem() {}
        virtual void format(std::ostream&           os,
                            std::shared_ptr<Logger> logger,
                            LogLevel::Level level, LogEvent::ptr event) = 0;
    };

private:
    void                         init();
    std::string                  m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool                         m_error = false;
};

// 日志输出地
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val) {
        m_formatter = val;
    }
    LogFormatter::ptr getFormatter() const {
        return m_formatter;
    }

protected:
    LogLevel::Level   m_level;
    LogFormatter::ptr m_formatter;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
             LogEvent::ptr event) override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
             LogEvent::ptr event) override;
    // 文件打开成功 返回true
    bool reopen();

private:
    std::string   m_filename;
    std::ofstream m_filestream;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    std::string getName() const {
        return m_name;
    }
    LogLevel::Level getLevel() const {
        return m_level;
    };
    void setLevel(LogLevel::Level val) {
        m_level = val;
    }

private:
    std::string                 m_name;       // 日志名称
    LogLevel::Level             m_level;      // 日志级别
    std::list<LogAppender::ptr> m_appenders;  // appender集合
    LogFormatter::ptr           m_formatter;
};

}  // namespace sylar
#endif