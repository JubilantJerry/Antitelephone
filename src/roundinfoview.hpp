#ifndef ROUNDINFO_VIEW_H
#define ROUNDINFO_VIEW_H

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <utility>
#include <boost/serialization/access.hpp>
#include <boost/serialization/unordered_map.hpp>
#include "boost_serialization_dynamic_bitset.hpp"
#include "aliases.hpp"
#include "symmetricbitmatrix.hpp"

namespace roundinfo {
class RoundInfo;

/**
 * @brief Information contained in @c RoundInfo as seen from a single player.
 *
 * Any information that a player is unauthorized to see is reported as
 * @c RoundInfo::kUnknown. Instances of this class can be serialized.
 * Almost all data stored in the instance is past-oriented, which means
 * that it describes a moment that has already passed. Only information
 * about whether players are active is future-oriented, which means it
 * describes an upcoming moment.
 */
class RoundInfoView {
  public:
    /**
     * @brief Default constructor.
     *
     * The fields are not initialized. The only mutator is the @c serialize
     * function, and so this constructor should only be used to deserialize
     * an instance.
     */
    RoundInfoView();

    /**
     * @brief Constructor.
     * @param source                    The @c RoundInfo instance as a
     *      centralized source to obtain data from.
     * @param player                    The ID of the player to view from.
     * @param location_omniscience      Whether the player is granted
     *      omniscience into the locations of other players.
     */
    RoundInfoView(RoundInfo const& source, int player,
                  bool location_omniscience = false);

    /**
     * @brief Accessor for the ID of the player that the view centers from.
     * @return Player ID of the viewer.
     */
    int player() const noexcept {
        return player_;
    }

    /**
     * @brief Accessor for the number of players in the game.
     * @return Number of players in the game.
     */
    int num_players() const noexcept {
        return (int)(allies_.size());
    }

    /**
     * @brief Accessor for the location of another player.
     * @param player        The ID of the player to query.
     * @return The location of the player queried, or @c RoundInfo::kUnknown
     *      if the viewer is not authorized to know.
     */
    int Location(int player) const noexcept {
        return TryFindValue(location_data_, player);
    }

    /**
     * @brief Accessor for the health remaining of another player.
     * @param player        The ID of the player to query.
     * @return The health remaining of the player queried, or
     *      @c RoundInfo::kUnknown if the viewer is not authorized to know.
     */
    int DamageReceived(int player) const noexcept {
        return TryFindValue(damage_received_data_, player);
    }

    /**
     * @brief Accessor for the health remaining of another player.
     * @param player        The ID of the player to query.
     * @return The health remaining of the player queried, or
     *      @c RoundInfo::kUnknown if the viewer is not authorized to know.
     */
    int HealthRemaining(int player) const noexcept {
        return TryFindValue(health_remaining_data_, player);
    }

    /**
     * @brief Accessor for the set of allies the player has.
     * @return A bit set representing who the player is allied with.
     */
    BitSet const& allies() const noexcept {
        return allies_;
    }

    /**
     * @brief Accessor for whether the player is actively in control.
     * @return Whether the player can actively control their moves.
     */
    bool active() const noexcept {
        return active_;
    }

    /**
     * @brief Mutator for whether the player is actively in control.
     *
     * Only this property has a mutator because it is a future-oriented
     * property, which allows it to be modified as a result of the actions
     * that occur during a round.
     * @param active        Whether the player is actively in control.
     */
    void set_active(bool active) noexcept {
        active_ = active;
    }

    /**
     * @brief Serialization function.
     *
     * @tparam Archive      The serialization archive type.
     * @param ar            The serialization archive.
     * @param version       The verion of the serialization protocol to use.
     */
    template <typename Archive>
    void serialize(Archive & ar, unsigned int const version);

  private:
    using IntIntMap = std::unordered_map<int, int>;
    friend class boost::serialization::access;

    int player_;
    IntIntMap location_data_;
    IntIntMap damage_received_data_;
    IntIntMap health_remaining_data_;
    bool active_;
    BitSet allies_;

    // Attempts to locate a value, returning kUnknown if not found.
    int TryFindValue(IntIntMap const& source, int key) const;
};

template <typename Archive>
void RoundInfoView::serialize(Archive &ar, const unsigned int version) {
    assert(version == 0);
    ar & player_ & location_data_ & damage_received_data_;
    ar & health_remaining_data_ & active_ & allies_;
}
}

#endif //ROUNDINFO_VIEW_H
