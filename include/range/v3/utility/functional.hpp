/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//
#ifndef RANGES_V3_UTILITY_FUNCTIONAL_HPP
#define RANGES_V3_UTILITY_FUNCTIONAL_HPP

#include <memory> // std::addressof
#include <utility>
#include <functional> // std::reference_wrapper
#include <type_traits>
#include <initializer_list>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/utility/box.hpp>
#include <range/v3/utility/move.hpp>
#include <range/v3/utility/concepts.hpp>
#include <range/v3/utility/static_const.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \addtogroup group-utility
        /// @{
        struct equal_to
        {
            template<typename T, typename U,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(EqualityComparable<T, U>::value)>
#else
                CONCEPT_REQUIRES_(EqualityComparable<T, U>())>
#endif
            constexpr bool operator()(T && t, U && u) const
            {
                return (T &&) t == (U &&) u;
            }
        };

        struct less
        {
            template<typename T, typename U,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(WeaklyOrdered<T, U>::value)>
#else
                CONCEPT_REQUIRES_(WeaklyOrdered<T, U>())>
#endif
            constexpr bool operator()(T && t, U && u) const
            {
                return (T &&) t < (U &&) u;
            }
        };

        struct ordered_less
        {
            template<typename T, typename U,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(TotallyOrdered<T, U>::value)>
#else
                CONCEPT_REQUIRES_(TotallyOrdered<T, U>())>
#endif
            constexpr bool operator()(T && t, U && u) const
            {
                return (T &&) t < (U &&) u;
            }
        };

        struct ident
        {
            template<typename T>
            T && operator()(T && t) const noexcept
            {
                return (T &&) t;
            }
        };

        struct plus
        {
            template<typename T, typename U>
            auto operator()(T && t, U && u) const ->
                decltype((T &&) t + (U &&) u)
            {
                return (T &&) t + (U &&) u;
            }
        };

        struct minus
        {
            template<typename T, typename U>
            auto operator()(T && t, U && u) const ->
                decltype((T &&) t - (U &&) u)
            {
                return (T &&) t - (U &&) u;
            }
        };

        struct multiplies
        {
            template<typename T, typename U>
            auto operator()(T && t, U && u) const ->
                decltype((T &&) t * (U &&) u)
            {
                return (T &&) t * (U &&) u;
            }
        };

        struct bitwise_or
        {
            template<typename T, typename U>
            auto operator()(T && t, U && u) const ->
                decltype((T &&) t | (U &&) u)
            {
                return (T &&) t | (U &&) u;
            }
        };

        template<typename T>
        struct convert_to
        {
            template<typename U,
                CONCEPT_REQUIRES_(std::is_constructible<T, U>::value)>
            constexpr auto operator()(U && u) const
            RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT
            (
                static_cast<T>((U &&) u)
            )
        };

        template<typename T>
        struct coerce
        {
            T & operator()(T & t) const
            {
                return t;
            }
            /// \overload
            T const & operator()(T const & t) const
            {
                return t;
            }
            /// \overload
            T operator()(T && t) const
            {
                return (T &&) t;
            }
            T operator()(T const &&) const = delete;
        };

        template<typename T>
        struct coerce<T const>
          : coerce<T>
        {};

        template<typename T>
        struct coerce<T &>
          : coerce<T>
        {};

        template<typename T>
        struct coerce<T &&>
          : coerce<T>
        {};

        /// \addtogroup group-utility
        struct as_function_fn
        {
            template<typename T, typename U = detail::decay_t<T>,
                CONCEPT_REQUIRES_(std::is_member_pointer<U>::value)>
            auto operator()(T && t) const
            RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT
            (
                std::mem_fn(t)
            )
            template<typename T, typename U = detail::decay_t<T>,
                CONCEPT_REQUIRES_(!std::is_member_pointer<U>::value)>
            T operator()(T && t) const
                noexcept(std::is_nothrow_constructible<T, T>::value)
            {
                return std::forward<T>(t);
            }
        };

        /// \ingroup group-utility
        /// \sa `make_invokable_fn`
        namespace
        {
            constexpr auto&& as_function = static_const<as_function_fn>::value;
        }

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_ALIAS_DECLTYPE
        template <typename T>
        using function_type_void_t = void;
        template <class T, class V = void> struct function_type_helper {};
        template <class T> struct function_type_helper<T, function_type_void_t<decltype(as_function(std::declval<T>()))>> {
            typedef decltype(as_function(std::declval<T>())) type;
        };
        template<typename T>
        using function_type = typename function_type_helper<T>::type;
#else
        template<typename T>
        using function_type = decltype(as_function(std::declval<T>()));
#endif

        template<typename Pred>
        struct logical_negate
        {
        private:
            Pred pred_;
        public:
            logical_negate() = default;

            explicit constexpr logical_negate(Pred pred)
              : pred_((Pred &&) pred)
            {}

            template<typename T,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(Predicate<Pred, T>::value)>
#else
                CONCEPT_REQUIRES_(Predicate<Pred, T>())>
#endif
            bool operator()(T && t)
            {
                return !pred_((T &&) t);
            }
            /// \overload
            template<typename T,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(Predicate<Pred const, T>::value)>
#else
                CONCEPT_REQUIRES_(Predicate<Pred const, T>())>
#endif
            constexpr bool operator()(T && t) const
            {
                return !pred_((T &&) t);
            }
            /// \overload
            template<typename T, typename U,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(Predicate<Pred, T, U>::value)>
#else
                CONCEPT_REQUIRES_(Predicate<Pred, T, U>())>
#endif
            bool operator()(T && t, U && u)
            {
                return !pred_((T &&) t, (U &&) u);
            }
            /// \overload
            template<typename T, typename U,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(Predicate<Pred const, T, U>::value)>
#else
                CONCEPT_REQUIRES_(Predicate<Pred const, T, U>())>
#endif
            constexpr bool operator()(T && t, U && u) const
            {
                return !pred_((T &&) t, (U &&) u);
            }
        };

        struct not_fn
        {
            template<typename Pred>
            constexpr logical_negate<Pred> operator()(Pred pred) const
            {
                return logical_negate<Pred>{(Pred &&) pred};
            }
        };

        /// \ingroup group-utility
        /// \sa `not_fn`
        namespace
        {
            constexpr auto&& not_ = static_const<not_fn>::value;
        }

        template<typename Second, typename First>
        struct composed
          : private box<Second, meta::size_t<0>>
          , private box<First, meta::size_t<1>>
        {
            composed() = default;
            composed(Second second, First first)
              : box<Second, meta::size_t<0>>{std::move(second)}
              , box<First, meta::size_t<1>>{std::move(first)}
            {}
            template<typename...Ts>
            auto operator()(Ts &&...ts)
            RANGES_DECLTYPE_AUTO_RETURN(
                ranges::get<0>(*this)(ranges::get<1>(*this)((Ts &&)ts)...)
            )
            template<typename...Ts>
            auto operator()(Ts &&...ts) const
            RANGES_DECLTYPE_AUTO_RETURN(
                ranges::get<0>(*this)(ranges::get<1>(*this)((Ts &&)ts)...)
            )
        };

        struct compose_fn
        {
            template<typename Second, typename First>
            composed<Second, First> operator()(Second second, First first) const
            {
                return {std::move(second), std::move(first)};
            }
        };

        /// \ingroup group-utility
        /// \sa `compose_fn`
        namespace
        {
            constexpr auto&& compose = static_const<compose_fn>::value;
        }

        template<>
        struct overloaded<>
        {};

        template<typename Fn>
        struct overloaded<Fn>
          : private function_type<Fn>
        {
            overloaded() = default;
            constexpr overloaded(Fn fn)
              : function_type<Fn>(as_function(std::move(fn)))
            {}
            using function_type<Fn>::operator();
        };

        template<typename First, typename...Rest>
        struct overloaded<First, Rest...>
          : private overloaded<First>
          , private overloaded<Rest...>
        {
            overloaded() = default;
            constexpr overloaded(First first, Rest... rest)
              : overloaded<First>{detail::move(first)}
              , overloaded<Rest...>{detail::move(rest)...}
            {}
            using overloaded<First>::operator();
            using overloaded<Rest...>::operator();
        };

        struct overload_fn
        {
            template<typename ...Fns>
            constexpr overloaded<Fns...> operator()(Fns... fns) const
            {
                return overloaded<Fns...>{detail::move(fns)...};
            }
        };

        /// \ingroup group-utility
        /// \sa `overload_fn`
        namespace
        {
            constexpr auto&& overload = static_const<overload_fn>::value;
        }

        template<typename Fn>
        struct indirected
            : private function_type<Fn>
        {
        private:
            using BaseFn = function_type<Fn>;

            BaseFn & base()                { return *this; }
            BaseFn const & base() const    { return *this; }
        public:
            indirected() = default;
            indirected(Fn fn)
              : BaseFn(as_function(std::move(fn)))
            {}
            // value_type (needs no impl)
            template<typename ...Its>
            auto operator()(copy_tag, Its ...its) const ->
                detail::decay_t<decltype(std::declval<BaseFn &>()(*its...))>;
            // Reference
            template<typename ...Its>
            auto operator()(Its ...its)
                noexcept(noexcept(std::declval<BaseFn &>()(*its...))) ->
                decltype(std::declval<BaseFn &>()(*its...))
            {
                return base()(*its...);
            }
            template<typename ...Its>
            auto operator()(Its ...its) const
                noexcept(noexcept(std::declval<BaseFn const &>()(*its...))) ->
                decltype(std::declval<BaseFn const &>()(*its...))
            {
                return base()(*its...);
            }
            // Rvalue reference
            template<typename ...Its>
            auto operator()(move_tag, Its ...its)
                noexcept(noexcept(aux::move(std::declval<BaseFn &>()(*its...)))) ->
                aux::move_t<decltype(std::declval<BaseFn &>()(*its...))>
            {
                return aux::move(base()(*its...));
            }
            template<typename ...Its>
            auto operator()(move_tag, Its ...its) const
                noexcept(noexcept(aux::move(std::declval<BaseFn const &>()(*its...)))) ->
                aux::move_t<decltype(std::declval<BaseFn const &>()(*its...))>
            {
                return aux::move(base()(*its...));
            }
        };

        struct indirect_fn
        {
            template<typename Fn>
            constexpr indirected<Fn> operator()(Fn fn) const
            {
                return indirected<Fn>{detail::move(fn)};
            }
        };

        /// \ingroup group-utility
        /// \sa `indirect_fn`
        namespace
        {
            constexpr auto&& indirect = static_const<indirect_fn>::value;
        }

        /// \cond
        namespace detail
        {
            template<typename Bind>
            struct pipeable_binder
              : Bind
              , pipeable<pipeable_binder<Bind>>
            {
                pipeable_binder(Bind bind)
                  : Bind(std::move(bind))
                {}
            };

            template<typename Pipe0, typename Pipe1>
            struct composed_pipe
            {
                Pipe0 pipe0_;
                Pipe1 pipe1_;
                template<typename Arg>
                auto operator()(Arg && arg) const
                RANGES_DECLTYPE_AUTO_RETURN
                (
                    std::forward<Arg>(arg) | pipe0_ | pipe1_
                )
            };
        }
        /// \endcond

        struct make_pipeable_fn
        {
            template<typename Fun>
            detail::pipeable_binder<Fun> operator()(Fun fun) const
            {
                return {std::move(fun)};
            }
        };

        /// \ingroup group-utility
        /// \sa `make_pipeable_fn`
        namespace
        {
            constexpr auto&& make_pipeable = static_const<make_pipeable_fn>::value;
        }

        template<typename T,
            typename U = meta::if_<
                std::is_lvalue_reference<T>,
                std::reference_wrapper<meta::_t<std::remove_reference<T>>>,
                T &&>>
        U bind_forward(meta::_t<std::remove_reference<T>> & t) noexcept
        {
            return static_cast<U>(t);
        }

        template<typename T>
        T && bind_forward(meta::_t<std::remove_reference<T>> && t) noexcept
        {
            // This is to catch way sketchy stuff like: forward<int const &>(42)
            static_assert(!std::is_lvalue_reference<T>::value, "You didn't just do that!");
            return static_cast<T &&>(t);
        }

        struct pipeable_base
        {};

        template<typename T>
        struct is_pipeable
          : std::is_base_of<pipeable_base, T>
        {};

        template<typename T>
        struct is_pipeable<T &>
          : is_pipeable<T>
        {};

        struct pipeable_access
        {
            template<typename Pipeable>
            struct impl : Pipeable
            {
                using Pipeable::pipe;
            };

            template<typename Pipeable>
            struct impl<Pipeable &> : impl<Pipeable>
            {};
        };

        template<typename Derived>
        struct pipeable : pipeable_base
        {
        private:
            friend pipeable_access;
            // Default Pipe behavior just passes the argument to the pipe's function call
            // operator
            template<typename Arg, typename Pipe>
            static auto pipe(Arg && arg, Pipe pipe)
            RANGES_DECLTYPE_AUTO_RETURN
            (
                pipe(std::forward<Arg>(arg))
            )
        };

        // Evaluate the pipe with an argument
        template<typename Arg, typename Pipe,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            CONCEPT_REQUIRES_(!is_pipeable<Arg>::value && is_pipeable<Pipe>::value)>
#else
            CONCEPT_REQUIRES_(!is_pipeable<Arg>() && is_pipeable<Pipe>())>
#endif
        auto operator|(Arg && arg, Pipe pipe)
        RANGES_DECLTYPE_AUTO_RETURN
        (
            pipeable_access::impl<Pipe>::pipe(std::forward<Arg>(arg), pipe)
        )

        // Compose two pipes
        template<typename Pipe0, typename Pipe1,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            CONCEPT_REQUIRES_(is_pipeable<Pipe0>::value && is_pipeable<Pipe1>::value)>
#else
            CONCEPT_REQUIRES_(is_pipeable<Pipe0>() && is_pipeable<Pipe1>())>
#endif
        auto operator|(Pipe0 pipe0, Pipe1 pipe1)
        RANGES_DECLTYPE_AUTO_RETURN
        (
            make_pipeable(detail::composed_pipe<Pipe0, Pipe1>{pipe0, pipe1})
        )

        template<typename T, bool RValue /* = false*/>
        struct reference_wrapper
        {
        private:
            T *t_;
        public:
            using type = T;
            using reference = meta::if_c<RValue, T &&, T &>;
            reference_wrapper() = default;
            reference_wrapper(reference t) noexcept
              : t_(std::addressof(t))
            {}
            reference get() const noexcept
            {
                RANGES_ASSERT(nullptr != t_);
                return static_cast<reference>(*t_);
            }
            operator reference() const noexcept
            {
                return get();
            }
            CONCEPT_REQUIRES(!RValue)
            operator std::reference_wrapper<T> () const noexcept
            {
                return {get()};
            }
            template<typename ...Args>
            auto operator()(Args &&...args) const ->
                decltype(std::declval<reference>()(std::declval<Args>()...))
            {
                return get()(std::forward<Args>(args)...);
            }
        };

        template<typename T>
        struct is_reference_wrapper
          : meta::if_<
                std::is_same<uncvref_t<T>, T>,
                std::false_type,
                is_reference_wrapper<uncvref_t<T>>>
        {};

        template<typename T, bool RValue>
        struct is_reference_wrapper<reference_wrapper<T, RValue>>
          : std::true_type
        {};

        template<typename T>
        struct is_reference_wrapper<std::reference_wrapper<T>>
          : std::true_type
        {};

        template<typename T>
        using is_reference_wrapper_t = meta::_t<is_reference_wrapper<T>>;

        template<typename T>
        struct referent_of
        {};

        template<typename T, bool RValue>
        struct referent_of<reference_wrapper<T, RValue>>
        {
            using type = T;
        };

        template<typename T>
        struct referent_of<std::reference_wrapper<T>>
        {
            using type = T;
        };

        template<typename T>
        struct referent_of<T &>
          : meta::if_<is_reference_wrapper<T>, referent_of<T>, meta::id<T>>
        {};

        template<typename T>
        struct referent_of<T &&>
          : meta::if_<is_reference_wrapper<T>, referent_of<T>, meta::id<T>>
        {};

        template<typename T>
        struct referent_of<T const>
          : referent_of<T>
        {};

        template<typename T>
        using referent_of_t = meta::_t<referent_of<T>>;

        template<typename T>
        struct reference_of
        {};

        template<typename T, bool RValue>
        struct reference_of<reference_wrapper<T, RValue>>
        {
            using type = meta::if_c<RValue, T &&, T &>;
        };

        template<typename T>
        struct reference_of<std::reference_wrapper<T>>
        {
            using type = T &;
        };

        template<typename T>
        struct reference_of<T &>
          : meta::if_<is_reference_wrapper<T>, reference_of<T>, meta::id<T &>>
        {};

        template<typename T>
        struct reference_of<T &&>
          : meta::if_<is_reference_wrapper<T>, reference_of<T>, meta::id<T &&>>
        {};

        template<typename T>
        struct reference_of<T const>
          : reference_of<T>
        {};

        template<typename T>
        using reference_of_t = meta::_t<reference_of<T>>;

        template<typename T>
        struct bind_element
          : meta::if_<
                std::is_same<detail::decay_t<T>, T>,
                meta::id<T>,
                bind_element<detail::decay_t<T>>>
        {};

        template<typename T>
        struct bind_element<std::reference_wrapper<T>>
        {
            using type = T &;
        };

        template<typename T, bool RValue>
        struct bind_element<reference_wrapper<T, RValue>>
        {
            using type = meta::if_c<RValue, T &&, T &>;
        };

        template<typename T>
        using bind_element_t = meta::_t<bind_element<T>>;

        struct ref_fn : pipeable<ref_fn>
        {
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename T, CONCEPT_REQUIRES_(!is_reference_wrapper_t<T>::value)>
#else
            template<typename T, CONCEPT_REQUIRES_(!is_reference_wrapper_t<T>())>
#endif
            reference_wrapper<T> operator()(T & t) const
            {
                return {t};
            }
            /// \overload
            template<typename T, bool RValue>
            reference_wrapper<T, RValue> operator()(reference_wrapper<T, RValue> t) const
            {
                return t;
            }
            /// \overload
            template<typename T>
            reference_wrapper<T> operator()(std::reference_wrapper<T> t) const
            {
                return {t.get()};
            }
        };

        /// \ingroup group-utility
        /// \sa `ref_fn`
        namespace
        {
            constexpr auto&& ref = static_const<ref_fn>::value;
        }

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_ALIAS_DECLTYPE
        template <typename T>
        using ref_t_void_t = void;
        template <class T, class V = void> struct ref_t_helper {};
        template <class T> struct ref_t_helper<T, ref_t_void_t<decltype(ref(std::declval<T>()))>> {
            typedef decltype(ref(std::declval<T>())) type;
        };
        template<typename T>
        using ref_t = typename ref_t_helper<T>::type;
#else
        template<typename T>
        using ref_t = decltype(ref(std::declval<T>()));
#endif

        struct rref_fn : pipeable<rref_fn>
        {
            template<typename T,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(!is_reference_wrapper_t<T>::value &&
                    !std::is_lvalue_reference<T>::value)>
#else
                CONCEPT_REQUIRES_(!is_reference_wrapper_t<T>() &&
                    !std::is_lvalue_reference<T>::value)>
#endif
            reference_wrapper<T, true> operator()(T && t) const
            {
                return {std::move(t)};
            }
            /// \overload
            template<typename T>
            reference_wrapper<T, true> operator()(reference_wrapper<T, true> t) const
            {
                return t;
            }
        };

        /// \ingroup group-utility
        /// \sa `rref_fn`
        namespace
        {
            constexpr auto&& rref = static_const<rref_fn>::value;
        }

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_ALIAS_DECLTYPE
        template <typename T>
        using rref_t_void_t = void;
        template <class T, class V = void> struct rref_t_helper {};
        template <class T> struct rref_t_helper<T, rref_t_void_t<decltype(rref(std::declval<T>()))>> {
            typedef decltype(rref(std::declval<T>())) type;
        };
        template<typename T>
        using rref_t = typename rref_t_helper<T>::type;
#else
        template<typename T>
        using rref_t = decltype(rref(std::declval<T>()));
#endif

        struct unwrap_reference_fn : pipeable<unwrap_reference_fn>
        {
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename T, CONCEPT_REQUIRES_(!is_reference_wrapper<T>::value)>
#else
            template<typename T, CONCEPT_REQUIRES_(!is_reference_wrapper<T>())>
#endif
            T && operator()(T && t) const noexcept
            {
                return std::forward<T>(t);
            }
            /// \overload
            template<typename T, bool RValue>
            meta::if_c<RValue, T &&, T &> operator()(reference_wrapper<T, RValue> t) const noexcept
            {
                return t.get();
            }
            /// \overload
            template<typename T>
            T & operator()(std::reference_wrapper<T> t) const noexcept
            {
                return t.get();
            }
        };

        /// \ingroup group-utility
        /// \sa `unwrap_reference_fn`
        namespace
        {
            constexpr auto&& unwrap_reference = static_const<unwrap_reference_fn>::value;
        }

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_ALIAS_DECLTYPE
        template <typename T>
        using unwrap_reference_t_void_t = void;
        template <class T, class V = void> struct unwrap_reference_t_helper {};
        template <class T> struct unwrap_reference_t_helper<T, unwrap_reference_t_void_t<decltype(unwrap_reference(std::declval<T>()))>> {
            typedef decltype(unwrap_reference(std::declval<T>())) type;
        };
        template<typename T>
        using unwrap_reference_t = typename unwrap_reference_t_helper<T>::type;
#else
        template<typename T>
        using unwrap_reference_t = decltype(unwrap_reference(std::declval<T>()));
#endif

        /// \ingroup group-utility
        template<typename T>
        using forward_ref_t =
            meta::if_<
                std::is_reference<T>,
                reference_wrapper<
                    meta::_t<std::remove_reference<T>>,
                    std::is_rvalue_reference<T>::value>,
                T &&>;

        /// Forward arguments, wrapping lvalue and rvalue refs in ranges::reference_wrapper.
        /// \ingroup group-utility
        template<typename T, typename U>
        forward_ref_t<T> forward_ref(U && t) noexcept
        {
            // This is to catch way sketchy stuff like: forward<int const &>(42)
            //static_assert(std::is_same<T &, U>::value, "You didn't just do that!");
            return static_cast<forward_ref_t<T>>((T &&) t);
        }

        /// \cond
        namespace detail
        {
            template<typename F>
            struct unwrap_args_fn
              : private box<F, meta::size_t<0>>
            {
                using expects_wrapped_references = void;
                unwrap_args_fn() = default;
                explicit unwrap_args_fn(F f)
                  : box<F, meta::size_t<0>>{std::move(f)}
                {}
                template<typename...Args>
                auto operator()(Args &&...args) ->
                    decltype(std::declval<F &>()(unwrap_reference(std::forward<Args>(args))...))
                {
                    return ranges::get<0>(*this)(unwrap_reference(std::forward<Args>(args))...);
                }
                template<typename...Args>
                auto operator()(Args &&...args) const ->
                    decltype(std::declval<F const &>()(unwrap_reference(std::forward<Args>(args))...))
                {
                    return ranges::get<0>(*this)(unwrap_reference(std::forward<Args>(args))...);
                }
            };

            template<typename T, typename Enable = void>
            struct expects_wrapped_references_
              : std::false_type
            {};

            template<typename T>
            struct expects_wrapped_references_<T, meta::void_<typename T::expects_wrapped_references>>
              : std::true_type
            {};
        }
        /// \endcond

        template<typename T>
        struct expects_wrapped_references
          : detail::expects_wrapped_references_<uncvref_t<T>>
        {};

        struct make_unwrap_args_fn
        {
            template<typename F, CONCEPT_REQUIRES_(!expects_wrapped_references<F>::value)>
            detail::unwrap_args_fn<F> operator()(F f) const
            {
                return detail::unwrap_args_fn<F>{std::move(f)};
            }
            template<typename F, CONCEPT_REQUIRES_(expects_wrapped_references<F>::value)>
            F operator()(F f) const
            {
                return std::move(f);
            }
        };

        /// \ingroup group-utility
        /// \sa `make_unwrap_args_fn`
        namespace
        {
            constexpr auto&& make_unwrap_args = static_const<make_unwrap_args_fn>::value;
        }

        template<typename F>
        using unwrap_args_t = decltype(make_unwrap_args(std::declval<F>()));

        /// \cond
        namespace detail
        {
            template<typename Bind>
            struct protect
            {
            private:
                Bind bind_;
            public:
                protect() = default;
                protect(Bind b)
                  : bind_(std::move(b))
                {}
                template<typename...Ts>
                auto operator()(Ts &&...ts)
                RANGES_DECLTYPE_AUTO_RETURN
                (
                    bind_(std::forward<Ts>(ts)...)
                )
                /// \overload
                template<typename...Ts>
                auto operator()(Ts &&...ts) const
                RANGES_DECLTYPE_AUTO_RETURN
                (
                    bind_(std::forward<Ts>(ts)...)
                )
            };
        }
        /// \endcond

        struct protect_fn
        {
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename F, CONCEPT_REQUIRES_(std::is_bind_expression<uncvref_t<F>>::value)>
#else
            template<typename F, CONCEPT_REQUIRES_(std::is_bind_expression<uncvref_t<F>>())>
#endif
            detail::protect<uncvref_t<F>> operator()(F && f) const
            {
                return {std::forward<F>(f)};
            }
            /// \overload

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename F, CONCEPT_REQUIRES_(!std::is_bind_expression<uncvref_t<F>>::value)>
#else
            template<typename F, CONCEPT_REQUIRES_(!std::is_bind_expression<uncvref_t<F>>())>
#endif
            F operator()(F && f) const
            {
                return std::forward<F>(f);
            }
        };

        /// Protect a callable so that it can be safely used in a bind expression without
        /// accidentally becoming a "nested" bind.
        /// \ingroup group-utility
        /// \sa `protect_fn`
        namespace
        {
            constexpr auto&& protect = static_const<protect_fn>::value;
        }

        // Accepts initializer_lists as either the first or second parameter, or both,
        // and forwards on to an implementation.
        template<typename ImplFn>
        struct with_braced_init_args
          : ImplFn
        {
        private:
            ImplFn const & base() const
            {
                return *this;
            }
        public:
            using ImplFn::operator();

            template<typename V0, typename...Args>
            auto operator()(std::initializer_list<V0> &&rng0, Args &&...args) const ->
                decltype(std::declval<ImplFn const &>()(std::move(rng0), std::declval<Args>()...))
            {
                return base()(std::move(rng0), std::forward<Args>(args)...);
            }
            /// \overload
            template<typename Rng0, typename V1, typename...Args>
            auto operator()(Rng0 && rng0, std::initializer_list<V1> &&rng1, Args &&...args) const ->
                decltype(std::declval<ImplFn const &>()(std::declval<Rng0>(), std::move(rng1), std::declval<Args>()...))
            {
                return base()(std::forward<Rng0>(rng0), std::move(rng1), std::forward<Args>(args)...);
            }
            /// \overload
            template<typename V0, typename V1, typename...Args>
            auto operator()(std::initializer_list<V0> rng0, std::initializer_list<V1> &&rng1, Args &&...args) const ->
                decltype(std::declval<ImplFn const &>()(std::move(rng0), std::move(rng1), std::declval<Args>()...))
            {
                return base()(std::move(rng0), std::move(rng1), std::forward<Args>(args)...);
            }
        };
        /// @}

        /// \addtogroup group-concepts
        /// @{
        namespace concepts
        {
            struct Callable
            {
                template<typename Fun, typename...Args>
                using result_t = Function::result_t<function_type<Fun>, Args...>;

                template<typename Fun, typename...Args>
                auto requires_(Fun&&, Args&&...) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Function, function_type<Fun>, Args...>()
                    ));
            };

            struct RegularCallable
              : refines<Callable>
            {};

            struct CallablePredicate
              : refines<RegularCallable>
            {
                template<typename Fun, typename...Args>
                auto requires_(Fun&&, Args&&...) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Predicate, function_type<Fun>, Args...>()
                    ));
            };

            struct CallableRelation
              : refines<CallablePredicate>
            {
                template<typename Fun, typename T, typename U>
                auto requires_(Fun&&, T&&, U&&) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Relation, function_type<Fun>, T, U>()
                    ));
            };

            struct CallableTransform
              : refines<RegularCallable>
            {
                template<typename Fun, typename T>
                auto requires_(Fun&&, T&&) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Transform, function_type<Fun>, T>()
                    ));
            };
        }

        template<typename Fun, typename...Args>
        using Callable = concepts::models<concepts::Callable, Fun, Args...>;

        template<typename Fun, typename...Args>
        using RegularCallable = concepts::models<concepts::RegularCallable, Fun, Args...>;

        template<typename Fun, typename...Args>
        using CallablePredicate = concepts::models<concepts::CallablePredicate, Fun, Args...>;

        template<typename Fun, typename T, typename U = T>
        using CallableRelation = concepts::models<concepts::CallableRelation, Fun, T, U>;

        template<typename F, typename T>
        using CallableTransform = concepts::models<concepts::CallableTransform, F, T>;
        /// @}
    }
}

#endif
