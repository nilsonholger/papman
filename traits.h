#pragma once

#include <map>
#include <string>
#include <vector>



static const std::string bib_base = "bib/";
static const std::string bib_files = bib_base + "pdf/";
static const std::string bib_index = bib_base + "tex/";
static const std::string bib_meta = bib_base + "meta/";
static const std::string bib_keywords = bib_meta + "keywords";



using Fields = std::vector<std::string>;
using TypeFields = std::map<std::string, Fields>;

#define CUSTOM_FIELDS "short", "url", "keywords", "note", "file"
static const TypeFields type_fields = {
/*	---type----------- ---required fields (from tex bib reference style)-----   ---optional----------------------------------------------------------------------------------------------------------   ---custom (internal)--- */
	{ "article",       { "author", "title", "journal", "year", "volume",        "number", "pages","month", "doi", "abstract",                                                                           CUSTOM_FIELDS } },
	{ "incollection",  { "author", "title", "booktitle", "publisher", "year",   "editor", "volume", "number", "series", "type", "chapter", "pages", "address", "edition", "month", "doi", "abstract",   CUSTOM_FIELDS } },
	{ "inproceedings", { "author", "title", "booktitle", "year",                "editor", "volume", "number", "series", "pages", "address", "month", "organization", "publisher", "doi", "abstract",    CUSTOM_FIELDS } },
	{ "unpublished",   { "author", "title",                                     "year", "month",                                                                                                        CUSTOM_FIELDS } }
};

static const TypeFields export_fields = {
	{ "short", { "author", "title", "short", "year" } },
	{ "long",  { "author", "title", "journal", "booktitle", "year", "month" } }
};

