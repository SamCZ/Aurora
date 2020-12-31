#pragma once

#include <functional>
#include "Vector.hpp"

namespace Aurora
{
	typedef std::function<void(const Vector2i&)> ResizeFnc;

	class FSizeable
	{
    protected:
		bool m_SizeInitialized;
		Vector2i m_Size;
		List<ResizeFnc> m_ResizeListeners;
	public:
		FSizeable() : m_SizeInitialized(false), m_Size(Vector2i(0, 0)), m_ResizeListeners()
		{

		}

		FSizeable(int width, int height) : m_SizeInitialized(false), m_Size(Vector2i(width, height)), m_ResizeListeners()
		{

		}

		virtual ~FSizeable() = default;

		void AddResizeListener(const ResizeFnc& listener)
		{
            m_ResizeListeners.emplace_back(listener);
		}

		void SetSize(const Vector2i& newSize)
		{
			if(!m_SizeInitialized) {
                m_Size = newSize;
                m_SizeInitialized = true;
				return;
			}

			if(m_Size != newSize) {
				for(ResizeFnc& fnc : m_ResizeListeners) {
					fnc(newSize);
				}
			}

            m_Size = newSize;
		}

		[[nodiscard]] Vector2i GetSize() const
		{
			return m_Size;
		}
	};
}