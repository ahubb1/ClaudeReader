#include "Page_archive.h"

//! Constructor of the archive page.
//!Initialize, load a page, render it and call the cases detection if needed (not a
//!first or last page).
Page_archive::Page_archive(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, archive *arch, archive_entry *entry, vector<NameNb> img_names):Page(page_nb, nb_pages, rd_mode, page_p)
{
    //Load the page=> to an OpenCv Mat
    Load(arch, entry, img_names);

    //Case detection on the page (test is done later for first or last page)
    DetectCases();
}

//! Load the page from the archive (render it and convert it to an OpenCv Mat)
void Page_archive::Load(archive *arch, archive_entry *entry, vector<NameNb> img_names)
{
	//Skip the pages until the one we want to load
	int i=0;
	while(archive_read_next_header(arch, &entry) == ARCHIVE_OK && i<img_names[PageNb-1].nb)
	{
		archive_read_data_skip(arch);
		i++;
	}

	//The page to load
	char buff[5242880]; //5MB max per image
	//The file should be valid and size>0 but parsing error possible
	int size = archive_read_data(arch, buff, sizeof(char)*5242880);
	if (size <= 0)
	{
	    std::cout << archive_error_string(arch) << std::endl;
	    PageRenderCv = Mat(200, 200, CV_8UC3, cvScalar(255,255,255));
	    line(PageRenderCv, cvPoint(0, 0), cvPoint(199, 199), cvScalar(0,0,255), 2);
	    line(PageRenderCv, cvPoint(199, 0), cvPoint(0, 199), cvScalar(0,0,255), 2);
	}
	else
	{
		//Decode the buffer into a cvMat
		std::vector<unsigned char> cv_buff(buff, buff+size);
		PageRenderCv = imdecode(Mat(cv_buff), 1);
	}
}
