#include "moment.hpp"
#include "roundinfoview.hpp"
#include "effect.hpp"
#include "itemproperties.hpp"
#include "oracle.hpp"

using namespace item;

Oracle::Oracle(Moment first_moment)
    :Item{first_moment, FirstProperties()} {}

ItemProperties Oracle::FirstProperties() noexcept {
    ItemProperties result{};
    result.set_lockdown(kUnlockRequirement);
    result.set_cooldown(kMaxCooldown);
    result.set_custom(kActivatedID, false);
    return result;
}

Effect Oracle::View(Moment m) {
    Effect result = Item::IncrementEffectIf(m);
    ItemProperties const& properties = GetProperties(m);
    if (properties.custom(kActivatedID)) {
        result.set_player_make_active(true);
    }
    return result;
}

std::pair<Effect, ItemProperties> Oracle::StepImpl(
    Moment curr, RoundInfoView const&, int energy_input) {
    std::pair<Effect, ItemProperties> result =
        std::make_pair(Effect{}, GetProperties(curr));
    if (Item::StandardStepUpdate(result.second, energy_input)) {
        // Oracle was activated.
        result.first = IncrementEffectIf(result.second);
        result.first.set_player_make_active(true);
        result.second.set_cooldown(kMaxCooldown);
        result.second.set_custom(kActivatedID, true);
    } else {
        result.first = IncrementEffectIf(result.second);
        result.second.set_custom(kActivatedID, false);
    }
    return result;
}

std::pair<Effect, ItemProperties> Oracle::BranchImpl(Moment, Moment dest) {
    ItemProperties const& properties = GetProperties(dest);
    std::pair<Effect, ItemProperties> result =
        std::make_pair(Item::IncrementEffectIf(properties), properties);
    if (properties.custom(kActivatedID)) {
        result.first.set_player_make_active(true);
    }
    return result;
}

Tags Oracle::StateTags(Moment m) const {
    ItemProperties properties = GetProperties(m);
    Tags result;
    int lockdown = properties.lockdown();
    if (lockdown > 0) {
        result.emplace_back("unlock_requirement", std::to_string(lockdown));
    } else {
        result.emplace_back("activation_energy",
                            std::to_string(properties.cooldown() + 1));
        int value = properties.custom(kActivatedID);
        if (value) {
            result.emplace_back("oracle_activated", "Inactive");
        } else {
            result.emplace_back("oracle_activated", "Active");
        }
    }
    return result;
}
