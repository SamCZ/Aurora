#pragma once

#include <memory>
#include <string>

#include "Aurora/Core/Library.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Math.hpp"
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Graphics/Base/SwapChain.hpp"

#include "CursorMode.hpp"

#include "Input/IManager.hpp"

namespace Aurora
{
	struct WindowDefinition
	{
		int Width;
		int Height;

		bool HasOSWindowBorder;
		bool Maximized;
		bool FullScreen = false;
		bool GPUMessageHandler = false;
		bool GraphicsDebug = false;

		String Title;
	};

	class AU_API ISystemWindow
	{
	protected:
		EventEmitter<const std::vector<Path>&> m_DropFileEmitter;
	public:
		virtual ~ISystemWindow() = default;

		EventEmitter<const std::vector<Path>&>& GetFileDropEmitter() { return m_DropFileEmitter; }
	public:
		virtual void Initialize(const WindowDefinition& windowDefinition, const std::shared_ptr<ISystemWindow>& parentWindow) = 0;

		virtual void SetSize(int width, int height) = 0;
		virtual void SetSize(const Vector2i& size) = 0;
		[[nodiscard]] virtual const Vector2i& GetSize() const = 0;
		[[nodiscard]] virtual int GetWidth() const = 0;
		[[nodiscard]] virtual int GetHeight() const = 0;
		virtual void AddResizeListener(const std::function<void(int, int)>& listener) = 0;

		virtual void Show() = 0;
		virtual void Hide() = 0;

		virtual void Destroy() = 0;

		virtual void Minimize() = 0;
		virtual void Maximize() = 0;
		virtual void Restore() = 0;

		virtual void Focus() = 0;

		virtual void SetTitle(const String& title) = 0;

		[[nodiscard]] virtual const String& GetOriginalTitle() const = 0;
		virtual void SetVsync(bool enabled) = 0;
		[[nodiscard]] virtual bool IsVsyncEnabled() const = 0;
		[[nodiscard]] virtual bool IsFocused() const = 0;
		[[nodiscard]] virtual bool IsShouldClose() const = 0;

		virtual void SetCursorMode(const ECursorMode& mode) = 0;
		[[nodiscard]] virtual const ECursorMode& GetCursorMode() const = 0;

		virtual bool IsIconified() = 0;

		virtual ::Aurora::Input::IManager_ptr& GetInputManager() = 0;

		virtual void SetClipboardString(const String& str) = 0;
		[[nodiscard]] virtual String GetClipboardString() = 0;

		virtual void SetSwapChain(ISwapChain_ptr swapChain) = 0;
		[[nodiscard]] virtual ISwapChain_ptr& GetSwapChain() = 0;
	};
}