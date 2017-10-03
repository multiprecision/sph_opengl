/*
 * Copyright (c) 2017, Samuel I. Gunadi
 * All rights reserved.
 */

#include "application.hpp"
#include <algorithm>

int main(int argc, char** argv)
{
    // use alternate scene if "-a" is specified in the command line argument
    sph::application app((std::find(argv, argv + argc, std::string("-a")) != argv + argc) ? 1 : 0);
    app.run();
}