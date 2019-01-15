#pragma once

#include <map>
#include <string>
#include <vector>



// TODO create/use config file in .config/papman/conf (use bibtex format)

static const std::string bib_base = "bib/";
static const std::string bib_files = bib_base + "pdf/";
static const std::string bib_tex = bib_base + "tex/";
static const std::string bib_meta = bib_base + "meta/";
static const std::string bib_keywords = bib_meta + "keywords";



using Fields = std::vector<std::string>;
using TypeFields = std::map<std::string, Fields>;

#define CUSTOM_FIELDS "short", "url", "keywords", "note", "file"
static const TypeFields type_fields = {
/*	---type----------- ---required fields (from tex bib reference style)-----   ---optional----------------------------------------------------------------------------------------------------------   ---custom (internal)--- */
	{ "article",       { "author", "title", "journal", "year", "volume",        "number", "pages","month", "doi", "abstract",                                                                           CUSTOM_FIELDS } },
	{ "book",          { "author", "title", "publisher", "year",                "volume", "number", "series", "address", "edition", "month",                                                            CUSTOM_FIELDS } },
	{ "incollection",  { "author", "title", "booktitle", "publisher", "year",   "editor", "volume", "number", "series", "type", "chapter", "pages", "address", "edition", "month", "doi", "abstract",   CUSTOM_FIELDS } },
	{ "inproceedings", { "author", "title", "booktitle", "year",                "editor", "volume", "number", "series", "pages", "address", "month", "organization", "publisher", "doi", "abstract",    CUSTOM_FIELDS } },
	{ "manual",        { "title",                                               "author", "organization", "address", "edition", "month", "year",                                                        CUSTOM_FIELDS } },
	{ "techreport",    { "author", "title", "institution", "year",              "type", "number", "address", "month",                                                                                   CUSTOM_FIELDS } },
	{ "unpublished",   { "author", "title",                                     "year", "month",                                                                                                        CUSTOM_FIELDS } }
};

static const TypeFields export_fields = {
	{ "short", { "author", "title", "short", "year" } },
	{ "long",  { "author", "title", "journal", "booktitle", "year", "month" } }
};

