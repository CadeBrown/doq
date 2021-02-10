/* doq.hh - 'doq' documentation generator header file
 *
 * In general, doq projects consist of "pages", which are created via '@node /section/subsection/name, { content... }'
 * 
 * 
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#pragma once
#ifndef DOQ_HH__
#define DOQ_HH__

/* C++ std */
#include <iostream>
#include <fstream>

/* C std */
#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


/* STL */
#include <vector>
#include <map>
#include <string>
#include <algorithm>


/* Using 'std::' */
using namespace std;

namespace doq {

/* Asset path */
extern string assetpath;

/* Forward declarations */
struct Project;
struct Item;

/* Type definition of a macro function implemented in C++ */
typedef Item* (*macro_f)(Project* proj, const vector<Item*>& args);



/* Represents a token in the source code
 *
 */
struct Token {

    /* Kind of token */
    enum Kind {
        NONE    = 0,

        /* Generic word, which is anything except special characters 
         * (this includes numbers, identifiers, etc)
         * 
         * Regex:
         * [a-zA-Z0-9_\.\-]+
         */
        WORD,

        /* Non-line-breaking space
         */
        SPACE,

        /* Line break
         *
         * Regex:
         * \n
         */
        NEWLINE,


        /* Any other character literal that should not be processed, but passed along
         */
        OTHER,

        /* ( */
        LPAR,
        /* ) */
        RPAR,

        /* [ */
        LBRK,
        /* ] */
        RBRK,

        /* ` */
        BQUOTE,
        /* ``` */
        BBBQUOTE,

        /* { */
        LBRC,
        /* } */
        RBRC,

        /* , */
        COM,
        /* : */
        COL,

        /* $ */
        CASH,
        /* @ */
        AT,
        /* ? */
        QUES,
        /* ! */
        EXCL,

        /* = */
        EQ,

    } kind;

    /* Position and length (bytes) */
    int pos, len;

    /* Line and column */
    int line, col;

    Token(Kind kind_, int pos_, int len_, int line_, int col_) : kind(kind_), pos(pos_), len(len_), line(line_), col(col_) { }

    /* Gets the relevant string source code */
    string get(const string& src) {
        return src.substr(pos, len);
    }

};


/* Represents an content item, like HTML but more abstract
 *
 * This is the "pure" form before any backend generates code -- backends should be prepared to evaluate
 *   and output each kind of item
 * 
 * NOTE: Implementation of each type depends on the backend
 */
struct Item {

    /* What kind of content it is (this is the basis ) */
    enum Kind {

        /** Text **/

        /* Joins all children items (first, sval, then all children)
         */
        JOIN,

        /* Math section
         */
        MATH,
        /* Monospace section
         */
        MONO,
        /* Monospace inline section
         */
        MONOI,
        /* Bold section
         */
        BOLD,
        /* Italic section
         */
        ITALIC,
        /* Underline section
         */
        UNDERLINE,


        /* Reference to another part of the project
         */
        REF,

        /* Reference to a URL (value in 'sval', view-content in 'sub')
         */
        URL,


        /* Code text, block segment
         */
        CODE,

        /* Math text, block segment
         */
        MATHBLOCK,

        /* Additional note
         */
        NOTE,


        /* List of items, with elements in 'sub'
         */
        LIST,

        /* Dictionary, with key/val pairs (val pairs are typically indented)
         */
        DICT,


    } kind;

    /* String value */
    string sval;

    /* Children Nodes*/
    vector<Item*> sub;

    Item(const string& sval_) : kind(Kind::JOIN), sval(sval_) {}
    Item(Kind kind_, const string& sval_, const vector<Item*>& sub_={}) : kind(kind_), sval(sval_), sub(sub_) {}
    Item(Kind kind_, const vector<Item*>& sub_={}) : kind(kind_), sub(sub_) {}

    ~Item() {
        for (size_t i = 0; i < sub.size(); ++i) {
            delete sub[i];
        }
    }

    /* Return a recursive copy of a node */
    Item* copy();


    /* Return a string of the item, flattened. Mainly used to have a quick and dirty conversion to string */
    string flatten();

    /* Empty string item, "" */
    static Item* empty;

};

/* Represents a node (typically, a single page) that has content as well as connections
 *
 */
struct Node {

    /* The name of the page */
    string name;

    /* The descrpition of the page */
    string desc;

    /* The content of the page */
    Item* val;

    /* Parent of this node, or NULL if there is none */
    Node* par;

    /* Depth of 'dict' commands */
    int dictdep = 0;

    /* Array of children nodes */
    vector<Node*> sub;

    /* reference IDs that the node contains */
    vector<string> contains;

    Node(const string& name_, const string& desc_, Item* val_, Node* par_=NULL, const vector<Node*>& sub_={}) : name(name_), desc(desc_), val(val_), par(par_), sub(sub_) {}

    ~Node() {
        delete val;
        for (size_t i = 0; i < sub.size(); ++i) {
            delete sub[i];
        }
    }

    /* Returns a vector of integers representing the indexes from the root */
    vector<int> get_posi();

    /* Returns a vector of strings representing the page directories */
    vector<string> get_posa();

    /* Generate a table of contents */
    Item* toc(bool recurse=false);

};

/* Macro function definition
 *
 */
struct Macro {

    /* What kind of function? */
    enum Kind {
        /* C-style function */
        CFUNC,

    } kind;

    /* When kind==CFUNC, this is the function that should be called */
    macro_f cfunc;

    Macro(macro_f cfunc_) : kind(Kind::CFUNC), cfunc(cfunc_) {}

};


/* Represents a Doq project
 *
 */
struct Project {

    /* The string source code the project contains */
    string src;

    /* Variables in the project */
    map<string, Item*> vars;

    /* Macros in the project */
    map<string, Macro*> macros;

    /* Root index page of the project */
    Node* root;

    /* Current node being traversed */
    Node* cur;


    /* Construct from file source */
    Project(const string& src_);

    ~Project() {
        for (map<string, Item*>::iterator it = vars.begin(); it != vars.end(); ++it) {
            delete it->second;
        }

        for (map<string, Macro*>::iterator it = macros.begin(); it != macros.end(); ++it) {
            delete it->second;
        }

        delete root;
    }

    /* Call '@<name>(*args)', and return the result */
    Item* call(const string& name, const vector<Item*>& args);

    /* Get a key, or "" if it wasn't found */
    Item* get(const string& key);
    
    /* Set a key */
    void set(const string& key, Item* val);

    /* (INTERNAL)
     * Parses from 'toks', and stops on seperators if 'stopsep' is given
     */
    Item* parse_text(vector<Token>& toks, int& toki, bool stopsep=false);


};


/* Base class of other output types, which explains the interface
 *   for transforming 'Item*' into a project
 *
 * The basic pattern for outputters is:
 * 
 * ```
 * Output* x = new OtherOutput(proj, dest);
 * x->init(); // Initialize output
 * x->exec(); // Execute and output all pages
 * x->fini(); // Finish and clean up
 * ```
 * 
 */
struct Output {
    
    /* The project that it is being output */
    Project* proj;

    /* The destination (may be a file or directory) */
    string dest;
    
    Output(Project* proj_, const string& dest_) : proj(proj_), dest(dest_) {}

    /* Initialize for the specific output format */
    virtual void init() = 0;
    
    /* Execute relevant code and output all pages */
    virtual void exec() = 0;

    /* Finish executing and clean up resources */
    virtual void fini() = 0;

};


/* Text output, which aims to output markdown-like syntax in a single file
 */
struct TextOutput : public Output {

    /* Output file stream */
    ofstream fp;

    /* Indent stack */
    vector<string> indstk;
    

    TextOutput(Project* proj_, const string& dest_) : Output(proj_, dest_) {}

    /* Overrides */
    void init();
    void exec();
    void fini();

    /* (INTERNAL)
     * apply indentation
     */
    void ind();

    /* (INTERNAL)
     * Dumps an object
     */
    template<typename T>
    void dump(T val) {
        fp << val;
    }

    /* (INTERNAL) 
     * Dumps an entire item
     */
    void dump_item(Item* item);

    /* (INTERNAL) 
     * Dumps an entire node
     */
    void dump_node(Node* node);


};


/* HTML output, which aims to output HTML syntax in a single file
 */
struct HTMLOutput : public Output {

    /* Output file stream */
    ofstream fp;

    /* Whether or not to respect paragraphs (use top value) */
    vector<bool> doparastk;

    /* Whether we are currently in a paragraph */
    bool inpara;

    /* Whether we need to add paragraph at next opportunity */
    bool needspara;

    HTMLOutput(Project* proj_, const string& dest_) : Output(proj_, dest_), inpara(false), needspara(true) {}

    /* Overrides */
    void init();
    void exec();
    void fini();

    /* (INTERNAL)
     * apply indentation
     */
    void ind();

    /* (INTERNAL)
     * Dumps an object
     */
    template<typename T>
    void dump(T val) {
        fp << val;
    }

    /* (INTERNAL)
     * Dumps a line
     */
    template<typename T>
    void dumpl(T val) {
        fp << val;
        fp << endl;
    }

    /* (INTERNAL) 
     * Dumps an entire item
     */
    void dump_item(Item* item);

    /* (INTERNAL) 
     * Dumps an entire node
     */
    void dump_node(Node* node);

    /* (INTERNAL) 
     * Dumps a string, HTML-escaped, uses 'doparastk'
     */
    void dump_esc(const string& x);

    /* (INTERNAL) 
     * Returns the plain-ified string of 'x', which replaces spaces
     *   and other characters with similar characters
     */
    string plain(const string& x);

    /* (INTERNAL)
     * Generate sidebar contents for 'node'
     */
    void sidebar(Node* node);


};


/* Read entire file and return its contents
 */
string readall(const string& fname);

/* Turns 'src' into a vector of tokens
 */
vector<Token> tokenize(const string& src);


/* Copies a file
 */
void copyfile(const string& dest, const string& src);


/** Builtin Macros **/

#define MACRO_ERROR(_msg) do { \
    throw runtime_error((string)(_msg)); \
} while (0)


namespace macro {


/* @today
 *
 * Returns a string of '%Y-%m-%d' of the current date
 */
Item* today(Project* proj, const vector<Item*>& args);


/* @get <name>
 *
 * Returns the given variable
 */
Item* get(Project* proj, const vector<Item*>& args);


/* @set <name>, <vals...>
 *
 * Sets the given variables
 */
Item* set(Project* proj, const vector<Item*>& args);


/* @mono <content>...
 * 
 * Creates a mono section
 */
Item* mono(Project* proj, const vector<Item*>& args);

/* @monoi <content>...
 * 
 * Creates a mono(inline) section
 */
Item* monoi(Project* proj, const vector<Item*>& args);

/* @bold <content>...
 * 
 * Creates a bold section
 */
Item* bold(Project* proj, const vector<Item*>& args);

/* @underline <content>...
 * 
 * Creates a mono section
 */
Item* underline(Project* proj, const vector<Item*>& args);

/* @italic <content>...
 * 
 * Creates a italic section
 */
Item* italic(Project* proj, const vector<Item*>& args);

/* @note <content>...
 * 
 * Creates an aside/note
 */
Item* note(Project* proj, const vector<Item*>& args);

/* @url <url>
 * @url <url>, <alt text>...
 * 
 * Creates a URL reference
 */
Item* url(Project* proj, const vector<Item*>& args);

/* @ref <id>
 * @ref <id>, <alt text>...
 * 
 * Creates an internal project reference
 */
Item* ref(Project* proj, const vector<Item*>& args);


/* @list <items>...
 *
 * Creates a list primitive
 */
Item* list(Project* proj, const vector<Item*>& args);

/* @dict <key, val>...
 *
 * Creates a dict primitive
 */
Item* dict(Project* proj, const vector<Item*>& args);

/* @cdict <key, val>...
 *
 * Creates a dict primitive, and adds content to the current page
 */
Item* cdict(Project* proj, const vector<Item*>& args);


/* @math <content>...
 * 
 * Creates a math section
 */
Item* math(Project* proj, const vector<Item*>& args);

/* @mathblock <content>...
 * 
 * Creates a non-inline math block
 */
Item* mathblock(Project* proj, const vector<Item*>& args);


} /* namespace macro */

} /* namespace doq */

#endif /* DOQ_HH__ */

