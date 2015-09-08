/*
 * vergesort.h - General-purpose hybrid sort
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Morwenn <morwenn29@hotmail.fr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef VERGESORT_H_
#define VERGESORT_H_

#include <algorithm>
#include <functional>
#include <iterator>
#include "pdqsort.h"

namespace vergesort_detail
{
    // Inplace merge where [first, middle1), [middle1, middle2)
    // and [middle2, last) are sorted. The two inplace merges are
    // done in the order that should result in the smallest number
    // of comparisons
    template<typename RandomAccessIterator, typename Compare>
    inline void inplace_merge3(RandomAccessIterator first,
                               RandomAccessIterator middle1,
                               RandomAccessIterator middle2,
                               RandomAccessIterator last,
                               Compare compare)
    {
        if (std::distance(first, middle1) < std::distance(middle2, last))
        {
            std::inplace_merge(first, middle1, middle2, compare);
            std::inplace_merge(first, middle2, last, compare);
        }
        else
        {
            std::inplace_merge(middle1, middle2, last, compare);
            std::inplace_merge(first, middle1, last, compare);
        }
    }

    // C++03 implementation of std::is_sorted_until
    template<typename ForwardIterator, typename Compare>
    ForwardIterator is_sorted_until(ForwardIterator first, ForwardIterator last, Compare compare)
    {
        if (first != last)
        {
            ForwardIterator next = first;
            while (++next != last)
            {
                if (compare(*next, *first))
                {
                    return next;
                }
                first = next;
            }
        }
        return last;
    }
}

template<typename RandomAccessIterator, typename Compare>
inline void vergesort(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type difference_type;
    difference_type dist = std::distance(first, last);

    if (dist < 80)
    {
        // vergesort is inefficient for small collections
        pdqsort(first, last, compare);
        return;
    }

    // Limit under which pdqsort is used
    int unstable_limit = dist / pdqsort_detail::log2(dist);

    // Beginning of an unstable partition, last if the
    // previous partition is stable
    RandomAccessIterator begin_unstable = last;

    // Pair of iterators to iterate through the collection
    RandomAccessIterator current = is_sorted_until(first, last, compare) - 1;
    RandomAccessIterator next = current + 1;

    while (true)
    {
        RandomAccessIterator begin_rng = current;

        // Decreasing range
        while (next != last)
        {
            if (compare(*current, *next)) break;
            ++current;
            ++next;
        }

        // Reverse and merge
        if (std::distance(begin_rng, next) > unstable_limit)
        {
            if (begin_unstable != last)
            {
                pdqsort(begin_unstable, begin_rng, compare);
                std::reverse(begin_rng, next);
                vergesort_detail::inplace_merge3(first, begin_unstable, begin_rng, next, compare);
                begin_unstable = last;
            }
            else
            {
                std::reverse(begin_rng, next);
                std::inplace_merge(first, begin_rng, next, compare);
            }
        }
        else
        {
            if (begin_unstable == last) begin_unstable = begin_rng;
        }

        if (next == last) break;

        ++current;
        ++next;

        begin_rng = current;

        // Increasing range
        while (next != last)
        {
            if (compare(*next, *current)) break;
            ++current;
            ++next;
        }

        // Merge
        if (std::distance(begin_rng, next) > unstable_limit)
        {
            if (begin_unstable != last)
            {
                pdqsort(begin_unstable, begin_rng, compare);
                vergesort_detail::inplace_merge3(first, begin_unstable, begin_rng, next, compare);
                begin_unstable = last;
            }
            else
            {
                std::inplace_merge(first, begin_rng, next, compare);
            }
        }
        else
        {
            if (begin_unstable == last) begin_unstable = begin_rng;
        }

        if (next == last) break;

        ++current;
        ++next;
    }

    if (begin_unstable != last)
    {
        pdqsort(begin_unstable, last, compare);
        std::inplace_merge(first, begin_unstable, last, compare);
    }
}

template<typename RandomAccessIterator>
inline void vergesort(RandomAccessIterator first, RandomAccessIterator last)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
    vergesort(first, last, std::less<value_type>());
}

#endif // VERGESORT_H_
