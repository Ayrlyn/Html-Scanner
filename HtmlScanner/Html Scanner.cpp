// This program scans a directory recursively for HTML files.
// It checks if these files contain any of a predefined list of keywords.
// The output is a text file that lists the matching filenames, grouped by the keyword they contained.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

int main(int argCount, char* argValues[])
{
	if (argCount < 3)
	{
		PrintUsage(argValues[0]);
		return 1;
	}

	std::filesystem::path scanDirectory;
	std::string outputFileName = "output.txt";
	std::vector<std::string> keywords;

    // --- Argument Parsing Logic ---
    bool outputFlagFound = false;
    for (int i = 1; i < argCount; ++i) {
        std::string arg = argValues[i];

        if (i == 1) {
            scanDirectory = arg;
            continue;
        }

        if (outputFlagFound) {
            // The argument directly after /o is the output filename.
            outputFileName = arg;
            outputFlagFound = false; // Reset the flag
            continue;
        }

        if (arg == "/o" || arg == "/O") {
            outputFlagFound = true;
            // The next argument will be the filename, handled in the next loop iteration.
            continue;
        }

        // If it's not the directory or an output flag/file, it's a keyword.
        keywords.push_back(arg);
    }

    if (scanDirectory.empty() || !std::filesystem::is_directory(scanDirectory)) {
        std::cerr << "Error: The first argument must be a valid directory." << std::endl;
        PrintUsage(argValues[0]);
        return 1;
    }

    if (keywords.empty()) {
        std::cerr << "Error: No keywords were provided." << std::endl;
        PrintUsage(argValues[0]);
        return 1;
    }

    if (outputFlagFound) {
        std::cerr << "Error: /o flag specified without a filename." << std::endl;
        PrintUsage(argValues[0]);
        return 1;
    }


    // Use a map to store a list of filenames for each keyword.
    std::map<std::string, std::vector<std::string>> foundFilesByKeyword;

    std::cout << "Scanning directory: " << std::filesystem::absolute(scanDirectory).string() << std::endl;
    std::cout << "Output file: " << outputFileName << std::endl;
    std::cout << "Looking for keywords: ";
    for (size_t i = 0; i < keywords.size(); ++i) {
        std::cout << "\"" << keywords[i] << "\"" << (i < keywords.size() - 1 ? ", " : "");
    }
    std::cout << std::endl << std::endl;

    try {
        // Recursively iterate through all files in the directory.
        for (const auto& entry : std::filesystem::recursive_directory_iterator(scanDirectory)) {
            // Check if the entry is a regular file with a .html or .htm extension.
            if (entry.is_regular_file() && (entry.path().extension() == ".html" || entry.path().extension() == ".htm")) {

                std::ifstream fileStream(entry.path());
                if (!fileStream.is_open()) {
                    std::cerr << "Warning: Could not open file: " << entry.path().string() << std::endl;
                    continue; // Skip to the next file
                }

                // Check the file for each keyword.
                std::string line;
                std::set<std::string> keywordsFoundInFile;
                while (std::getline(fileStream, line)) {
                    for (const auto& keyword : keywords) {
                        if (line.find(keyword) != std::string::npos) {
                            keywordsFoundInFile.insert(keyword);
                        }
                    }
                }

                // If any keywords were found, add the file path to the map for each keyword.
                for (const auto& foundKeyword : keywordsFoundInFile) {
                    foundFilesByKeyword[foundKeyword].push_back(entry.path().string());
                    std::cout << "Found \"" << foundKeyword << "\" in: " << entry.path().string() << std::endl;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    // Write the grouped results to the output file.
    std::ofstream outputFile(outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file for writing: " << outputFileName << std::endl;
        return 1;
    }

    outputFile << "Scan results for directory: " << std::filesystem::absolute(scanDirectory).string() << std::endl;

    if (foundFilesByKeyword.empty()) {
        outputFile << "\nNo files were found containing the specified keywords." << std::endl;
    }
    else {
        for (const auto& pair : foundFilesByKeyword) {
            const std::string& keyword = pair.first;
            const std::vector<std::string>& files = pair.second;

            outputFile << "\n==================================================" << std::endl;
            outputFile << "Files containing keyword: \"" << keyword << "\"" << std::endl;
            outputFile << "==================================================" << std::endl;

            for (const auto& filePath : files) {
                outputFile << filePath << std::endl;
            }
        }
    }

    outputFile.close();
    std::cout << "\nScan complete. Results saved to " << outputFileName << std::endl;

    return 0;
}


void PrintUsage(const char* programName) {
	std::cerr << "Usage: " << programName << " <directory_to_scan> <keyword1> [keyword2] [keyword3] ..." << std::endl;
	std::cerr << "Example: " << programName << " \"C:\\MyWebsite\" form gallery table" << std::endl;
	std::cerr << "An optional output file can be specified with the /o flag." << std::endl;
	std::cerr << "Example with output file: " << programName << " \"C:\\MyWebsite\" /o \"results.txt\" form gallery" << std::endl;
}
