#ifndef TUPLE_TUPLE_H
#define TUPLE_TUPLE_H

#include <iostream>

template<typename... T_rest>
class Tuple;

template<typename T1, typename... T_rest>
struct Tuple<T1, T_rest...> : public Tuple<T_rest...> {

    using value_type = T1;

    using _base = Tuple<T_rest...>;

    T1 _value;

    constexpr Tuple<T_rest...>& _getParent() {
        return *static_cast<Tuple<T_rest...>*>(this);
    }

    constexpr const Tuple<T_rest...>& _getParent() const {
        return *static_cast<const Tuple<T_rest...>*>(this);
    }

    constexpr Tuple(): _value(), Tuple<T_rest...>() {}

    // direct constructor
    explicit constexpr Tuple(const T1& value, const T_rest&... args): _value(value), Tuple<T_rest...>(args...) {}


    // universal reference constructor
    template<typename U1, typename... U_rest, typename = std::enable_if_t<sizeof...(U_rest) == sizeof...(T_rest)> >
    explicit constexpr Tuple(U1&& value, U_rest&&... args):
            _value(std::forward<U1>(value)),
            Tuple<T_rest...>(std::forward<U_rest>(args)...) {}

    // copy
    Tuple(const Tuple&) = default;

    template<typename U1, typename... U_rest,
            typename = std::enable_if_t<sizeof...(U_rest) == sizeof...(T_rest)> >
    constexpr Tuple(const Tuple<U1, U_rest...>& other):
            _value(other._value),
            Tuple<T_rest...>(other._getParent()) {}

    // move
    Tuple(Tuple&&) = default;

    template<typename U1, typename... U_rest,
            typename = std::enable_if_t<sizeof...(U_rest) == sizeof...(T_rest)> >
    constexpr Tuple(Tuple<U1, U_rest...>&& other):
            _value(std::move(other._value)),
            Tuple<T_rest...>(std::move(other._getParent())) {}

    // copy assign
    Tuple& operator=(const Tuple& other) = default;

    template<typename U, typename... U_rest,
            typename = std::enable_if_t<sizeof...(U_rest) == sizeof...(T_rest)> >
    Tuple& operator=(const Tuple<U, U_rest...>& other) {
        _value = other._value;
        _getParent() = other._getParent();
        return *this;
    }

    // move assign
    Tuple& operator=(Tuple&& other) = default;

    template<typename U, typename... U_rest,
            typename = std::enable_if_t<sizeof...(U_rest) == sizeof...(T_rest)> >
    Tuple& operator=(Tuple<U, U_rest...>&& other) {
        _value = std::move(other._value);
        _getParent() = std::move(other._getParent());
        return *this;
    }

    void swap(Tuple<T1, T_rest...>& other) {
        std::swap(other._value, _value);
        _getParent().swap(other._getParent());
    }

    constexpr static std::size_t size() {
        return 1 + sizeof...(T_rest);
    }
};

template<>
class Tuple<>{
public:
    using value_type = void;
    void swap(Tuple<>& other) {}
};

// hiding implementation details in namespace
namespace _implementation {

    struct tupleComparator {

        template<typename T, typename... T_rest, typename U, typename... U_rest>
        static constexpr bool _less(const Tuple<T, T_rest...>& first, const Tuple<U, U_rest...>& other) {

            static_assert(sizeof...(T_rest) == sizeof...(U_rest));

            return first._value < other._value || (!(other._value < first._value) && tupleComparator::_less(
                    first._getParent(), other._getParent()));
        }

        template<typename T, typename U>
        static constexpr bool _less(const Tuple<T>& first, const Tuple<U>& other) {
            return first._value < other._value;
        }

        template<typename T, typename... T_rest, typename U, typename... U_rest,
                typename =std::enable_if_t<sizeof...(T_rest) != 0> >
        static constexpr bool _equal(const Tuple<T, T_rest...>& first, const Tuple<U, U_rest...>& other) {
            return sizeof...(T_rest) == sizeof...(U_rest) && other._value == first._value && tupleComparator::_equal(
                    first._getParent(), other._getParent());
        }

        template<typename T, typename U>
        static constexpr bool _equal(const Tuple<T>& first, const Tuple<U>& other) {
            return first._value == other._value;
        }

        tupleComparator() = delete;
    };

    template<typename T>
    struct _referenceUnwrapper {
        using type = T;
    };

    template<typename T>
    struct _referenceUnwrapper<std::reference_wrapper<T>> {
        using type = T&;
    };

    template<typename T>
    struct _decayUnwrap {
        using type = typename _referenceUnwrapper<std::decay_t<T> >::type;
    };

    template<typename Tp1, typename Tp2>
    struct _mergeTupleTypes {
        using type = void;
    };

    template<typename... T_rest, typename... U_rest>
    struct _mergeTupleTypes<Tuple<T_rest...>, Tuple<U_rest...> > {
        using type = Tuple<T_rest..., U_rest...>;
    };

    template<typename Tp1, typename Tp2>
    constexpr auto
    _mergeTwoTuples(Tp1&& first, Tp2&& other, typename std::enable_if_t<(std::decay_t<Tp1>::size() == 1)>* = 0) {
        typename _mergeTupleTypes<std::decay_t<Tp1>, std::decay_t<Tp2> >::type result;
        result._value = std::forward<typename std::remove_reference_t<Tp1>::value_type>(first._value);
        result._getParent() = std::forward<Tp2>(other);
        return result;
    }

    template<typename Tp1, typename Tp2, typename = std::enable_if_t<(std::decay_t<Tp1>::size() > 1)>>
    constexpr auto
    _mergeTwoTuples(Tp1&& first, Tp2&& other) {
        typename _mergeTupleTypes<std::decay_t<Tp1>, std::decay_t<Tp2> >::type result;
        result._value = std::forward<typename std::remove_reference_t<Tp1>::value_type>(first._value);
        using first_base = typename std::remove_reference_t<Tp1>::_base;
        result._getParent() = _mergeTwoTuples(std::forward<first_base>(first._getParent()), std::forward<Tp2>(other));
        return result;
    }
}

// makeTuple function
template<typename... U_rest>
constexpr auto makeTuple(U_rest&&... args) {
    using _tuple_type = Tuple<typename _implementation::_decayUnwrap<U_rest>::type...>;
    return _tuple_type(std::forward<U_rest>(args)...);
}

// get type function

template<typename T, typename T1, typename... T_rest>
constexpr typename std::enable_if_t<std::is_same<T, T1>::value, const T&>
get(const Tuple<T1, T_rest...>& tuple) {
    return tuple._value;
}

template<typename T, typename T1, typename... T_rest>
constexpr typename std::enable_if_t<!std::is_same<T, T1>::value, const T&>
get(const Tuple<T1, T_rest...>& tuple) {
    return get<T>(tuple._getParent());
}


template<typename T, typename T1, typename... T_rest>
constexpr typename std::enable_if_t<std::is_same<T, T1>::value, T&>
get(Tuple<T1, T_rest...>& tuple) {
    return tuple._value;
}

template<typename T, typename T1, typename... T_rest>
constexpr typename std::enable_if_t<!std::is_same<T, T1>::value, T&>
get(Tuple<T1, T_rest...>& tuple) {
    return get<T>(tuple._getParent());
}


template<typename T, typename T1, typename... T_rest>
constexpr typename std::enable_if_t<std::is_same<T, T1>::value, T&&>
get(Tuple<T1, T_rest...>&& tuple) {
    return std::move(tuple._value);
}

template<typename T, typename T1, typename... T_rest>
constexpr typename std::enable_if_t<!std::is_same<T, T1>::value, T&&>
get(Tuple<T1, T_rest...>&& tuple) {
    return get<T>(std::move(tuple._getParent()));
}

// tuple comparison

template<typename U1, typename... U1_rest, typename U2, typename... U2_rest>
constexpr bool operator<(const Tuple<U1, U1_rest...>& first, const Tuple<U2, U2_rest...>& other) {
    return _implementation::tupleComparator::_less(first, other);
}

template<typename U1, typename... U1_rest, typename U2, typename... U2_rest>
constexpr bool operator==(const Tuple<U1, U1_rest...>& first, const Tuple<U2, U2_rest...>& other) {
    return _implementation::tupleComparator::_equal(first, other);
}

template<typename U1, typename... U1_rest, typename U2, typename... U2_rest>
constexpr bool operator!=(const Tuple<U1, U1_rest...>& first, const Tuple<U2, U2_rest...>& other) {
    return !_implementation::tupleComparator::_equal(first, other);
}

template<typename U1, typename... U1_rest, typename U2, typename... U2_rest>
constexpr bool operator<=(const Tuple<U1, U1_rest...>& first, const Tuple<U2, U2_rest...>& other) {
    return _implementation::tupleComparator::_equal(first, other) || _implementation::tupleComparator::_less(first, other);
}

template<typename U1, typename... U1_rest, typename U2, typename... U2_rest>
constexpr bool operator>(const Tuple<U1, U1_rest...>& first, const Tuple<U2, U2_rest...>& other) {
    return !_implementation::tupleComparator::_equal(first, other) && !_implementation::tupleComparator::_less(first, other);
}

template<typename U1, typename... U1_rest, typename U2, typename... U2_rest>
constexpr bool operator>=(const Tuple<U1, U1_rest...>& first, const Tuple<U2, U2_rest...>& other) {
    return !_implementation::tupleComparator::_less(first, other);
}


// tuple concat
template<typename Tp1, typename Tp2, typename... Tp_rest>
constexpr auto tupleCat(Tp1&& first, Tp2&& second, Tp_rest&&... rest) {
    return _implementation::_mergeTwoTuples(std::forward<Tp1>(first), std::move(tupleCat(std::forward<Tp2>(second), std::forward<Tp_rest>(rest)...)));
}

template<typename Tp1, typename Tp2>
constexpr auto tupleCat(Tp1&& first, Tp2&& second) {
    return _implementation::_mergeTwoTuples(std::forward<Tp1>(first), std::forward<Tp2>(second));
}

// get idx function

template<int idx, typename T1, typename... T_rest,
        typename = std::enable_if_t<idx == 0> >
constexpr T1&
get(Tuple<T1, T_rest...>& tuple) {
    return tuple._value;
}

template<int idx, typename T1, typename... T_rest,
        typename = std::enable_if_t<idx != 0> >
constexpr decltype(auto)
get(Tuple<T1, T_rest...>& tuple) {
    return get<idx - 1, T_rest...>(tuple._getParent());
}


template<int idx, typename T1, typename... T_rest,
        typename = std::enable_if_t<idx == 0> >
constexpr const T1&
get(const Tuple<T1, T_rest...>& tuple) {
    return tuple._value;
}

template<int idx, typename T1, typename... T_rest,
        typename = std::enable_if_t<idx != 0> >
constexpr decltype(auto)
get(const Tuple<T1, T_rest...>& tuple) {
    return get<idx - 1, T_rest...>(tuple._getParent());
}


template<int idx, typename T1, typename... T_rest, class = typename std::enable_if_t<idx == 0> >
constexpr T1&&
get(Tuple<T1, T_rest...>&& tuple) {
    return std::move(tuple._value);
}

template<int idx, typename T1, typename... T_rest,
        typename = std::enable_if_t<idx != 0> >
constexpr decltype(auto)
get(Tuple<T1, T_rest...>&& tuple) {
    return get<idx - 1, T_rest...>(std::move(tuple._getParent()));
}

#endif //TUPLE_TUPLE_H