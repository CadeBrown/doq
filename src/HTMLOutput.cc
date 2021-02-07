/* HTMLOutput.cc - implementation of the 'HTMLOutput' class
 *
 * The goal is to produce readable, HTML output
 * 
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq {


void HTMLOutput::dump_esc(const string& x) {
    bool para = doparastk.back();
    for (size_t i = 0; i < x.size(); ++i) {
        char c = x[i];
        if (c == '\n' && para) {
            if (inpara) {
                dump("</p>\n");
                inpara = false;
            }
            
            i++;
            /* Skip newlines */
            while (i < x.size() && x[i] == '\n') {
                i++;
            }
            needspara = true;

        } else {
            if (para && needspara) {
                dump("<p>");
                needspara = false;
                inpara = true;
            }

            if (c == '<') {
                dump("&lt;");
            } else if (c == '>') {
                dump("&gt;");
            } else {
                dump(c);
            }
        }
    }
}

string HTMLOutput::plain(const string& x) {
    string r = "";
    for (size_t i = 0; i < x.size(); ++i) {
        char c = x[i];
        if (c == '<' || c == '>' || c == '?' || c == '!' || c == '(' || c == ':' || c == ';') {
            return r;
        } else if (c == ' ' || c == '\t') {
            r += "_";
        } else {
            r += c;
        }
    }
    return r;
}

void HTMLOutput::dump_item(Item* item) {
    switch (item->kind)
    {
    case Item::Kind::MONO:
        dump("<code>");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</code>");
        break;
    case Item::Kind::BOLD:
        dump("<b>");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</b>");
        break;
    case Item::Kind::ITALIC:
        dump("<i>");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</i>");
        break;
    case Item::Kind::UNDERLINE:
        dump("<u>");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</u>");
        break;
    case Item::Kind::REF:
        dump("<a href='#");
        dump(plain(item->sval));
        dump("'>");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</a>");
        break;

    case Item::Kind::URL:
        dump("<a href='");
        dump(item->sval);
        dump("'>");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</a>");
        break;

    case Item::Kind::CODE:
        doparastk.push_back(false);

        dump("<pre class='language-");
        dump_esc(item->sval);
        dump("'><code>");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</code></pre>");
        doparastk.pop_back();
        break;

    case Item::Kind::MATH:
        doparastk.push_back(false);
        dump("$");
        dump_esc(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("$");
        doparastk.pop_back();
        break;

    case Item::Kind::MATHBLOCK:
        doparastk.push_back(false);
        dump("$$");
        dump_esc(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("$$");
        doparastk.pop_back();
        break;

    case Item::Kind::NOTE:
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        break;

    case Item::Kind::LIST:
        doparastk.push_back(false);
        dump("<ul>");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump("<li>");
            dump_item(item->sub[i]);
            dump("</li>");
        }
        dump("</ul>");
        doparastk.pop_back();
        break;

    case Item::Kind::DICT:
        dump("<dl>");
        for (size_t i = 0; i < item->sub.size(); ++i) {
            if (i % 2 == 0) {
                doparastk.push_back(false);

                string id = plain(item->sub[i]->flatten());
                if (id.size() > 0) {
                    /* ID-label */
                    dump("<dt id='");
                    dump(id);
                    dump("'>");
                } else {
                    dump("<dt>");
                }

                dump_item(item->sub[i]);
                dump("</dt>");
                doparastk.pop_back();

            } else {
                dump("<dd>");
                dump_item(item->sub[i]);
                dump("</dd>");
            }
        }
        dump("</dl>");
        break;

    default:
        /* Default is to join everything together */
        dump_esc(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        break;
    }
}

void HTMLOutput::dump_node(Node* node) {
    vector<int> idxs = node->get_posi();

    /* Output header */
    string id = plain(node->name);
    if (id.size() > 0) {
        dump("<h");
        dump(idxs.size());
        dump(" id='");
        dump(id);
        dump("'>");
    } else {
        dump("<h");
        dump(idxs.size());
        dump(">");
    }

    /* Output a section ID */
    for (size_t i = 1; i < idxs.size(); ++i) {
        dump(idxs[i] + 1);
        dump(".");
    }

    /* And the name */
    dump(" ");
    dump(node->name);

    /* Close tag */
    dump("</h");
    dump(idxs.size());
    dump(">");
    dump("\n");


    /* Dump table of contents */
    if (id.size() > 0) {
        /* If we are top level, do a full TOC */
        bool recurse = idxs.size() == 1;
        Item* toc = node->toc(recurse);
        dump_item(toc);
        delete toc;
    }

    /* Dump the content of this node */
    dump_item(node->val);

    /* Also output the children nodes */
    for (size_t i = 0; i < node->sub.size(); ++i) {
        dump_node(node->sub[i]);
    }
}

void HTMLOutput::sidebar(Node* node) {
    dump("<li>");

    dump("<a href='#");
    dump(plain(node->name));
    dump("'>");

    dump(node->name);
    dump("</a>");
    
    dump("<ul>");
    for (size_t i = 0; i < node->sub.size(); ++i) {
        sidebar(node->sub[i]);
    }
    dump("</ul>");
    dump("</li>");

}

void HTMLOutput::init() {
    // Make output directory
    mkdir(dest.c_str(), 0777);

    // Copy existing assets
    copyfile(dest + "/doq.css", assetpath + "/doq.css");
    copyfile(dest + "/hljs-ks.js", assetpath + "/hljs-ks.js");

    // Open the main file
    fp.open(dest + "/index.html", ios::out);
    doparastk.push_back(true);

}

void HTMLOutput::exec() {

    /* HTML text */
    dumpl("<!DOCTYPE html>");
    dumpl("<html lang='en'>");
    dumpl("<head>");
    dumpl("    <meta charset='utf-8'>");
    dumpl("    <meta http-equiv='X-UA-Compatible' content='IE=edge'>");
    dumpl("    <meta name='viewport' content='width=device-width,initial-scale=1.0'>");
    dumpl("    <title>kscript docs | v0.0.1</title>");
    dumpl("");
    dumpl("<!-- MathJax -->");
    dumpl("    <script>");
    dumpl("    MathJax = {");
    dumpl("        tex: {");
    dumpl("            inlineMath: [['$', '$']]");
    dumpl("        }");
    dumpl("    };");
    dumpl("    </script>");
    dumpl("    <script src='//polyfill.io/v3/polyfill.min.js?features=es6'></script>");
    dumpl("    <script id='MathJax-script' async src='//cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js'></script>");
    dumpl("");
    dumpl("<!-- highlight.js -->");
    dumpl("    <script src='//cdnjs.cloudflare.com/ajax/libs/highlight.js/10.4.0/highlight.min.js'></script>");
    dumpl("    <script>hljs.initHighlightingOnLoad();</script>");
    dumpl("");
    dumpl("<!-- doq specific assets -->");
    dumpl("    <link rel='stylesheet' href='doq.css'>");
    dumpl("    <script src='./hljs-ks.js'></script>");
    dumpl("");

    dumpl("</head>");
    dumpl("<body>");

    /* Generate sidebar */
    dumpl("<div class='sidenav'><div>");
    dump("<ul>");
    for (size_t i = 0; i < proj->root->sub.size(); ++i) {
        sidebar(proj->root->sub[i]);
    }
    dump("</ul>");
    dumpl("</div></div>");

    /* Main content */
    dumpl("<div class='main'><div>");
    dump_node(proj->root);
    dumpl("</div></div>");

    /* HTML end */
    dumpl("</body>");
    dumpl("</html>");

}

void HTMLOutput::fini() {
    fp.close();
}

}

