#ifndef ITEMS_UTIL_H
#define ITEMS_UTIL_H

#include <cassert>
#include "moment.hpp"
#include "../src/effect.hpp"
#include "../src/itemproperties.hpp"
#include "../src/item.hpp"
#include "../src/antitelephone.hpp"
#include "../src/bridge.hpp"
#include "../src/oracle.hpp"
#include "../src/shield.hpp"

namespace item {

/**
 * @brief Constructs all the items in a vector in the order of their ID's.
 * @param first_moment      The first moment of the game.
 * @return A vector containing a new instance of all the items.
 */
inline ItemArr MakeItemPtrs(Moment first_moment) {
    assert(ItemTypeID(ItemType::kAntitelephone) == 0);
    assert(ItemTypeID(ItemType::kBridge) == 1);
    assert(ItemTypeID(ItemType::kOracle) == 2);
    assert(ItemTypeID(ItemType::kShield) == 3);
    return {std::make_unique<Antitelephone>(first_moment),
            std::make_unique<Bridge>(first_moment),
            std::make_unique<Oracle>(first_moment),
            std::make_unique<Shield>(first_moment)};
}
}

#endif //ITEMS_UTIL_H
