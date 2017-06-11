#ifndef ITEM_H
#define ITEM_H

#include <unordered_map>
#include <utility>
#include "aliases.hpp"

namespace timeplane {
class Moment;
}

namespace roundinfo {
class RoundInfoView;
}

namespace item {
class ItemProperties;
class Effect;

class Item {
  public:
    Effect Step(Moment curr, RoundInfoView const& turn_data) {
        std::pair<Effect, ItemProperties> pair = StepImpl(curr, turn_data);
        pending_new_properties = pair.second;
        return pair.first;
    }

    Effect Branch(Moment curr, Moment dest) {
        std::pair<Effect, ItemProperties> pair = BranchImpl(curr, dest);
        pending_new_properties = pair.second;
        return pair.first;
    }

    void ConfirmPending(Moment new_moment) {
        properties_[new_moment] = pending_new_properties;
    }

    virtual std::pair<Effect, ItemProperties> View(Moment m) = 0;

    virtual void ~Item();

  protected:
    Item(Moment first_moment, ItemProperties const& first_properties) {
        properties_[first_moment] = first_properties;
    }

    ItemProperties const& GetProperties(Moment m) const {
        return properties_[m];
    }

    virtual std::pair<Effect, ItemProperties> StepImpl(
        Moment curr, RoundInfoView const& turn_data) = 0;

    virtual std::pair<Effect, ItemProperties> BranchImpl(
        Moment curr, Moment dest) = 0;

  private:
    using namespace timeplane;
    using namespace roundinfo;

    ItemProperties pending_new_properties;
    std::unordered_map<Moment, ItemProperties const> properties_;
    void MomentDeleter(MomentIterators m);
};

virtual void Item::~Item() {}
}

#endif //ITEM_H
