#ifndef ALIASES_H
#define ALIASES_H

namespace timeplane {
class Moment;

using MomentIterators = ::std::pair<
                        ::std::vector<Moment>::const_iterator,
                        ::std::vector<Moment>::const_iterator>;
using MomentDeleter = ::std::function<void(typename MomentIterators)>;
}

#endif //ALIASES_H
