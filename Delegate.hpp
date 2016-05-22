#ifndef DELEGATE_HPP_INCLUDED
#define DELEGATE_HPP_INCLUDED

#include <algorithm>
#include <memory>
#include <list>

namespace Delegate
{
//----------------------------------------------------------

/** BASE TYPE EREASURE OBJECT */
template <typename FunctionT>
struct BaseFunctionWrapper;

template <typename ReturnT, typename... ArgumentsP>
struct BaseFunctionWrapper<ReturnT(ArgumentsP...)>
{
    using BaseT = BaseFunctionWrapper;

    virtual ~BaseFunctionWrapper() = default;
    virtual bool Compare(BaseT* pComparable);
    virtual ReturnT Invoke(ArgumentsP&&... pack) const = 0;
};

/** ***************************** WRAPPERS *************************************/


// static function wrapper
template <typename FunctionT>
class StaticFunctionWrapper;

// specialization
template <typename ReturnT, typename... ArgumentsP>
class StaticFunctionWrapper<ReturnT(ArgumentsP...)> :
    public BaseFunctionWrapper<ReturnT(ArgumentsP...)>
{
public:
    using BaseT = BaseFunctionWrapper<ReturnT(ArgumentsP...)>;
    using ThisT = StaticFunctionWrapper;
    using PointerToCallback = ReturnT(*)(ArgumentsP...);

    StaticFunctionWrapper(PointerToCallback pCallback):
        callable_(pCallback) { }

    // invoke virtual function
    ReturnT Invoke(ArgumentsP&&... pack) const override
    {
        callable_(std::forward<ArgumentsP>(pack)...);
    }

    bool Compare(BaseT* pComparable) override
    {
        if(auto pStaticWrap = dynamic_cast<ThisT*>(pComparable))
        {
            if(callable_ == pStaticWrap->callable_)
            {
                return true;
            }
        }

        return false;
    }

private:
    PointerToCallback callable_;
};


// member function wrapper
template <typename ObjectT, typename MethodT>
class MemberFunctionWrapper;

template <typename ObjectT, typename ReturnT, typename... ArgumentsP>
class MemberFunctionWrapper<ObjectT, ReturnT (ObjectT::*)(ArgumentsP...)> :
    public BaseFunctionWrapper<ReturnT(ArgumentsP...)>
{
public:
    using PointerToCallback = ReturnT (ObjectT::*)(ArgumentsP...);
    using BaseT = BaseFunctionWrapper<ReturnT(ArgumentsP...)>;
    using ThisT = MemberFunctionWrapper;

    MemberFunctionWrapper(ObjectT* callableObject, PointerToCallback pCallback):
        callableObject_(callableObject), callableMethod_(pCallback) { }

    // invoke virtual function
    ReturnT Invoke(ArgumentsP&&... pack) const override
    {
        (callableObject_->*callableMethod_)(std::forward<ArgumentsP>(pack)...);
    }

    bool Compare(BaseT* pComparable) override
    {
        if(auto pMemberWrap = dynamic_cast<ThisT*>(pComparable))
        {
            if(callableMethod_ == pMemberWrap->callableMethod_ &&
               callableObject_ == pMemberWrap->callableObject_)
            {
                return true;
            }
        }

        return false;
    }

private:
    ObjectT* callableObject_;
    PointerToCallback callableMethod_;
};


// overloaded creators of function wrappers
template <typename ReturnT, typename... ArgumentsP>
typename std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>>
Function(ReturnT(*pCallable)(ArgumentsP...))
{
    return std::make_shared<StaticFunctionWrapper<ReturnT(ArgumentsP...)>>(pCallable);
}

template <typename ObjectT, typename ReturnT, typename... ArgumentsP>
typename std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>>
Function(ObjectT* pCallableObject, ReturnT(ObjectT::*pCallable)(ArgumentsP...))
{
    return std::make_shared<MemberFunctionWrapper<ObjectT, ReturnT(ObjectT::*)(ArgumentsP...)>>
                                (pCallableObject, pCallable);
}

// result Delegate class
template <typename FunctionT>
class Delegate;

// hides methods and static functions under this type(TYPE EREASURE)
template <typename ReturnT, typename... ArgumentsP>
class Delegate<ReturnT(ArgumentsP...)>
{
public:
    template <typename ObjectT>
    using PointerToMemberFunctionType = ReturnT(ObjectT::*)(ArgumentsP...);
    using PointerToStaticFunctionType = ReturnT(*)(ArgumentsP...);

    Delegate() = default;

    // for static functions
    Delegate(std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>> pCallable)
    {
        wrappers_.push_back(pCallable);
    }

    Delegate& operator+=(std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>> pCallable)
    {
        // callable object for compare two callable objects
        auto compare =
        [&pCallable](std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>> source)
        {
            return source->Compare(pCallable.get());
        };

        auto it = std::find_if(std::begin(wrappers_), std::end(wrappers_), compare);
        if(it == std::end(wrappers_))
        {
            wrappers_.push_back(pCallable);
        }

        return *this;
    }

    Delegate& operator-=(std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>> pCallable)
    {
        // callable object for compare two callable objects
        auto compare =
        [&pCallable](std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>> source)
        {
            return source->Compare(pCallable.get());
        };

        auto it = std::find_if(std::begin(wrappers_), std::end(wrappers_), compare);
        if(it != std::end(wrappers_))
        {
            wrappers_.erase(it);
        }

        return *this;
    }

    Delegate& operator=(std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>> pCallable)
    {
        wrappers_.clear();
        wrappers_.push_back(pCallable);
    }

    template <typename... ParamPack>
    void operator()(ParamPack&&... pack)
    {
        for(auto call : wrappers_)
        {
            call->Invoke(std::forward<ParamPack>(pack)...);
        }
    }

private:
    std::list<std::shared_ptr<BaseFunctionWrapper<ReturnT(ArgumentsP...)>>> wrappers_;
};

//----------------------------------------------------------
} // end of namespace Delegate

#endif // DELEGATE_HPP_INCLUDED
