#ifndef MOVEDATA_H
#define MOVEDATA_H

#include <cassert>
#include <unordered_set>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/vector.hpp>
#include "itemtype.hpp"

namespace external {
using ItemType = item::ItemType;

/**
 * @brief A representation of a player's move for a round in the game.
 *
 * This instance does not include antitelephone arrival times specified by
 * the player. Such data is captured at a different phase of a round.
 */
class MoveData {
  public:
    /**
     * @brief Constructor.
     *
     * By default, energy input for all items is 0 and no alliances
     * are added or removed. The new location value is unspecified.
     */
    MoveData()
        :energy_input_data_(item::ItemTypeCount, 0),
         added_alliances_{},
         removed_alliances_{} {}

    /**
     * @brief Accessor for the energy input for each item.
     * @param item      The item to query.
     * @return The energy input for the item.
     */
    int EnergyInput(ItemType item) const noexcept {
        return energy_input_data_[item::ItemTypeID(item)];
    }

    /**
     * @brief Mutator for the energy input for each item.
     * @param item              The item to modify.
     * @param energy_input      The amount of energy to put into the item.
     */
    void SetEnergyInput(ItemType item, int energy_input) noexcept {
        energy_input_data_[item::ItemTypeID(item)] = energy_input;
    }

    /**
     * @brief Accessor for the location that the player travels to.
     * @return The location that the player moves to in the next round.
     */
    int new_location() const noexcept {
        return new_location_;
    }

    /**
     * @brief Mutator for the location that the player travels to.
     * @param new_location      The location that the player moves
     *      to in the next round.
     */
    void set_new_location(int new_location) {
        new_location_ = new_location;
    }

    /**
     * @brief Accessor for the alliances that the player attempts to add.
     * @return A reference to the set of player IDs that are added.
     */
    std::unordered_set<int> const& added_alliances() const noexcept {
        return added_alliances_;
    }

    /**
     * @brief Adds the specified player as a potential ally.
     * @param player        The ID of the player to add.
     */
    void add_alliance(int player) {
        added_alliances_.insert(player);
        removed_alliances_.erase(player);
    }

    /**
     * @brief Accessor for the alliances that the player removes.
     * @return A reference to the set of player IDs that are removed.
     */
    std::unordered_set<int> const& removed_alliances() {
        return removed_alliances_;
    }

    /**
     * @brief Removes the specified player as an ally.
     * @param player        The ID of the player to remove.
     */
    void remove_alliance(int player) {
        added_alliances_.erase(player);
        removed_alliances_.insert(player);
    }

    /**
     * @brief Leaves the alliance status of the specified player unchanged.
     * @param player        The ID of the player to leave alone.
     */
    void leave_unchanged_alliance(int player) {
        added_alliances_.erase(player);
        removed_alliances_.erase(player);
    }

    /**
     * @brief Serialization function.
     *
     * @tparam Archive      The serialization archive type.
     * @param ar            The serialization archive.
     * @param version       The verion of the serialization protocol to use.
     */
    template<typename Archive>
    void serialize(Archive& ar, unsigned int const version) {
        assert(version == 0);
        ar & energy_input_data_ & added_alliances_;
        ar & removed_alliances_ & new_location_;
    }

  private:
    std::vector<int> energy_input_data_;
    std::unordered_set<int> added_alliances_;
    std::unordered_set<int> removed_alliances_;
    int new_location_;
};
}

#endif //MOVEDATA_H
