// Boost token_iterator.hpp  -------------------------------------------------//

// Copyright John R. Bandela 2001
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/tokenizer for documentation.

// Revision History:
// 16 Jul 2003   John Bandela
//      Allowed conversions from convertible base iterators
// 03 Jul 2003   John Bandela
//      Converted to new iterator adapter

#ifndef BOOST_TOKENIZER_POLICY_JRB070303_HPP_
#define BOOST_TOKENIZER_POLICY_JRB070303_HPP_

#include <assert.h>
#include <etl/iterator.h>
#include <etl/token_functions.h>
#include <etl/utility.h>
#include <etl/type_traits.h>

namespace etl
{
    namespace detail
    {

        //
        // Result type used in enable_if_convertible meta function.
        // This can be an incomplete type, as only pointers to
        // enable_if_convertible< ... >::type are used.
        // We could have used void for this, but conversion to
        // void* is just to easy.
        //
        struct enable_type;
    } // namespace detail

    template <typename From, typename To>
    struct enable_if_convertible
        : etl::enable_if<
              etl::is_convertible_v<From, To>, etl::detail::enable_type>
    {
    };

    template <class TokenizerFunc, class Iterator, class Type>
    class token_iterator
    {
        TokenizerFunc f_;
        Iterator begin_;
        Iterator end_;
        bool valid_;
        Type tok_;

        void increment()
        {
            assert(valid_);
            valid_ = f_(begin_, end_, tok_);
        }

        const Type &dereference() const
        {
            assert(valid_);
            return tok_;
        }

        template <class Other>
        bool equal(const Other &a) const
        {
            return (a.valid_ && valid_)
                       ? ((a.begin_ == begin_) && (a.end_ == end_))
                       : (a.valid_ == valid_);
        }

        void initialize()
        {
            if (valid_)
                return;

            f_.reset();

            valid_ = (begin_ != end_) ? f_(begin_, end_, tok_) : false;
        }

    public:
        token_iterator() : begin_(), end_(), valid_(false), tok_() {}

        token_iterator(TokenizerFunc f, Iterator begin, Iterator e = Iterator())
            : f_(f), begin_(begin), end_(e), valid_(false), tok_() { initialize(); }

        token_iterator(Iterator begin, Iterator e = Iterator())
            : f_(), begin_(begin), end_(e), valid_(false), tok_() { initialize(); }

        template <class OtherIter>
        token_iterator(
            token_iterator<TokenizerFunc, OtherIter, Type> const &t, 
            typename enable_if_convertible<OtherIter, Iterator>::type * = 0) : 
                f_(t.tokenizer_function()), 
                begin_(t.base()),
                end_(t.end()),
                valid_(!t.at_end()), 
                tok_(t.current_token()) {}

        ~token_iterator() {}

        Iterator base() const { return begin_; }

        Iterator end() const { return end_; }

        TokenizerFunc tokenizer_function() const { return f_; }

        Type current_token() const { return tok_; }

        bool at_end() const { return !valid_; }

        bool operator!=(const token_iterator &) const
        {
            return valid_;
        }

        Type operator*() const
        {
            return tok_;
        }

        Type *operator->() const
        {
            return &(*tok_);
        }

        Type &operator++()
        {
            this->increment();
            return tok_;
        }
    };
} // namespace etl

#endif
