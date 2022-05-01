#pragma once

#include <type_traits>  //  std::index_sequence, std::conditional, std::declval
#include <tuple>  //  std::tuple

namespace sqlite_orm {
    namespace internal {

        template<typename... input_t>
        using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

        template<class... Args>
        struct conc_tuple {
            using type = tuple_cat_t<Args...>;
        };

        template<class... Args>
        using conc_tuple_t = typename conc_tuple<Args...>::type;

        template<class T, template<class... C> class F>
        struct tuple_filter;

#ifndef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
        template<class... Args, template<class... C> class F>
        struct tuple_filter<std::tuple<Args...>, F>
            : conc_tuple<std::conditional_t<F<Args>::value, std::tuple<Args>, std::tuple<>>...> {};
#else
        template<class T, template<class... C> class F, class SFINAE = void>
        struct tuple_filter_single;

        template<class T, template<class... C> class F>
        struct tuple_filter_single<T, F, std::enable_if_t<!F<T>::value>> {
            using type = std::tuple<>;
        };

        template<class T, template<class... C> class F>
        struct tuple_filter_single<T, F, std::enable_if_t<F<T>::value>> {
            using type = std::tuple<T>;
        };

        template<class... Args, template<class... C> class F>
        struct tuple_filter<std::tuple<Args...>, F> : conc_tuple<typename tuple_filter_single<Args, F>::type...> {};
#endif
        template<class Tpl, template<class... C> class F>
        using filter_tuple_t = typename tuple_filter<Tpl, F>::type;

        template<class... Seq>
        struct concat_idx_seq {
            using type = std::index_sequence<>;
        };

        template<size_t... Idx>
        struct concat_idx_seq<std::index_sequence<Idx...>> {
            using type = std::index_sequence<Idx...>;
        };

        template<size_t... As, size_t... Bs, class... Seq>
        struct concat_idx_seq<std::index_sequence<As...>, std::index_sequence<Bs...>, Seq...>
            : concat_idx_seq<std::index_sequence<As..., Bs...>, Seq...> {};

        template<class Tpl, template<class...> class F, class Seq>
        struct filter_tuple_sequence;

#ifndef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
        template<class Tpl, template<class...> class F, size_t... Idx>
        struct filter_tuple_sequence<Tpl, F, std::index_sequence<Idx...>>
            : concat_idx_seq<std::conditional_t<F<std::tuple_element_t<Idx, Tpl>>::value,
                                                std::index_sequence<Idx>,
                                                std::index_sequence<>>...> {};
#else
        template<size_t Idx, class T, template<class... C> class F, class SFINAE = void>
        struct tuple_seq_single;

        template<size_t Idx, class T, template<class... C> class F>
        struct tuple_seq_single<Idx, T, F, std::enable_if_t<!F<T>::value>> {
            using type = std::index_sequence<>;
        };

        template<size_t Idx, class T, template<class... C> class F>
        struct tuple_seq_single<Idx, T, F, std::enable_if_t<F<T>::value>> {
            using type = std::index_sequence<Idx>;
        };

        template<class... Args, template<class...> class F, size_t... Idx>
        struct filter_tuple_sequence<std::tuple<Args...>, F, std::index_sequence<Idx...>>
            : concat_idx_seq<typename tuple_seq_single<Idx, Args, F>::type...> {};
#endif

        template<class Tpl, template<class...> class F>
        using filter_tuple_sequence_t =
            typename filter_tuple_sequence<Tpl, F, std::make_index_sequence<std::tuple_size<Tpl>::value>>::type;

        template<class T, template<class...> class F>
        struct count_tuple {
            static constexpr int value = int(filter_tuple_sequence_t<T, F>::size());
        };
    }
}