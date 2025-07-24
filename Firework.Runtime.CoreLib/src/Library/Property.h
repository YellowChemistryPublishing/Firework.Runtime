#pragma once

#include <function.h>
#include <type_traits>

namespace Firework
{
    template <typename GetterReturnType, typename SetterInputType>
    struct Property
    {
        inline explicit Property(auto&& getter, auto&& setter) :
            getter(std::forward<typename std::remove_cvref<decltype(getter)>::type>(getter)), setter(std::forward<typename std::remove_cvref<decltype(setter)>::type>(setter))
        { }
        inline explicit Property(const Property<GetterReturnType, SetterInputType>& other) = delete;
        inline explicit Property(Property<GetterReturnType, SetterInputType>&& other) = delete;

        inline GetterReturnType operator=(SetterInputType value) const
        {
            this->setter(value);
            return this->getter();
        }
        inline GetterReturnType operator=(const Property<GetterReturnType, SetterInputType>& rhs) = delete;
        inline GetterReturnType operator=(Property<GetterReturnType, SetterInputType>&&) = delete;

#pragma region Arithmetic
        inline auto operator+(auto rhs) const -> decltype(std::declval<GetterReturnType>() + rhs)
        requires requires(GetterReturnType _lhs, decltype(rhs) _rhs) { _lhs + _rhs; }
        {
            return (this->getter() + rhs);
        }
        inline GetterReturnType operator-(SetterInputType rhs) const
        {
            return (this->getter() - rhs);
        }
        inline GetterReturnType operator*(SetterInputType rhs) const
        {
            return (this->getter() * rhs);
        }
        inline GetterReturnType operator/(SetterInputType rhs) const
        {
            return (this->getter() / rhs);
        }
        inline GetterReturnType operator%(SetterInputType rhs) const
        {
            return (this->getter() % rhs);
        }

        inline GetterReturnType operator++() const
        {
            GetterReturnType type = this->getter();
            ++type;
            this->setter(type);
            return type;
        }
        inline GetterReturnType operator--() const
        {
            GetterReturnType type = this->getter();
            --type;
            this->setter(type);
            return type;
        }
#pragma endregion

#pragma region Assignment
        inline GetterReturnType operator+=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type += rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator-=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type -= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator*=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type *= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator/=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type /= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator%=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type %= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
#pragma endregion

#pragma region Comparison
        inline GetterReturnType operator==(SetterInputType rhs) const
        {
            return this->getter() == rhs;
        }
        inline GetterReturnType operator!=(SetterInputType rhs) const
        {
            return this->getter() != rhs;
        }
        inline GetterReturnType operator>=(SetterInputType rhs) const
        {
            return this->getter() >= rhs;
        }
        inline GetterReturnType operator<=(SetterInputType rhs) const
        {
            return this->getter() <= rhs;
        }
        inline GetterReturnType operator>(SetterInputType rhs) const
        {
            return this->getter() > rhs;
        }
        inline GetterReturnType operator<(SetterInputType rhs) const
        {
            return this->getter() < rhs;
        }
#pragma endregion

        inline GetterReturnType operator()() const
        {
            return this->getter();
        }
        inline operator GetterReturnType() const
        {
            return this->getter();
        }
    private:
        [[no_unique_address]] const func::function<GetterReturnType()> getter;
        [[no_unique_address]] const func::function<void(SetterInputType)> setter;
    };
    template <typename SetterInputType>
    struct Property<void, SetterInputType>
    {
        constexpr Property(auto&& setter) : setter(std::forward<typename std::remove_cvref<decltype(setter)>::type>(setter))
        { }

        constexpr void operator=(SetterInputType other) const
        {
            return this->setter(other);
        }
    private:
        [[no_unique_address]] const func::function<void(SetterInputType)> setter;
    };
} // namespace Firework
