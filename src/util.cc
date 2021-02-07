/* util.c - Various utilities
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

namespace doq {

/* Asset path */
string assetpath = "./assets";


/* Return whether a character is a word-character
 */
static bool isword(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_' || c == '.' || c == '-';
}

/* Return whether a character cannot be the end of a word
 */
static bool isnotwordend(int c) {
    return c == '.';
}

/* Return whether a character is a space-character
 */
static bool isspace(int c) {
    return c == ' ' || c == '\t' || c == '\r';
}


string readall(const string& fname) {
    ifstream ifs(fname);
    if (!ifs.is_open()) {
        throw runtime_error((string)"Unknown file: " + fname);
    }

    return string(istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}


void copyfile(const string& dest, const string& src) {
    std::ifstream srcfp(src, std::ios::binary);
    if (!srcfp.is_open()) {
        throw runtime_error((string)"Unknown file: " + src);
    }
    std::ofstream destfp(dest, std::ios::binary);
    if (!destfp.is_open()) {
        throw runtime_error((string)"Unknown file: " + dest);
    }
    destfp << srcfp.rdbuf();
}


vector<Token> tokenize(const string& src) {
    vector<Token> res;

    /* Yields whether the next string is '_str' */
    #define NEXTIS(_str) (strncmp(&src[i], _str, sizeof(_str) - 1) == 0)
    //#define NEXTIS(_str) (src.rfind(_str, i) == i)

    /* Whether we are done */
    #define DONE (i >= sl)

    /* Advances a single character */
    #define ADV() do { \
        char _c = src[i]; \
        if (_c == '\n') { \
            line++; \
            col = 0; \
        } else { \
            col++; \
        } \
        i++; \
    } while (0)


    /* Emit a token of the given kind */
    #define EMIT(_kind) do { \
        res.push_back(Token(_kind, spos, i - spos, sline, scol)); \
    } while (0)

    /* Position in the string */
    int line = 0, col = 0;

    /* Start of token values */
    int sline = 0, scol = 0, spos = 0;

    int sl = src.size();
    int i = 0;
    while (!DONE) {
        sline = line;
        scol = col;
        spos = i;
        if (NEXTIS(";;")) {
            /* Comment, skip until EOL */
            while (!DONE) {
                if (src[i] == '\n') {
                    ADV();
                    break;
                } else {
                    ADV();
                }
            }
        } else if (NEXTIS("```")) {
            ADV();
            ADV();
            ADV();
            EMIT(Token::Kind::BBBQUOTE);
        } else if (NEXTIS("`")) {
            ADV();
            EMIT(Token::Kind::BQUOTE);
        } else if (NEXTIS("(")) {
            ADV();
            EMIT(Token::Kind::LPAR);
        } else if (NEXTIS(")")) {
            ADV();
            EMIT(Token::Kind::RPAR);
        } else if (NEXTIS("[")) {
            ADV();
            EMIT(Token::Kind::LBRK);
        } else if (NEXTIS("]")) {
            ADV();
            EMIT(Token::Kind::RBRK);
        } else if (NEXTIS("{")) {
            ADV();
            EMIT(Token::Kind::LBRC);
        } else if (NEXTIS("}")) {
            ADV();
            EMIT(Token::Kind::RBRC);
        } else if (NEXTIS(",")) {
            ADV();
            EMIT(Token::Kind::COM);
        } else if (NEXTIS(":")) {
            ADV();
            EMIT(Token::Kind::COL);
        } else if (NEXTIS("$")) {
            ADV();
            EMIT(Token::Kind::CASH);
        } else if (NEXTIS("@")) {
            ADV();
            EMIT(Token::Kind::AT);
        } else if (NEXTIS("?")) {
            ADV();
            EMIT(Token::Kind::QUES);
        } else if (NEXTIS("!")) {
            ADV();
            EMIT(Token::Kind::EXCL);
        } else if (NEXTIS("\n")) {
            ADV();
            EMIT(Token::Kind::NEWLINE);
        } else if (NEXTIS("\\{")) {
            ADV();
            spos = i;
            ADV();
            EMIT(Token::Kind::OTHER);
        } else if (NEXTIS("\\}")) {
            ADV();
            spos = i;
            ADV();
            EMIT(Token::Kind::OTHER);
        } else if (isword(src[i])) {
            int l = 0;
            do {
                l++;
            } while (!DONE && isword(src[i + l]));

            /* Back off word end */
            while (l > 1 && isnotwordend(src[i + l - 1])) {
                l--;
            }

            while (l > 0) {
                ADV();
                l--;
            }


            EMIT(Token::Kind::WORD);
        } else if (isspace(src[i])) {
            do {
                ADV();
            } while (!DONE && isspace(src[i]));

            EMIT(Token::Kind::SPACE);
        } else {
            /* Literal character */
            ADV();
            EMIT(Token::Kind::OTHER);
        }
    }

    sline = line;
    scol = col;
    spos = i;
    EMIT(Token::Kind::NONE);

    return res;
}


}
