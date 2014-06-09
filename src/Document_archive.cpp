#include "Document_archive.h"

//-----------------------Archive specific function---------------------
bool operator<(const NameNb &a, const NameNb &b)
{
    return a.name < b.name;
}
//---------------------------------------------------------------------



//! Archive document constructor.
//!First call the default Document constructor and then open and close the doc (to retrieve number of pages and get the pages order).
Document_archive::Document_archive(char *filename):Document(filename)
{
    //Open the file
    Open();

    //Order and get the number of pages (images)
    Order();

    //Close the file
    Close();
}

//! Try to open the archive file.
//! \return 0 if everything ok
//! \return -1 if unable to open the file
int Document_archive::Open()
{
    //Open the archive
	Arch = archive_read_new();
	archive_read_support_filter_all(Arch);
	archive_read_support_format_all(Arch);
	if (archive_read_open_filename(Arch, Filename, 2048) != ARCHIVE_OK)
        return -1;
    else
        return 0;
}

//! Close the archive.
void Document_archive::Close()
{
    archive_read_free(Arch);
}


//! Get the number of images (pages) in the archive and keep track of their order for
//!further access.
void Document_archive::Order()
{
    int i=0, nbpages=0;
	while (archive_read_next_header(Arch, &Entry) == ARCHIVE_OK)
	{
		//Files names
		string buff_s(archive_entry_pathname(Entry));
		//Check the size to know if valid file (and not directory or corrupted)
		int size = archive_entry_size(Entry);
		//if ok, add it to the list
		if(size>0)
		{
            ImgNames.push_back(NameNb());
            ImgNames[nbpages].name=buff_s;
            ImgNames[nbpages].nb=i;
            nbpages++;
		}
        i++;
	}

	//Sort the pages according to their name (should be properly named: blabla00.jpg, blabla01.jpg ...)
	sort(ImgNames.begin(), ImgNames.end());

    //Set the number of pages
	NbPages=nbpages;
}

//! Create a new Page_archive.
Page_archive *Document_archive::CreatePage(int pagenb)
{
    return new Page_archive(pagenb, NbPages, ReadingMode, PagePreview, Arch, Entry, ImgNames);
}
