#ifndef EFFECT_H
#define EFFECT_H

namespace item {
class Effect {
  public:
    /**
     * @brief Constructor.
     * @param attack_boost                  Increase in damage output.
     * @param shield_amount                 Increase in shield absorption.
     * @param antitelephone_dest_allowed    Whether to explicitly allow
     *      antitelephone travel in spite of normal rules.
     * @param player_make_active            Whether to make the player
     *      actively controllable in spite of normal rules
     */
    Effect(int attack_boost = 0, int shield_amount = 0,
           bool antitelephone_dest_allowed = false,
           bool player_make_active = false)
        :attack_boost_{attack_boost},
         shield_amount_{shield_amount},
         antitelephone_dest_allowed_{antitelephone_dest_allowed},
         player_make_active_{player_make_active} {}

    /**
     * @brief Accessor for the attack boost quantity.
     * @return Attack boost, in extra damage per encounter.
     */
    int attack_boost() {
        return attack_boost_;
    }

    /**
     * @brief Accessor for the shield absorption strength.
     * @return Shield absorption, in damage absorbed by the shield.
     */
    int shield_amount() {
        return shield_amount_;
    }

    /**
     * @brief Accessor for the explicit allowance of antitelephone travel.
     *
     * In general, a @c true value overrides any @c false values, so a
     * value of @c false does not explicitly disallow antitelephone travel.
     * @return Whether antitelephone travel is explicitly allowed.
     */
    bool antitelephone_dest_allowed() {
        return antitelephone_dest_allowed_;
    }

    /**
     * @brief Accessor for directly making the player actively controllable.
     *
     * In general, a @c true value overrides any @c false values, so a
     * value of @c false does not force the player to be uncontrollable.
     * @return Whether the player is directly to be made controllable.
     */
    bool player_make_active() {
        return player_make_active_;
    }

    /**
     * @brief Addition operator to combine effects.
     *
     * The attack boost and shield absorption strength is added directly.
     * The boolean values are combined with a boolean OR operation.
     * @param rhs       The other @c Effect to combine with.
     * @return The combined effect of the two inputs.
     */
    Effect operator+(Effect const& rhs) {
        return Effect {
            attack_boost_ + rhs.attack_boost_,
            shield_amount_ + rhs.shield_amount_,
            antitelephone_dest_allowed_ || rhs.antitelephone_dest_allowed_,
            player_make_active_ || rhs.player_make_active_
        };
    }

  private:
    int attack_boost_;
    int shield_amount_;
    bool antitelephone_dest_allowed_;
    bool player_make_active_;
};
}

#endif //EFFECT_H
