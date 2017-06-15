#include "timeplane.hpp"

using namespace timeplane;

TimePlane::TimePlane()
    :second_rightmost_timeline_{boost::none},
     handlers_(),
     latest_antitelephone_arrival_{kNoAntitelephoneArrival},
     rightmost_timeline_{
    [this] (MomentIterators iter) {
        this->MomentDeleterFacade(iter);
    }} {}

TimeLine& TimePlane::MakeNewTimeLine(int branch_time) {
    TimeLine new_timeline{
        rightmost_timeline_, branch_time, [this] (MomentIterators iter) {
            this->MomentDeleterFacade(iter);
        }};
    second_rightmost_timeline_ = std::move(rightmost_timeline_);
    rightmost_timeline_ = std::move(new_timeline);
    if (branch_time > latest_antitelephone_arrival_) {
        latest_antitelephone_arrival_ = branch_time;
    }
    return rightmost_timeline_;
}

void TimePlane::MomentDeleterFacade(MomentIterators iterators) {
    for (MomentDeleterFn const& handler : handlers_) {
        handler(iterators);
    }
}
