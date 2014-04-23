//--------------------------------------------------------------------------
//
//    Copyright (C) 2014  Vincent Crocher
//
//	  This file is part of BDReader.
//
//    BDReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    BDReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with BDReader.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
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
