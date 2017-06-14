#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

#include <type_traits>
#include "moment.hpp"
#include "aliases.hpp"

namespace item {
/**
 * @brief A enumeration representing all the items in the game.
 *
 * Each value has an associated ID which are consecutive and start from 0.
 */
enum class ItemType {
    kAntitelephone = 0,
    kBridge = 1,
    kOracle = 2,
    kShield = 3
};

/**
 * The type of the underlying ID in the @c ItemType enumeration.
 */
using ItemType_t = typename std::underlying_type<ItemType>::type;

/**
 * @brief Obtains the ID associated with the @c ItemType enumeration.
 * @param item      The item type to query.
 * @return The ID of the item type.
 */
ItemType_t constexpr ItemTypeID(ItemType item) noexcept {
    return static_cast<ItemType_t>(item);
}

/**
 * @brief The number of distinct items.
 */
int constexpr ItemTypeCount = 4;
}

#endif //ITEM_TYPE_H
