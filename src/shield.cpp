#include "moment.hpp"
#include "roundinfoview.hpp"
#include "effect.hpp"
#include "shield.hpp"

using namespace item;

Shield::Shield(Moment first_moment)
    :Item{first_moment, FirstProperties()} {}

ItemProperties Shield::FirstProperties() noexcept {
    ItemProperties result{};
    result.set_lockdown(kUnlockRequirement);
    result.set_cooldown(kMaxCooldown);
    result.set_custom(kPhantomEnergyID, 0);
    return result;
}

Effect Shield::View(Moment m) const {
    ItemProperties const& properties = GetProperties(m);
    Effect result = Item::IncrementEffectIf(properties);
    if (properties.lockdown() > 0) {
        return result;
    }
    int regular = kMaxCooldown - properties.cooldown();
    int phantom = properties.custom(kPhantomEnergyID);
    result.set_shield_amount(regular + phantom);
    return result;
}

std::pair<Effect, ItemProperties> Shield::StepImpl(
    Moment curr, RoundInfoView const& round_info_view, int energy_input) {

    std::pair<Effect, ItemProperties> result =
        std::make_pair(Effect{}, GetProperties(curr));

    if (result.second.lockdown() == 0) {
        int regular = RegularFromCooldown(result.second.cooldown());
        int phantom = result.second.custom(kPhantomEnergyID);
        int damage = round_info_view.DamageReceived(round_info_view.player());
        if (damage > phantom) {
            result.second.set_custom(kPhantomEnergyID, 0);
            damage -= phantom;
            if (damage > regular) {
                result.second.set_cooldown(kMaxCooldown);
            }
            result.second.set_cooldown(kMaxCooldown - (regular - damage));
        } else {
            result.second.set_custom(kPhantomEnergyID, phantom - damage);
        }
    }

    Item::StandardStepUpdate(result.second, energy_input);
    result.first = IncrementEffectIf(result.second);

    // Compute the shield strength.
    if (result.second.lockdown() == 0) {
        int regular = RegularFromCooldown(result.second.cooldown());
        int phantom = result.second.custom(kPhantomEnergyID);
        result.first.set_shield_amount(regular + phantom);
    }
    return result;
}

std::pair<Effect, ItemProperties> Shield::BranchImpl(
    Moment curr, Moment dest) {

    ItemProperties const& dest_properties = GetProperties(dest);
    std::pair<Effect, ItemProperties> result =
        std::make_pair(Item::IncrementEffectIf(dest), dest_properties);
    if (dest_properties.lockdown() > 0) {
        return result;
    }
    ItemProperties const& curr_properties = GetProperties(curr);
    int regular_curr = RegularFromCooldown(curr_properties.cooldown());
    int phantom_curr = curr_properties.custom(kPhantomEnergyID);
    int regular_dest = RegularFromCooldown(result.second.cooldown());
    int phantom_dest = result.second.custom(kPhantomEnergyID);

    // Take the current energy and half of the current phantom energy.
    // Only use it if it's better than the destination phantom energy.
    int phantom_new = CarryableFromRegularPhantom(regular_curr, phantom_curr);
    if (phantom_dest > phantom_new) {
        phantom_new = phantom_dest;
    }
    result.second.set_custom(kPhantomEnergyID, phantom_new);
    result.first.set_shield_amount(regular_dest + phantom_new);
    return result;
}

TaggedValues Shield::StateTaggedValues(Moment m) const {
    ItemProperties const& properties = GetProperties(m);
    TaggedValues result;
    int lockdown = properties.lockdown();
    if (lockdown > 0) {
        result.emplace_back("unlock_requirement", std::to_string(lockdown));
    } else {
        int regular = RegularFromCooldown(properties.cooldown());
        result.emplace_back("shield_regular", std::to_string(regular));
        int phantom = properties.custom(kPhantomEnergyID);
        if (phantom > 0) {
            result.emplace_back("shield_phantom", std::to_string(phantom));
        }
        result.emplace_back(
            "shield_carryable", std::to_string(
                CarryableFromRegularPhantom(regular, phantom)));
    }
    return result;
}
