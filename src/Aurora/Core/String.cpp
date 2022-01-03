#include "String.hpp"

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> list;
	std::stringstream buffer;

	for (char c : str) {
		if(c == delimiter) {
			if(buffer.tellp() != 0) {
				list.push_back(buffer.str());
				buffer.str("");
			}
		} else {
			buffer << c;
		}
	}

	if(buffer.tellp() != 0) {
		list.push_back(buffer.str());
	}

	return list;
}

std::string FormatBytes(uint64_t bytes)
{
	int marker = 1024; // Change to 1000 if required
	int decimal = 3; // Change as required
	int kiloBytes = marker; // One Kilobyte is 1024 bytes
	int megaBytes = marker * marker; // One MB is 1024 KB
	int gigaBytes = marker * marker * marker; // One GB is 1024 MB
	//uint64_t teraBytes = marker * marker * marker * marker; // One TB is 1024 GB

	std::stringstream ss;

	// return bytes if less than a KB
	if(bytes < kiloBytes)
	{
		ss << bytes << " Bytes";
	} // return KB if less than a MB
	else if(bytes < megaBytes)
	{
		ss << std::setprecision(decimal) << ((float)bytes / (float)kiloBytes) << " KB";
	}// return MB if less than a GB
	else if(bytes < gigaBytes)
	{
		ss << std::setprecision(decimal) << ((float)bytes / (float)megaBytes) << " MB";
	} // return GB if less than a TB
	else
	{
		ss << std::setprecision(decimal) << ((float)bytes / (float)gigaBytes) << " GB";
	}

	return ss.str();
}