#include "SystemUtils.hpp"
#include "Debug.hpp"

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <Aurora/Engine.hpp>
#include <Aurora/App/GLFWWindow.hpp>
#include <Aurora/Tools/backward.hpp>

#pragma comment(lib, "dbghelp.lib")

namespace Aurora
{
	void InitDebugSymbols()
	{
		char buff[MAX_PATH + 1];
		GetModuleFileNameA(nullptr, buff, MAX_PATH);
		char* end = strrchr(buff, '\\');
		if (end == nullptr)
		{
			buff[0] = '.';
			buff[1] = 0;
		}
		else
			*end = 0;

		BOOL ret = ::SymInitialize(::GetCurrentProcess(), buff, TRUE);
		if (!ret)
		{
			//::GetLastError();
		}
	}

	void ShowErrorTraceWindow(const std::string& message)
	{
#ifdef _WIN32
		HWND hwnd = nullptr;

		if (GEngine && GEngine->GetWindow())
		{
			hwnd = static_cast<GLFWWindow*>(GEngine->GetWindow())->GetWindowWin32Handle();
		}

		InitDebugSymbols();

		using namespace backward;
		StackTrace st; st.load_here(32);

		std::stringstream ss;
		ss << message << std::endl;

		TraceResolver tr; tr.load_stacktrace(st);
		for (size_t i = 0; i < st.size(); ++i)
		{
			ResolvedTrace trace = tr.resolve(st[i]);
			ss << "#" << i
			   << " " << trace.object_filename
			   << " " << trace.object_function << ":" << trace.source.line
			   << " [" << trace.addr << "]"
			   << std::endl;
		}

		int result = ::MessageBoxA(hwnd, ss.str().c_str(), "Error !", MB_ABORTRETRYIGNORE | MB_ICONERROR);

		switch (result)
		{
			case IDABORT:
				exit(1);
			case IDRETRY:
				ES_BREAK;
				break;
			case IDTRYAGAIN:
				break;
			default:
				break;
		}
#endif
	}
}