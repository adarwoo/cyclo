#ifndef BOOST_TOKEN_FUNCTIONS_JRB120303_HPP_
#define BOOST_TOKEN_FUNCTIONS_JRB120303_HPP_

#include <string.h>
#include <ctype.h>

#include <etl/string.h>
#include <etl/iterator.h>
#include <etl/char_traits.h>
#include <etl/basic_string.h>
#include <etl/string_view.h>

namespace etl
{
    namespace tokenizer_detail
    {
        //===========================================================================
        // Tokenizer was broken for wide character separators, at least on Windows, since
        // CRT functions isspace etc only expect values in [0, 0xFF]. Debug build asserts
        // if higher values are passed in. The traits extension class should take care of this.
        // Assuming that the conditional will always get optimized out in the function
        // implementations, argument types are not a problem since both forms of character classifiers
        // expect an int.

        // In case there is no cwctype header, we implement the checks manually.
        // We make use of the fact that the tested categories should fit in ASCII.
        template <typename traits>
        struct traits_extension : public traits
        {
            typedef typename traits::char_type char_type;

            static bool isspace(char_type c)
            {
                return static_cast<unsigned>(c) <= 255 && isspace(c) != 0;
            }

            static bool ispunct(char_type c)
            {
                return static_cast<unsigned>(c) <= 255 && ispunct(c) != 0;
            }
        };

        // The assign_or_plus_equal struct contains functions that implement
        // assign, +=, and clearing based on the iterator type.  The
        // generic case does nothing for plus_equal and clearing, while
        // passing through the call for assign.
        //
        // When an input iterator is being used, the situation is reversed.
        // The assign method does nothing, plus_equal invokes operator +=,
        // and the clearing method sets the supplied token to the default
        // token constructor's result.
        //

        template <class IteratorTag>
        struct assign_or_plus_equal
        {
            template <class Iterator, class Token>
            static void assign(Iterator b, Iterator e, Token &t)
            {
                t.assign(b, e);
            }

            template <class Token, class Value>
            static void plus_equal(Token &, const Value &) {}

            // If we are doing an assign, there is no need for the
            // the clear.
            //
            template <class Token>
            static void clear(Token &) {}
        };

        template <>
        struct assign_or_plus_equal<etl::input_iterator_tag>
        {
            template <class Iterator, class Token>
            static void assign(Iterator, Iterator, Token &) {}
            
            template <class Token, class Value>
            static void plus_equal(Token &t, const Value &v)
            {
                t += v;
            }

            template <class Token>
            static void clear(Token &t)
            {
                t = Token();
            }
        };

        template <class Iterator>
        struct pointer_iterator_category
        {
            using type = etl::random_access_iterator_tag;
        };

        template <class Iterator>
        struct class_iterator_category
        {
            using type = typename Iterator::iterator_category;
        };

        // This portably gets the iterator_tag without partial template specialization
        template <class Iterator>
        struct get_iterator_category
        {
            using iterator_category = typename conditional<
                is_pointer<Iterator>::value,
                pointer_iterator_category<Iterator>,
                class_iterator_category<Iterator>>::type;
        };
    } // namespace tokenizer_detail

    //===========================================================================
    // The char_separator class breaks a sequence of characters into
    // tokens based on the character delimiters (very much like bad old
    // strtok). A delimiter character can either be kept or dropped. A
    // kept delimiter shows up as an output token, whereas a dropped
    // delimiter does not.

    // This class replaces the char_delimiters_separator class. The
    // constructor for the char_delimiters_separator class was too
    // confusing and needed to be deprecated. However, because of the
    // default arguments to the constructor, adding the new constructor
    // would cause ambiguity, so instead I deprecated the whole class.
    // The implementation of the class was also simplified considerably.

    enum empty_token_policy : uint_least8_t
    {
        drop_empty_tokens,
        keep_empty_tokens
    };

    // The out of the box GCC 2.95 on cygwin does not have a char_traits class.
    template <typename Char, class Tr = etl::char_traits_types<Char>>
    class char_separator
    {
        typedef tokenizer_detail::traits_extension<Tr> Traits;
        typedef etl::basic_string_view<Char> string_view_type;

    public:
        explicit char_separator(const Char *dropped_delims,
                                const Char *kept_delims = 0,
                                empty_token_policy empty_tokens = drop_empty_tokens)
            : m_dropped_delims(dropped_delims),
              m_use_ispunct(false),
              m_use_isspace(false),
              m_empty_tokens(empty_tokens),
              m_output_done(false)
        {
        }

        // use ispunct() for kept delimiters and isspace for dropped.
        explicit char_separator()
            : m_use_ispunct(true),
              m_use_isspace(true),
              m_empty_tokens(drop_empty_tokens),
              m_output_done(false)
        {}

        ~char_separator(){};

        void reset() {}

        template <typename InputIterator, typename Token>
        bool operator()(InputIterator &next, InputIterator end, Token &tok)
        {
            using assigner_category = typename tokenizer_detail::get_iterator_category<InputIterator>::iterator_category;
            using assigner = tokenizer_detail::assign_or_plus_equal<assigner_category>;

            assigner::clear(tok);

            // skip past all dropped_delims
            if (m_empty_tokens == drop_empty_tokens)
                for (; next != end && is_dropped(*next); ++next)
                {
                }

            InputIterator start(next);

            if (m_empty_tokens == drop_empty_tokens)
            {

                if (next == end)
                    return false;

                // if we are on a kept_delims move past it and stop
                if (is_kept(*next))
                {
                    assigner::plus_equal(tok, *next);
                    ++next;
                }
                else
                    // append all the non delim characters
                    for (; next != end && !is_dropped(*next) && !is_kept(*next); ++next)
                        assigner::plus_equal(tok, *next);
            }
            else
            { // m_empty_tokens == keep_empty_tokens
                // Handle empty token at the end
                if (next == end)
                {
                    if (m_output_done == false)
                    {
                        m_output_done = true;
                        assigner::assign(start, next, tok);
                        return true;
                    }
                    else
                        return false;
                }

                if (is_kept(*next))
                {
                    if (m_output_done == false)
                        m_output_done = true;
                    else
                    {
                        assigner::plus_equal(tok, *next);
                        ++next;
                        m_output_done = false;
                    }
                }
                else if (m_output_done == false && is_dropped(*next))
                {
                    m_output_done = true;
                }
                else
                {
                    if (is_dropped(*next))
                        start = ++next;
                    for (; next != end && !is_dropped(*next) && !is_kept(*next); ++next)
                        assigner::plus_equal(tok, *next);
                    m_output_done = true;
                }
            }
            assigner::assign(start, next, tok);
            return true;
        }

    private:
        string_view_type m_kept_delims;
        string_view_type m_dropped_delims;
        bool m_use_ispunct;
        bool m_use_isspace;
        empty_token_policy m_empty_tokens;
        bool m_output_done;

        bool is_kept(Char E) const
        {
            if (m_kept_delims.length())
                return m_kept_delims.find(E) != string_view_type::npos;

            else if (m_use_ispunct)
            {
                return Traits::ispunct(E) != 0;
            }
            else
                return false;
        }

        bool is_dropped(Char E) const
        {
            if (m_dropped_delims.length())
                return m_dropped_delims.find(E) != string_view_type::npos;
            else if (m_use_isspace)
            {
                return Traits::isspace(E) != 0;
            }
            else
                return false;
        }
    };
} // namespace etl

#endif
