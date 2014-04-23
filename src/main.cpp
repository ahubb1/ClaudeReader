/**
 * \file main.cpp
 * \brief Main file of the program containing the main function and creating a MainWindow object.
 * \author Vincent Crocher
 * \version 0.1
 * \date April 2014
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>

#include "MainWindow.h"


int main()
{
    Fl::visual(FL_RGB);
    MainWindow *MW=new MainWindow();
    Fl::run();

    delete MW;

    return 0;
}
