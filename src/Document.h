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
//    along with BDReader.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Document.h
 * \brief Declaration file of the Document class
 * \author Vincent Crocher
 * \version 0.5
 * \date June 2015
 *
 *
 */

#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <Page.h>


//! Class managing the opened document and creating the different Page objects when needed
class Document
{
    public:
        Document(char *filename);
        virtual ~Document();

        virtual int Open() =0;
        virtual void Close() =0;

        virtual Page *CreatePage(int pagenb) =0;
        int LoadPage();
        int LoadPage(int p);
        void PreloadPages();
        void UpdatePreloadedPages();
        void ReloadPages();

        int WriteDocumentThumbnail(const char *name, int width=128);

        int NextCase();
        int PreviousCase();
        int NextPage();
        int PreviousPage();

        Mat *RenderMat() {return ImgRender=currentPage->RenderCaseMat();}
        Rect *Render() {return RectRender=currentPage->RenderCase();}
        Mat *RenderPage() {return ImgRender=currentPage->RenderPage();}
        Mat *ThumbnailsRenderCurrent();
        Mat *ThumbnailsRenderPrevious();
        Mat *ThumbnailsRenderNext();


        char *GetFilename() {return Filename;}
        void SetReadingMode(READINGMODE val);
        READINGMODE GetReadingMode() {return ReadingMode;}
        int GetNbOfPages() {return NbPages;}
        int GetCurrentPageNb() {return ActivePage;}
        //! Page reached without loading it (scroll mode: the webtoon view loads its pages itself)
        void SetCurrentPageNb(int p) {if(p>0 && p<=NbPages) ActivePage=p;}
        void SetPagePreview(bool val);
        bool IsPagePreview() {return PagePreview;}
        void SetDisplayRatio(float val);
        void SetRotation(unsigned int rot){Rotation=(rot<4)?rot:0;}
        int GetRotation(){return Rotation;}
        void SetContrast(bool cont){ContrastEnhancement=cont;}
        bool GetContrast(){return ContrastEnhancement;}
        void SetRightToLeft(bool val);
        bool GetRightToLeft(){return RightToLeft;}

        Page *currentPage, *nextPage, *previousPage;

    protected:
        char Filename[2048];
        int NbPages;
        int ActivePage;
        bool PagePreview;
        READINGMODE ReadingMode;
        float DisplayRatio;
        unsigned int Rotation;
        bool ContrastEnhancement;
        bool RightToLeft;
    private:
        Mat *ImgRender;
        Rect *RectRender;
        Mat *ThumbnailsImgRenderCurrent;
};

//! Kind of document, used to select the Document class able to open a file
enum DOCUMENT_TYPE { DOC_UNSUPPORTED, DOC_MUPDF, DOC_ARCHIVE, DOC_FOLDER };
DOCUMENT_TYPE GetDocumentType(const char *Filename);

//! Is the file name (or archive entry) a page image, from its extension (case insensitive)
bool IsPageImage(const std::string &name);

//! Read a whole file. The path is UTF-8 (also on Windows, where the C/OpenCV functions use the ANSI code page)
bool ReadFileUtf8(const char *path, vector<unsigned char> *data);
//! Decode a page image (BGR): OpenCV, and FLTK for the GIF images when OpenCV cannot read them (OpenCV 4)
Mat DecodePageImage(const vector<unsigned char> &data);
//! Encode an image according to the path extension and write it. The path is UTF-8
bool WriteImageUtf8(const char *path, const Mat &img);

Document * OpenPerExtension(char *Filename);


#endif // DOCUMENT_H_INCLUDED
