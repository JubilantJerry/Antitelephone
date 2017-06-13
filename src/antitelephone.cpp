#include "moment.hpp"
#include "roundinfoview.hpp"
#include "effect.hpp"
#include "itemproperties.hpp"
#include "antitelephone.hpp"

using namespace item;

Antitelephone::Antitelephone(Moment first_moment)
    :Item{first_moment, FirstProperties()} {}

ItemProperties Antitelephone::FirstProperties() noexcept {
    ItemProperties result{};
    result.set_lockdown(0);
    result.set_cooldown(kMaxCooldown);
    return result;
}

Effect Antitelephone::View(Moment) {
    return Item::BasicEffect();
}

std::pair<Effect, ItemProperties> Antitelephone::StepImpl(
    Moment curr, RoundInfoView const&, int energy_input) {
    std::pair<Effect, ItemProperties> result =
        std::make_pair(Item::BasicEffect(), GetProperties(curr));
    if (Item::StandardStepUpdate(result.second, energy_input)) {
        // Antitelephone was activated.
        result.first.set_antitelephone_departure(true);
    }
    return result;
}

std::pair<Effect, ItemProperties> Antitelephone::BranchImpl(
    Moment curr, Moment dest) {
    std::pair<Effect, ItemProperties> result =
        std::make_pair(Item::BasicEffect(), GetProperties(dest));
    // Energy debt is increased by the distance traveled.
    int dist = curr.time() - dest.time();
    result.second.set_lockdown(result.second.lockdown() + dist);
    result.second.set_cooldown(Item::kMaxCooldown);
    return result;
}

Tags Antitelephone::StateTags(Moment m) const {
    ItemProperties properties = GetProperties(m);
    Tags result;
    int lockdown = properties.lockdown();
    if (lockdown > 0) {
        result.emplace_back("energy_debt", std::to_string(lockdown));
    } else {
        result.emplace_back("activation_energy",
                            std::to_string(properties.cooldown() + 1));
    }
    return result;
}
