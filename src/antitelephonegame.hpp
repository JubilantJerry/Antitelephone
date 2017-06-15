#ifndef ANTITELEPHONE_GAME_H
#define ANTITELEPHONE_GAME_H

#include <cstdint>
#include <functional>
#include <memory>
#include <boost/optional/optional_fwd.hpp>

namespace timeplane {
class Moment;
class TimePlane;
}

namespace external {
class MomentOverview;
class MoveData;
}

class QueryResult;

/**
 * @brief Top level manager for Antitelephone internal game logic.
 *
 * Interactions with this manager is mostly facilitated through
 * objects designed for external message passing.
 * The internal game logic is not thread-safe, but not global data is used.
 */
class AntitelephoneGame {
  public:
    using TimePlane = timeplane::TimePlane;
    using Moment = timeplane::Moment;
    using MomentOverview = external::MomentOverview;
    using MoveData = external::MoveData;

    /**
     * @brief Number of valid locations added for every player in the game.
     */
    static int constexpr kRoomsPerPlayer = 5;

    /**
     * @brief Minimum number of players.
     */
    static int constexpr kMinNumPlayers = 2;

    /**
     * @brief Maximum number of players.
     */
    static int constexpr kMaxNumPlayers = 6;

    /**
     * @brief Energy available to players every round.
     */
    static int constexpr kEnergyPerRound = 3;

    /**
     * @brief Damage multiplier for a reenacted encounter.
     */
    static double constexpr kFamiliarEncounterMultiplier = 1.5;

    /**
     * @brief Constructor.
     * @param game_id           A numeric ID assigned to the game.
     * @param num_players       The number of players in the game.
     * @param random_seed       A seed for random number generation.
     */
    AntitelephoneGame(int game_id, int num_players,
                      uint64_t random_seed = 1337133713371337UL);

    /**
     * @brief Accessor for the timeplane manager.
     * @return A reference to the @c TimePlane instance stored internally.
     */
    TimePlane const& time_plane() const noexcept;

    /**
     * @brief Alias for the result of a query for a moment overview.
     */
    using MomentOverviewQueryResult =
        std::pair<QueryResult, boost::optional<MomentOverview>>;

    /**
     * @brief Requests an overview of the given moment.
     *
     * @param player                The ID of the player making the request.
     * @param m                     The moment to query.
     * @param from_rightmost        Whether the moment was from the
     *      rightmost timeline in the timeplane.
     * @return Whether the request was successfully granted, and the
     *      resulting moment overview if it was.
     */
    MomentOverviewQueryResult GetOverview(int player, Moment m) const;

    /**
     * @brief Submits data for a regular move for the upcoming round.
     * @param player        The player making the move.
     * @param move          The move that the player makes.
     * @return Whether the command was run successfully.
     */
    QueryResult MakeRegularMove(int player, MoveData move);

    /**
     * @brief Submits data for antitelephone travel.
     * @param player        The player making the move.
     * @param dest_time     The time of the antitelephone destination.
     * @return Whether the command was run successfully.
     */
    QueryResult MakeAntitelephoneMove(int player, int dest_time);

    /**
     * @brief Alias for the type of a handler called for every new round.
     *
     * The argument represents the overview of the new moment as seen from
     * each player in the game.
     */
    using NewRoundHandler =
        std::function<void (int, std::vector<MomentOverview> const&&)>;

    /**
     * @brief Registers a handler to be called for every new round.
     * @param handler       The handler to register.
     */
    void RegisterNewRoundHandler(NewRoundHandler handler);

    /**
     * @brief Alias for the type of a handler called for time travel.
     *
     * The argument represents the ID of the player that traveled in time.
     */
    using TravelHandler = std::function<void (int, int)>;

    /**
     * @brief Registers a handler to be called for time travel.
     * @param handler       The handler to register.
     */
    void RegisterTravelHandler(TravelHandler handler);

    /**
     * @brief Alias for the type of a handler called at the end of the game.
     */
    using EndGameHandler = std::function<void (int)>;

    /**
     * @brief Registers a handler to be called once the game is over.
     * @param handler       The handler to register.
     */
    void RegisterEndGameHandler(EndGameHandler handler);

    /// @cond INTERNAL
    ~AntitelephoneGame();
    AntitelephoneGame(AntitelephoneGame&&);
    AntitelephoneGame& operator=(AntitelephoneGame&&);
    /// @endcond

  private:
    /// @cond INTERNAL
    class Impl;
    /// @endcond
    std::unique_ptr<Impl> pimpl_;
};

#endif //ANTITELEPHONE_GAME_H
