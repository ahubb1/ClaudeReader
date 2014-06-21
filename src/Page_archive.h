#ifndef PAGE_ARCHIVE_H
#define PAGE_ARCHIVE_H

#define LIBARCHIVE_STATIC
#define LIBZ_STATIC
#include <archive.h>
#include <archive_entry.h>

#include "Page.h"


struct NameNb
{
	string name;
	int nb;
};

class Page_archive : public Page
{
    public:
        Page_archive(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, archive *arch, archive_entry *entry, int img_idx);
    protected:
        void Load(archive *arch, archive_entry *entry, int img_idx);
};

#endif // PAGE_ARCHIVE_H
