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
#include "Page.h"



//------------------------------------Rectangles functions------------------------

//! Rectangles ordering function.
//! \return true if area of Rect A is smaller than area of Rect B, false otherwise (greater or equal)
bool isSmaller (Rect A, Rect B) { return ((A.width*A.height)<(B.width*B.height)); }

//! Is A at least 20% in B ?
//! \return true if Rect A has, at least, 20% of its surface in B
bool isIn(Rect A, Rect B)
{
    int IntersectL=0, IntersectR=0, IntersectT=0, IntersectB=0;
    IntersectL = max(A.x, B.x);
    IntersectT = max(A.y, B.y);
    IntersectR = min(A.x+A.width, B.x+B.width);
    IntersectB = min(A.y+A.height, B.y+B.height);

   //printf("%d %d\t", (A.width*A.height), (B.width*B.height));

    //Is an intersection ?
    if(IntersectR > IntersectL && IntersectB > IntersectT)
    {
        //Determine the size of the intersection
        int IntersectArea=(IntersectR-IntersectL)*(IntersectB-IntersectT);
        //printf("%d\n", (int)(IntersectArea/((float)A.width*A.height)*100.));

        //If more than 20% of the "inside" rectangle, return true
        if(IntersectArea>0.2*(A.width*A.height))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        //printf("0\n");
        return false;
    }
}

//! Merge rectangle B in A.
//!Redefine the position and size of A (x, y, width and height values) to have B included in it.
void Merge(Rect *A, Rect B)
{
    int x = min(A->x, B.x);
    int y = min(A->y, B.y);
    int w = max(A->x+A->width, B.x+B.width)-x;
    int h = max(A->y+A->height, B.y+B.height)-y;
    A->x=x;A->y=y;A->width=w;A->height=h;
}

//! Min distance between corners of two rectangles.
//! \return the minimum distance possible between two corners of A and B.
int Distance(Rect A, Rect B)
{
    //Define the four corners points of each rectangle
    int a[4][2], b[4][2];
    a[0][0]=A.x;a[0][1]=A.y;
    b[0][0]=B.x;b[0][1]=B.y;

    a[1][0]=A.x+A.width;a[1][1]=A.y;
    b[1][0]=B.x+B.width;b[1][1]=B.y;

    a[2][0]=A.x+A.width;a[2][1]=A.y+A.height;
    b[2][0]=B.x+B.width;b[2][1]=B.y+B.height;

    a[3][0]=A.x;a[3][1]=A.y+A.height;
    b[3][0]=B.x;b[3][1]=B.y+B.height;


    //Find the min distance between all the corners of the two rectangles
    int min=10000000;
    for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
        {
            int d=sqrt( (a[i][0]-b[j][0])*(a[i][0]-b[j][0]) + (a[i][1]-b[j][1])*(a[i][1]-b[j][1]) );
            if( d<min )
                min=d;
        }

    return min;
}


//! Sort and merge a vector of Rects.
//!Sort the Rects by area (see isSmaller()) and merge them (using Merge()) if they
//!are overlapping (as defined by isIn()). The function is recursive.
void SortAndMerge(vector<Rect> *CasesTmp, int counter)
{
    //Remember the nb of time the function is called recursively
    counter++;

    //Enough cases left ?
    if(CasesTmp->size()>1)
    {

        //Order them by area (smallest to largest)
        sort(CasesTmp->begin(), CasesTmp->end(), isSmaller);

        //Merge them when necessary
        std::vector<Rect>::iterator current=CasesTmp->end();
        int currentIdx=0;
        while(current>=CasesTmp->begin())
        {
            current=CasesTmp->end()-currentIdx;
            std::vector<Rect>::iterator i=current-1;
            while(i>=CasesTmp->begin())
            {
                if(isIn(*i, *current) && (*i)!=(*current))
                {
                    Merge(&(*current), *i);
                    i=CasesTmp->erase(i);
                    //Not too much...
                    if(counter<5)
                    {
                        SortAndMerge(CasesTmp, counter);
                        return;
                    }
                    //i--;
                }
                i--;
            }
            currentIdx++;
        }

        //Re-order them by area (smallest to largest)=>their sizes could have changed due to merging
        sort(CasesTmp->begin(), CasesTmp->end(), isSmaller);
    }
}

//---------------------------------------------------------------------------

//! Virtual constructor of the page
//!Initialize some values (common to all page types).
Page::Page(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio)
{
    //Init values common to all type of pages
    Loaded=false;
    ReadingMode=rd_mode;
    PreviewPage=page_p;
    DisplayRatio=display_ratio;
    if(ReadingMode!=PAGE && !PreviewPage)
    {
        ActiveCase=0;
    }
    else
    {
        ActiveCase=-1;
    }
    NbCases=0;
    PageNb=page_nb;
    NbPages=nb_pages;
}


//! Destructor.
Page::~Page()
{
    if(Cases.size()>0)
        Cases.clear();
    ImgRender.release();
    PageRenderCv.release();
}


//! Detect and order the cases on the page.
//!A real detection is done if in CASE or PAGE mode, or a strip cutting is done if
//!in STRIP mode. If the cases detection failed, a default strip cutting is done.
//!The method actually fill the vector<Rect>Cases vector and get the actual number
//! of cases on the page (NbCases).
void Page::DetectCases()
{
    //If we are not on special pages (first or last one)
    if(PageNb>1 && (NbPages-PageNb)>0)
    {
        Mat src, test, test2[3];
        cvtColor(PageRenderCv, src, CV_BGR2GRAY);


        //Only if we are in case or page mode
        if(ReadingMode==CASE || ReadingMode==PAGE)
        {
            //List of Case
            vector<Rect> CasesTmp;
            //Try the detection several times (if it's failed) with a different range of "white" detection
            bool detectionFailed=true;
            int nbdetections=0;
            while(detectionFailed && nbdetections<43)
            {
                detectionFailed=false;

                //To binary
                cvtColor(PageRenderCv, src, CV_BGR2GRAY);
                /*inRange(src, 0, 210+nbdetections, src);*/
                Mat src2;
                char truc[255];
                adaptiveThreshold(src, src, 250+nbdetections, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15, 0);
                sprintf(truc, "truc%d_%dB.png", PageNb, 250+nbdetections);
                imwrite(truc,src2);
                inRange(src, 0, 250+nbdetections, src);
                sprintf(truc, "truc%d_%d.png", PageNb, 250+nbdetections);
                imwrite(truc,src);

                //Contours detection
                vector<vector <Point> > contours;
                vector<Vec4i> hierarchy;
                findContours( src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

                //Rectangle bounding box for each contour found
                for( int i = contours.size()-1; i >-1; i-- )
                {
                   CasesTmp.push_back(boundingRect( contours[i] ));
                }

                //Remove according to ratio perimeter/area (keep only rectangles) ???
                /*approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                   printf("%f\n", matchShapes( approx, contours[i], CV_CONTOURS_MATCH_I1, .0));*/

                //Remove the too LARGE ones (covering almost the whole page: more than 65%)
                for(unsigned int i = 0; i < CasesTmp.size(); i++ )
                {
                    if((CasesTmp[i].width*CasesTmp[i].height)>(0.65*src.rows*src.cols))
                    {
                        CasesTmp.erase(CasesTmp.begin()+i);
                        i--;
                    }
                }

                //Sort the cases by size and merge them if overlaping
                SortAndMerge(&CasesTmp, 0);


                //Remove the too small ones (noise)
                for(unsigned int i = 0; i < CasesTmp.size(); i++ )
                {
                    if((CasesTmp[i].width*CasesTmp[i].height)<(0.01*src.rows*src.cols))
                    {
                        CasesTmp.erase(CasesTmp.begin()+i);
                        i--;
                    }
                }

                //Merge cases between 0.01 and 0.04 to their closest one
                for(unsigned int i = 0; i < CasesTmp.size(); i++ )
                {
                    if((CasesTmp[i].width*CasesTmp[i].height)<(0.04*src.rows*src.cols))
                    {
                        //Find the closest one
                        int mind=src.rows*src.cols; //init to some huge number
                        int closest=-1;
                        for(unsigned int k = 0; k < CasesTmp.size(); k++ )
                        {
                            if(i!=k)
                            {
                                int d=Distance(CasesTmp[i], CasesTmp[k]);
                                if(d<mind)
                                {
                                    mind=d;
                                    closest=k;
                                }
                            }
                        }
                        //Merge
                        if(closest!=-1)
                        {
                            Merge(&CasesTmp[closest], CasesTmp[i]);
                            CasesTmp.erase(CasesTmp.begin()+i);
                            i--;
                        }
                    }
                }

                //Sort the cases by size and merge them if overlaping: done again because sizes and overlapping could have changed
                SortAndMerge(&CasesTmp, 0);


                //Check if parts of the page not covered
                if(CasesTmp.size()>0)
                {
                    //Compute the "within area": the extreme total area covered by the detected cases and sum the actual cases areas
                    Mat testCoveredArea; //Image representing the covered area in white
                    cvtColor(PageRenderCv, testCoveredArea, CV_BGR2GRAY);
                    rectangle(testCoveredArea, CasesTmp[0], cvScalar(255), -1);
                    Rect withinRect(CasesTmp[0]); //Initilaize as the first case
                    int coveredArea=CasesTmp[0].width*CasesTmp[0].height;
                    for(int i = 1; i < CasesTmp.size(); i++ )
                    {
                        //do it by merging all the rectangles
                        Merge(&withinRect, CasesTmp[i]);
                        //sum the cases area
                        coveredArea+=CasesTmp[i].width*CasesTmp[i].height;
                        //paint each case space in white (255)
                        rectangle(testCoveredArea, CasesTmp[i], cvScalar(255), -1);
                    }

                    //If covered area is small (<65% of page), check if the total non-covered area is almost white
                    if(withinRect.width*withinRect.height<(0.65*src.rows*src.cols))
                    {
                        //TODO: et verifier que pas de contour dedans (apres avoir passe un Canny): s'il y a des contours, c'est qu'on a bouffer une bulle ou un texte...
                        //il faudrait aussi faire la meme chose pour les cases...
                        //Check if not covered area (outside the within Rect) is almost white
                        if(mean(testCoveredArea).val[0]<250)
                        {
                            //Detection failed
                            detectionFailed=true;
                            CasesTmp.clear();
                        }
                    }

                    //Then paint the page "contour" (area outside the withinRect) in white (0)
                    rectangle(testCoveredArea, Rect(0, 0, withinRect.x, src.rows), cvScalar(255), -1);//left band
                    rectangle(testCoveredArea, Rect(withinRect.x+withinRect.width, 0, src.cols-(withinRect.x+withinRect.width), src.rows), cvScalar(255), -1);//right band
                    rectangle(testCoveredArea, Rect(0, 0, src.cols, withinRect.y), cvScalar(255), -1);//top band
                    rectangle(testCoveredArea, Rect(0, withinRect.y+withinRect.height, src.cols, src.rows-(withinRect.y+withinRect.height)), cvScalar(255), -1);//bottom band

                    //compare the two values: if two large difference, it means that there are some internal part not covered by a case
                    if(coveredArea<0.95*withinRect.width*withinRect.height)
                    {
                        //TODO: et verifier que pas de contour dedans (apres avoir passe un Canny): s'il y a des contours, c'est qu'on a bouffer une bulle ou un texte...
                        //il faudrait aussi faire la meme chose pour les cases...
                        //Inspect the not covered area: if not 255 means that not fully blank
                        if(mean(testCoveredArea).val[0]<250)
                        {
                            //Detection failed
                            detectionFailed=true;
                            CasesTmp.clear();
                        }
                    }
                    //Everything looks covered...
                    else
                    {
                        //but maybe covered by only one case (not likely to happen since too lage cases have been removed)
                        if(CasesTmp.size()<2)
                        {
                            //Detection failed
                            detectionFailed=true;
                            CasesTmp.clear();
                        }
                    }

                    //Copy the within rectancgle area for further use
                    WithinRect=withinRect;
                }
                else
                {
                    detectionFailed=true;
                }
                nbdetections+=2;
            }


            //Check if we have more than one case
            if(CasesTmp.size()>1)
            {
                //Reorder them in a comics style (TopLeft to Bottom Right)
                //Create a cases tree to reprensent the page organization
                ntree<Rect> CasesTree;

                //Create (and fill) the tree
                    //First the root node: a band on the left of the page
                    //CasesTree.insert(Rect(0, 0, src.cols*.1, src.rows));
                    //First the root node: a band on the left of the covered part of the page
                    CasesTree.insert(Rect(WithinRect.x, WithinRect.y, WithinRect.width*0.1, WithinRect.height));
                    ntree<Rect>::prefix_iterator currentParentPre=CasesTree.prefix_begin();

                    while (CasesTmp.size()>0)
                    {
                        //Use the prefix iterator (updated at the end of the loop) as a simple iterator
                        ntree<Rect>::iterator currentParent=currentParentPre.simplify();

                        //Get all the ones at the same vertical level (same value +10% of page width) and lower than parent
                        //and order them
                        for(unsigned int i = 0; i < CasesTmp.size(); i++ )
                        {
                            //If just on the right and lower or equal to the parent
                            int currentParentRight=currentParent->x+currentParent->width+0.1*src.cols;
                            int currentParentBottom=currentParent->y+currentParent->height+0.1*src.cols;
                            if( CasesTmp[i].x<currentParentRight && (CasesTmp[i].y+CasesTmp[i].height)<currentParentBottom)
                            {
                                //Order them according to their height in the page (lower first)
                                int k=0;
                                int nb_children=CasesTree.children(currentParent);
                                //look for the right position
                                while(k<nb_children && (CasesTmp[i].y+CasesTmp[i].height>=CasesTree.child(currentParent, k)->y+CasesTree.child(currentParent, k)->height)) //tant qu'il y'a des feuilles a ce niveau de l'arbre et que la bottom val est + petite, on avance dans l'arbre a ce niveau
                                {
                                    k++;
                                }
                                //Insert the case at it's right "position" in the child list
                                CasesTree.insert(currentParent, k, CasesTmp[i]);
                                //Remove it from the tmp list
                                CasesTmp.erase(CasesTmp.begin()+i);
                                i--;
                            }
                        }

                        //Go to the next node in prefix mode: deeper if possible, neighbour otherwise, up otherwise
                        currentParentPre++;
                        //Test if we have reached the end of the tree... (not supposed to happen if some cases left in CasesTmp)
                        if(currentParentPre==CasesTree.prefix_end())
                        {
                            break;
                        }
                    }

                    //Copy the cases from the tree to a list, in the right order (except the first one which is a fake case)
                    Cases.clear();
                    for (ntree<Rect>::prefix_iterator i = ++CasesTree.prefix_begin(); i != CasesTree.prefix_end(); i++)
                    {
                        Cases.push_back(*i);
                    }


                //Save the nb of cases
                NbCases=Cases.size();

                //Free some memory
                CasesTree.empty();
            }
            //If less than 2 cases (or the detection has failed)
            else
            {
                float idealStripHeight=(src.cols/(float)DisplayRatio);
                if(idealStripHeight>src.rows)
                    idealStripHeight=src.rows;
                //Creation of cases depending on display size
                Cases.clear();
                float decalage=20*src.rows/(src.rows-idealStripHeight);
                for(int i=0; i<20; i++)
                {
                    //Cases.push_back(Rect(0, i*src.rows/20., src.cols, src.rows*.4-1));
                    Cases.push_back(Rect(0, i*src.rows/decalage, src.cols, idealStripHeight-1));
                }
                NbCases=Cases.size();
            }
            //Release some memory
            CasesTmp.clear();
        } //if CASE or PAGE mode

        //If in STRIP mode
        else
        {
            float idealStripHeight=(src.cols/(float)DisplayRatio);
            if(idealStripHeight>src.rows)
                idealStripHeight=src.rows;
            //Creation of cases depending on display size
            Cases.clear();
            float decalage=20*src.rows/(src.rows-idealStripHeight);
            for(int i=0; i<20; i++)
            {
                //Cases.push_back(Rect(0, i*src.rows/20., src.cols, src.rows*.4-1));
                Cases.push_back(Rect(0, i*src.rows/decalage, src.cols, idealStripHeight-1));
            }
            NbCases=Cases.size();
        }

        //Release some memory
        src.release();

    }//if not special pages
    else
    {
        //If we are on first or last page, no case detectdion, so only case -1 exists
        ActiveCase=-1;
    }

    //Say the page is ready
    Loaded=true;
}

//! Call SetActiveCase with ActiveCase+1 in parameter
//! \return the return value of SetActiveCase
int Page::NextCase()
{
    return SetActiveCase(ActiveCase+1);
}

//! Call SetActiveCase with ActiveCase-1 in parameter
//! \return the return value of SetActiveCase
int Page::PreviousCase()
{
    return SetActiveCase(ActiveCase-1);
}


//! Set a case as active and render it
//! \param val the case number (-1 for the whole page)
//! \return 0 if the page is loaded and the case is valid (the case has been rendered)
//! \return -1 if val if less than -1
//! \return -2 if val if greater than the number of cases (int NbCases)
//! \return -3 if the page is not loaded (no render done)
int Page::SetActiveCase(int val)
{
    //Check if the page is ready to be displayed
    if(Loaded)
    {
        //Check if it's an existing case
        if(val>=-1 && val<NbCases)
        {
            ActiveCase = val;

            //Produce a new render
            RenderCase();

            //Ok
            return 0;
        }

        //No previous case
        if(val<-1)
            return -1;

        //No next case
        if(val>=NbCases)
            return -2;
    }
    return -3;
}

//! Update the case/page rendering according to ActiveCase value.
//!The ImgRender cv::Mat is filled with the case image or the whole page if ReadingMode==PAGE.
//! \return A pointer on the valid ImgRender cv::Mat
Mat * Page::RenderCase()
{
    //Render the whole page
    if(ActiveCase==-1 || ReadingMode==PAGE)
    {
        ImgRender.release();
        PageRenderCv.copyTo(ImgRender);
    }
    //or the active case
    else
    {
        ImgRender.release();
        (PageRenderCv(Cases[ActiveCase])).copyTo(ImgRender);
    }
    return &ImgRender;
}

//! Update the page rendering (whole page).
//!The ImgRender cv::Mat is filled with the page image.
//! \return A pointer on the valid PageRenderCv cv::Mat
Mat * Page::RenderPage()
{
    if(Loaded)
    {
        return &PageRenderCv;
    }
    else
    {
        return NULL;
    }
}
