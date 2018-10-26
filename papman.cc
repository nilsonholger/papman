#include "bibtex.h"
#include "traits.h"

#include <iostream>
#include <fstream>



void usage()
{
		std::cout << "usage: <command> [args...]    -- <mandatory> [optional] *default*\n"
			<< "\nbuild:\n"
			<< "   add <file.pdf> [tex.bib]   -- add file.pdf, try file.bib [use tex.bib]\n"
			<< "   edit <id>                  -- edit given id\n"
			<< "\ndiscovery:\n"
			<< "   find <key>                 -- search entries for given key\n"
			<< "   keywords [key]             -- list keywords [or belonging papers]\n"
			<< "   link [keyword]             -- creates keyword[s] hardlink folder[s]\n"
			<< "   list [key]                 -- list paper ids [sort by key: *name*, year]\n" // TODO conf
			<< "\nexport:\n"
			<< "   bibtex <id> [format]       -- print selected [id] (can be regexp, e.g., '.*'), format: short, long, *full*\n"
			<< "\nmaintenance:\n"
			<< "   check                      -- check entries and files in './bib/'\n"
			<< "   format                     -- order and (re)format entries in '" << bib_index << "'\n"
			<< "   dump [id]                  -- dump all/given [id]\n"
			<< std::endl;
		exit(0);
}

int main (int argc, char** argv)
{
	if (argc==1) usage();
	Bibliography bib = bib_load(get_home() + bib_index);
	//TODO LOG STUFF std::cout << get_home() + bib_index << std::endl;
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
		bib_print_entries(bib, argc>2 ? argv[2] : ".*", argc>3 ? argv[3] : "full");
	} else if (arg=="check") {
		bib_check(bib);
	} else if (arg=="format") {
		bib_format(bib);
	} else if (arg=="dump") {
		bib_dump(bib, argc>2 ? argv[2] : "");
	} else {
		usage();
	}

	return 0;
}

