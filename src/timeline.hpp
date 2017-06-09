#ifndef TIMELINE_H
#define TIMELINE_H

#include <cassert>
#include <utility>
#include <functional>
#include <vector>
#include <memory>

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
    using MomentIterators = ::std::pair<
                            ::std::vector<Moment>::const_iterator,
                            ::std::vector<Moment>::const_iterator>;
    using MomentDeleter = ::std::function<void(typename MomentIterators)>;

    /**
     * @brief Default constructor.
     *
     * This constructor is used to make the leftmost timeline,
     * and in the process it creates the earliest moment as well.
     * @param moment_deleter        A handler for moments that
     *      fall out of scope from outside clients.
     */
    TimeLine(MomentDeleter moment_deleter = MomentDeleter{});

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
             MomentDeleter moment_deleter = MomentDeleter{});

    /**
     * @brief Destructor.
     *
     * The moment deletion handler, if active, is called to handle
     * the removal of all moments contained within the timeline
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
    TimeLine& operator=(TimeLine&&) = default;

  private:
    //Declaring the pimpl idiom with a shared pointer.
    ///@cond INTERNAL
    class Impl;
    static void ImplDeleter(Impl* pimpl);
    friend Impl Impl(TimeLine const& left_timeline, int branch_time);
    ::std::shared_ptr<Impl> pimpl_;
    ///@endcond
};
}

#endif //TIMELINE_H
