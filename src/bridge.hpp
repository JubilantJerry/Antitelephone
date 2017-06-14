#ifndef BRIDGE_H
#define BRIDGE_H

#include "itemtype.hpp"
#include "item.hpp"

namespace item {

/**
 * @brief Item which bridges together separated moments.
 *
 * Moments that are otherwise separated because the latest antitelephone
 * arrival lies in between can be connected with the Bridge. Then it would
 * be possible to travel from the later moment to the earlier moment.
 */
class Bridge : public Item {
  public:
    /**
     * @brief An enum representation of the type of the item.
     *
     * The value of the enum is a unique value for each item, and
     * are consecutive starting at 0 across all items.
     */
    static ItemType constexpr type = ItemType::kBridge;

    /**
     * @brief Energy requirement to unlock the item.
     */
    static int constexpr kUnlockRequirement = 45;

    Bridge(Moment first_moment);

    Effect View(Moment m);

    TaggedValues StateTaggedValues(Moment m) const;

  protected:
    std::pair<Effect, ItemProperties> StepImpl(Moment curr,
            RoundInfoView const& round_info_view, int energy_input);

    std::pair<Effect, ItemProperties> BranchImpl(
        Moment curr, Moment dest);

  private:
    /* ID for a property that encodes whether the item is active
     * and a unique value for each set of moments that are connected
     * by the bridge, which is chosen to be the time of activation
     * for the connected set of moments. */
    static int constexpr kStartupTimeID = 0;

    // Initial set of properties for the item.
    static ItemProperties FirstProperties() noexcept;
};
}

#endif //BRIDGE_H
