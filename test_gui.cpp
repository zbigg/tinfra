//
// This is a Longest common subsequence problem solution
// as described 
//    http://en.wikipedia.org/wiki/Longest_common_subsequence_problem
//
// this is _naive_, recursive approach without memoization.
// proof of concept, checking if i can still program some algorithm
// DONT TRY IT IN PRODUCTION
//

#include "tinfra/cmd.h"


#include "context.h"
#include "model.h"
#include "layout.h"
#include "render.h"

namespace tg = tinfra::gui;

static tg::dialog  about_box("About box");
static tg::button  confirm_button(&about_box, "confirm_button");


static tg::vertical_box    about_box_layout;
static tg::layout_element  confirm_button_lay(&about_box_layout, &confirm_button);

int test_gui_main(int argc, char** argv)
{
    tg::global_context gui(argc, argv);
    
    tg::show(&about_box, &about_box_layout);
    return 0;
}

TINFRA_MAIN(test_gui_main);

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

