/**
 * @file YRMapsUpdater.cpp
 * @brief Application to build MPMaps.ini for CnCNet map updates.
 * @author Chrono Vortex#9916@Discord
 */

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <fstream>
#include <filesystem>
#include <winsock.h>
#include <windows.h>
#include <regex>
#include <vector>
#include <map>
#include "eqor.h"
#include "initfuncwrap.h"

#define NULLSTR ""
#define BUFFSIZE 1024

namespace fs = std::filesystem;

/**
 * Wrapper for the windows function to get location of this program.
 *
 * @return path to the directory containing this program.
 */
fs::path program_path() {
	wchar_t result[MAX_PATH];
	return fs::path(std::wstring(result, GetModuleFileNameW(NULL, result, MAX_PATH))).remove_filename();
}

/**
 * Get yes/no input from the user.
 *
 * @return true is user entered yes, false if not.
 */
bool get_yes_no() {
	std::string s; // using a char can leave junk in the input buffer
	std::getline(std::cin >> std::ws, s);
	return std::tolower(s[0]) == 'y';
}

/**
 * Check if a string begins with a substring.
 *
 * @param s string to check.
 * @param start substring to check beginning for.
 * @return true if 's' begins with 'start', false if not.
 */
bool str_startswith(const std::string& s, const std::string& start) {
	return (s.length() >= start.length()) ? (s.compare(0, start.length(), start) == 0) : false;
}

/**
 * Check if a string ends with a substring.
 *
 * @param s string to check.
 * @param end substring to check ending for.
 * @return true if 's' ends with 'end', false if not.
 */
bool str_endswith(const std::string& s, const std::string& end) {
	return (s.length() >= end.length()) ?
		(s.compare(s.length() - end.length(), end.length(), end) == 0) : false;
}

/**
 * Remove specified number of characters from
 * the beginning and end of a string.
 *
 * @param s string to modify.
 * @param left number of characters to remove from the beginning of 's'.
 * @param right number of characters to remove from the end of 's'.
 * @return modified copy of 's'.
 */
std::string str_cutends(const std::string& s, const int& left, const int& right) {
	int len = s.size() - left - right;
	if (len <= 0)
		return NULLSTR;
	return s.substr(left, len);
}

/**
 * Capitalizes the first letter of each word in a string,
 * e.g. "hello, world!" becomes "Hello, World!".
 *
 * @param s string to capitalize.
 * @return capitalized copy of 's'.
 */
std::string str_titlecase(std::string s) {
	bool capNextLetter = true;
	for (std::string::size_type i = 0; i < s.size(); ++i) {
		if (capNextLetter) {
			if (std::isalpha(s[i])) {
				s[i] = std::toupper(s[i]);
				capNextLetter = false;
			}
		}
		else {
			if (std::isalpha(s[i])) {
				s[i] = std::tolower(s[i]);
			}
			else {
				capNextLetter = true;
			}
		}
	}
	return s;
}

/**
 * https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf#answer-14539953
 * Creates a progress bar string.
 *
 * @param completed how much progress has been made.
 * @param toComplete how much progress must be made for completion.
 * @param barWidth number of characters to represent the progress bar with.
 * @return string representing progress made.
 */
std::string progress_to_string(const float& completed, const float& toComplete, const int& barWidth) {
	std::string progressStr = "[";
	float progress = completed / toComplete;
	int pos = barWidth * progress;
	for (size_t i = 0; i < barWidth; ++i) {
		if (i < pos) progressStr += '=';
		else if (i == pos) progressStr += '>';
		else progressStr += ' ';
	}
	return progressStr + "] " + std::to_string(int(progress * 100.0)) + '%';
}

/**
 * https://stackoverflow.com/questions/5354459/c-how-to-get-the-image-size-of-a-png-file-in-directory#answer-5354657
 * Gets the dimensions of a PNG image
 * in a path as a pair of integers.
 *
 * @param pngPath path to the PNG image to get the dimensions of.
 * @return pair of integers containing the dimensions.
 */
std::pair<int, int> png_getsize(const fs::path& pngPath) {
	if (!fs::exists(pngPath))
		throw std::invalid_argument("no file at specified path");

	std::ifstream in(pngPath);
	int magic;
	in.read((char*)&magic, 4);
	if (ntohl(magic) != 0x89504E47)
		throw std::invalid_argument("specified file is not a png");

	int width, height;
	in.seekg(16);
	in.read((char*)&width, 4);
	in.read((char*)&height, 4);
	return std::pair<int, int>(ntohl(width), ntohl(height));
}

/**
 * Check if the file exists before settiling on a path,
 * give user the option to rename if it does.
 *
 * @param dir directory for the file.
 * @param fname name of the file.
 * @return path to the file.
 */
fs::path path_check_exists(const fs::path& dir, const std::string& fname) {
	auto path = dir / fname;
	if (fs::exists(path)) {
		std::cout << path.string() << " already exists, would you like to replace it? [y/N] ";
		if (!get_yes_no()) {
			std::cout << "Please enter an alternate filename: " << std::endl;
			std::string newFname;
			std::getline(std::cin >> std::ws, newFname);
			return path_check_exists(dir, newFname);
		}
	}
	return path;
}

const fs::path pathsIniPath = program_path() / "PathsYRMU.ini";
const fs::path mapsPathRelative("Maps\\Yuri's Revenge");

int main(int argc, const char** argv) {
	// we'll use this buffer to get the string from GetPrivateProfileString every time we use it
	char buffer[BUFFSIZE];

	// create PathsYRMU.ini to save required paths if it doesn't already exists
	if (!fs::exists(pathsIniPath))
		std::ofstream(pathsIniPath).close();
	// prevent it from being moved while the program is running
	std::ifstream pathsIniPathOpen(pathsIniPath);

	// get cncnet path from PathsYRMU.ini
	fs::path ptmp1(std::string(buffer, GetPrivateProfileString(
		"PATHS", "CNCNET", NULLSTR, buffer, BUFFSIZE, pathsIniPath)));
	auto ptmp2 = ptmp1 / mapsPathRelative;
	auto ptmp3 = ptmp1 / "INI\\MPMaps.ini";
	if (!(fs::exists(ptmp1) && fs::exists(ptmp2) && fs::exists(ptmp3))) {
		std::cout << "Enter full path to CnCNet: " << std::endl;
		std::string newPath;
		std::getline(std::cin >> std::ws, newPath);
		ptmp1 = fs::path(newPath);
		ptmp2 = ptmp1 / mapsPathRelative;
		ptmp3 = ptmp1 / "INI\\MPMaps.ini";
		while (!(fs::exists(ptmp1) && fs::exists(ptmp2) && fs::exists(ptmp3))) {
			std::cout << "CnCNet directories not found, enter full path to CnCNet: " << std::endl;
			std::getline(std::cin >> std::ws, newPath);
			ptmp1 = fs::path(newPath);
			ptmp2 = ptmp1 / mapsPathRelative;
			ptmp3 = ptmp1 / "INI\\MPMaps.ini";
		}
		WritePrivateProfileString("PATHS", "CNCNET", ptmp1.string(), pathsIniPath);
	}
	const fs::path cncnetPath = ptmp1;
	const fs::path mapsPathFull = ptmp2;
	const fs::path mpmapsOldPath = ptmp3;

	// list new maps for versionconfig.ini
	std::cout << "Would like to create a list of new maps and previews? [y/N] ";
	if (get_yes_no()) {
		// get versionconfig.ini path from PathsYRMU.ini
		fs::path configPath(std::string(buffer, GetPrivateProfileString(
			"PATHS", "VCONFIG", NULLSTR, buffer, BUFFSIZE, pathsIniPath)));
		if (!(fs::exists(configPath) && configPath.filename() == "versionconfig.ini")) {
			std::cout << "Enter full path to versionconfig.ini:" << std::endl;
			std::string newPath;
			std::getline(std::cin >> std::ws, newPath);
			configPath = fs::path(newPath);
			while (!(fs::exists(configPath) && configPath.filename() == "versionconfig.ini")) {
				std::cout << "Invalid path, please try again:" << std::endl;
				std::getline(std::cin >> std::ws, newPath);
				configPath = fs::path(newPath);
			}
			WritePrivateProfileString("PATHS", "VCONFIG", configPath.string(), pathsIniPath);
		}
		std::ifstream config(configPath);

		// read map and preview entries from config into a vector
		std::vector<std::string> configEntries;
		std::string line;
		while (std::getline(config, line))
			if (str_startswith(line, mapsPathRelative.string()))
				configEntries.push_back(line);
		config.close();

		// read all maps, output to file if not in config entries
		fs::path outPath = path_check_exists(program_path(), "versionconfig_missing.txt");
		std::ofstream newMaps(outPath);
		for (const auto& e : fs::recursive_directory_iterator(mapsPathFull)) {
			fs::path dirEntry = e.path();
			if (dirEntry.extension() == ".map" or dirEntry.extension() == ".png") {
				std::string newEntryCandidate = str_cutends(
					dirEntry.string(), cncnetPath.string().length() + 1, 0); // remove cncnetPath from this path
				if (std::find(configEntries.begin(), configEntries.end(), newEntryCandidate) == configEntries.end())
					newMaps << newEntryCandidate << std::endl; // candidate is not in config entries, write it
			}
		}
		newMaps.close();
		std::cout << "Created list of new maps and previews in " << outPath.string() << std::endl;
	}

	// getting everything we need for MPMaps
	const fs::path mpmapsPath = program_path() / "MPMaps.ini";

	// read map names and paths into an std::map to sort by player number, followed by map title
	// if we can't find the name for any map, write all maps with missing names to a file
	std::map<std::string, fs::path> mapPathsOrdered;
	std::vector<std::string> missing;
	const std::basic_regex titlePattern("^\\[\\d\\] \\S.+$"); // regex for map names
	for (const auto& e : fs::recursive_directory_iterator(mapsPathFull)) {
		fs::path dirEntry = e.path();
		if (dirEntry.extension() == ".map") {
			// start looking for the name in the map itself
			std::string mapTitle(buffer, GetPrivateProfileString(
				"Basic", "Name", NULLSTR, buffer, BUFFSIZE, dirEntry));
			if (std::regex_match(mapTitle, titlePattern)) {
				mapPathsOrdered[mapTitle + dirEntry.string()] = dirEntry; // add directory to key in case maps have the same name
				continue;
			}

			// valid name not found in map, fall back on old MPMaps.ini
			// remove parts of path not included in MPMaps section name
			std::string mapSection = str_cutends(
				dirEntry.string(), cncnetPath.string().length() + 1, 4);
			// get new map name and validate
			std::string mapTitleMP(buffer, GetPrivateProfileString(
				mapSection, "Description", NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
			if (std::regex_match(mapTitleMP, titlePattern)) {
				mapPathsOrdered[mapTitleMP + dirEntry.string()] = dirEntry;
				continue;
			}

			// valid name not found in MPMaps.ini, push to missing name vector
			missing.push_back(
				mapSection + "\nname in map was " + ((mapTitle == NULLSTR) ? "not found" : mapTitle) +
				", name in MPMaps.ini was " + ((mapTitleMP == NULLSTR) ? "not found" : mapTitleMP) + '\n');
		}
	}
	if (!missing.empty()) { // if any maps were missing names, write them all to a file
		std::cout << "Unable to find valid names for " << missing.size() << " maps" << std::endl;
		fs::path outPath = path_check_exists(program_path(), "map_names_missing.txt");
		std::ofstream missingMaps(outPath);
		for (const std::string& s : missing)
			missingMaps << s << std::endl;
		std::cout << "Wrote list of missing maps to " << outPath.string() << std::endl;
		missingMaps.close();

		// give user the option to abort
		std::cout << "Would you like to continue? Maps with missing names will not be processed [y/N] ";
		if (!get_yes_no())
			return 0;
	}

	// copy MPMapsBase.ini, this copy is where we'll write all the info we get
	fs::path mpmapsBasePath = program_path() / "MPMapsBase.ini";
	while (!fs::exists(mpmapsBasePath) || mpmapsBasePath.filename() != "MPMapsBase.ini") {
		std::cout << "Unable to find MPMapsBase.ini, please enter full path:" << std::endl;
		std::string inputTemp;
		std::getline(std::cin >> std::ws, inputTemp);
		mpmapsBasePath = fs::path(inputTemp);
	}
	if (fs::exists(mpmapsPath)) {
		std::cout << mpmapsPath.string() << " already exists, would you like to delete it? [y/N] ";
		if (get_yes_no())
			fs::remove(mpmapsPath);
		else
			return 0;
	}
	fs::copy(mpmapsBasePath, mpmapsPath);

	// HERE WE GO, BITCHES!!!!!!
	std::cout << "Building MPMaps.ini..." << std::endl;
	std::vector<std::string> notes; // save notes on whatever was found missing to this, append them all to the end of MPMaps as comments when it's done building

	// go through each map, add to [MultiMaps], and collect info for their individual sections
	int multiMapsIndex = 0;
	const std::basic_regex badBriefPattern("^Brief:(ALL|TRN)\\d{2}(md)?$"); // regex for bad briefings
	time_t lastPrintTime = time(0); // timer for printing progress bar
	for (auto const& [key, mapPath] : mapPathsOrdered) {
		// print progress bar every few seconds
		if (difftime(time(0), lastPrintTime) >= 3) {
			std::cout << progress_to_string(multiMapsIndex, mapPathsOrdered.size(), 70) << std::endl;
			lastPrintTime = time(0);
		}

		// write MultiMaps entry
		std::string mapSection = str_cutends(mapPath.string(), cncnetPath.string().length() + 1, 4);
		WritePrivateProfileString("MultiMaps", std::to_string(multiMapsIndex++), mapSection, mpmapsPath);

		// write map name (can be taken from key by removing directory)
		std::string mapTitle = str_cutends(key, 0, mapPath.string().length());
		WritePrivateProfileString(mapSection, "Description", mapTitle, mpmapsPath);

		// write author, prioritize old MPMaps for this one so maps don't need authors updated individually
		std::string mapAuthor(buffer, GetPrivateProfileString(mapSection, "Author", NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
		if (mapAuthor == NULLSTR) {
			// author not in old MPMaps, check map
			mapAuthor = std::string(buffer, GetPrivateProfileString("Basic", "Author", NULLSTR, buffer, BUFFSIZE, mapPath));
			if (mapAuthor == NULLSTR) {
				// author not in old MPMaps, set defaut and make note
				notes.push_back("; " + mapSection + " missing Author, set to \"Unknown Author\"");
				mapAuthor = "Unknown Author";
			}
		}
		WritePrivateProfileString(mapSection, "Author", mapAuthor, mpmapsPath);

		// write briefing if we can find it
		std::string mapBrief(buffer, GetPrivateProfileString("Basic", "Briefing", NULLSTR, buffer, BUFFSIZE, mapPath));
		if (mapBrief == NULLSTR || std::regex_match(mapBrief, badBriefPattern)) // valid briefing not in map, check old MPMaps
			mapBrief = std::string(buffer, GetPrivateProfileString(mapSection, "Briefing", NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
		if (mapBrief != NULLSTR)
			WritePrivateProfileString(mapSection, "Briefing", mapBrief, mpmapsPath);

		// write gamemodes, prioritize old MPMaps for this one 'cause lots of maps don't have the correct gamemodes set
		std::string mapModes(buffer, GetPrivateProfileString(
			mapSection, "GameModes", NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
		if (mapModes == NULLSTR) // gamemodes not found in old MPMapsn check map
			mapModes = std::string(buffer, GetPrivateProfileString(
				"Basic", "GameMode", NULLSTR, buffer, BUFFSIZE, mapPath));
		if (mapModes == NULLSTR) { // gamemodes not found in map, set default and make note
			notes.push_back("; " + mapSection + " missing GameModes, set to \"Battle\"");
			mapModes = "Battle";
		}
		// when writing, replace "standard" with "battle", then capitalize each word
		size_t pos = mapModes.find("standard");
		if (pos != std::string::npos)
			mapModes.replace(pos, 8, "battle");
		mapModes = str_titlecase(mapModes);
		WritePrivateProfileString(mapSection, "GameModes", mapModes, mpmapsPath);

		// write coop info if map is coop, check map and MPMaps for IsCoopMission
		std::string mapCoopVal(buffer, GetPrivateProfileString(
			"Basic", "IsCoopMission", NULLSTR, buffer, BUFFSIZE, mapPath));
		std::transform(mapCoopVal.begin(), mapCoopVal.end(), mapCoopVal.begin(), std::tolower);
		std::string iniCoopVal(buffer, GetPrivateProfileString(
			mapSection, "IsCoopMission", NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
		std::transform(iniCoopVal.begin(), iniCoopVal.end(), iniCoopVal.begin(), std::tolower);
		std::vector<int> coopEnemyWaypnts; // we need a list of waypoints the player can't choose when we write starting waypoints
		if (eqor(mapCoopVal, "yes", "true") || eqor(iniCoopVal, "yes", "true")) {
			// regex for enemy house entry values
			const std::basic_regex enemyHousePattern("^(\\d+,\\d+,\\d+)\\s*;?.*$");
			
			// duh
			WritePrivateProfileString(mapSection, "IsCoopMission", "yes", mpmapsPath);

			// write sides and colors player is now allowed to choose
			for (std::string bannedKey : {"DisallowedPlayerSides", "DisallowedPlayerColors"}) {
				std::string mapBannedItems(buffer, GetPrivateProfileString(
					"Basic", bannedKey, NULLSTR, buffer, BUFFSIZE, mapPath));
				if (mapBannedItems == NULLSTR)
					mapBannedItems = std::string(buffer, GetPrivateProfileString(
						mapSection, bannedKey, NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
				if (mapBannedItems == NULLSTR) {
					notes.push_back("; " + mapSection + " missing " + bannedKey);
				}
				else {
					WritePrivateProfileString(mapSection, bannedKey, mapBannedItems, mpmapsPath);
				}
			}

			// write enemy house info
			size_t enemyHouseNum = 0;
			bool useMP = false;
			std::string mapEnemyHouse(buffer, GetPrivateProfileString(
				"Basic", "EnemyHouse" + std::to_string(enemyHouseNum), NULLSTR, buffer, BUFFSIZE, mapPath));
			if (!std::regex_match(mapEnemyHouse, enemyHousePattern)) {
				useMP = true;
				mapEnemyHouse = std::string(buffer, GetPrivateProfileString(
					mapSection, "EnemyHouse" + std::to_string(enemyHouseNum), NULLSTR, buffer, BUFFSIZE, mpmapsOldPath));
			}
			if (!std::regex_match(mapEnemyHouse, enemyHousePattern)) {
				notes.push_back("; " + mapSection + " missing EnemyHouse entries (this has affected Waypoint entires as well)");
			}
			else {
				while (enemyHouseNum <= 8 && mapEnemyHouse != NULLSTR) {
					// strip comment from mapEnemyHouse if there is one,
					// we need the last character to be the waypoint for the enemy house
					auto mapEnemyHouseStripped = std::regex_replace(mapEnemyHouse, enemyHousePattern, "$1");
					// last character of mapEnemyHouseStripped is the waypoint for the enemy house
					coopEnemyWaypnts.push_back(mapEnemyHouseStripped[mapEnemyHouseStripped.size() - 1] - '0');
					WritePrivateProfileString(
						mapSection, "EnemyHouse" + std::to_string(enemyHouseNum++), mapEnemyHouse, mpmapsPath);
					mapEnemyHouse = (useMP) ?
						std::string(buffer, GetPrivateProfileString(
							mapSection, "EnemyHouse" + std::to_string(enemyHouseNum), NULLSTR, buffer, BUFFSIZE, mpmapsOldPath)) :
						std::string(buffer, GetPrivateProfileString(
							"Basic", "EnemyHouse" + std::to_string(enemyHouseNum), NULLSTR, buffer, BUFFSIZE, mapPath));
				}
			}
		}

		// write min/max players, EnforceMaxPlayers and starting waypoints, base on coop info if map is coop
		size_t itterWaypnt = 0;
		std::string mapWaypnt(buffer, GetPrivateProfileString(
			"Waypoints", std::to_string(itterWaypnt), NULLSTR, buffer, BUFFSIZE, mapPath));
		while (itterWaypnt <= 8 && mapWaypnt != NULLSTR) {
			// only write if this waypoint doesn't belong to an enemy in coop
			if (std::find(coopEnemyWaypnts.begin(), coopEnemyWaypnts.end(), itterWaypnt) == coopEnemyWaypnts.end())
				WritePrivateProfileString(
					mapSection, "Waypoint" + std::to_string(itterWaypnt), mapWaypnt, mpmapsPath);
			++itterWaypnt;
			mapWaypnt = std::string(buffer, GetPrivateProfileString(
				"Waypoints", std::to_string(itterWaypnt), NULLSTR, buffer, BUFFSIZE, mapPath));
		}
		WritePrivateProfileString(mapSection, "MinPlayers", "2", mpmapsPath);
		WritePrivateProfileString(
			mapSection, "MaxPlayers", std::to_string(itterWaypnt - coopEnemyWaypnts.size()), mpmapsPath);
		WritePrivateProfileString(mapSection, "EnforceMaxPlayers", "True", mpmapsPath);

		// get ForcedOptions and ForcedSpawnIniOptions from map,
		// write it as ForcedOptions-mapname or ForcedSpawnIniOptions-mapname in MPMaps
		for (std::string forcedKey : {"ForcedOptions", "ForcedSpawnIniOptions"}) {
			if (GetPrivateProfileSection(forcedKey, buffer, BUFFSIZE, mapPath)) {
				std::string forcedOptionsName = forcedKey + '-' + mapSection;
				WritePrivateProfileString(mapSection, forcedKey, forcedOptionsName, mpmapsPath);
				WritePrivateProfileSection(forcedOptionsName, buffer, mpmapsPath);
			}
		}

		// write map sizes and preview size
		WritePrivateProfileString(mapSection, "Size", std::string(buffer, GetPrivateProfileString(
			"Map", "Size", NULLSTR, buffer, BUFFSIZE, mapPath)), mpmapsPath);
		WritePrivateProfileString(mapSection, "LocalSize", std::string(buffer, GetPrivateProfileString(
			"Map", "LocalSize", NULLSTR, buffer, BUFFSIZE, mapPath)), mpmapsPath);
		std::pair<int, int> mapPreviewSize;
		try {
			auto mapPreviewSize = png_getsize(fs::path(mapPath).replace_extension(".png"));
			WritePrivateProfileString(mapSection, "PreviewSize",
				std::to_string(mapPreviewSize.first) + ',' + std::to_string(mapPreviewSize.second), mpmapsPath);
		}
		catch (std::invalid_argument) { // couldn't find png preview, make note
			notes.push_back("; " + mapSection + " missing PreviewSize");
		}
	}

	// add comments to the end of the new MPMaps listing all entries which are missing
	std::ofstream out(mpmapsPath, std::ofstream::app);
	for (const std::string& s : notes)
		out << s << std::endl;
	out.close();

	// tell the user we're done
	std::cout << "MPMaps.ini has been built";
	if (!notes.empty())
		std::cout << ", notes on missing data were written to the end of the file";
	std::cout << std::endl;

	// wait for input to return success
	std::cout << "Press [Enter] to exit" << std::endl;
	std::cin.get();
	pathsIniPathOpen.close();
	return 0;
}
