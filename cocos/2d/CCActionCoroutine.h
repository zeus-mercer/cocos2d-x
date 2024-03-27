/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __ACTION_CCCOROUTINE_ACTION_H__
#define __ACTION_CCCOROUTINE_ACTION_H__

#include "2d/CCAction.h"
#include "2d/CCNode.h"

#if __has_include(<coroutine>)
#    include <coroutine>
namespace cocos2d
{
using suspend_always = std::suspend_always;
template <typename _Ty>
using coroutine_handle = std::coroutine_handle<_Ty>;
}  // namespace cocos2d
#elif __has_include(<experimental/coroutine>)
// fallback to experimental, currently only for android, in the future may don't required,
// for example: we upgrade ndk in the future releases of axmol
#    include <experimental/coroutine>
namespace cocos2d
{
using suspend_always = std::experimental::suspend_always;
template <typename _Ty>
using coroutine_handle = std::experimental::coroutine_handle<_Ty>;
}  // namespace cocos2d
#else
#    error This compiler missing c++20 coroutine
#endif

NS_CC_BEGIN

/**
 * @addtogroup actions
 * @{
 */
class CC_DLL Coroutine
{
public:
    class promise_type;

    using handle = cocos2d::coroutine_handle<promise_type>;
    class promise_type
    {
    public:
        virtual ~promise_type() { CC_SAFE_RELEASE(_currentAction); }

        Action* currentAction() const noexcept { return _currentAction; }

        auto final_suspend() const noexcept { return cocos2d::suspend_always{}; }

        auto get_return_object() noexcept { return Coroutine(handle::from_promise(*this)); }

        auto initial_suspend() const noexcept { return cocos2d::suspend_always{}; }

        auto yield_value(Action* action)
        {
            CC_SAFE_RELEASE(_currentAction);
            _currentAction = action;
            CC_SAFE_RETAIN(_currentAction);
            return cocos2d::suspend_always{};
        }

        void return_void() {}
        void unhandled_exception() {}

        void resume() { handle::from_promise(*this).resume(); }

    private:
        Action* _currentAction{nullptr};
    };

    Coroutine()                 = default;
    Coroutine(const Coroutine&) = delete;
    Coroutine(Coroutine&& rhs) noexcept;
    virtual ~Coroutine();

    Coroutine& operator=(const Coroutine&) = delete;
    Coroutine& operator=(Coroutine&& rhs) noexcept;

    Action* currentAction() const noexcept;
    bool isDone() const;
    bool moveNext() const;

private:
    Coroutine(handle h) noexcept : _handle{h} {}

    handle _handle;
};

/**
 *@brief Base class for Coroutine actions.
 *@ingroup Actions
 */
class CC_DLL ActionCoroutine : public Action
{
public:
    /**
     * @brief Create the action with callback.
     * @param function A callback function need to be executed.
     * @return A pointer of ActionCoroutine. If creation failed, return nil.
     * @lua NA
     */
    static ActionCoroutine* create(const std::function<Coroutine()>& function);

    /**
     * @brief Create the action with coroutine.
     * @param coroutine A coroutine function need to be executed.
     * @param callback A callback function handled after action is complete.
     * @return A pointer of ActionCoroutine. If creation failed, return nil.
     * @lua NA
     */
    static ActionCoroutine *create(Coroutine&& coroutine, std::function<void()> callback = nullptr);

    //
    // Overrides
    //
    bool isDone() const override;
    void step(float dt) override;
    void stop() override;

public:
    ActionCoroutine() = default;
    virtual ~ActionCoroutine() {}

    /**
     * initializes the action with the callback
     * @param function A callback function need to be executed.
     * @lua NA
     */
    bool initWithCoroutine(const std::function<Coroutine()>& function) noexcept;

    /**
     * initializes the action with the callback
     * @param coroutine A callback function need to be executed.
     * @lua NA
     */
    bool initWithCoroutine(Coroutine&& coroutine, std::function<void()> callback) noexcept;

protected:
    /** Coroutine */
    Coroutine _coroutine;

    /** Callback */
    std::function<void()> _callback;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(ActionCoroutine);
};

NS_CC_END

#endif  // __ACTION_CCCOROUTINE_ACTION_H__
