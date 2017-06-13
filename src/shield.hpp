#ifndef SHEILD_H
#define SHIELD_H

#include "itemtype.hpp"
#include "item.hpp"

namespace item {

/**
 * @brief Item that shields that player from damage.
 *
 * Shielding energy can be built up regularly, and can be transferred
 * to the past when the player uses the Antitelephone.
 */
class Shield : public Item {
  public:
    /**
     * @brief An enum representation of the type of the item.
     *
     * The value of the enum is a unique value for each item, and
     * are consecutive starting at 0 across all items.
     */
    static ItemType constexpr type = ItemType::kShield;

    /**
     * @brief Energy requirement to unlock the item.
     */
    static int constexpr kUnlockRequirement = 45;

    Shield(Moment first_moment);

    Effect View(Moment m);

    Tags StateTags(Moment m) const;

  protected:
    std::pair<Effect, ItemProperties> StepImpl(Moment curr,
            RoundInfoView const& round_info_view, int energy_input);

    std::pair<Effect, ItemProperties> BranchImpl(
        Moment curr, Moment dest);

  private:
    /* ID for the amount of stored energy transferred from other timelines
     * which can contribute to the shield strength. */
    static int constexpr kPhantomEnergyID = 0;

    // Initial set of properties for the item.
    static ItemProperties FirstProperties() noexcept;

    // Helper function to compute regular shield strength.
    int RegularFromCooldown(int cooldown) const noexcept {
        return kMaxCooldown - cooldown;
    }

    // Helper function to compute shield strength carryable to the past.
    int CarryableFromRegularPhantom(int regular,
                                    int phantom) const noexcept {
        return regular + phantom / 2;
    }
};
}

#endif //SHIELD_H
