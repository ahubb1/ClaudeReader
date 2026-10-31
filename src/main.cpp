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
#include "StoreCli.h"
#include "version.h"


int main(int argc, char **argv)
{
    MainWindow *MW;

    //Arguments as UTF-8 (on Windows they come in the ANSI code page: non-English filenames would be lost)
    Fl::args_to_utf8(argc, argv);

    //Threads (store downloads) can wake up the user interface
    Fl::lock();

    //Store command line: ClaudeReader --store ...
    if(argc>1 && strcmp(argv[1], "--store")==0)
        return StoreCommandLine(argc-2, argv+2);

    //POSIX comand-line options management (-v and -h)
    int OptionChar;             //Option character
    bool rightToLeft=false;     //Manga reading order for export (-r before -e)
    while (1)
    {
        static struct option long_options[] =
        {
            //Supported options
            {"export", required_argument,  0, 'e'},
            {"rtl", no_argument,           0, 'r'},
            {"version", no_argument,       0, 'v'},
            {"help",  no_argument,         0, 'h'},
            //{"output",    required_argument, 0, 'o'},
            {0, 0, 0, 0}
        };
        // getopt_long stores the option index here.
        int option_index = 0;

        OptionChar = getopt_long (argc, argv, ":vhre:", long_options, &option_index);

        // Detect the end of the options
        if (OptionChar == -1)
         break;


         Document *doc;
         switch (OptionChar)
         {
             case 0:
                break;

             //Manga mode: panels read from right to left
             case 'r':
                rightToLeft=true;
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

             //Command line export
             case 'e':
             {
                printf("Exporting %s\n", optarg);
                //Parse parameters is any
                string export_folder, extension;
                bool panelmode=true, pagepreview=true;
                //Positional parameters following the filename
                int nbparams=argc-optind;
                char **params=argv+optind;
                if(nbparams>0)
                    export_folder=params[0];
                else
                    export_folder="./";
                if(nbparams>1)
                    extension=params[1];
                else
                    extension=".jpg";
                if(nbparams>2)
                    if(params[2][0]=='n')
                        panelmode=false;
                if(nbparams>3)
                    if(params[3][0]=='n')
                        pagepreview=false;

                doc=OpenPerExtension(optarg);
                if(doc && doc->GetNbOfPages()>0)
                {
                    doc->SetRightToLeft(rightToLeft);
                    doc->LoadPage(1);
                    Export_Document Export(doc, export_folder.c_str(), extension.c_str(), panelmode, pagepreview);
                    int exported=Export.Export();
                    if(exported>0)
                        printf("%s successfully exported.\n", optarg);
                    else if(exported==-4)
                        printf("Error: some images could not be written in %s (missing folder, no space or permission?).\n", export_folder.c_str());
                    else
                    {
                        printf("An error occured during export.\n");
                        printf("Usage: ClaudeReader [-r] -e filename export_folder [.jpg/.png/.bmp] [y/n] [y/n]\n");
                    }
                    delete doc;
                }
                else
                {
                    printf("Unable to open %s.\n", optarg);
                }
                return 0;
             }
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
