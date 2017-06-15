#ifndef MOMENT_H
#define MOMENT_H

#include <cassert>
#include <functional>
#include <boost/serialization/access.hpp>

namespace timeplane {

/**
 * @brief A unique moment in a @c TimePlane.
 *
 * This class represents a fixed moment in the game, regardless of time
 * manipulation. The instance is very small and is often passed by value.
 * Each moment is associated with a timeline (through the timeline number)
 * and a time value (which may be the same across different moments).
 * It is also possible to serialize the instance.
 */
class Moment {
  public:
    /**
     * @brief Default constructor.
     *
     * The fields are not initialized. The only mutator is the @c serialize
     * function, and so this constructor should only be used to deserialize
     * an instance.
     */
    Moment() noexcept {}

    /**
     * @brief Constructor.
     *
     * @param parent_timeline_num       The timeline number of the moment.
     * @param time                      The time associated with the moment.
     */
    Moment(int parent_timeline_num, int time) noexcept:
        parent_timeline_num_(parent_timeline_num),
        time_(time) {}

    /**
     * @brief Accessor for the timeline number.
     *
     * @return The timeline number of the moment.
     */
    int parent_timeline_num() const noexcept {
        return parent_timeline_num_;
    }

    /**
     * @brief Accessor for the time.
     *
     * @return The time associated with the moment.
     */
    int time() const noexcept {
        return time_;
    }

    /**
     * @brief Equality operator.
     *
     * Both the timeline number and time are compared.
     * @param rhs       The instance to compare against.
     * @return Whether the instances are equal.
     */
    bool operator==(const Moment& rhs) const noexcept {
        return ((parent_timeline_num_ == rhs.parent_timeline_num_) &&
                (time_ == rhs.time_));
    }

    /**
     * @brief Inequality operator.
     *
     * Both the timeline number and time are compared.
     * @param rhs       The instance to compare against.
     * @return Whether the instances are not equal.
     */
    bool operator !=(const Moment& rhs) const noexcept {
        return !(operator ==(rhs));
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
        (void)version;
        assert(version == 0);
        ar & parent_timeline_num_ & time_;
    }

  private:
    friend class boost::serialization::access;
    friend struct std::hash<Moment>;

    int parent_timeline_num_;
    int time_;
};
}

namespace std {
template <>
/**
 * @brief Hash function for @c Moment instances.
 */
struct hash<timeplane::Moment> {
    size_t operator()(timeplane::Moment const& m) const {
        size_t v0 = std::hash<int> {} (m.parent_timeline_num_);
        size_t v1 = std::hash<int> {} (m.time_);
        return (v0 * 3) ^ v1;
    }
};
}

#endif //MOMENT_H
