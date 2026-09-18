//This file holds a container that determines either if there is a succesful value or if an error has occured
//Using a class template for the purpose of this project is higher value as had we done just optional<Trade>, we would know the trade creation failed, but not the exact reason as to why  
#pragma once

#include <optional>

//T for when the operation succeeds, and E for when the operation fails
template <typename T, typename E>
class Result
{
private:

    //The reason for two different constructors is because in the event of accidently creating Result(value, error) or nullopt in both parameters creates an invalid state
    Result(std::optional<T> value, std::optional<E> error) : m_value(value), m_error(error) {}

    std::optional<T> m_value;
    std::optional<E> m_error;

public:
    //Factory functions used for better readability 
    static Result success(const T& value)
    {
        return Result(value, std::nullopt);
    }

    static Result failure(E error)
    {
        return Result(std::nullopt, error);
    }

    bool hasValue() const
    {
        return m_value.has_value();
    }

    const T& value() const
    {
        return m_value.value();
    }

    E error() const
    {
        return m_error.value();
    }
};