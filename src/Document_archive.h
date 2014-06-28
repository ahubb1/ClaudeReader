#ifndef DOCUMENT_ARCHIVE_H
#define DOCUMENT_ARCHIVE_H

#include "Document.h"
#include "Page_archive.h"


class Document_archive : public Document
{
    public:
        Document_archive(char *filename);
    protected:
        int Open();
        void Close();
        void Order();
        Page_archive *CreatePage(int pagenb);
    private:
        struct archive *Arch;
        struct archive_entry *Entry;
        vector<NameNb> ImgNames; //Store the files names and order in the archive
};

#endif // DOCUMENT_ARCHIVE_H
