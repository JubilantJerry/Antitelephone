#ifndef ITEM_PROPERTIES_H
#define ITEM_PROPERTIES_H

#include <unordered_map>
#include <boost/serialization/access.hpp>

namespace item {

/**
 * @brief A class holding properties of an item at a specific moment.
 */
class ItemProperties {
  public:
    /**
     * @brief Accessor for the lockdown of the item.
     *
     * The lockdown represents the amount of energy needed to make the
     * item accessible for use. Once the lockdown becomes zero the item
     * becomes available.
     * @return Lockdown quantity.
     */
    int lockdown() const noexcept {
        return lockdown_;
    }

    /**
     * @brief Mutator for the lockdown of the item.
     *
     * @param lockdown      Lockdown quantity.
     */
    void set_lockdown(int lockdown) noexcept {
        lockdown_ = lockdown;
    }

    /**
     * @brief Accessor for the cooldown of the item.
     *
     * The cooldown in general represents the amount of energy needed
     * to trigger an action in the item, though it can represent other
     * quantities too. Essentially, the item is used by reducing its
     * cooldown value. The cooldown usually increases by 1 per turn
     * if the player leaves an item alone.
     * @return Cooldown quantity.
     */
    int cooldown() const noexcept {
        return cooldown_;
    }

    /**
     * @brief Mutator for the cooldown of the item.
     *
     * @param cooldown      Cooldown quantity.
     */
    void set_cooldown(int cooldown) noexcept {
        cooldown_ = cooldown;
    }

    /**
     * @brief Accessor for custom properties of an item.
     *
     * @param key       The ID of the property to access.
     * @return The value of the custom property.
     * @throws std::out_of_range if the key is an invalid property ID.
     */
    int custom(int key) const {
        return custom_.at(key);
    }

    /**
     * @brief Mutator for the cooldown of the item.
     *
     * @param lockdown      The ID of the property to access.
     * @value value         The value of the custom property to set.
     */
    void set_custom(int key, int value) {
        custom_[key] = value;
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
        ar & lockdown_ & cooldown_ & custom_;
    }

  private:
    friend class boost::serialization::access;

    int lockdown_;
    int cooldown_;
    std::unordered_map<int, int> custom_;
};
}

#endif //ITEM_PROPERTIES_H
