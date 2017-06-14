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
     * Alias for the result of a query for a moment overview.
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
    MomentOverviewQueryResult GetOverview(
        int player, Moment m, bool from_rightmost = true) const;

    /**
     * @brief Submits data for a regular move for the upcoming round.
     * @param player        The player making the move.
     * @param move          The move that the player makes.
     * @return Whether the command was run successfully.
     */
    QueryResult MakeRegularMove(int player, MoveData const& move);

    /**
     * @brief Submits data for antitelephone travel.
     * @param player        The player making the move.
     * @param move          The move that the player makes.
     * @return Whether the command was run successfully.
     */
    QueryResult MakeAntitelephoneMove(int player, Moment dest);

    /**
     * @brief Registers a handler to be called for every new round.
     * @param handler       The handler to register.
     */
    void RegisterNewRoundHandler(
        std::function<void (MomentOverview)> handler);

    /**
     * @brief Registers a handler to be called once the game is over.
     * @param handler       The handler to register.
     */
    void RegisterEndGameHandler(
        std::function<void ()> handler);

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
