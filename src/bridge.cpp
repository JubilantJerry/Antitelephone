#include <cassert>
#include "moment.hpp"
#include "roundinfoview.hpp"
#include "effect.hpp"
#include "bridge.hpp"

using namespace item;

Bridge::Bridge(Moment first_moment)
    :Item{first_moment, FirstProperties()} {}

ItemProperties Bridge::FirstProperties() noexcept {
    ItemProperties result{};
    result.set_lockdown(kUnlockRequirement);
    result.set_cooldown(kMaxCooldown);
    result.set_custom(kStartupTimeID, -1);
    return result;
}

Effect Bridge::View(Moment m) const {
    return Item::IncrementEffectIf(m);
}

std::pair<Effect, ItemProperties> Bridge::StepImpl(
    Moment curr, RoundInfoView const&, int energy_input) {

    std::pair<Effect, ItemProperties> result =
        std::make_pair(Effect{}, GetProperties(curr));
    int value = result.second.custom(kStartupTimeID);

    if (Item::StandardStepUpdate(result.second, energy_input) &&
            value <= 0) {
        /* Bridge is inactive, but the player has put in enough energy
         * to activate it. Set the property value to the startup time. */
        assert(value == -1);
        result.second.set_custom(kStartupTimeID, curr.time() + 1);
    } else if (result.second.cooldown() == Item::kMaxCooldown
               && value > 0) {
        /* Bridge is active and the cooldown reached the maximum value.
         * So the Bridge deactivates itself by setting the cover number
         * to a negative value */
        assert(value <= curr.time());
        result.second.set_custom(kStartupTimeID, -1);
    }
    result.first = IncrementEffectIf(result.second);
    return result;
}

std::pair<Effect, ItemProperties> Bridge::BranchImpl(
    Moment curr, Moment dest) {

    std::pair<Effect, ItemProperties> result =
        std::make_pair(Item::IncrementEffectIf(dest), GetProperties(dest));
    ItemProperties const& curr_properties = GetProperties(curr);
    int value_curr = curr_properties.custom(kStartupTimeID);
    int value_dest = result.second.custom(kStartupTimeID);

    if (value_curr == value_dest && value_curr > 0) {
        /* The Bridge was active between the current and destination points.
         * So Antitelephone arrival is explicitly allowed here. The value
         * is set to be the destination time to prevent crossing
         * the Bridge multiple times across Antitelephone arrivals. */
        assert(value_dest <= dest.time());
        result.first.set_antitelephone_dest_allowed(true);
        result.second.set_custom(kStartupTimeID, dest.time());
    }
    return result;
}

TaggedValues Bridge::StateTaggedValues(Moment m) const {
    ItemProperties const& properties = GetProperties(m);
    TaggedValues result;
    int lockdown = properties.lockdown();
    if (lockdown > 0) {
        result.emplace_back("unlock_requirement", std::to_string(lockdown));
    } else {
        int value = properties.custom(kStartupTimeID);
        int cooldown = properties.cooldown();
        if (value < 0) {
            result.emplace_back("bridge_activated", "Inactive");
            result.emplace_back("activation_energy",
                                std::to_string(cooldown + 1));
        } else {
            result.emplace_back("bridge_activated", "Active");
            result.emplace_back("bridge_charge_remaining",
                                std::to_string(kMaxCooldown - cooldown));
            result.emplace_back("bridge_startup_time",
                                std::to_string(value));
        }
    }
    return result;
}
