#include "bibtex.h"
#include "traits.h"

#include <iostream>
#include <fstream>



void usage()
{
		std::cout
			<< "usage: <command> [args...]    -- <mandatory> [optional] *default*\n"
			<< "\nbuild:\n"
			<< "   add <file.pdf> [tex.bib]   -- add file.pdf, try file.bib [use tex.bib]\n"
			<< "   edit <id>                  -- edit given id\n"
			<< "\ndiscovery:\n"
			// TODO author <id>                -- list authors papers in chronological order
			<< "   find <key>                 -- search entries for given key\n"
			<< "   keywords [key]             -- list keywords [or belonging papers]\n"
			<< "   link [keyword]             -- creates keyword[s] hardlink folder[s]\n"
			<< "   list [key]                 -- list paper ids [sort by key: *name*, year]\n" // TODO key: conf
			// TODO refs <id>                  -- list/show paper's references (parsed from paper's txt)
			// TODO stats [key]                -- list statistics [by key: ...] (#, per year/conference/author)
			<< "\nexport:\n"
			<< "   bibtex <id> [format [mod]] -- print selected [id] (can be regexp, e.g., '.*'), format: short, long, *full*, mod: see [modifier]\n"
			<< "\nmaintenance:\n"
			<< "   check                      -- check entries and files in './bib/'\n"
			<< "   format [key]               -- order and (re)format entry(s) in '" << bib_tex << "[key]'\n"
			<< "   dump [id]                  -- dump all/given [id]\n"
			// TODO rename <id>                -- rename id (and try to rename associated files)
			<< "\n\n[modifier]\n"
			<< "   none       -- don't modify (default)\n"
			<< "   link       -- use DOI or URL, prefer DOI\n"
			<< "   doi        -- if DOI available: \\href{https://doi.org/<DOI>}{<title>}\n"
			<< "   url        -- if URL available: \\href{<URL>}{<title>}\n"
			<< "   urlonly    -- if DOI empty, but URL available: \\href{<URL>}{<title>}\n"
			<< std::endl;
		exit(0);
}

int main (int argc, char** argv)
{
	if (argc==1) usage();
	Bibliography bib = bib_load(get_home() + bib_tex);
	// TODO logging facility with verbosity level
	// TODO LOG STUFF std::cout << get_home() + bib_tex << std::endl;
	//std::cout << bib.size() << std::endl;

	// <action> *args*
	std::string arg = argv[1];
	if (arg=="add" && argc>2) {
		std::string bib_file;
		if (argc==4) {
			bib_file = argv[3];
		} else {
			bib_file=argv[2];
			bib_file.replace(bib_file.length()-3, std::string::npos, "bib");
		}
		std::ifstream file{bib_file};
		if (file.is_open()) bib_new_entry(argv[2], bib_import_entry(bib_file));
		else bib_new_entry(argv[2]);
	} else if (arg=="edit" && argc>2) {
		bib_store(bib_edit_entry(bib_match(bib, argv[2]).begin()->second)); // TODO should edit all matched entries
	} else if (arg=="find") {
		bib_find(bib, argc>2 ? argv[2] : "");
	} else if (arg=="keywords") {
		keywords_search(bib, (argc>2 ? argv[2] : ""));
	} else if (arg=="link") {
		keywords_symlink(bib, (argc>2 ? argv[2] : ""));
	} else if (arg=="list") {
		bib_list(bib, (argc>2 ? argv[2] : "name"));
	} else if (arg=="bibtex") {
		bib_print_entries(bib, argc>2 ? argv[2] : ".*", argc>3 ? argv[3] : "full", argc>4 ? argv[4] : "none");
	} else if (arg=="check") {
		bib_check(bib);
	} else if (arg=="format") {
		bib_format(bib, argc>2 ? argv[2] : "");
	} else if (arg=="dump") {
		bib_dump(bib, argc>2 ? argv[2] : "");
	} else {
		usage();
	}

	return 0;
}

