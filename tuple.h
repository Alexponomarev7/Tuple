#ifndef TUPLE_TUPLE_H
#define TUPLE_TUPLE_H

#include <iostream>
#include <cassert>

template<typename... T_n>
class Tuple;

template<>
class Tuple<> {
public:
    void swap(Tuple<>& other) {}
};

template<typename First, typename... T_other>
class Tuple<First, T_other...> : public  Tuple<T_other...> {
private:
    using value_type = First;
    using value_reference = First&;

    value_type _value;
public:
    constexpr Tuple() : _value(), Tuple<T_other...>() {}
    explicit constexpr Tuple(const First& first, const T_other&... other) : _value(first), Tuple<T_other...>(other...) {}

    template<typename Second, typename... S_other, typename = std::enable_if_t<sizeof...(T_other) == sizeof...(S_other)
            && std::is_same<Second, value_type>::value>>
    explicit constexpr Tuple(Second&& second, S_other&&... other) : _value(std::forward<Second>(second)),
                                                             Tuple<T_other...>(std::forward<S_other>(other)...) {}

    Tuple(const Tuple&) = default;
    Tuple(Tuple&&) = default;

    template<typename Second, typename... S_other, typename = std::enable_if_t<sizeof...(S_other) == sizeof...(T_other)>>
    constexpr Tuple(const Tuple<Second, S_other...>& other) : _value(other.get()),
                                                         Tuple<T_other...>(other.next()) {}

    template<typename Second, typename... S_other, typename = std::enable_if_t<sizeof...(S_other) == sizeof...(T_other)>>
    constexpr Tuple(Tuple<Second, S_other...>&& other) : _value(std::move(other.get())),
                                                         Tuple<T_other...>(std::move(other.next())) {}

    Tuple& operator=(const Tuple& other) = default;

    template<typename Second, typename... S_other, typename = std::enable_if_t<sizeof...(S_other) == sizeof...(T_other)>>
    Tuple& operator=(const Tuple<Second, S_other...>& other) {
        _value = other.get();
        next() = other.next();
        return *this;
    }

    Tuple& operator=(Tuple&& other) = default;

    template<typename Second, typename... S_other, typename = std::enable_if_t<sizeof...(S_other) == sizeof...(T_other)>>
    Tuple& operator=(Tuple<Second, S_other...>&& other) {
        _value = std::move(other.get());
        next() = std::move(other.next());
        return *this;
    }

    ~Tuple() = default;

    constexpr value_reference get() {
        return _value;
    }

    constexpr value_type cget() const {
        return _value;
    }

    constexpr Tuple<T_other...>& next() {
        return *static_cast<Tuple<T_other...>*>(this);
    }

    constexpr const Tuple<T_other...>& next() const {
        return *static_cast<const Tuple<T_other...>*>(this);
    }

    void swap(Tuple<First, T_other...>& second) {
        std::swap(_value, second.get());
        next().swap(second.next());
    }

    constexpr static std::size_t size() {
        return 1 + sizeof...(T_other);
    }
};


// get by type

template<typename T, typename First, typename... T_other>
constexpr typename std::enable_if_t<std::is_same<T, First>::value, const T&> get(const Tuple<First, T_other...>& tuple) {
    return tuple.get();
}

template<typename T, typename First, typename... T_other>
constexpr typename std::enable_if_t<!std::is_same<T, First>::value, const T&> get(const Tuple<First, T_other...>& tuple) {
    return get<T>(tuple.next());
}

template<typename T, typename First, typename... T_other>
constexpr typename std::enable_if_t<std::is_same<T, First>::value, T&> get(Tuple<First, T_other...>& tuple) {
    return tuple.get();
}

template<typename T, typename First, typename... T_other>
constexpr typename std::enable_if_t<!std::is_same<T, First>::value, T&> get(Tuple<First, T_other...>& tuple) {
    return get<T>(tuple.next());
}


template<typename T, typename First, typename... T_other>
constexpr typename std::enable_if_t<std::is_same<T, First>::value, T&&> get(Tuple<First, T_other...>&& tuple) {
    return std::move(tuple.get());
}

template<typename T, typename First, typename... T_other>
constexpr typename std::enable_if_t<!std::is_same<T, First>::value, T&&> get(Tuple<First, T_other...>&& tuple) {
    return get<T>(std::move(tuple.next()));
}

// get by pos

template<int N, typename First, typename... T_other, typename = std::enable_if_t<N == 0>>
constexpr First& get(Tuple<First, T_other...>& tuple) {
    return tuple.get();
}

template<int N, typename First, typename... T_other, typename = std::enable_if_t<N != 0>>
constexpr decltype(auto) get(Tuple<First, T_other...>& tuple) {
    return get<N - 1>(tuple.next());
}

template<int N, typename First, typename... T_other, typename = std::enable_if_t<N == 0>>
constexpr const First& get(const Tuple<First, T_other...>& tuple) {
    return tuple.get();
}

template<int N, typename First, typename... T_other, typename = std::enable_if_t<N != 0>>
constexpr decltype(auto) get(const Tuple<First, T_other...>& tuple) {
    return get<N - 1>(tuple.next());
}

template<int N, typename First, typename... T_other, typename = std::enable_if_t<N == 0>>
constexpr First&& get(Tuple<First, T_other...>&& tuple) {
    return std::move(tuple.get());
}

template<int N, typename First, typename... T_other, typename = std::enable_if_t<N != 0>>
constexpr decltype(auto) get(Tuple<First, T_other...>&& tuple) {
    return get<N - 1>(std::move(tuple.next()));
}


// operators
namespace Tuple_Traits {
    template<typename F_first, typename... F_other, typename S_second, typename... S_other,
            typename =std::enable_if_t<sizeof...(F_other) != 0>>
    constexpr bool lower(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
        assert(sizeof...(F_other) == sizeof...(S_other));

        return first.cget() < second.cget() || ((first.cget() == second.cget()) && lower(first.next(), second.next()));
    };

    template <typename First, typename Second>
    constexpr bool lower(const Tuple<First>& first, const Tuple<Second>& second) {
        return first.cget() < second.cget();
    };

    template<typename F_first, typename... F_other, typename S_second, typename... S_other,
            typename =std::enable_if_t<sizeof...(F_other) != 0>>
    constexpr bool equal(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
        assert(sizeof...(F_other) == sizeof...(S_other));

        return first.cget() == second.cget() && equal(first.next(), second.next());
    };

    template <typename First, typename Second>
    constexpr bool equal(const Tuple<First>& first, const Tuple<Second>& second) {
        return first.cget() == second.cget();
    };

    template <class T>
    struct make_tuple_return_impl
    {
        using type = T;
    };

    template <class T>
    struct make_tuple_return_impl<std::reference_wrapper<T>>
    {
        using type = T&;
    };

    template <class T>
    struct make_tuple_return
    {
        using type = typename make_tuple_return_impl<typename std::decay<T>::type>::type;
    };

    /** template<typename... F_other, typename... S_other>
    struct mergeTupleTypes<Tuple<F_other...>, Tuple<S_other...>> {
        using type = Tuple<F_other..., S_other...>;
    };

    template<typename First, typename Second>
    Tuple<typename mergeTupleTypes<First, Second>::type>mergeTuple(First&& f, Second&& s) {
        Tuple<typename  mergeTupleTypes<First, Second>> result();
        size_t pos = 0;
        for (int i )
        return makeTuple(f, f_other, s, s_other);
    }  **/
}

template<typename... S_other>
constexpr auto makeTuple(S_other&&... other) {
    return Tuple<typename Tuple_Traits::make_tuple_return<S_other>::type...>(std::forward<S_other>(other)...);
}
// < > == !=

template<typename F_first, typename... F_other, typename S_second, typename... S_other>
constexpr bool operator<(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
    return Tuple_Traits::lower(first, second);
}

template<typename F_first, typename... F_other, typename S_second, typename... S_other>
constexpr bool operator==(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
    return Tuple_Traits::equal(first, second);
}

template<typename F_first, typename... F_other, typename S_second, typename... S_other>
constexpr bool operator!=(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
    return !(first == second);
}

template<typename F_first, typename... F_other, typename S_second, typename... S_other>
constexpr bool operator>(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
    return !(first == second || first < second);
}

template<typename F_first, typename... F_other, typename S_second, typename... S_other>
constexpr bool operator<=(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
    return first < second || first == second;
}

template<typename F_first, typename... F_other, typename S_second, typename... S_other>
constexpr bool operator>=(const Tuple<F_first, F_other...>& first, const Tuple<S_second, S_other...>& second) {
    return first > second || first == second;
}

// tupleCat
/**template<typename First, typename Second, typename... Other>
constexpr decltype(auto) tupleCat(First&& first, Second&& second, Other&&... other) {
    return Tuple_Traits::merge(std::forward<First>(first),
            std::move(tupleCat(std::forward<Second>(second), std::forward<Other>(other)...)));
}

template<typename First, typename Tp2>
constexpr decltype(auto) tupleCat(First&& first, Second&& second) {
    return Tuple_Traits::merge(std::forward<First>(first), std::forward<Second>(second));
}**/

#endif //TUPLE_TUPLE_H