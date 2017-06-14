#ifndef EFFECT_H
#define EFFECT_H

#include <cassert>
#include <boost/serialization/access.hpp>

namespace item {
class Effect {
  public:
    /**
     * @brief Constructor.
     * @param attack_increase                  Increase in damage output.
     * @param shield_amount                 Increase in shield absorption.
     * @param antitelephone_dest_allowed    Whether to explicitly allow
     *      antitelephone travel in spite of normal rules.
     * @param player_make_active            Whether to make the player
     *      actively controllable in spite of normal rules
     */
    Effect(int attack_increase = 0,
           int max_hitpoint_increase = 0,
           int shield_amount = 0,
           bool antitelephone_departure = false,
           bool antitelephone_dest_allowed = false,
           bool player_make_active = false)
        :attack_increase_{attack_increase},
         max_hitpoint_increase_{max_hitpoint_increase},
         shield_amount_{shield_amount},
         antitelephone_departure_{antitelephone_departure},
         antitelephone_dest_allowed_{antitelephone_dest_allowed},
         player_make_active_{player_make_active} {}

    /**
     * @brief Accessor for the attack increase quantity.
     * @return Attack increase, in extra damage per encounter.
     */
    int attack_increase() const noexcept {
        return attack_increase_;
    }

    /**
     * @brief Mutator for the attack increase quantity.
     * @param attack_increase       Attack increase, in extra damage
     *      per encounter.
     */
    void set_attack_increase(int attack_increase) noexcept {
        attack_increase_ = attack_increase;
    }

    /**
     * @brief Accessor for the increase in maximum hitpoints.
     * @return Increase in maximum hit points granted to the player.
     */
    int max_hitpoint_increase() const noexcept {
        return max_hitpoint_increase_;
    }

    /**
     * @brief Mutator for the increase in maximum hitpoints.
     * @param max_hitpoint_increase     Increase in maximum hit points
     *      granted to the player.
     */
    void set_max_hitpoint_increase(int max_hitpoint_increase) noexcept {
        max_hitpoint_increase_ = max_hitpoint_increase;
    }

    /**
     * @brief Accessor for the shield absorption strength.
     * @return Shield absorption, in damage absorbed by the shield.
     */
    int shield_amount() const noexcept {
        return shield_amount_;
    }

    /**
     * @brief Mutator for the shield absorption strength.
     * @param shield_amount       Shield absorption, in damage
     *      absorbed by the shield.
     */
    void set_shield_amount(int shield_amount) noexcept {
        shield_amount_ = shield_amount;
    }

    /**
     * @brief Accessor for an explicit antitelephone departure.
     *
     * In general, a @c true value overrides any @c false values, so a
     * value of @c false does not explicitly rule out antitelephone departure.
     * @return Whether antitelephone departure has explicitly occurred.
     */
    bool antitelephone_departure() const noexcept {
        return antitelephone_departure_;
    }

    /**
     * @brief Mutator for an explicit antitelephone departure.
     * @param antitelephone_departure       Whether antitelephone departure
     *      has explicitly occurred.
     */
    void set_antitelephone_departure(bool antitelephone_departure) noexcept {
        antitelephone_departure_ = antitelephone_departure;
    }

    /**
     * @brief Accessor for the explicit allowance of antitelephone travel.
     *
     * In general, a @c true value overrides any @c false values, so a
     * value of @c false does not explicitly disallow antitelephone travel.
     * @return Whether antitelephone travel is explicitly allowed.
     */
    bool antitelephone_dest_allowed() const noexcept {
        return antitelephone_dest_allowed_;
    }

    /**
     * @brief Mutator for the explicit allowance of antitelephone travel.
     * @param antitelephone_dest_allowed       Whether antitelephone travel
     *      is explicitly allowed.
     */
    void set_antitelephone_dest_allowed(
        bool antitelephone_dest_allowed) noexcept {
        antitelephone_dest_allowed_ = antitelephone_dest_allowed;
    }

    /**
     * @brief Accessor for directly making the player actively controllable.
     *
     * In general, a @c true value overrides any @c false values, so a
     * value of @c false does not force the player to be uncontrollable.
     * @return Whether the player is directly to be made controllable.
     */
    bool player_make_active() const noexcept {
        return player_make_active_;
    }

    /**
     * @brief Mutator for directly making the player actively controllable.
     * @param player_make_active       Whether the player is directly
     *      to be made controllable.
     */
    void set_player_make_active(bool player_make_active) noexcept {
        player_make_active_ = player_make_active;
    }

    /**
     * @brief Addition operator to combine effects.
     *
     * The attack increase and shield absorption strength is added directly.
     * The boolean values are combined with a boolean OR operation.
     * @param rhs       The other @c Effect to combine with.
     * @return The combined effect of the two inputs.
     */
    Effect operator+(Effect const& rhs) {
        return Effect {
            attack_increase_ + rhs.attack_increase_,
            max_hitpoint_increase_ + rhs.max_hitpoint_increase_,
            shield_amount_ + rhs.shield_amount_,
            antitelephone_departure_ || rhs.antitelephone_departure_,
            antitelephone_dest_allowed_ || rhs.antitelephone_dest_allowed_,
            player_make_active_ || rhs.player_make_active_
        };
    }

    /**
     * @brief Addition assignment operator.
     * @see operator+
     * @param rhs       The other @c Effect to combine with.
     * @return The left hand side, after modification.
     */
    Effect& operator+=(Effect const& rhs) {
        attack_increase_ += rhs.attack_increase_;
        max_hitpoint_increase_ += rhs.max_hitpoint_increase_;
        shield_amount_ += rhs.shield_amount_;
        antitelephone_departure_ = antitelephone_departure_ ||
                                   rhs.antitelephone_departure_;
        antitelephone_dest_allowed_  = antitelephone_dest_allowed_ ||
                                       rhs.antitelephone_dest_allowed_;
        player_make_active_ = player_make_active_ ||
                              rhs.player_make_active_;
        return *this;
    }

    /**
     * @brief Serialization function.
     *
     * @tparam Archive      The serialization archive type.
     * @param ar            The serialization archive.
     * @param version       The verion of the serialization protocol to use.
     */
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const version) {
        assert(version == 0);
        ar & attack_increase_ & max_hitpoint_increase_ & shield_amount_;
        ar & antitelephone_departure_ &
        antitelephone_dest_allowed_ & player_make_active_;
    }

  private:
    int attack_increase_;
    int max_hitpoint_increase_;
    int shield_amount_;
    bool antitelephone_departure_;
    bool antitelephone_dest_allowed_;
    bool player_make_active_;
};
}

#endif //EFFECT_H
