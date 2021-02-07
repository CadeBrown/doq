/* TextOutput.cc - implementation of the 'TextOutput' class
 *
 * The goal is to produce readable, markdown-like plaintext output
 * 
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq {

void TextOutput::ind() {
    for (size_t i = 0; i < indstk.size(); ++i) {
        dump(indstk[i]);
    }
}

void TextOutput::dump_item(Item* item) {
    switch (item->kind)
    {
    case Item::Kind::MONO:
        dump("`");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("`");
        break;
    case Item::Kind::BOLD:
        dump("**");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("**");
        break;
    case Item::Kind::ITALIC:
        dump("*");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("*");
        break;

    case Item::Kind::REF:
        dump("[");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("](#");
        dump(item->sval);
        dump(")");
        break;

    case Item::Kind::URL:
        dump("[");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("](");
        dump(item->sval);
        dump(")");
        break;

    case Item::Kind::CODE:
        dump("```");
        dump(item->sval);
        dump("\n");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("```");
        break;
    case Item::Kind::MATH:
        dump("$");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("$");
        break;
    case Item::Kind::MATHBLOCK:
        dump("$$");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("$$");
        break;

    case Item::Kind::NOTE:
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        break;

    case Item::Kind::LIST:
        dump("\n");
        indstk.push_back("  ");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            ind();
            dump("* ");
            dump_item(item->sub[i]);
            if (i < item->sub.size() - 1 || indstk.size() <= 1) dump("\n");
        }
        indstk.pop_back();
        break;

    case Item::Kind::DICT:
        dump("\n");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            if (i % 2 == 0) {
                /* Key */
                ind();
                dump_item(item->sub[i]);
                dump("\n");

            } else {
                /* Value */
                indstk.push_back(": ");
                ind();
                dump_item(item->sub[i]);
                indstk.pop_back();
                if (i < item->sub.size() - 1 || indstk.size() <= 1) dump("\n");
            }
        }
        break;

    default:
        /* Default is to join everything together */
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        break;
    }
}

void TextOutput::dump_node(Node* node) {
    vector<int> idxs = node->get_posi();

    /* If we are not the root node */
    if (idxs.size() > 0) {
        /* Output header */
        for (size_t i = 0; i < idxs.size(); ++i) {
            dump("#");
        }
        dump(" ");

        /* Output a section ID */
        for (size_t i = 0; i < idxs.size(); ++i) {
            dump(idxs[i] + 1);
            dump(".");
        }

        /* And the name */
        dump(" ");
        dump(node->name);

        dump("\n");
    }

    /* Dump the content of this node */
    dump_item(node->val);

    /* Also output the children nodes */
    for (size_t i = 0; i < node->sub.size(); ++i) {
        dump_node(node->sub[i]);
    }
}


void TextOutput::init() {
    fp.open(dest, ios::out);
}

void TextOutput::exec() {
    dump_node(proj->root);
}

void TextOutput::fini() {
    fp.close();
}

}

