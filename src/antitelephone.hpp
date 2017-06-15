#ifndef ANTITELEPHONE_H
#define ANTITELEPHONE_H

#include "itemtype.hpp"
#include "item.hpp"

namespace item {

/**
 * @brief Item which allows traveling to the past.
 */
class Antitelephone : public Item {
  public:
    /**
     * @brief An enum representation of the type of the item.
     *
     * The value of the enum is a unique value for each item, and
     * are consecutive starting at 0 across all items.
     */
    static ItemType constexpr type = ItemType::kAntitelephone;

    Antitelephone(Moment first_moment);

    Effect View(Moment) const;

    TaggedValues StateTaggedValues(Moment m) const;

  protected:
    std::pair<Effect, ItemProperties> StepImpl(Moment curr,
            RoundInfoView const& round_info_view, int energy_input);

    std::pair<Effect, ItemProperties> BranchImpl(
        Moment curr, Moment dest);

  private:
    // Initial set of properties for the item.
    static ItemProperties FirstProperties() noexcept;
};
}

#endif //ANTITELEPHONE_H
