/* Node.cc - implementation of the 'doq::Node' type
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq {

vector<int> Node::get_posi() {
    vector<int> res;

    /* Go upwards in the tree */
    Node* it = this;
    while (it && it->par) {
        int r = -1;
        for (size_t i = 0; i < it->par->sub.size(); ++i) {
            if (it->par->sub[i] == it) {
                r = i;
                break;
            }
        }

        /* We must have found it */
        assert (r >= 0);
        res.push_back(r);

        it = it->par;
    }

    /* Now, reverse it since we started at the end */
    reverse(res.begin(), res.end());

    return res;
}

vector<string> Node::get_posa() {
    vector<string> res;

    /* Go upwards in the tree */
    Node* it = this;
    while (it && it->par) {
        res.push_back(it->name);

        it = it->par;
    }

    /* Now, reverse it since we started at the end */
    reverse(res.begin(), res.end());
    
    return res;
}

Item* Node::toc(bool recurse) {
    Item* res = new Item(Item::Kind::LIST);

    if (!recurse && par && par->name.size() > 0) {
        /* Have 'up' node */
        /*
        Item* v = new Item("");

        v->sub.push_back(new Item(Item::Kind::REF, par->name, { new Item("(^) "), new Item(par->name) }));
        
        res->sub.push_back(v);
        */
    }

    for (size_t i = 0; i < sub.size(); ++i) {
        Item* v = new Item("");
        //v->sub.push_back(new Item(Item::Kind::REF, sub[i]->name, { new Item(to_string(i + 1) + ". "), new Item(sub[i]->name) }));
        v->sub.push_back(new Item(Item::Kind::REF, sub[i]->name, { new Item(sub[i]->name) }));
        if (sub[i]->desc.size() > 0) {
            v->sub.push_back(new Item(": "));
            v->sub.push_back(new Item(sub[i]->desc));
        }
        if (recurse) {
            v->sub.push_back(sub[i]->toc(recurse));
        }
        res->sub.push_back(v);
    }

    return res;
}


}
