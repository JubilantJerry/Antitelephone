#ifndef BOOST_SERIALIZATION_DYNAMIC_BITSET_H
#define BOOST_SERIALIZATION_DYNAMIC_BITSET_H

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
namespace serialization {

template <typename Ar, typename Block, typename Alloc>
void save(Ar& ar, dynamic_bitset<Block, Alloc> const& bs, unsigned) {
    size_t num_bits = bs.size();
    std::vector<Block> blocks(bs.num_blocks());
    to_block_range(bs, blocks.begin());

    ar & num_bits & blocks;
}

template <typename Ar, typename Block, typename Alloc>
void load(Ar& ar, dynamic_bitset<Block, Alloc>& bs, unsigned) {
    size_t num_bits;
    std::vector<Block> blocks;
    ar & num_bits & blocks;

    bs.resize(num_bits);
    from_block_range(blocks.begin(), blocks.end(), bs);
    bs.resize(num_bits);
}

template <typename Ar, typename Block, typename Alloc>
void serialize(Ar& ar, dynamic_bitset<Block, Alloc>& bs, unsigned version) {
    split_free(ar, bs, version);
}

}
}

#endif //BOOST_SERIALIZATION_DYNAMIC_BITSET_H
