#ifndef ORACLE_H
#define ORACLE_H

#include "itemtype.hpp"
#include "item.hpp"

namespace item {

/**
 * @brief Item that can make a player controllable.
 *
 * The player is made directly controllable when the Oracle is activated,
 * regardless of what timeline the activation has occurred in.
 */
class Oracle : public Item {
  public:
    /**
     * @brief An enum representation of the type of the item.
     *
     * The value of the enum is a unique value for each item, and
     * are consecutive starting at 0 across all items.
     */
    static ItemType constexpr type = ItemType::kOracle;

    /**
     * @brief Energy requirement to unlock the item.
     */
    static int constexpr kUnlockRequirement = 45;

    Oracle(Moment first_moment);

    Effect View(Moment m);

    TaggedValues StateTaggedValues(Moment m) const;

  protected:
    std::pair<Effect, ItemProperties> StepImpl(Moment curr,
            RoundInfoView const& round_info_view, int energy_input);

    std::pair<Effect, ItemProperties> BranchImpl(
        Moment curr, Moment dest);

  private:
    /* ID for whether the oracle was activated at a moment. */
    static int constexpr kActivatedID = 0;

    // Initial set of properties for the item.
    static ItemProperties FirstProperties() noexcept;
};
}

#endif //ORACLE_H
