#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <codecvt>
#include <chrono>
#if __linux__
#include <csignal>
#endif
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>
#include <utility>

#include <Library/Lock.h>

namespace Firework
{
	namespace Internal
	{
		// This is a bit of a hack.
		inline static std::wostream& operator<<(std::wostream& stream, const std::string& rhs)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
			stream << conv.from_bytes(rhs);
			return stream;
		}
		inline static std::wostream& operator<<(std::wostream& stream, const std::string_view& rhs)
		{
			for (auto it = rhs.begin(); it != rhs.end(); ++it)
				stream << *it;
			return stream;
		}
	}

	enum class EscapeSequence : uint_fast8_t
	{
		Reset
	};
	enum class LogLevel : uint_fast8_t
	{
		Trace,
		Info,
		Warn,
		Error
	};

	class __firework_corelib_api Debug final
	{
		static Firework::SpinLock outputLock;

		template<typename... T>
		static void variadicToString(std::wostringstream& str, const T&... args)
		{
			using namespace Firework::Internal;
			using expander = int[];
			(void)expander
			{
				0,
				(void(str << args), 0)...
			};
		}
		inline static std::wstring escapeCodeFromColor(uint8_t r, uint8_t g, uint8_t b)
		{
			std::wostringstream out;
			out << L"\x1b[38;2;" << r << L';' << g << L';' << b << L'm';
			return std::move(out).str();
		}
		inline static std::wstring escapeCodeFromColor(uint8_t code)
		{
			std::wostringstream out;
			out << L"\x1b[38;5;" << code << L'm';
			return std::move(out).str();
		}
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
			
			Debug::outputLock.lock();
			std::wcout <<

			Debug::escapeCodeFromColor(12) <<

			L"[UTC: " <<
			std::put_time(std::gmtime(&t), L"%Y/%m/%d %X") <<
			L"]" <<

			Debug::escapeCode(EscapeSequence::Reset) <<

			L' ' <<
			Debug::escapeCodeFromColor(255, 165, 0) <<

			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]";

			std::wcout << Debug::escapeCode(EscapeSequence::Reset);

			std::wcout << beginLogLevel;
			switch (severity)
			{
			case LogLevel::Trace:
				std::wcout << L" [TRACE]";
				break;
			case LogLevel::Info:
				std::wcout << L" [INFO] ";
				break;
			case LogLevel::Warn:
				std::wcout << L" [WARN] ";
				break;
			case LogLevel::Error:
				std::wcout << L" [ERROR]";
				break;
			}
			std::wcout << endLogLevel;

			if (hasNewline)
				std::wcout << '\n';
			else std::wcout << " | ";
			std::wcout << beginLogLevel;

			std::wcout << logText;

			std::wcout << endLogLevel;

			std::wcout << L'\n';
			Debug::outputLock.unlock();
		}
		template <typename... T>
		inline static void logTrace(const T&... log)
		{
			Debug::log(LogLevel::Trace, log...);
		}
		template <typename... T>
		inline static void LogInfo(const T&... log)
		{
			Debug::log(LogLevel::Info, log...);
		}
		template <typename... T>
		inline static void LogWarn(const T&... log)
		{
			Debug::log(LogLevel::Warn, log...);
		}
		template <typename... T>
		inline static void LogError(const T&... log)
		{
			Debug::log(LogLevel::Error, log...);
		}

		static void printHierarchy();

		inline static void breakPoint()
		{
			#if _WIN32
			__debugbreak();
			#elif __linux__
			raise(SIGTRAP);
			#endif
		}
	};
}
