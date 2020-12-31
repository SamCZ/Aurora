#include "InputManager.hpp"

#include "../Window.hpp"

namespace Aurora::App
{
	FInputManager::FInputManager(FWindow* window) : m_MousePosInitialized(false), WindowHandle(window)
	{

	}

	FInputManager::~FInputManager() = default;

    void FInputManager::AddActionMapping(const InputActionKeyMapping & mapping)
    {
        m_ActionMappings.push_back(mapping);
    }

    void FInputManager::AddActionMapping(const String& InActionName, const Key& InKey, bool bInShift, bool bInCtrl, bool bInAlt, bool bInCmd)
    {
        InputActionKeyMapping mapping = {
                InActionName, InKey, bInShift, bInCtrl, bInAlt, bInCmd
        };

        m_ActionMappings.push_back(mapping);
    }

    void FInputManager::RemoveActionMapping(const InputActionKeyMapping & mapping)
    {
        List_Remove(m_ActionMappings, mapping);
    }

    void FInputManager::AddAxisMapping(const InputAxisKeyMapping & mapping)
    {
        m_AxisMappings.push_back(mapping);
    }

    void FInputManager::AddAxisMapping(const String& InAxisName, const Key& InKey, float InScale)
    {
        InputAxisKeyMapping mapping = { InAxisName, InKey, InScale };
        m_AxisMappings.push_back(mapping);
    }

    void FInputManager::RemoveAxisMapping(const InputAxisKeyMapping & mapping)
    {
        List_Remove(m_AxisMappings, mapping);
    }

	const Vector2i &FInputManager::GetMousePos()
	{
		return m_MousePos;
	}

	const Vector2i &FInputManager::GetMouseDelta()
	{
		return m_MouseDelta;
	}

	void FInputManager::UpdateMousePos(int x, int y)
	{
    	Vector2i newMousePos(x, y);

    	if(!m_MousePosInitialized) {
			m_MousePosInitialized = true;
			m_MousePos = newMousePos;
    	} else {
			m_MouseDelta = newMousePos - m_MousePos;
			m_MousePos = newMousePos;
    	}

    	if(m_LastMousePos != m_MousePos) {
    		bool changedX = m_LastMousePos.x != m_MousePos.x;
    		bool changedY = m_LastMousePos.y != m_MousePos.y;

			for (InputAxisKeyMapping& mapping : m_AxisMappings)
			{
				for (InputEventAction<void, float>& action : m_InputAxisListeners)
				{
					if (mapping.AxisName == action.ActionName)
					{
						if (mapping.KeyType == Keys::MouseX && changedX)
						{
							action.Delegate->Invoke(m_MouseDelta.x * mapping.Scale);
						}

						if (mapping.KeyType == Keys::MouseY && changedY)
						{
							action.Delegate->Invoke(m_MouseDelta.y * mapping.Scale);
						}
					}
				}
			}
			m_LastMousePos = m_MousePos;
    	}
	}

	void FInputManager::OnKeyAction(const Key &key, const InputEvent &eventType)
	{
		m_KeyStates[key] = eventType == IE_Pressed || eventType == IE_Repeat;

		for (InputActionKeyMapping& mapping : m_ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : m_InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == eventType)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}
	}

	void FInputManager::OnMouseAction(const EMouseButtons& mouseBtn, const InputEvent& eventType)
	{
		Key key = MouseButtonToKey(mouseBtn);

		m_KeyStates[key] = eventType == IE_Pressed;

		for (InputActionKeyMapping& mapping : m_ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : m_InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == eventType)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}
	}

	Key FInputManager::MouseButtonToKey(const EMouseButtons& btn)
	{
		switch (btn)
		{
			case EMouseButtons::Left:
				return Keys::LeftMouseButton;
			case EMouseButtons::Right:
				return Keys::RightMouseButton;
			case EMouseButtons::Thumb01:
				return Keys::ThumbMouseButton;
			case EMouseButtons::Thumb02:
				return Keys::ThumbMouseButton2;

			default:
				return Keys::Invalid;
		}
	}

	void FInputManager::OnMouseScrollAction(float dx, float dy)
	{
		//TODO: Support delta x scroll for touch pad features.
		(void)dx;

		Key key;
		if (dy > 0)
		{
			key = Keys::MouseScrollUp;
		}
		else
		{
			key = Keys::MouseScrollDown;
		}

		for (InputAxisKeyMapping& mapping : m_AxisMappings)
		{
			if (!(mapping.KeyType == Keys::MouseWheelAxis || mapping.KeyType == key))
			{
				continue;
			}

			for (InputEventAction<void, float>& action : m_InputAxisListeners)
			{
				if (mapping.AxisName == action.ActionName)
				{
					action.Delegate->Invoke(dy * mapping.Scale);
				}
			}
		}

		for (InputActionKeyMapping& mapping : m_ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : m_InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}
	}

	void FInputManager::Update()
	{
        for(auto& it : m_KeyStates) {
            if (it.second)
            {
                const Key& key = it.first;

                for (InputAxisKeyMapping& mapping : m_AxisMappings)
                {
                    if (mapping.KeyType != key)
                    {
                        continue;
                    }

                    for (InputEventAction<void, float>& action : m_InputAxisListeners)
                    {
                        if (mapping.AxisName == action.ActionName)
                        {
                            action.Delegate->Invoke(1.0f * mapping.Scale);
                        }
                    }
                }
            }
        }
	}

	void FInputManager::SetCursorMode(const ECursorMode &mode)
	{
		WindowHandle->SetCursorMode(mode);
	}

	const ECursorMode &FInputManager::GetCursorMode() const
	{
		return WindowHandle->GetCursorMode();
	}
}