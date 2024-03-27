/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

https://axmolengine.github.io/

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

#include "CCActionCoroutine.h"

NS_CC_BEGIN

//
// Coroutine
//
Coroutine::Coroutine(Coroutine&& rhs) noexcept : _handle(rhs._handle)
{
    rhs._handle = nullptr;
}

Coroutine::~Coroutine()
{
    if (_handle)
    {
        _handle.destroy();
    }
}

Coroutine& Coroutine::operator=(Coroutine&& rhs) noexcept
{
    if (this != std::addressof(rhs))
    {
        _handle     = rhs._handle;
        rhs._handle = nullptr;
    }
    return *this;
}

Action* Coroutine::currentAction() const noexcept
{
    return _handle.promise().currentAction();
}

bool Coroutine::isDone() const
{
    return _handle && _handle.done();
}

bool Coroutine::moveNext() const
{
    if (_handle)
    {
        _handle.promise().resume();
        return !_handle.done();
    }
    return false;
}

//
// ActionCoroutine
//
ActionCoroutine* ActionCoroutine::create(const std::function<Coroutine()>& function)
{
    auto ret = new ActionCoroutine();
    if (ret && ret->initWithCoroutine(function))
    {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

ActionCoroutine* ActionCoroutine::create(Coroutine &&coroutine, std::function<void()> callback)
{
    ActionCoroutine *ret = new (std::nothrow) ActionCoroutine();
    if (ret && ret->initWithCoroutine(std::forward<Coroutine>(coroutine), callback))
    {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ActionCoroutine::initWithCoroutine(const std::function<Coroutine()>& function) noexcept
{
    _coroutine = std::forward<Coroutine>(function());
    return true;
}

bool ActionCoroutine::initWithCoroutine(Coroutine &&coroutine, std::function<void()> callback) noexcept
{
    _coroutine = std::forward<Coroutine>(coroutine);
    _callback = callback;
    return true;
}

bool ActionCoroutine::isDone() const
{
    auto action = _coroutine.currentAction();
    if (action && !action->isDone())
        return false;

    return _coroutine.isDone();
}

void ActionCoroutine::step(float dt)
{
    auto action = _coroutine.currentAction();
    if (action && !action->isDone())
    {
        action->step(dt);
        return;
    }
    _coroutine.moveNext();
}

void ActionCoroutine::stop()
{
    Action::stop();
    if (_callback)
    {
        _callback();
    }
}

NS_CC_END
