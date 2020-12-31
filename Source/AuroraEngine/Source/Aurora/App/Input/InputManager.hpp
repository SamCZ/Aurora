#pragma once

#include <utility>

#include <Aurora/Core/SmartPointer.hpp>
#include <Aurora/Core/Container.hpp>
#include <Aurora/Core/Delegate.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>

#include "InputMapping.hpp"
#include "../CursorMode.hpp"

namespace Aurora::App
{
    enum InputEvent
    {
        IE_Pressed = 0,
        IE_Released = 1,
        IE_Repeat = 2,
        IE_DoubleClick = 3,
        IE_Axis = 4,
        IE_MAX = 5,
    };

    enum class EMouseButtons
    {
        Left = 0,
        Middle,
        Right,
        Thumb01,
        Thumb02,

        Invalid,
    };

    template<typename ReturnType, typename... ArgsType>
    struct InputEventAction
    {
        String ActionName;
        InputEvent EventType;
        SharedPtr<IDelegate<ReturnType, ArgsType...>> Delegate;
    };

    class FWindow;

    class FInputManager
    {
    private:
        List<InputActionKeyMapping> m_ActionMappings;
        List<InputAxisKeyMapping> m_AxisMappings;

        List<InputEventAction<void>> m_InputActionsListeners;
        List<InputEventAction<void, char>> m_InputKeyTypeListeners;
        List<InputEventAction<void, float>> m_InputAxisListeners;
	private:
    	Vector2i m_MousePos{};
    	Vector2i m_MouseDelta{};
    	Vector2i m_LastMousePos{};
    	bool m_MousePosInitialized;

		Map<Key, bool> m_KeyStates;

		FWindow* WindowHandle;
    public:
		explicit FInputManager(FWindow* window);
		~FInputManager();

        void AddActionMapping(const InputActionKeyMapping& mapping);
        void AddActionMapping(const String& InActionName, const Key& InKey, bool bInShift = false, bool bInCtrl = false, bool bInAlt = false, bool bInCmd = false);
        void RemoveActionMapping(const InputActionKeyMapping& mapping);

        void AddAxisMapping(const InputAxisKeyMapping& mapping);
        void AddAxisMapping(const String& InAxisName, const Key& InKey, float InScale = 1.f);
        void RemoveAxisMapping(const InputAxisKeyMapping& mapping);

        template<class UserClass>
        inline void BindAction(const String& actionName, const InputEvent& keyEvent, UserClass* object, typename Delegate<UserClass, void>::MethodPtr fnc)
        {
            SharedPtr<IDelegate<void>> baseDelegate = MakeShared<Delegate<UserClass, void>>(object, fnc);

            InputEventAction<void> action = {
                    actionName, keyEvent, baseDelegate
            };

            m_InputActionsListeners.push_back(action);
        }

		inline void BindAction(const String& actionName, const InputEvent& keyEvent, IDelegate<void>* actionDelegate)
		{
			InputEventAction<void> action = {
					actionName, keyEvent, MakeShareable(actionDelegate)
			};

			m_InputActionsListeners.push_back(action);
		}

        template<class UserClass>
        inline void BindKeyTypeAction(UserClass* object, typename Delegate<UserClass, void, char>::MethodPtr fnc)
        {
            auto* del = new Delegate<UserClass, void, char>(object, fnc);
            SharedPtr<IDelegate<void, char>> baseDelegate = MakeShareable(del);

            InputEventAction<void, char> action;
            action.Delegate = baseDelegate;

            m_InputKeyTypeListeners.push_back(action);
        }

        template<class UserClass>
        inline void BindAxis(const String& actionName, UserClass* object, typename Delegate<UserClass, void, float>::MethodPtr fnc)
        {
            auto* del = new Delegate<UserClass, void, float>(object, fnc);
            SharedPtr<IDelegate<void, float>> baseDelegate = MakeShareable(del);

            InputEventAction<void, float> action = {
                    actionName, InputEvent::IE_Axis, baseDelegate
            };

            m_InputAxisListeners.push_back(action);
        }

		inline void BindAxis(const String& actionName, IDelegate<void, float>* delegate)
		{
			SharedPtr<IDelegate<void, float>> baseDelegate = MakeShareable(delegate);

			InputEventAction<void, float> action = {
					actionName, InputEvent::IE_Axis, baseDelegate
			};

			m_InputAxisListeners.push_back(action);
		}

		Key MouseButtonToKey(const EMouseButtons& btn);

        void OnKeyAction(const Key& key, const InputEvent& eventType);
        void OnMouseAction(const EMouseButtons& mouseBtn, const InputEvent& eventType);
        void OnMouseScrollAction(float dx, float dy);

        const Vector2i& GetMousePos();
        const Vector2i& GetMouseDelta();

        void UpdateMousePos(int x, int y);

        void Update();

		void SetCursorMode(const ECursorMode& mode);
		[[nodiscard]] const ECursorMode& GetCursorMode() const;
    };
    DEFINE_PTR(FInputManager)
}