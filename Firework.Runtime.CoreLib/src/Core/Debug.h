#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <chrono>
#include <codecvt>
#if __linux__
#include <csignal>
#endif
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

#include <Components/EntityAttributes.h>
#include <Core/Application.h>
#include <EntityComponentSystem/Entity.h>
#include <Firework/Config.h>

namespace Firework::Internal
{
    // This is a bit of a hack.
    inline static std::wostream& operator<<(std::wostream& stream, const std::string& rhs)
    {
        _push_nowarn_gcc(_clWarn_gcc_deprecated);
        _push_nowarn_clang(_clWarn_clang_deprecated);
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
        _pop_nowarn_clang();
        _pop_nowarn_gcc();
        stream << conv.from_bytes(rhs);
        return stream;
    }
    inline static std::wostream& operator<<(std::wostream& stream, const std::string_view& rhs)
    {
        for (auto it = rhs.begin(); it != rhs.end(); ++it) stream << *it;
        return stream;
    }
} // namespace Firework::Internal

namespace Firework
{

    /// @internal
    /// @brief Internal API. Special escape sequences.
    enum class EscapeSequence : uint_fast8_t
    {
        Reset
    };
    /// @brief Specifies the severity level of a log message
    /// @see ```Firework::Debug::log```
    enum class LogLevel : uint_fast8_t
    {
        Trace,
        Info,
        Warn,
        Error
    };

    /// @brief Static class containing functionality relevant to debugging and logging.
    class _fw_core_api Debug final
    {
        /// @internal
        /// @brief Internal API. Concatenates args... into a string.
        /// @tparam ...T
        /// @param[out] str Output wostringstream.
        /// @param ...args Values to concatenate.
        /// @note Thread-safe, given any access to ```str``` across the duration of this function is thread-safe.
        template <typename... T>
        static void variadicToString(std::wostringstream& str, const T&... args)
        {
            using namespace Firework::Internal;
            using expander = int[];
            (void)expander { 0, (void(str << args), 0)... };
        }
        /// @internal
        /// @brief Internal API. Returns the ANSI escape sequence for a certain rgb color.
        /// @param r Red, from 0 - 255.
        /// @param g Green, from 0 - 255.
        /// @param b Blue, from 0 - 255.
        /// @return Escape sequence wstring.
        /// @note Thread-safe.
        inline static std::wstring escapeCodeFromColor(uint8_t r, uint8_t g, uint8_t b)
        {
            std::wostringstream out;
            out << L"\x1b[38;2;" << r << L';' << g << L';' << b << L'm';
            return std::move(out).str();
        }
        /// @internal
        /// @brief Internal API. Returns the ANSI escape sequence for a color code.
        /// @param code Color code value.
        /// @return Escape sequence wstring.
        /// @note Thread-safe.
        inline static std::wstring escapeCodeFromColor(uint8_t code)
        {
            std::wostringstream out;
            out << L"\x1b[38;5;" << code << L'm';
            return std::move(out).str();
        }
        /// @internal
        /// @brief Internal API. Returns the ANSI escape sequence for a special type of escape code.
        /// @param seq Escape code type.
        /// @return Escape sequence wstring.
        /// @note Thread-safe.
        constexpr static std::wstring escapeCode(EscapeSequence seq)
        {
            switch (seq)
            {
            case EscapeSequence::Reset:
                return L"\x1b[0m";
            }
            return L"";
        }
    public:
        Debug() = delete;

        /// @brief Log a message to the console.
        /// @warning Whilst Debug::log is thread-safe, it is synchronized by spinlock, so this will be slow if you run it in parallel!
        /// @tparam ...T
        /// @param severity Severity level of message.
        /// @param ...log Message to concatenate together.
        /// @see Firework::LogLevel
        /// @note Thread-safe.
        template <typename... T>
        static void log(LogLevel severity, const T&... log)
        {
            std::wostringstream logStr;
            variadicToString(logStr, log...);
            std::wstring logText = std::move(logStr).str();

            std::wstring beginLogLevel = [&]() -> std::wstring
            {
                switch (severity)
                {
                case LogLevel::Trace:
                    return L"";
                case LogLevel::Info:
                    return Debug::escapeCodeFromColor(10);
                case LogLevel::Warn:
                    return Debug::escapeCodeFromColor(255, 255, 0);
                case LogLevel::Error:
                    return Debug::escapeCodeFromColor(9);
                }
                std::unreachable();
            }();
            std::wstring endLogLevel = (severity == LogLevel::Trace ? L"" : Debug::escapeCode(EscapeSequence::Reset));

            bool hasNewline = logText.contains('\n');
            time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            std::wstringstream out;

            out << Debug::escapeCodeFromColor(12) << L"[UTC: " << std::put_time(std::gmtime(&t), L"%Y/%m/%d %X") << L"]" << Debug::escapeCode(EscapeSequence::Reset) << L' '
                << Debug::escapeCodeFromColor(255, 165, 0) << L"[tID: " << std::this_thread::get_id() << L"]" << Debug::escapeCode(EscapeSequence::Reset) << beginLogLevel;

            switch (severity)
            {
            case LogLevel::Trace:
                out << L" [TRACE]";
                break;
            case LogLevel::Info:
                out << L" [INFO] ";
                break;
            case LogLevel::Warn:
                out << L" [WARN] ";
                break;
            case LogLevel::Error:
                out << L" [ERROR]";
                break;
            }

            out << endLogLevel;

            if (hasNewline)
                out << '\n';
            else
                out << " | ";

            out << beginLogLevel << logText << endLogLevel << L'\n';

#if FIREWORK_DEBUG_LOG_ASYNC
            Application::queueJobForWorkerThread([out = std::move(out).str()]() -> void { std::wcout << out; });
#else
            std::wcout << out.rdbuf();
#endif
        }
        /// @brief Logs a message at the trace severity.
        /// @warning Whilst Debug::logTrace is thread-safe, it is synchronized by spinlock, so this will be slow if you run it in parallel!
        /// @tparam ...T
        /// @param ...log Message to concatenate together.
        /// @see Firework::Debug::log
        /// @note Thread-safe.
        template <typename... T>
        inline static void logTrace(const T&... log)
        {
            Debug::log(LogLevel::Trace, log...);
        }
        /// @brief Logs a message at the info severity.
        /// @warning Whilst Debug::logInfo is thread-safe, it is synchronized by spinlock, so this will be slow if you run it in parallel!
        /// @tparam ...T
        /// @param ...log Message to concatenate together.
        /// @see Firework::Debug::log
        /// @note Thread-safe.
        template <typename... T>
        inline static void logInfo(const T&... log)
        {
            Debug::log(LogLevel::Info, log...);
        }
        /// @brief Logs a message at the warn severity.
        /// @warning Whilst Debug::logWarn is thread-safe, it is synchronized by spinlock, so this will be slow if you run it in parallel!
        /// @tparam ...T
        /// @param ...log Message to concatenate together.
        /// @see Firework::Debug::log
        /// @note Thread-safe.
        template <typename... T>
        inline static void logWarn(const T&... log)
        {
            Debug::log(LogLevel::Warn, log...);
        }
        /// @brief Logs a message at the error severity.
        /// @warning Whilst Debug::logError is thread-safe, it is synchronized by spinlock, so this will be slow if you run it in parallel!
        /// @tparam ...T
        /// @param ...log Message to concatenate together.
        /// @see Firework::Debug::log
        /// @note Thread-safe.
        template <typename... T>
        inline static void logError(const T&... log)
        {
            Debug::log(LogLevel::Error, log...);
        }
        /// @brief cowsay
        /// @tparam ...T 🐄
        /// @param ...log cow is say
        /// @note Thread-safe.
        template <typename... T>
        inline static void logError_final_final_actual_v2_final3(const T&... log)
        {
            std::wostringstream logStr;
            Debug::variadicToString(logStr, log...);

            std::vector<std::wstring> lines;
            std::wistringstream readStr(std::move(logStr).str());
            std::wstring line;
            size_t width = 16;
            while (std::getline(readStr, line))
            {
                width = std::max(width, line.size());
                lines.push_back(line);
            }

            std::wstring out;
            if (!lines.empty())
            {
                out.push_back(L' ');
                for (size_t i = width + 2; i--;) out.push_back(L'_');
                out.push_back(L'\n');
            }
            switch (lines.size())
            {
            case 0:
                break;
            case 1:
                out.append(L"< ").append(lines.front());
                for (size_t i = width - lines.front().size(); i--;) out.push_back(L' ');
                out.append(L" >\n");
                break;
            default:
                out.append(L"/ ").append(lines.front());
                for (size_t i = width - lines.front().size(); i--;) out.push_back(L' ');
                out.append(L" \\\n");
                for (auto it = ++lines.begin(); it != --lines.end(); ++it)
                {
                    out.append(L"| ").append(*it);
                    for (size_t i = width - it->size(); i--;) out.push_back(L' ');
                    out.append(L" |\n");
                }
                out.append(L"\\ ").append(lines.back());
                for (size_t i = width - lines.back().size(); i--;) out.push_back(L' ');
                out.append(L" /\n");
            }
            out.push_back(L' ');
            for (size_t i = width + 2; i--;) out.push_back(L'-');

            out.append(LR"(
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||)");

            Debug::log(LogLevel::Error, out);
        }

        inline static void printHierarchy()
        {
            std::string logStr("Hierachy\n-------\n");

            auto recurse = [&](auto&& recurse, Entity& entity, ssz depth = 0) -> void
            {
                for (ssz i = 0; i < depth; i++) logStr.push_back('\t');
                if (std::shared_ptr<EntityAttributes> attr = entity.getComponent<EntityAttributes>())
                    logStr.append(attr->name);
                else
                    logStr.append(std::format("{}", static_cast<void*>(&entity)));
                logStr.push_back('\n');

                for (Entity& child : entity.children()) recurse(recurse, child, depth + 1_z);
            };
            for (Entity& entity : Entities::range()) recurse(recurse, entity);

            logStr.append("--------\n");
            std::cout << logStr;
        }

        static void showF3Menu(bool visible = true);
        static void showWireframes(bool visible = true);

        /// @brief Triggers a breakpoint trap.
        /// @note Thread-safe.
        _inline_always static void breakPoint()
        {
#if _WIN32
            __debugbreak();
#else // Hope target platform has this.
            raise(SIGTRAP);
#endif
        }

        /// @brief Displays a simple message box.
        /// @param severity What kind of message box to display.
        /// @param title Name of the message box.
        /// @param message Message to display.
        /// @note Thread-safe. TODO: Is this a lie?
        static void messageBox(LogLevel severity, std::string_view title, std::string_view message);
    };
} // namespace Firework
