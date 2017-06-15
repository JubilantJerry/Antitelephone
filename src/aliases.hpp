#ifndef ALIASES_H
#define ALIASES_H

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "itemtype.hpp"

using IntIterator = std::vector<int>::iterator;
using BitSet = boost::dynamic_bitset<uintptr_t>;
using TaggedValue = std::pair<std::string, std::string>;
using TaggedValues = std::vector<TaggedValue>;

namespace timeplane {
class Moment;

using MomentIterators = std::pair<
                        std::vector<Moment>::const_iterator,
                        std::vector<Moment>::const_iterator>;
using MomentDeleterFn = std::function<void (typename MomentIterators)>;
}

namespace item {
class Item;

using ItemPtr = std::unique_ptr<Item>;
using ItemArr = std::array<ItemPtr, ItemTypeCount>;
}

#endif //ALIASES_H
