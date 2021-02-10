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
    if (x.size() == 0) {
        if (para && needspara) {
            dump("<p>");
            needspara = false;
            inpara = true;
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
        dump_esc("");
        dump("<code>");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</code>");
        break;
    case Item::Kind::MONOI:
        dump_esc("");
        dump("<span class='monoi'>");
        dump(item->sval);
        for (size_t i = 0; i < item->sub.size(); ++i) {
            dump_item(item->sub[i]);
        }
        dump("</span>");
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

                dump("<a href='#");
                dump(id);
                dump("' class='link-div'><svg viewBox='0 0 16 16' aria-hidden='true'><use xlink:href='#svg-link'></use></svg></a>");

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
    doparastk.push_back(true);

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

    /* Add a generated link button */

    dump("<a href='#");
    dump(id);
    dump("' class='link-div'><svg viewBox='0 0 16 16' aria-hidden='true'><use xlink:href='#svg-link'></use></svg></a>");

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

    doparastk.pop_back();

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
    copyfile(dest + "/doq.js", assetpath + "/doq.js");

    // Open the main file
    fp.open(dest + "/index.html", ios::out);

    doparastk.push_back(false);


}

void HTMLOutput::exec() {

    /* HTML text */
    dumpl("<!DOCTYPE html>");
    dumpl("<html lang='en'>");
    dumpl("<head>");
    dumpl("    <meta charset='utf-8'>");
    dumpl("    <meta http-equiv='X-UA-Compatible' content='IE=edge'>");
    dumpl("    <meta name='viewport' content='width=device-width,initial-scale=1.0'>");
    dump("    <title>");
    dump_esc(proj->get("project")->flatten());
    dumpl("</title>");
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
    dumpl("    <script src='./doq.js'></script>");
    dumpl("    <script src='./hljs-ks.js'></script>");
    dumpl("");

    dumpl("</head>");
    dumpl("<body>");
    dumpl("");
    dumpl("<!-- SVG -->");
    dumpl("<svg xmlns='http://www.w3.org/2000/svg' style='display: none;'>");
    dumpl("    <!-- Link SVG -->");
    dumpl("    <symbol id='svg-link' viewBox='0 0 24 24'>");
    dumpl("    <title>Link</title>");
    dumpl("    <svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2' stroke-linecap='round' stroke-linejoin='round' class='feather feather-link'>");
    dumpl("        <path d='M10 13a5 5 0 0 0 7.54.54l3-3a5 5 0 0 0-7.07-7.07l-1.72 1.71'></path>");
    dumpl("        <path d='M14 11a5 5 0 0 0-7.54-.54l-3 3a5 5 0 0 0 7.07 7.07l1.71-1.71'></path>");
    dumpl("    </svg>");
    dumpl("    </symbol>");
    dumpl("</svg>");
    dumpl("");

    /* Generate sidebar */
    dumpl("<div id='sidenav' class='sidenav'><div>");
    dump("<ul>");
    for (size_t i = 0; i < proj->root->sub.size(); ++i) {
        sidebar(proj->root->sub[i]);
    }
    dump("</ul>");

    dumpl("<div class='sidenav-bottom'>");
    dump_item(proj->get("copyright"));
    dumpl("</div>");

    dumpl("</div></div>");

    /* Main content */
    dumpl("<div class='main'><div>");
    dump_node(proj->root);
    dumpl("</div></div>");

    dumpl("<svg class='sidenav-button' onclick='doq_togglesidenav()' viewBox='0 0 100 80' width='40' height='40'><rect width='100' height='20'></rect><rect y='30' width='100' height='20'></rect><rect y='60' width='100' height='20'></rect></svg>");

    /* HTML end */
    dumpl("</body>");
    dumpl("</html>");

}

void HTMLOutput::fini() {
    fp.close();
}

}

