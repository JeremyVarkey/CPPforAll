// ============================================================================
//  demo.cpp  —  a tiny program that USES the geo library  (provided, complete)
// ----------------------------------------------------------------------------
//  This is the "application" side. It knows NOTHING about how the geo functions
//  work — it only #includes the header (the contract) and calls the API through
//  the geo:: qualifier. The actual code that runs comes from whichever geo.cpp
//  the Makefile links in (starter/ for `make run`, solution/ for `make solution`).
//
//  You don't have to edit this file. Run it with `make run` to watch your
//  library in action: while your bodies are still stubs the numbers are wrong,
//  and they become correct as you finish each task. (`make test` is the real
//  grader; this is just a friendly demonstration.)
// ============================================================================

#include <iostream>
#include "geo.h"        // declarations only — the library's public interface

int main()
{
    // A 3 x 4 rectangle is the running example (its diagonal closes a 3-4-5
    // right triangle, so every number below is clean).
    int w { 3 };
    int h { 4 };

    std::cout << "geo demo — rectangle " << w << " x " << h << "\n";
    std::cout << "  area:       " << geo::rectangleArea(w, h)       << "\n";
    std::cout << "  perimeter:  " << geo::rectanglePerimeter(w, h)  << "\n";
    std::cout << "  diagonal:   " << geo::rectangleDiagonal(w, h)   << "\n";

    std::cout << "right triangle with legs " << w << " and " << h << "\n";
    std::cout << "  area:       " << geo::rightTriangleArea(w, h)   << "\n";
    std::cout << "  hypotenuse: " << geo::hypotenuse(w, h)          << "\n";

    return 0;
}
