/* Node.cc - implementation of the 'doq::Node' type
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq {

Item* Item::empty = new Item("");

Item* Item::copy() {
    Item* res = new Item(kind, sval);
    for (size_t i = 0; i < sub.size(); ++i) {
        res->sub.push_back(sub[i]->copy());
    }

    return res;
}


string Item::flatten() {
    string res = sval;
    for (size_t i = 0; i < sub.size(); ++i) {
        res += sub[i]->flatten();
    }
    return res;
}

}
