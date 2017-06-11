#ifndef ROUNDINFO_H
#define ROUNDINFO_H

#include <cstdint>
#include <utility>
#include <vector>
#include "aliases.hpp"
#include "symmetricbitmatrix.hpp"

namespace roundinfo {

/**
 * @brief A representation of data relevant to a single round of gameplay.
 *
 * Information about the players as well as any interactions between players
 * in a single round of gameplay is stored in this instance. Information
 * about the items held by players is @bold not stored in this instance.
 * Using alliance and encounter data, the instance also enforces the privacy
 * of certain player information from other viewers.
 */
class RoundInfo {
  public:
    static int constexpr kUnknown = -1;
    static int constexpr kOmniscientViewer = -1;

    /**
     * @brief Constructor.
     *
     * The alliances of the players are by default set such that each
     * player is not allied to any different player (though they are
     * trivially allied to themselves). Initial values of locations,
     * health data and player active status are unspecified.
     * @param num_players       The number of players in the game.
     */
    RoundInfo(int num_players)
        :num_players_(num_players),
         location_data_(num_players),
         health_remaining_data_(num_players),
         active_data_(num_players),
         alliance_data_(num_players) {
        for (int i = 0; i < num_players; i++) {
            alliance_data_.SetValue(i, i, true);
        }
    }

    /**
     * @brief Accessor for the number of players in the game.
     *
     * @return The number of players in the game.
     */
    int num_players() const noexcept {
        return num_players_;
    }

    /**
     * @brief Accessor for the location of a player.
     *
     * @param player                The player ID to query.
     * @param viewing_player        The player attempting to make the query.
     * @return The location of the player queried, or @c kUnknown if the
     *      player making the query is not authorized to know.
     * @throws std::out_of_range If no player with the specified ID exists.
     */
    int Location(int player, int viewing_player) const {
        if (player < 0 || player >= num_players_ ||
                viewing_player < kOmniscientViewer ||
                viewing_player >= num_players_) {
            throw std::out_of_range("Player ID is invalid");
        }
        return KeepIfAlliedOrEncounter(player, viewing_player,
                                       location_data_[player]);
    }

    /**
     * @brief Accessor for the health a player has remaining.
     *
     * @param player                The player ID to query.
     * @param viewing_player        The player attempting to make the query.
     * @return The location of the player queried, or @c kUnknown if the
     *      player making the query is not authorized to know.
     * @throws std::out_of_range If no player with the specified ID exists.
     */
    int HealthRemaining(int player, int viewing_player) const {
        if (player < 0 || player >= num_players_ ||
                viewing_player < kOmniscientViewer ||
                viewing_player >= num_players_) {
            throw std::out_of_range("Player ID is invalid");
        }
        return KeepIfAlliedOrEncounter(player, viewing_player,
                                       health_remaining_data_[player]);
    }

    /**
     * @brief Accessor to view the alliance data.
     * @return A constant reference to the alliance data.
     */
    SymmetricBitMatrix const& calliance_data() const noexcept {
        return alliance_data_;
    }

    /**
     * @brief Accessor for whether a player is active.
     *
     * @param player                The player ID to query.
     * @return Whether the player queried is active.
     * @throws std::out_of_range If no player with the specified ID exists.
     */
    bool Active(int player) const {
        if (player < 0 || player >= num_players_) {
            throw std::out_of_range("Player ID is invalid");
        }
        return active_data_.test(player);
    }

    /**
     * @brief Iterator to modify the location data.
     * @return A random access iterator to the location data.
     */
    IntIterator LocationIterator() {
        return location_data_.begin();
    }

    /**
     * @brief Iterator to modify the health remaining data.
     * @return A random access iterator to the health remaining data.
     */
    IntIterator HealthRemainingIterator() {
        return health_remaining_data_.begin();
    }

    /**
     * @brief Mutator to set the active status of a player.
     * @param player        The player whose active status is modified.
     * @param value         The active status value to set.
     * @throws std::out_of_range If no player with the specified ID exists.
     */
    void SetActive(int player, bool value) {
        if (player < 0 || player >= num_players_) {
            throw std::out_of_range("Player ID is invalid");
        }
        active_data_.set(player, value);
    }

    /**
     * @brief Accessor to modify the alliance data.
     * @return A mutable reference to the alliance data.
     */
    SymmetricBitMatrix& alliance_data() noexcept {
        return const_cast<SymmetricBitMatrix&> (
                   static_cast<RoundInfo&>(*this).calliance_data());
    }

  private:
    int num_players_;
    std::vector<int> location_data_;
    std::vector<int> health_remaining_data_;
    BitSet active_data_;
    SymmetricBitMatrix alliance_data_;

    // Keep the value if the players are allied only.
    int KeepIfAlliedOrEncounter(int player, int viewing_player,
                                int value) const;
};

int RoundInfo::KeepIfAlliedOrEncounter(int player, int viewing_player,
                                       int value) const {
    if (viewing_player == kOmniscientViewer ||
            alliance_data_.Value(viewing_player, player) ||
            location_data_[player] ==
            location_data_[viewing_player]) {
        return value;
    } else {
        return kUnknown;
    }
}
}

#endif //ROUNDINFO_H
