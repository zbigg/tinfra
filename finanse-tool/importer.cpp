#include "tinfra/csv.h"
#include "tinfra/cmd.h"

#ifdef BUILD_UNITTEST
#include "tinfra/test.h"
#endif

int importer_main(int argc, char** argv)
{
#ifdef BUILD_UNITTEST
    if( argc>1 && strcmp(argv[1], "--tests") == 0)
        return tinfra::test::test_main(argc,argv);
#endif

    return 0;
}

TINFRA_MAIN(importer_main);

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
