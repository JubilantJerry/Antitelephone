#ifndef ITEM_H
#define ITEM_H

#include <unordered_map>
#include <utility>
#include <boost/optional.hpp>
#include "moment.hpp"
#include "aliases.hpp"

namespace roundinfo {
class RoundInfoView;
}

namespace item {
using Moment = timeplane::Moment;
using RoundInfoView = roundinfo::RoundInfoView;

class ItemProperties;
class Effect;

/**
 * @brief The properties of an item in the game across moments.
 *
 * This class contains is virtual and cannot be instantiated directly.
 * The various items in the game inherit from this type.
 */
class Item {
  public:
    /**
     * @brief Maximum cooldown of items, by default.
     */
    static int constexpr kMaxCooldown = 4;

    /**
     * @brief Maximum hitpoints the player has as a basic guarantee.
     */
    static int constexpr kBasicMaxHitpoints = 20;

    /**
     * @brief Maximum hitpoints gained for each new unlocked item.
     */
    static int constexpr kMaxHitpointIncrement = 20;

    /**
     * @brief Maximum hitpoints the player has as a basic guarantee.
     */
    static int constexpr kBasicAttack = 4;

    /**
     * @brief Attack boost gained for each new unlocked item.
     */
    static int constexpr kAttackIncrement = 4;

    /**
     * @brief Update the item across a regular step into the future.
     *
     * The resulting item properties are stored temporarily until the client
     * calls @c ConfirmPending to finalize them. The item properties are
     * updated after considering the events that have occurred in the turn.
     * @param curr          The current moment before the step.
     * @param turn_data     The information about the turn just played.
     * @return The effects granted by the item for the next round.
     */
    Effect Step(Moment curr, RoundInfoView const& round_info_view,
                int energy_input);

    /**
     * @brief Update the item while traveling to the past (branching).
     *
     * The resulting item properties are stored temporarily until the client
     * calls @c ConfirmPending to finalize them.
     * @param curr          The current moment before branching.
     * @param dest          The destination moment to reach.
     * @return The effects granted by the item for the next round.
     * @throws std::invalid_argument If the destination is not in the past.
     */
    Effect Branch(Moment curr, Moment dest);

    /**
     * @brief Finalize the changes in the item after a call to @c Step
     * or @c Branch.
     *
     * This requires that the new moment occurs at time one more than the one
     * provided to @c Step, or has the same time as the destination moment
     * providede to @c Branch.
     * @param new_moment
     */
    void ConfirmPending(Moment new_moment);

    /**
     * @brief View the effects of an item at a specific moment.
     * @param m     The moment to query.
     * @return The effects granted by the item at the specified moment.
     */
    virtual Effect View(Moment m) = 0;

    /**
     * @brief Virtual method to provide user-friendly tagged values.
     *
     * The tags are used as a user-friendly indication of the internal
     * state of the item at a given moment without revealing implementation
     * detail, and can also be used to attach specific meaning to the
     * abstract lockdown and cooldown values associated with all items.
     * @param m     The moment to query.
     * @return A group of tagged values describing the state of an item.
     */
    virtual Tags StateTags(Moment m) const = 0;

    virtual ~Item();

  protected:
    /**
     * @brief Constructor.
     * @param first_moment          The first moment in the game.
     */
    Item(Moment first_moment, ItemProperties const& first_properties);

    /**
     * @brief Accessor for the properties of an item at a specific moment.
     * @param m     The moment to query.
     * @return The properties of the item at the specified moment.
     * @throws std::out_of_range If no properties are stored for the moment.
     */
    ItemProperties const& GetProperties(Moment m) const;

    /**
     * @brief @c Effect instance with basic attack and maximum hitpoints.
     * @return An effect with basic parameters already set.
     */
    static Effect BasicEffect() noexcept;

    /**
     * @brief @c Effect instance with an attack and maximum hitpoint increase.
     *
     * The increase for this overloaded function is only present if the
     * item is not locked down for the specified item properties.
     * @return An effect with the parameters set to increase attack
     *      and maximum hitpoints.
     */
    static Effect IncrementEffectIf(ItemProperties const& p) noexcept;

    /**
     * @brief @c Effect instance with an attack and maximum hitpoint increase.
     *
     * The increase for this overloaded function is only present if the
     * item is not locked down for the specified moment.
     * @return An effect with the parameters set to increase attack
     *      and maximum hitpoints.
     * @throws std::out_of_range If no properties are stored for the moment.
     */
    Effect IncrementEffectIf(Moment m) const;

    /**
     * @brief Standard update algorithm for properties.
     *
     * @param properties        The properties to update.
     * @param energy_input      The amount of energy put into the item.
     * @return Whether the item was actived as a result of the energy input.
     */
    static bool StandardStepUpdate(ItemProperties& properties,
                                   int energy_input);

    /**
     * @brief Virtual method for computing the results of making a step.
     * @param curr          The current moment before the step.
     * @param turn_data     The information about the turn just played.
     * @return The effects granted by the item for the next round, and the
     *      properties of the item for the next round.
     */
    virtual std::pair<Effect, ItemProperties> StepImpl(
        Moment curr, RoundInfoView const& turn_data, int energy_input) = 0;

    /**
     * @brief Virtual method for computing the results of branching.
     * @param curr          The current moment before branching.
     * @param dest          The destination moment to reach.
     * @return The effects granted by the item for the next round, and the
     *      properties of the item for the next round.
     */
    virtual std::pair<Effect, ItemProperties> BranchImpl(
        Moment curr, Moment dest) = 0;

  private:
    boost::optional<ItemProperties> pending_new_properties_;
    std::unordered_map<Moment, ItemProperties const> properties_;

    // Deleter for inaccessible moments.
    void MomentDeleter(timeplane::MomentIterators iterators);
};
}

#endif //ITEM_H
