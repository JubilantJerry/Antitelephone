#include "moment.hpp"
#include "effect.hpp"
#include "roundinfoview.hpp"
#include "itemproperties.hpp"
#include "item.hpp"

#include <algorithm>

using namespace item;

Effect Item::Step(Moment curr, RoundInfoView const& round_info_view,
                  int energy_input) {
    if (round_info_view.HealthRemaining(round_info_view.player()) == 0) {
        // Player is dead, and can't input any energy.
        energy_input = 0;
    }
    std::pair<Effect, ItemProperties> pair =
        StepImpl(curr, round_info_view, energy_input);
    pending_new_properties_ = pair.second;
    return pair.first;
}

Effect Item::Branch(Moment curr, Moment dest) {
    if (dest.time() >= curr.time()) {
        throw std::invalid_argument("Destination is not in the past");
    }
    std::pair<Effect, ItemProperties> pair = BranchImpl(curr, dest);
    pending_new_properties_ = pair.second;
    return pair.first;
}

void Item::Duplicate(Moment to_duplicate) {
    pending_new_properties_ = GetProperties(to_duplicate);
}

void Item::ConfirmPending(Moment new_moment) {
    if (!pending_new_properties_) {
        throw std::runtime_error("Item does not have pending properties");
    }
    properties_.emplace(new_moment, pending_new_properties_.get());
    pending_new_properties_ = boost::none;
}

Item::Item(Moment first_moment, ItemProperties const& first_properties) {
    properties_.emplace(first_moment, first_properties);
}

inline ItemProperties const& Item::GetProperties(Moment m) const {
    return properties_.at(m);
}

Item::~Item() {}

Effect Item::BasicEffect() noexcept {
    return Effect{kBasicAttack, kBasicMaxHitpoints,
                  0, false, false, false};
}

Effect Item::IncrementEffectIf(ItemProperties const& p) noexcept {
    if (p.lockdown() == 0) {
        return Effect{kAttackIncrement, kMaxHitpointIncrement,
                      0, false, false, false};
    }
    return Effect{};
}

Effect Item::IncrementEffectIf(Moment m) const {
    return IncrementEffectIf(GetProperties(m));
}

bool Item::StandardStepUpdate(ItemProperties& properties,
                              int energy_input) {
    int lockdown = properties.lockdown();
    if (lockdown <= energy_input) {
        properties.set_lockdown(0);
        energy_input -= lockdown;
        int cooldown = properties.cooldown();
        if (cooldown < energy_input - 1) {
            properties.set_cooldown(0);
            return true;
        }
        int new_cooldown = cooldown - energy_input + 1;
        if (new_cooldown > kMaxCooldown) {
            new_cooldown = kMaxCooldown;
        }
        properties.set_cooldown(new_cooldown);
        return false;
    }
    properties.set_lockdown(lockdown - energy_input);
    return false;
}

void Item::MomentDeleter(timeplane::MomentIterators iterators) {
    std::for_each(iterators.first, iterators.second,
    [this] (Moment m) {
        this->properties_.erase(m);
    });
}
