#ifndef MOMENT_OVERVIEW_H
#define MOMENT_OVERVIEW_H

#include <array>
#include <cassert>
#include <type_traits>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/utility.hpp>
#include "moment.hpp"
#include "effect.hpp"
#include "itemproperties.hpp"
#include "itemtype.hpp"
#include "roundinfoview.hpp"
#include "aliases.hpp"

namespace external {
using Moment = timeplane::Moment;
using RoundInfoView = roundinfo::RoundInfoView;
using Effect = item::Effect;
using ItemProperties = item::ItemProperties;
using ItemType = item::ItemType;

/**
 * @brief A class that gives a user-friendly overview of a moment.
 *
 * The overview given is specific to one player of the game. The instance
 * can also be serialized.
 */
class MomentOverview {
  public:
    using TaggedValuesArr = std::array<TaggedValues, item::ItemTypeCount>;

    /**
     * @brief Default constructor.
     *
     * The fields are not initialized. The only mutator is the @c serialize
     * function, and so this constructor should only be used to deserialize
     * an instance.
     */
    MomentOverview() {}

    /**
     * @brief Constructor.
     *
     * The item state argument must have an entry for all items in the game.
     * @param moment                The moment that is overviewed.
     * @param item_state_data       The state of the items.
     * @param round_info_           Information about the round.
     */
    MomentOverview(Moment moment, Effect effect,
                   TaggedValuesArr item_state_data, RoundInfoView round_info)
        :moment_{moment},
         effect_{effect},
         item_state_data_{std::move(item_state_data)},
         round_info_{std::move(round_info)} {
        assert(item_state_data_.size() == item::ItemTypeCount);
    }

    /**
     * @brief Accessor for the ID of the overviewed player.
     * @return Player ID of the player whose viewpoint is used.
     */
    int player() const noexcept {
        return round_info_.player();
    }

    /**
     * @brief Accessor for the moment that is overviewed.
     * @return The moment that is looked at.
     */
    Moment moment() const noexcept {
        return moment_;
    }

    /**
     * @brief Accessor for the items' effects to the player at the moment.
     * @return The effects of the items on player at the moment overviewed.
     */
    Effect effect() const noexcept {
        return effect_;
    }

    /**
     * @brief Accessor for the state of an item.
     * @param itemID        The item queried.
     * @return A group of tagged values that offer a user-friendly view
     *      into the internal state of the item.
     */
    TaggedValues const& ItemState(int itemID) const noexcept {
        return item_state_data_[itemID];
    }

    /**
     * @brief Accessor for a view into information about the round.
     * @return A reference to a view of the round information.
     */
    RoundInfoView const& round_info() const noexcept {
        return round_info_;
    }

    /**
     * @brief Serialization function.
     *
     * @tparam Archive      The serialization archive type.
     * @param ar            The serialization archive.
     * @param version       The verion of the serialization protocol to use.
     */
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const version) {
        (void)version;
        assert(version == 0);
        ar & moment_ & effect_ & item_state_data_ & round_info_;
    }

  private:
    Moment moment_;
    Effect effect_;
    TaggedValuesArr item_state_data_;
    RoundInfoView round_info_;
};
}

#endif //MOMENT_OVERVIEW_H
