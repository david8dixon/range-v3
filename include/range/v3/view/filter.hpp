// Boost.Range library
//
//  Copyright Thorsten Ottosen, Neil Groves 2006 - 2008.
//  Copyright Eric Niebler 2013.
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef RANGES_V3_VIEW_FILTER_HPP
#define RANGES_V3_VIEW_FILTER_HPP

#include <utility>
#include <type_traits>
#include <range/v3/range_fwd.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/range_facade.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/utility/bindable.hpp>
#include <range/v3/utility/invokable.hpp>

namespace ranges
{
    inline namespace v3
    {
        template<typename InputIterable, typename UnaryPredicate>
        struct filter_iterable_view
          : range_adaptor<
                filter_iterable_view<InputIterable, UnaryPredicate>,
                InputIterable>
        {
        private:
            friend range_core_access;
            using base_cursor_t = base_cursor_t<filter_iterable_view>;
            invokable_t<UnaryPredicate> pred_;

            struct adaptor : adaptor_defaults
            {
            private:
                filter_iterable_view const *rng_;
                void satisfy(base_cursor_t &pos) const
                {
                    auto const e = rng_->base_end();
                    while (!e.equal(pos) && !rng_->pred_(pos.current()))
                        pos.next();
                }
            public:
                adaptor() = default;
                adaptor(filter_iterable_view const &rng)
                  : rng_(&rng)
                {}
                base_cursor_t begin(filter_iterable_view const &rng) const
                {
                    auto pos = adaptor_defaults::begin(rng);
                    this->satisfy(pos);
                    return pos;
                }
                void next(base_cursor_t &pos) const
                {
                    pos.next();
                    this->satisfy(pos);
                }
                CONCEPT_REQUIRES(BidirectionalIterator<range_iterator_t<InputIterable>>())
                void prev(base_cursor_t &pos) const
                {
                    do
                    {
                        pos.prev();
                    } while (!rng_->pred_(pos.current()));
                }
            };
            // TODO: if end is a sentinel, it hold an unnecessary pointer back to
            // this range.
            adaptor get_adaptor(begin_end_tag) const
            {
                return {*this};
            }
        public:
            filter_iterable_view(InputIterable && rng, UnaryPredicate pred)
              : range_adaptor_t<filter_iterable_view>(std::forward<InputIterable>(rng))
              , pred_(ranges::make_invokable(std::move(pred)))
            {}
        };

        namespace view
        {
            struct filterer : bindable<filterer>
            {
            private:
                template<typename UnaryPredicate>
                struct filterer1 : pipeable<filterer1<UnaryPredicate>>
                {
                private:
                    UnaryPredicate pred_;
                public:
                    filterer1(UnaryPredicate pred)
                      : pred_(std::move(pred))
                    {}
                    template<typename InputIterable, typename This>
                    static filter_iterable_view<InputIterable, UnaryPredicate> pipe(InputIterable && rng, This && this_)
                    {
                        return {std::forward<InputIterable>(rng), std::forward<This>(this_).pred_};
                    }
                };
            public:
                template<typename InputIterable, typename UnaryPredicate>
                static filter_iterable_view<InputIterable, UnaryPredicate>
                invoke(filterer, InputIterable && rng, UnaryPredicate pred)
                {
                    return {std::forward<InputIterable>(rng), std::move(pred)};
                }
                template<typename UnaryPredicate>
                static filterer1<UnaryPredicate> invoke(filterer, UnaryPredicate pred)
                {
                    return {std::move(pred)};
                }
            };

            RANGES_CONSTEXPR filterer filter {};
        }
    }
}

#endif
