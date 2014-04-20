#include "Preferences.h"

Preferences::Preferences()
{
    NbRecentDocs=0;
}

Preferences::~Preferences()
{
    //dtor
}

//! Load preferences from the given file
int Preferences::Load(const char prefFilename[255])
{
    //First remember the preferences file filename
    sprintf(Filename, "%s", prefFilename);

    //Open the file
    FILE *prefFile=fopen(Filename, "r");
    if(prefFile)
    {
        //The reading mode, background color and preview page option
        if(fscanf(prefFile, "%d %d %d ", &ReadingMode, &BkgColor, &PagePreview)!=3)
            return -1;

        //The last document filename
        fgets(LastDoc, 1024, prefFile);
        (*strchr(LastDoc,'\n'))='\0'; //replace the last \n by a \0

        //The last doc current page
        if(fscanf(prefFile, "%d ", &LastDocPageNb)!=1)
            return -1;

        //The recent docs
        int i=0;
        while(i<5 && !feof(prefFile))
        {
            fgets(RecentDocs[i], 1024, prefFile);
            //replace the last \n by a \0
            char *c = strchr(RecentDocs[i],'\n');
            if(c)
                *c='\0';
            i++;
        }
        NbRecentDocs=i;

        /*printf("%d %d %d %s %d %d\n", ReadingMode, BkgColor, PagePreview, LastDoc, LastDocPageNb, NbRecentDocs);
        //The recent docs
        for(int j=0; j<NbRecentDocs; j++)
            printf("%d %s\n", j, RecentDocs[j]);
        printf("\n");*/

        //Close the file
        fclose(prefFile);
    }

    return 0;
}

//! Apply the loaded parameters to the document
void Preferences::Apply(Document *doc)
{
    //The reading mode
    doc->SetReadingMode(ReadingMode);

    //The preview/not preview
    doc->SetPagePreview(PagePreview);

    //Check if it was previously open
    if(strcmp(LastDoc, doc->GetFilename())==0)
    {
        //if yes, load the last page
        doc->LoadPage(LastDocPageNb);
    }

}

//! Save the current preferences to the preference file
void Preferences::Save(Document *doc)
{
    //If a doc exists
    if(doc)
    {
        //Open the preferences file
        FILE *prefFile=fopen(Filename, "w");
        if(prefFile)
        {
            //The reading mode, background color and preview page option
            fprintf(prefFile, "%d\n%d\n%d\n", doc->GetReadingMode(), BkgColor, doc->IsPagePreview());

            //The last document filename
            fprintf(prefFile, "%s\n", doc->GetFilename());

            //The last doc current page
            fprintf(prefFile, "%d", doc->GetCurrentPageNb());

            //The recent docs
            //first the current one (if not already)
            if(strcmp(doc->GetFilename(), RecentDocs[0])!=0)
            {
                fprintf(prefFile, "\n%s", doc->GetFilename());
            }
            //then the loaded ones
            for(int i=0; i<NbRecentDocs; i++)
            {
                fprintf(prefFile, "\n%s", RecentDocs[i]);
            }

            //Close the file
            fclose(prefFile);
        }
    }
}
