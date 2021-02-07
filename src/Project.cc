/* Project.cc - implemenation of the 'doq::Project' type
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq {



/** Internal Parsing routines **/


/* Whether we are done */
#define DONE (toki >= toks.size() - 1)

/* Current Token */
#define TOK (toks[toki])

/* Consume current token */
#define EAT() (toks[toki++])


/* Skip space*/
#define SKIP_S() do { \
    while (!DONE && (TOK.kind == Token::Kind::SPACE)) EAT(); \
} while (0)
/* Skip space and newlines */
#define SKIP_SN() do { \
    while (!DONE && (TOK.kind == Token::Kind::SPACE || TOK.kind == Token::Kind::NEWLINE)) EAT(); \
} while (0)




Item* Project::parse_text(vector<Token>& toks, int& toki, bool stopsep) {
    Item* res = new Item(Item::Kind::JOIN);
    while (!DONE && !(TOK.kind == Token::Kind::RBRC || (stopsep && (TOK.kind == Token::Kind::COM || TOK.kind == Token::Kind::NEWLINE)))) {
        if (TOK.kind == Token::Kind::LBRC) {
            /* Block of input with '{}' */
            EAT();

            /* Go until '}' */
            Item* v = new Item(Item::Kind::JOIN);
            while (!DONE && TOK.kind != Token::Kind::RBRC) {
                v->sub.push_back(parse_text(toks, toki));
            }

            if (TOK.kind != Token::Kind::RBRC) {
                throw runtime_error("Expected '}' after block");
            }
            EAT();

            /* Add temporary to output */
            res->sub.push_back(v);

        } else if (TOK.kind == Token::Kind::CASH) {
            /* Internal reference */
            EAT();
            string ref = EAT().get(src);

            /* Create reference */
            Item* v = new Item(Item::Kind::REF, ref, { new Item(ref) });

            /* Add temporary to output */
            res->sub.push_back(v);

        } else if (TOK.kind == Token::Kind::AT) {
            /* Macro call */
            EAT();

            /* Get command being called */
            string cmd = EAT().get(src);
            if (cmd == "@") {
                /* @@ == escape code */
                res->sub.push_back(new Item("@"));

            } else if (cmd == "node") {
                /* Special command, handle that here */
                Item* v = new Item("");

                SKIP_S();
                Item* vt = parse_text(toks, toki, true);
                string pagename = vt->flatten();
                delete vt;
                SKIP_SN();
                if (TOK.kind == Token::Kind::COM) {
                    EAT();
                    SKIP_SN();
                }
                SKIP_S();
                vt = parse_text(toks, toki, true);
                string pagedesc = vt->flatten();
                delete vt;

                /* Last node */
                Node* ln = cur;
                /* New node */         
                Node* nn = new Node(pagename, pagedesc, v, ln);

                /* Append new node */
                ln->sub.push_back(nn);
                cur = nn;

                while (!DONE && TOK.kind == Token::Kind::COM) {
                    EAT();
                    SKIP_SN();
                    nn->val->sub.push_back(parse_text(toks, toki, true));
                }

                cur = ln;

                /* Don't add to output, since it is a page */

            } else {
                /* Parse arguments */
                vector<Item*> args;
                
                /* Parse arguments and append while there are ',' seperators */
                SKIP_SN();
                args.push_back(parse_text(toks, toki, true));
                while (!DONE && TOK.kind == Token::Kind::COM) {
                    EAT();
                    SKIP_SN();
                    args.push_back(parse_text(toks, toki, true));
                }

                /* Add temporary to output */
                Item* v = call(cmd, args);

                res->sub.push_back(v);

                /* TODO: free here? */
                for (size_t i = 0; i < args.size(); ++i) {
                //    delete args[i];
                }
            }

        } else if (TOK.kind == Token::Kind::BBBQUOTE) {
            /* ``` code block */
            EAT();

            /* Determine language */
            string lang = "text";
            if (TOK.kind == Token::Kind::NEWLINE) {
                EAT();
            } else {
                lang = EAT().get(src);
                if (TOK.kind == Token::Kind::NEWLINE) EAT();
            }

            string code = "";
            while (!DONE && TOK.kind != Token::Kind::BBBQUOTE) {
                code += EAT().get(src);
            }
            if (TOK.kind != Token::Kind::BBBQUOTE) {
                throw runtime_error("Expected '```' after code block");
            }
            EAT();

            /* Create code */
            Item* v = new Item(Item::Kind::CODE, lang, { new Item(code) });

            res->sub.push_back(v);

        } else if (TOK.kind == Token::Kind::BQUOTE) {
            /* ` code block */
            EAT();

            string code = "";
            while (!DONE && TOK.kind != Token::Kind::BQUOTE) {
                code += EAT().get(src);
            }
            if (TOK.kind != Token::Kind::BQUOTE) {
                throw runtime_error("Expected '`' after code block");
            }
            EAT();

            /* Create code */
            Item* v = new Item(Item::Kind::MONO, { new Item(code) });

            res->sub.push_back(v);
        } else {
            /* Literal token */
            Token tok = EAT();
            res->sub.push_back(new Item(tok.get(src)));
            if (tok.kind == Token::Kind::NEWLINE) {
                while (!DONE && TOK.kind == Token::Kind::NEWLINE) {
                    EAT();
                }
            }
        }
    }

    return res;
}

Item* Project::call(const string& name, const vector<Item*>& args) {
    map<string, Macro*>::iterator it = macros.find(name);
    if (it == macros.end()) {
        throw runtime_error((string)"Unknown macro: '@" + name + "'");
    }

    Macro* m = it->second;
    if (m->kind == Macro::Kind::CFUNC) {
        return m->cfunc(this, args);
    }
    
    assert(false && "Unknown macro kind (internal error)");
    return NULL;
}


Item* Project::get(const string& key) {
    map<string, Item *>::iterator it = vars.find(key);
    if (it == vars.end()) {
        return Item::empty;
    } else {
        return it->second;
    }
}

void Project::set(const string& key, Item* val) {
    map<string, Item *>::iterator it = vars.find(key);
    if (it == vars.end()) {
        vars[key] = val->copy();
    } else {
        delete it->second;
        it->second = val;
    }
}


/* Construct from file source */
Project::Project(const string& src_) {
    src = src_;

    vars["project"] = new Item("ProjectName");

    /* Initialize functions & variables */
    macros["get"] = new Macro(macro::get);
    macros["set"] = new Macro(macro::set);

    macros["today"] = new Macro(macro::today);
    
    macros["mono"] = new Macro(macro::mono);
    macros["bold"] = new Macro(macro::bold);
    macros["italic"] = new Macro(macro::italic);
    macros["underline"] = new Macro(macro::underline);

    macros["url"] = new Macro(macro::url);
    macros["ref"] = new Macro(macro::ref);

    macros["note"] = new Macro(macro::note);

    macros["list"] = new Macro(macro::list);
    macros["dict"] = new Macro(macro::dict);
    macros["math"] = new Macro(macro::math);
    macros["mathblock"] = new Macro(macro::mathblock);

    /*


    macros["get"] = new Macro(mf_get);
    macros["set"] = new Macro(mf_set);

    macros["list"] = new Macro(mf_list);
    macros["dlist"] = new Macro(mf_dlist);


    macros["code"] = new Macro(mf_code);

    macros["url"] = new Macro(mf_url);
    macros["ref"] = new Macro(mf_ref);

    macros["note"] = new Macro(mf_note);
    */


    /* Transform into tokens */
    vector<Token> toks = tokenize(src);

    /* Create root node */
    cur = root = new Node("", "", new Item(""));
    
    /* Parse and append to root */
    int toki = 0;
    while (!DONE) {
        Item* v = parse_text(toks, toki);
        root->val->sub.push_back(v);
    }

}




}

