#include <iostream>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "drsreadoffline.h"
using namespace std;

static const char *optString = "mho:";
static const struct option longOpts[] = {
    {"help", 0, 0, 'h'},
    {"oneline-mode", 0, 0, 'm'},
    {"outfile", 1, 0, 'o'},
//    {"ddd", 0, 0, 0},
    {0, 0, 0, 0}
};
void help();

int main(int argc, char** argv)
{
    bool oneline_mode = false; // offline or oneline
    int opt = 0;
    int longIndex = 0;

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    while( opt != -1 ) {
        switch( opt ) {
        case 'h':
            cout << " help: " << endl;
            help();
            break;
        case 'o':
            cout << " outputfile " << /*optind <<*/ argv[optind-1] <<endl;
            break;
        case 'm':
            cout << "oneline mode " << endl;
            cout << "***********************************" << endl;
            oneline_mode = true;
            break;
        case 0:
//            if (strcmp("ddd",longOpts[longIndex].name) == 0)
//            cout << "ddd " << endl;
            exit(0);
        default:
            exit(0);
        }
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    }
    cout << optind << "   " << argc << endl;
    if (argc-optind==0) { cout << "No input file specified" << endl; exit(0);}
    if (argc-optind>1) {cout << " many arguments! " << endl; exit(0);}

//    DRSReadOffline *tmp = new DRSReadOffline;
//    tmp->DRS4read(argv[optind]);
//    cout << argv[optind] << endl;
    //tmp->DRS4read(argv[argc-optind]);

    //    if (optind == 0) cout << " inputfile " << argv[optind+2] << endl;
//    if (optind > 1) cout << " many arguments! " << endl;
//    else cout << "No input file specified" << endl;
    return 0;
}

void help()
{
    cout << "Usage:\t drsspectrum INPUTFILE [-o|--outfile] outfile " << endl;
    exit(0);
}
