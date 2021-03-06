#ifndef TIMELINE_H
#define TIMELINE_H

#include <cassert>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include "aliases.hpp"

namespace timeplane {
class Moment;

/**
 * @brief A chronological sequence of @c Moment instances.
 *
 * The times of the @c Moment instances in a timeline are consecutive
 * and begin at 0. It is possible to create new moments at the end of the
 * timeline or retrieve moments at any valid time in the timeline.
 */
class TimeLine {
  public:
    /**
     * @brief Default constructor.
     *
     * This constructor is used to make the leftmost timeline,
     * and in the process it creates the earliest moment as well.
     * @param moment_deleter        A handler for moments that
     *      fall out of scope from outside clients.
     */
    TimeLine(MomentDeleterFn moment_deleter = MomentDeleterFn{});

    /**
     * @brief Constructor from an existing timeline.
     *
     * This constructor creates a timeline as a branch of an existing
     * timeline. It also creates a distinct @c Moment instance in
     * the newly created timeline whose time is equal to the branch time.
     * @param left_timeline     The timeline to branch from.
     * @param branch_time       The time to branch off from.
     * @param moment_deleter        A handler for moments that
     *      fall out of scope from outside clients.
     */
    TimeLine(TimeLine const& left_timeline, int branch_time,
             MomentDeleterFn moment_deleter = MomentDeleterFn{});

    /**
     * @brief Custom destructor.
     *
     * The moment deletion handler, if active, is called to handle
     * the removal of all unreachable moments within the timeline
     * before it is destroyed.
     */
    ~TimeLine();

    /**
     * @brief Accessor for the timeline number.
     *
     * @return An ID number unique to a timeline.
     */
    int const timeline_number() const noexcept;

    /**
     * @brief Accesses the moment with the specified time.
     *
     * @param time      The time to query.
     * @return The @c Moment instance at the specified time.
     * @throws std::out_of_range If no moment exists at the specified time.
     */
    Moment const GetMoment(int time) const;

    /**
     * @brief Creates a new moment at the end of the timeline.
     *
     * @return The moment newly created.
     */
    Moment const MakeMoment();

    /**
     * @brief Retrieve the latest moment from the timeline.
     *
     * @return The moment at the end of the timeline.
     */
    Moment const LatestMoment() const noexcept;

    TimeLine(TimeLine const&) = delete;
    TimeLine& operator=(TimeLine const&) = delete;
    TimeLine(TimeLine&&) = default;

    /**
     * @brief Custom move assignment operator.
     *
     * This function also cleans up any moments that cannot be accessed
     * as a result of the assignment.
     * @param rhs       The object to be moved.
     * @return The object that was assigned to.
     */
    TimeLine& operator=(TimeLine&& rhs) {
        CleanUp();
        pimpl_ = std::move(rhs.pimpl_);
        return *this;
    }

  private:
    //Declaring the pimpl idiom with a shared pointer.
    ///@cond INTERNAL
    class Impl;
    friend Impl Impl(TimeLine const& left_timeline, int branch_time);
    ///@endcond

    std::shared_ptr<Impl> pimpl_;
    static void ImplDeleter(Impl* pimpl);
    void CleanUp();
};
}

#endif //TIMELINE_H
