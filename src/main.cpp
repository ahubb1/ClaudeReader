//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016  Vincent Crocher
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
//    along with BDReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file main.cpp
 * \brief Main file of the program containing the main function and creating a MainWindow object.
 * \author Vincent Crocher
 * \version 0.5
 * \date Feb 2016
 *
 *
 */
#include <getopt.h>

#include "MainWindow.h"
#include "helpcmd.h"
#include "version.h"


int main(int argc, char **argv)
{
    MainWindow *MW;

    //POSIX comand-line options management (-v and -h)
    int OptionChar;             //Option character
    while (1)
    {
        static struct option long_options[] =
        {
            //Supported options
            {"export", required_argument,  0, 'e'},
            {"version", no_argument,       0, 'v'},
            {"help",  no_argument,         0, 'h'},
            //{"output",    required_argument, 0, 'o'},
            {0, 0, 0, 0}
        };
        // getopt_long stores the option index here.
        int option_index = 0;

        OptionChar = getopt_long (argc, argv, ":vhe:", long_options, &option_index);

        // Detect the end of the options
        if (OptionChar == -1)
         break;


         Document *doc;
         switch (OptionChar)
         {
             case 0:
                break;

             //Display version (and license) and quit
             case 'v':
                printf("%s\n", VERSION_STRING);
                return 0;
                break;
             //Display help and quit
             case '?':
             case 'h':
                printf("%s\n", HELPCMD_STRING);
                return 0;
                break;

             case 'e':
                printf("Export %s\n", optarg);
                doc=OpenPerExtension(optarg);
                if(doc && doc->GetNbOfPages()>0)
                {
                    doc->LoadPage(1);
                    bool panelmode=true;
                    bool pagepreview=true;
                    Export_Document Export(doc, "./", ".jpg", panelmode, pagepreview);
                    if(Export.Export()>0)
                        printf("%s succesfully exported.\n", optarg);
                    else
                        printf("An error occured during export.\n");
                    delete doc;
                }
                else
                {
                    printf("Unable to open %s.\n", optarg);
                }
                return 0;
                break;

             default:
               abort ();
         }
    }

    //Pass the executable filename/path and if a parameter is passed, use the first one as a potential filename to open
    if(argc>1)
        MW=new MainWindow(argv[0], argv[1]);
    else
        MW=new MainWindow(argv[0]);

    //Run the Fltk main loop
    Fl::run();

    //Delete the MainWindow object when Fltk loops return (on closing)
    delete MW;

    return 0;
}
