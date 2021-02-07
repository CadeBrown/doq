/* macro.c - Implementation of default macros
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq::macro {


Item* get(Project* proj, const vector<Item*>& args) {
    if (args.size() != 1) {
        MACRO_ERROR("'@get' requires 1 argument");
    }

    return proj->get(args[0]->flatten());
}


Item* set(Project* proj, const vector<Item*>& args) {
    if (args.size() < 1) {
        MACRO_ERROR("'@set' requires at least 1 argument");
    }

    string key = args[0]->flatten();
    if (args.size() == 1) {
        proj->set(key, Item::empty);
    } else if (args.size() == 2) {
        proj->set(key, args[1]);
    } else {
        Item* v = new Item(Item::Kind::JOIN);
        for (size_t i = 1; i < args.size(); ++i) {
            v->sub.push_back(args[i]->copy());
        }
        proj->set(key, v);
        delete v;
    }

    return new Item("");
}



Item* today(Project* proj, const vector<Item*>& args) {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);

    return new Item(string(buffer));
}


Item* url(Project* proj, const vector<Item*>& args) {
    if (args.size() == 0) {
        MACRO_ERROR("'@url' requires at least 1 argument");
    }

    string url = args[0]->flatten();
    if (args.size() == 1) {
        return new Item(Item::Kind::URL, url, { new Item(url) });
    } else {
        vector<Item*> sub;
        for (size_t i = 1; i < args.size(); ++i) {
            sub.push_back(args[i]->copy());
        }
        return new Item(Item::Kind::URL, url, sub);
    }
}


Item* ref(Project* proj, const vector<Item*>& args) {
    if (args.size() == 0) {
        MACRO_ERROR("'@ref' requires at least 1 argument");
    }

    string id = args[0]->flatten();
    if (args.size() == 1) {
        return new Item(Item::Kind::REF, id, { new Item(id) });
    } else {
        vector<Item*> sub;
        for (size_t i = 1; i < args.size(); ++i) {
            sub.push_back(args[i]->copy());
        }
        return new Item(Item::Kind::REF, id, sub);
    }
}


Item* mono(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::MONO);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}

Item* bold(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::BOLD);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}

Item* underline(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::UNDERLINE);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}

Item* italic(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::ITALIC);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}

Item* note(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::NOTE);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}


Item* list(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::LIST);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}

Item* dict(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::DICT);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}


Item* math(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::MATH);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}


Item* mathblock(Project* proj, const vector<Item*>& args) {
    Item* res = new Item(Item::Kind::MATHBLOCK);
    for (size_t i = 0; i < args.size(); ++i) {
        res->sub.push_back(args[i]->copy());
    }
    return res;
}



}
