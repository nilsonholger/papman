#pragma once

#include "traits.h"

#include <experimental/filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::experimental::filesystem;

using Map = std::map<std::string, std::string>;
struct BibEntry { Map meta {}; Map data {}; };
using BibEntryVec = std::vector<std::pair<std::string, std::string>>;
using Bibliography = std::map<std::string, BibEntry>;



// bibliography commands
Bibliography bib_load(const fs::path& dir); // read files from 'bib_tex'/*.bib
void bib_check(const Bibliography& bib); // check bibliography entries and directories
size_t bib_dump(const Bibliography& bib, const std::string& key); // print all fields + meta
void bib_find(const Bibliography& bib, const std::string& key); // search for given key
void bib_format(const Bibliography& bib, const std::string& key = {}); // (re-)format (all) entry(s)
void bib_list(const Bibliography& bib, const std::string& key); // list all entries' ids
// TODO handle multiple item matches in all actions -> edit, open, show, bibtex
Bibliography bib_match(const Bibliography& bib, const std::string& regex); // find/match entry(s)
void bib_store(const BibEntry& entry); // store in 'bib_tex'/<id>.bib

// bibliography entry manipulation commands
BibEntry bib_edit_entry(const BibEntry& entry); // edit only
std::string bib_format_entry(const BibEntry& entry, const std::string& format); // prepare/format entry for export/print
BibEntry bib_import_entry(const fs::path& file); // import/parse from file
void bib_new_entry(const std::string& name, const BibEntry& bibtex = {}); // create new bib item
void bib_print_entries(const Bibliography & bib, const std::string& key, const std::string& format); // display/show/print selected entry(s)

// keyword commands
void keywords_search(const Bibliography& bib, const std::string& keyword);
void keywords_symlink(const Bibliography& bib, const std::string& keyword);

// internal
std::string get_home();



// bibtex entry types and fields (https://ctan.org/tex-archive/biblio/bibtex/base)
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
	{ "phdthesis",     { "author", "title", "school", "year",                   "type", "address", "month"                                                                                              CUSTOM_FIELDS } },
	{ "techreport",    { "author", "title", "institution", "year",              "type", "number", "address", "month",                                                                                   CUSTOM_FIELDS } },
	{ "unpublished",   { "author", "title",                                     "year", "month",                                                                                                        CUSTOM_FIELDS } }
};

static const TypeFields export_fields = {
	{ "short", { "author", "title", "short", "year" } },
	{ "long",  { "author", "title", "journal", "booktitle", "year", "month" } }
};
