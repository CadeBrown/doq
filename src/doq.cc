/* doq.cc - 'doq' executable main file
 *
 * @author: Cade Brown <cade@kscript.org>
 */

#include <doq.hh>

using namespace doq;

int main(int argc, char** argv) {
    if (argc < 1 + 2) {
        throw runtime_error("Usage: doq [file] [output]");
    }

    string src = readall(argv[1]);

    Project* proj = new Project(src);

    /* Output */
    //Output* out = new TextOutput(proj, argv[2]);
    Output* out = new HTMLOutput(proj, argv[2]);
    out->init();
    out->exec();
    out->fini();

    delete proj;
    delete out;


}
