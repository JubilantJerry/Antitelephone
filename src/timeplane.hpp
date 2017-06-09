#ifndef TIMEPLANE_H
#define TIMEPLANE_H

#include <utility>
#include <vector>
#include <functional>
#include <boost/optional/optional.hpp>
#include "aliases.hpp"
#include "timeline.hpp"

namespace  timeplane {
class Moment;
class TimeLine;

/**
 * @brief A wrapper to access individual @c TimeLine instances.
 *
 * A timeplane can logically be thought of as a sequence of linearly
 * connected timelines spanning left to right (where time flows upward
 * within each timeline). Currently the class allows access to the rightmost
 * and second-rightmost timelines only. The user can register handlers
 * to clean up their data structures whenever any @c Moment instances are
 * no longer externally accessible.
 */
class TimePlane {
  public:
    /**
     * @brief Constant representing a lack of antitelephone arrivals.
     */
    static constexpr int kNoAntitelephoneArrival = -1;

    /**
     * @brief Default constructor.
     *
     * A single timeline is created and within it, a single moment
     * is also created.
     */
    TimePlane();

    /**
     * @brief Accessor for the latest Antitelephone arrival time.
     *
     * The value applies to the rightmost timeline, though note that
     * Antitelephone arrivals all still occur for timelines to the
     * right of the leftmost arrival, even if some of these timelines have
     * early Antitelephone arrivals.
     *
     * If there is no Antitelephone arrival, then the constant
     * @c kNoAntitelephoneArrival is returned.
     * @return Time of the latest Antitelephone arrival.
     */
    int latest_antitelephone_arrival() const noexcept {
        return latest_antitelephone_arrival_;
    }

    /**
     * @brief Accessor for the rightmost timeline.
     *
     * The reference returned is invalidated once a new timeline is created.
     * @return A reference to the rightmost timeline.
     */
    TimeLine& rightmost_timeline() noexcept {
        return rightmost_timeline_;
    }

    /**
     * @brief Accessor for the rightmost timeline.
     *
     * The reference returned is invalidated once a new timeline is created.
     * @return A reference to an optional type containing the
     * rightmost timeline, or @c boost::none if there is no such timeline.
     */
    boost::optional<TimeLine>& second_rightmost_timeLine() noexcept {
        return second_rightmost_timeline_;
    }

    /**
     * @brief Creates a new timeline branching from the rightmost timeline.
     *
     * The reference returned is invalidated once a new timeline is created.
     * @param branch_time       The time of the branch, which will be the
     *      the time of the sole moment available in the new timeline.
     * @return A reference to the newly created timeline.
     */
    TimeLine& MakeNewTimeLine(int branch_time);

    /**
     * @brief Registers a moment deletion handler.
     *
     * The moment deletion handler is called with a pair of output iterators
     * denoting @c Moment instances that are no longer externally accessible.
     * @param handler       The moment deletion handler to register.
     */
    void RegisterMomentDeleter(MomentDeleter handler) {
        handlers_.push_back(handler);
    }

  private:
    TimeLine rightmost_timeline_;
    boost::optional<TimeLine> second_rightmost_timeline_;
    std::vector<MomentDeleter> handlers_;
    int latest_antitelephone_arrival_;

    /* Calls all the moment deletion handlers. */
    void MomentDeleterFacade(MomentIterators iterators);
};
}

#endif
