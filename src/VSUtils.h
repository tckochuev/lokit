#ifndef VS_UTILS
#define VS_UTILS

#include <iterator>
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <vector>

template<typename ForwardIterator, typename PermutationForwardIterator>
size_t permutate(ForwardIterator first, ForwardIterator last,
                 PermutationForwardIterator permutationFirst,
                 typename std::iterator_traits<PermutationForwardIterator>::difference_type permutationSize
)
{
    assert(permutationSize != 0);
    //all unique
    assert(
        std::unordered_set<std::iterator_traits<PermutationForwardIterator>::value_type>(
            permutationFirst,
            std::next(permutationFirst, permutationSize)
            ).size() == permutationSize
    );
    //all in range [0, permutationSize)
    assert(
        std::all_of(
            permutationFirst, std::next(permutationFirst, permutationSize),
            [&](const auto& value) {return value >= 0 && value < permutationSize;}
        )
    );
    auto imageOf = [&](typename std::iterator_traits<PermutationForwardIterator>::difference_type index) -> auto
    {
        assert(index >= 0 && index < permutationSize);
        return *std::next(permutationFirst, index);
    };
    struct IteratorAndPermutationStatus
    {
        ForwardIterator iterator;
        bool needsPermutation;
    };
    using Buffer = std::vector<IteratorAndPermutationStatus>;
    Buffer buffer(permutationSize);
    typename Buffer::size_type vacantIndex = 0;
    while (first != last)
    {
        buffer[vacantIndex++] = {first++, true};
        if (vacantIndex == permutationSize)
        {
            for (typename Buffer::size_type i = 0; i < buffer.size(); ++i)
            {
                auto toPermutate = i;
                while (buffer[i].needsPermutation)
                {
                    auto indexSwap = [&buffer](auto lhs, auto rhs) {
                        std::iter_swap(buffer[lhs].iterator, buffer[rhs].iterator);
                    };
                    auto image = imageOf(toPermutate);
                    indexSwap(i, image);
                    buffer[image].needsPermutation = false;
                    toPermutate = image;
                }
                assert(toPermutate == i);
            }
            vacantIndex = 0;
        }
    }
    return vacantIndex;
}

template<typename ForwardIterator, typename PermutationValueType>
size_t permutate(ForwardIterator first, ForwardIterator last,
                 const std::initializer_list<PermutationValueType>& permutation
)
{
    return permutate(first, last, permutation.begin(), permutation.size());
}

#endif
