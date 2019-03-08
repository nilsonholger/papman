#include "bibtex.h"

#include <unistd.h>
#include <pwd.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <stack>
#include <thread>
#include <vector>



Bibliography bib_load(const fs::path& dir)
{
	Bibliography bib;

	for (auto& file: fs::directory_iterator{dir})
		bib[file.path().stem().string()] = bib_import_entry(file);

	return bib;
}



void bib_check(const Bibliography& bib)
{
	std::function check_msg = [](std::stringstream& ss, const std::string& header) {
		std::string separator;
		separator.resize(header.length(), '=');
		if (!ss.str().empty())
			std::cout
				<< std::endl
				<< header << std::endl
				<< separator << std::endl
				<< ss.str();
		ss.str({});
	};

	// check entries (individual fields)
	std::stringstream msg;
	Bibliography b = bib;
	for (auto& entry: b) {
		std::string missing;
		std::string type;
		try {
			type = entry.second.meta.at("type");
		} catch (std::out_of_range) {
			std::cout << entry.second.meta.at("file") << ": no 'type' field found in meta data!" << std::endl;
			continue;
		}
		try {
			type_fields.at(type);
		} catch (std::out_of_range) {
			std::cout << entry.second.meta.at("file") << ": '" << type << "' not a valid type!" << std::endl;
			continue;
		}
		for (auto& field: type_fields.at(type)) {
			try {
				if (entry.second.data.at(field).empty())
					missing += " <" + field + ">";
			} catch (std::out_of_range) {
				missing += " [" + field + "]";
			}
			if (field=="file") {
				std::string path;
				try { path = entry.second.data.at(field); }
				catch (std::exception e) {
					std::cerr << "Error checking " << entry.first << "." << field << ": " << e.what() << std::endl;
				}
				if (!fs::is_regular_file(get_home() + path))
					missing += " !file!";
			}
			entry.second.data.erase(field);
		}

		if (!entry.second.data.empty())
			for (auto& field: entry.second.data)
				missing += " +" + field.first + "+";
		if (! missing.empty()) msg << entry.first << ":" << missing << std::endl;
	}
	check_msg(msg, "PLEASE FIX FIELDS: <EMPTY> !NOT_FOUND! [MISSING] +EXTRA+");

	// check bib directories (correct files in correct places, with correct id???)
	for (auto& dir: {get_home() + bib_files, get_home() + bib_tex}) {
		for (fs::directory_iterator file{dir}; file != fs::directory_iterator(); ++file) {
			std::string id = file->path().string();
			id.erase(0, id.find_last_of('/')+1);
			if(id.find(".") != std::string::npos)
				id.erase(id.find("."), std::string::npos);
			if (bib.find(id)==bib.end())
				msg << *file << std::endl;
		}
	}
	check_msg(msg, "NO BIB ENTRY FOUND");

	// check keyword list
	BibEntry keywords = bib_import_entry(get_home() + bib_keywords);
	for (auto& entry: bib) {
		std::string k;
		try {
			k = entry.second.data.at("keywords");
		} catch (std::out_of_range e) {
			msg << "entry missing keywords field" << std::endl;
			continue;
		}
		std::string incorrect;
		while (!k.empty()) {
			auto sep = k.find_first_of(';');
			if (sep!=std::string::npos) {
				if (keywords.data.find(k.substr(0, sep))==keywords.data.end())
					incorrect += k.substr(0, sep) + ";";
				k.erase(0, sep+1);
			} else {
				if (keywords.data.find(k)==keywords.data.end())
					incorrect += k.substr(0, sep) + ";";
				k.clear();
			}
		}
		if (!incorrect.empty()) msg << entry.first << ": " << incorrect << std::endl;
	}
	check_msg(msg, "PLEASE FIX (OR ADD) INCORRECT KEYWORDS:");
}



size_t bib_dump(const Bibliography& bib, const std::string& key)
{
	Bibliography entries = bib_match(bib, key);
	if (entries.empty()) return 0;

	for (auto& entry: entries) {
		unsigned int padding = 0;
		for (auto& e: entry.second.meta)
			if (e.first.length()>padding) padding = e.first.length();
		for (auto& e: entry.second.data)
			if (e.first.length()>padding) padding = e.first.length();

		try {
			std::cout << entry.second.meta.at("id") << ":" << std::endl;
		} catch (std::out_of_range) {
			std::cout << entry.second.meta.at("file") << ": no id found!" << std::endl << std::endl;
			continue;
		}
		std::cout << "|-------META" << std::endl;
		for (auto& e: entry.second.meta)
			std::cout << "|\t" << e.first << std::setw(padding-e.first.length()+2) << ": " << std::setw(0) << e.second << std::endl;
		std::cout << "\\-------DATA" << std::endl;
		for (auto& e: entry.second.data)
			std::cout << "\t" << e.first << std::setw(padding-e.first.length()+2) << ": " << std::setw(0) << e.second << std::endl;
		std::cout << std::endl;
	}

	return bib.size();
}



void bib_find(const Bibliography& bib, const std::string& key)
{
	// use lowercase key
	std::string k{key};
	std::transform(k.begin(), k.end(), k.begin(), ::tolower);

	// search for key occurences
	for (auto& item: bib) {
		std::vector<std::string> matches;
		// always compare lowercase versions
		for (auto& item: item.second.data) {
			std::string data = item.second;
			std::transform(data.begin(), data.end(), data.begin(), ::tolower);
			if (data.find(k)!=std::string::npos)
				matches.emplace_back(item.first + ": " + item.second);
		}
		// output matches
		if (!matches.empty()) {
			std::cout << item.first << ":" << std::endl;
			for (auto match: matches)
				std::cout << "\t" << match << std::endl;
		}
	}
}



void bib_format(const Bibliography& bib, const std::string& key)
{
	if (!key.empty()) {
		// format matching entries only
		Bibliography entries = bib_match(bib, key);
		for (auto& entry: entries)
			bib_store(entry.second);
	} else {
		// format all entries
		for (auto& entry: bib)
			bib_store(entry.second);
	}
}

void bib_list(const Bibliography& bib, const std::string& key)
{
	if (key=="name") {
		for (auto& entry: bib)
			std::cout << entry.first << std::endl;
	} else if (key=="year") {
		std::vector<std::pair<std::string, std::string>> entries;
		for (auto& entry: bib)
			try {
				entries.push_back({entry.first, entry.second.data.at("year")});
			} catch (std::out_of_range) {
				std::cout << "WARNING " << entry.first
					<< " has no \"year\" field!" << std::endl;
			}
		std::sort(entries.begin(), entries.end(),
				[](auto& e1, auto& e2) {return e1.second<e2.second;});
		for (auto& entry: entries)
			std::cout << entry.first << std::endl;
	} else {
		std::cout << "ERROR Key '" << key << "' not valid!" << std::endl;
	}
}



Bibliography bib_match(const Bibliography& bib, const std::string& regex)
{
	Bibliography entries;
	if (bib.find(regex) != bib.end()) {
		entries[regex] = bib.at(regex);
	} else {
		try {
			std::regex match{".*"+regex+".*"};
			for (auto& entry: bib)
				if (std::regex_match(entry.first, match))
					entries[entry.first] = entry.second;
		} catch (std::regex_error e) {
			std::cout << "WARNING Regex problem: " << e.what() << std::endl;
		}
	}
	return entries;
}



void bib_store(const BibEntry& entry)
{
	std::ofstream file;
	try {
		file.open(get_home() + bib_tex + entry.meta.at("id") + ".bib");
	} catch (std::out_of_range e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	std::string str = bib_format_entry(entry, "custom");
	if (!str.empty())
		file << str;
	file.close();
}



BibEntry bib_edit_entry(const BibEntry& entry)
{
	fs::path file{fs::temp_directory_path().string() + "/papman_" + fs::path(entry.meta.at("file")).filename().string() + ".bib"};

	// check tmp file status and populate
	if (fs::exists(file)) {
		std::cout << "Using existing file: " << file.string() << std::endl;
	} else {
		std::ofstream out{file};
		out << bib_format_entry(entry, "custom");
		out.close();
	}
	std::system(std::string{"${EDITOR} \"" + file.string() + "\""}.c_str());

	// TODO check item's id, should be lastnameCONFYEARdesc
	// TODO prevent duplicate entry addition/creation
	// TODO if temp file empty, delete it

	// load and check (edited) item
	BibEntry entry_new = bib_import_entry(file);
	if (entry_new.data.empty()) {
		std::cerr << "Could not add '" << entry.meta.at("id")
			<< "' from: " << file.string() << std::endl;
		std::exit(EXIT_FAILURE);
	} else {
		std::system(std::string{"rm \"" + file.string() + "\""}.c_str());
		return entry_new;
	}
}



std::string bib_format_entry(const BibEntry& entry, const std::string& format)
{
	std::string s;
	BibEntryVec vec;

	// add meta information
	try {
		vec = { { "_type", entry.meta.at("type") }, { "_id", entry.meta.at("id") } };
	} catch (std::out_of_range e) {
		std::cout << e.what() << std::endl;
		return {};
	}

	// format found in export_fields -> use only specified fields
	if (export_fields.find(format) != export_fields.end()) {
		BibEntryVec selection{};
		for (auto& field: export_fields.at(format)) {
			try {
				vec.push_back(std::make_pair(field, entry.data.at(field)));
			} catch (std::out_of_range) {
				//std::cout << "WARNING " << entry.meta.at("id") << " has no field: " << field << std::endl;
			}
		}
	} else {
		// check given format
		if (!(format=="full" || format=="custom")) {
			std::cout << "WARNING Invalid format: " << format;
			return {};
		}

		// "reorder" fields as specified by type_fields (includes ALL fields)
		try {
			if (type_fields.find(entry.meta.at("type")) != type_fields.end()) {
				for (auto& field: type_fields.at(entry.meta.at("type"))) {
					try {
						vec.push_back(std::make_pair(field, entry.data.at(field)));
					} catch (std::out_of_range) {
						//std::cout << "WARNING " << entry.meta.at("id") << " has no field: " << field << std::endl;
					}
				}
			}
		} catch (std::out_of_range) {
			std::cout << "WARNING Unknown bibtex entry type: " << entry.meta.at("type") << std::endl;
			return {};
		}
	}

	// format: full -> remove CUSTOM_FIELDS
	if (format=="full") {
		Fields custom_fields = { CUSTOM_FIELDS };
		for (auto& field: custom_fields) {
			try {
				for (auto it=vec.end(); it !=vec.begin(); it--) {
					if (it->first==field)
						vec.erase(it);
				}
			} catch (std::exception e) {
				std::cout << "WARNING: " << e.what() << std::endl;
			}
		}
	}

	// sort keywords (if necessary)
	auto it = std::find_if(vec.begin(), vec.end(), [](std::pair<std::string,std::string> p){return p.first=="keywords";});
	if (it!=vec.end()) {
		std::string keywords = it->second;
		std::vector<std::string> tmp;
		std::string sorted;
		while (keywords.find_first_of(';')!=std::string::npos) {
			tmp.emplace_back(keywords.substr(0, keywords.find_first_of(';')));
			keywords.erase(0, keywords.find_first_of(';')+1);
		}
		if (!keywords.empty()) tmp.emplace_back(keywords);
		std::sort(tmp.begin(), tmp.end());
		for (auto& t: tmp) sorted += t + ";";
		if (!sorted.empty()) sorted.erase(sorted.end()-1);
		it->second = sorted;
	}

	// calculate padding
	unsigned int padding = 0;
	for (auto& v: vec)
		if (v.first.length()>padding) padding = v.first.length();

	// assemble final string
	s = "@" + vec[0].second + "{" + vec[1].second;
	for (auto e = vec.begin()+2; e != vec.end(); ++e) {
		std::string content = e->second;
		size_t lead = content.find_first_not_of(' ');
		size_t tail = content.find_last_not_of(' ');
		content = content.substr(lead==std::string::npos?0:lead, tail==0?std::string::npos:tail+1);
		s += ",\n\t" + e->first;
		s.insert(s.size(), padding-e->first.length(), ' ');
		s += " = {" + content + "}";
	}
	s += "\n}\n";

	return s;
}



BibEntry bib_import_entry(const fs::path& file)
{
	BibEntry entry;
	entry.meta["file"] = file.filename().string();
	std::ifstream in{file};
	char token {};
	std::string str {};
	std::string field {};
	bool escape_char {};
	std::stack<char> delim {};
	enum class type : short {
		none, entry_type, id, field, value
	} type {type::none};

	if (!in.is_open()) return {};

	while (in.get(token)) // TODO speedup using std::basic_istream::getline() searching for specific stop token?
	{
		// simply add escaped characters to string
		if (escape_char) {
			str += token;
			escape_char = false;
			continue;
		}

		switch (token) {
			case '@': // '@' usually starts new bibtex entry
				if (type==type::none) type = type::entry_type;
				else if (type==type::value) str+= token;
				break;

			case ' ': case '\t': // ignore whitespace unless inside value
				if (type==type::value) str += token;
				break;

			case '=': // separate field and value
				if (type==type::field) {
					field = str;
					str = {};
					type = type::none;
				} else if (type==type::value) {
					str += token;
				}
				break;

			case '\\': // ignore next token
				if (type==type::value) str += token;
				escape_char = true;
				break;

			case ',': // store 'id' or field=value pair
				if (type==type::id) {
					entry.meta["id"] = str;
					str = {};
					type = type::field;
				} else if (type==type::none) {
					std::transform(field.begin(), field.end(), field.begin(), ::tolower);
					entry.data[field] = str;
					str = {};
					type = type::field;
				} else if (type==type::value) {
					str += token;
				}
				break;

			case '{': // { used as value delimiter, inside value or delimits type and id
				if (delim.size()==1) type = type::value;
				if (delim.size()>1 && type==type::value) str += token;
				if (type==type::entry_type) {
					std::transform(str.begin(), str.end(), str.begin(), ::tolower);
					entry.meta["type"] = str;
					str = {};
					type=type::id;
				}
				delim.push('{');
				break;

			case '}': // } used as delimiter or inside value, might have to store last field=value pair
				if (delim.top()=='{') delim.pop();
				if (delim.size()==1) type = type::none;
				if (delim.size()>1 && type==type::value) str += token;
				if (delim.size()==0 && !str.empty()) {
					std::transform(field.begin(), field.end(), field.begin(), ::tolower);
					entry.data[field] = str;
					str = {};
				}
				break;

			case '"': // " used as delimiter (must be escaped inside value)
				if (delim.top()=='"') delim.pop();
				else {
					type = type::value;
					delim.push('"');
				}
				if (delim.size()==1) type = type::none;
				break;

			case '\n': // newline should (sometimets) turn into a space
				if (type==type::value && !str.empty()) str += " ";
				break;

			default: // simple (leftover) token
				str += token;
		}
	}

	// entry will ALWAYS contain .meta["file"], even if nothing else
	return entry;
}



BibEntry bib_modify_entry(const BibEntry& entry, const std::string& modifier)
{
	BibEntry e = entry;
	std::function try_mod_catch = [&e](const std::string& field, const std::string& prefix){
		try {
			if (!e.data.at(field).empty())
				e.data.at("title") = "\\href{" + prefix + e.data.at(field) + "}{" + e.data.at("title") + "}";
		} catch (std::out_of_range) {
			std::cerr << "WARNING " << e.meta.at("id")
				<< " has no \"" << field << "\" field!" << std::endl;
		}
	};

	if (modifier=="none") {
		return e;
	} else if (modifier=="link") {
		if (e.data.find("doi")!=e.data.end() && !e.data.at("doi").empty())
			try_mod_catch("doi", "https://doi.org/");
		else if (e.data.find("url")!=e.data.end() && !e.data.at("url").empty())
			try_mod_catch("url", "");
	} else if (modifier=="doi") {
		try_mod_catch("doi", "https://doi.org/");
	} else if (modifier=="url") {
		try_mod_catch("url", "");
	} else if (modifier=="urlonly") {
		try {
			if (!e.data.at("url").empty()) {
				if (e.data.find("doi")==e.data.end())
					try_mod_catch("url", "");
				else if (e.data.at("doi").empty())
					try_mod_catch("url", "");
			}
		} catch (std::out_of_range) {
			std::cerr << "WARNING " << e.meta.at("id")
				<< " has no \"url\" field!" << std::endl;
		}
	} else {
		std::cerr << "WARNING Invalid modifier: " << modifier << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return e;
}

void bib_new_entry(const std::string& name, const BibEntry& bibtex)
{
	BibEntry entry = bibtex;
	entry.meta["file"] = name;

	// select type if entry empty
	if (entry.data.empty()) {
		std::cout << "Please select entry type:" << std::endl;
		unsigned short counter = 0;
		unsigned short selection = 0;
		for (auto& type: type_fields) {
			std::cout << "(" << counter++ << ") " << type.first << std::endl;
		}
		std::cout << "? ";
		std::cin >> selection;
		if (selection>type_fields.size()-1) {
			std::cout << "Invalid selection: " << selection << std::endl;
			return;
		} else {
			// TODO catch map::at
			entry.meta["type"] = std::next(type_fields.begin(), selection)->first;
			std::cout << "Selected: " << entry.meta.at("type") << std::endl;
		}
	}

	// fill/create (empty) entry using selected type
	try { entry.meta.at("id"); } catch (std::out_of_range) { entry.meta["id"] = "CHANGE_ME"; }
	try {
		for (auto& field: type_fields.at(entry.meta.at("type"))) {
			try { entry.data.at(field); } catch (std::out_of_range) { entry.data[field] = ""; }
		}
	} catch (std::out_of_range) {
		std::cout << "Unsupported bibtex entry type requested: '" << entry.meta.at("type") << "'" << std::endl;
		return;
	}

	// TODO create output without extra pair of curly braces
	// suggest title protection (prevents bib style to alter case)
	if (entry.data.at("title").empty()) entry.data.at("title") = "{}";

	// list all currently known keywords
	BibEntry keywords = bib_import_entry(get_home() + bib_keywords);
	std::string keywords_str {};
	for (auto& k: keywords.data) keywords_str = keywords_str + k.first + ";";
	if (!keywords_str.empty()) keywords_str.erase(keywords_str.end()-1);
	entry.data.at("keywords") = keywords_str;

	// edit entry
	entry = bib_edit_entry(entry);
	while (entry.meta.at("id")=="CHANGE_ME") {
		std::cout << "Please edit the item's id (CHANGE_ME)!" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
		entry = bib_edit_entry(entry);
	}

	// TODO check entry's correctness: id, ...
	// empty keyword list if unmodified
	if (entry.data.at("keywords")==keywords_str) entry.data.at("keywords") = "";

	// set file path, add to entry, move given file into bib/...
	std::string ext = name.substr(name.find_last_of('.')+1, std::string::npos);
	std::string filepath = bib_files + entry.meta.at("id") + ".pdf";
	// TODO check if target already exist(s)?
	try {
		fs::rename({name}, get_home() + filepath);
		entry.data["file"] = bib_files + entry.meta.at("id") + ".pdf";
	} catch (fs::filesystem_error e) { std::cerr << e.what() << std::endl; }

	// save to bib
	bib_store(entry);

	// TODO sanity checks...file existence...check entry?
}



void bib_print_entries(const Bibliography& bib, const std::string& key, const std::string& format, const std::string& modifier)
{
	Bibliography entries = bib_match(bib, key);
	if (entries.empty()) return;

	for (auto& entry: entries) {
		entry.second = bib_modify_entry(entry.second, modifier);
		std::cout << bib_format_entry(entry.second, format) << std::endl;
	}
}



void keywords_search(const Bibliography& bib, const std::string& keyword)
{
	if (!keyword.empty()) {
		// keyword given: display all matches
		std::string key = keyword;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		for (auto& item: bib) {
			try {
				std::string keywords = item.second.data.at("keywords");
				if (keywords.find(key)!=std::string::npos)
					std::cout << item.first << ": " << item.second.data.at("title") << std::endl;
			} catch (std::out_of_range) {}
		}
	} else {
		// no keyword given: display list of existing keywords
		std::vector<std::pair<std::string, std::string>> keywords;
		std::vector<unsigned int> counts;
		size_t key_length = 0;
		size_t pos = 0;
		size_t padding = 0;

		// get list of keywords and descriptions
		BibEntry keyword_list = bib_import_entry(get_home() + bib_keywords);

		for (auto& k: keyword_list.data) {
			keywords.emplace_back(k.first, k.second.data());
			if (k.first.length() > key_length)
				key_length = k.first.length();
		}

		// collect #keyword-occurences
		for (auto& k: keywords) {
			unsigned int count = 0;
			for (auto& item: bib)
				try {
					if (item.second.data.at("keywords").find(k.first)!=std::string::npos)
						count++;
				} catch (std::out_of_range) {}
			counts.emplace_back(count);
			if (std::to_string(count).length()>padding)
				padding = std::to_string(count).length();
		}
		
		// print list
		std::cout << "available keywords (keyword [#] description):" << std::endl;
		for (auto& k: keywords) {
			std::cout << " " << std::setw(key_length) << k.first
				<< " [" << std::setw(padding) << counts.at(pos++) << std::setw(0) << "] "
				<< k.second << std::endl;
		}
	}
}



void keywords_symlink(const Bibliography& bib, const std::string& keyword)
{
	// clean up any leftovers
	system("rm -rf keywords");

	// collect required keywords
	std::vector<std::string> keyword_list;
	if (keyword.empty())
		for (auto& item: bib_import_entry(get_home() + bib_keywords).data)
			keyword_list.emplace_back(item.first);
	else
		if (bib_import_entry(get_home() + bib_keywords).data.count(keyword)>0)
			keyword_list.emplace_back(keyword);

	for (auto& k: keyword_list) {
		system(("mkdir -p keywords/" + k).c_str());
		for (auto& item: bib)
			try {
				if (item.second.data.at("keywords").find(k)!=std::string::npos) {
					std::string file = item.second.data.at("file");
					system(("ln " + get_home() + file.substr(file.find("bib"), std::string::npos)
							+ " keywords/" + k + file.substr(file.rfind("/"), std::string::npos)).c_str());
				}
			} catch (std::out_of_range) {}
	}
}



std::string get_home()
{
	return std::string{getpwuid(getuid())->pw_dir} + "/";
}

