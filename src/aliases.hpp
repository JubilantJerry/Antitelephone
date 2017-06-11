#ifndef ALIASES_H
#define ALIASES_H

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <boost/dynamic_bitset.hpp>

using IntIterator = std::vector<int>::iterator;
using BitSet = boost::dynamic_bitset<uintptr_t>;

namespace timeplane {
class Moment;

using MomentIterators = std::pair<
                        std::vector<Moment>::const_iterator,
                        std::vector<Moment>::const_iterator>;
using MomentDeleterFn = std::function<void(typename MomentIterators)>;
}

namespace Item {
class Item;

using ItemPtr = std::unique_ptr<Item>;
}

#endif //ALIASES_H
