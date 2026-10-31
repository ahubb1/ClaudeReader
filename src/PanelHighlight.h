//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file PanelHighlight.h
 * \brief How the panel being read is shown in panel mode (Settings, saved in the global preferences)
 *
 */
#ifndef PANELHIGHLIGHT_H
#define PANELHIGHLIGHT_H

enum HIGHLIGHT_STYLE { HIGHLIGHT_FRAME, HIGHLIGHT_LINE, HIGHLIGHT_DIM, HIGHLIGHT_CORNERS };
#define HIGHLIGHT_STYLES 4
#define HIGHLIGHT_MAX_THICKNESS 12

struct PanelHighlight
{
    int Style=HIGHLIGHT_FRAME;      //!< HIGHLIGHT_STYLE
    int Thickness=2;                //!< Of the line, in pixels
    unsigned int Color=0x0078D4;    //!< 0xRRGGBB (theme accent by default)

    static const char *StyleName(int style)
    {
        static const char *names[HIGHLIGHT_STYLES]={"Frame", "Line", "Dim the rest", "Corners"};
        return (style>=0 && style<HIGHLIGHT_STYLES) ? names[style] : names[0];
    }
};

#endif // PANELHIGHLIGHT_H
