// depak - a tool to decrypt and unpack .dat archives made by PakMaker for Blitz3D
// ermaccer

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <memory>


enum eModes {
	MODE_CLEAR = 1,
	MODE_ENCRYPTED,
	MODE_FAST_DEC
};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "DePak - An utility to extract archives created by PakMaker\nby ermaccer\n"
			<< "Usage: depak <params> <file>\n"
			<< "    -x             Allows to extract archives\n"
			<< "    -e             Allows to extract encrypted archives\n"
			<< "    -k <key>       Specifies archive key\n"
			<< "    -m <mask>      Specifies header mask\n"
			<< "    -o <folder>    Specifies a folder for extraction\n"
			<< "Specify keys in hex only, eg abc123.\n";
		return 1;
	}

	int key = 0, mask = 0;
	int mode = 0;
	std::string o_param;
	std::string l_param;

	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'x': mode = MODE_CLEAR;
			break;
		case 'e': mode = MODE_ENCRYPTED;
			break;
		case 'k':
			i++;
			sscanf(argv[i], "%x", &key);
			break;
		case 'm':
			i++;
			sscanf(argv[i], "%x", &mask);
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			return 1;
			break;
		}
	}

	if (mode == MODE_CLEAR)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{

			int offset = 0;
			pFile.read((char*)&offset, sizeof(int));
			pFile.seekg((offset - 1), pFile.beg);

			int files = 0;

			pFile.read((char*)&files, sizeof(int));

			// read file entries stored as strings (name, size, offset)

			char temp = 0;
			char tempString[255];
			int strlen = 0;

			std::vector<std::string> pakEntires;

			for (int i = 0; i < files * 3; i++)
			{
				while (temp != 0xD)
				{
					pFile.read((char*)&temp, sizeof(char));
					tempString[strlen] = temp;
					strlen++;
				}
				std::string entry(tempString, strlen);
				pakEntires.push_back(entry);
				strlen = 0;
				temp = 0;
				sprintf(tempString, "");
			}

			if (!o_param.empty())
			{
				if (!std::experimental::filesystem::exists(o_param))
					std::experimental::filesystem::create_directory(o_param);

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
			}

			// extract

			for (unsigned int i = 0; i < pakEntires.size(); i += 3)
			{
				int offset = 0, size = 0;

				sscanf(pakEntires[i + 1].c_str(), "%d", &offset);
				sscanf(pakEntires[i + 2].c_str(), "%d", &size);

				pFile.seekg(offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size);
				pFile.read(dataBuff.get(), size);


				std::string out(pakEntires[i].c_str(), pakEntires[i].length() - 1);
				std::cout << "Processing: " << out << std::endl;
				std::ofstream oFile(out, std::ofstream::binary);
				oFile.write(dataBuff.get(), size);

			}
			std::cout << "Finished." << std::endl;
		}
	}
	if (mode == MODE_ENCRYPTED)
	{
		if (key == 0 || mask == 0)
		{
			std::cout << "ERROR: Key(s) missing! Specify them using -m for header mask and -k for file key" << std::endl;
			return 1;
		}
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{
			int offset = 0;
			pFile.read((char*)&offset, sizeof(int));
			offset ^= mask;
			pFile.seekg((offset - 1), pFile.beg);

			int files = 0;

			pFile.read((char*)&files, sizeof(int));


			char temp = 0;
			char tempString[255];
			int strlen = 0;
			int fileSize = (int)std::experimental::filesystem::file_size(argv[argc - 1]);
			int headerSize = (int)std::experimental::filesystem::file_size(argv[argc - 1]) - (int)pFile.tellg() + 4;

			std::unique_ptr<char[]> headerBuff = std::make_unique<char[]>(headerSize);
			pFile.read(headerBuff.get(), headerSize);


			std::vector<std::string> pakEntires;
			for (int i = 0; i < headerSize; i += 4)
			{
				*(int*)(&headerBuff[i]) ^= (int)key;
			}

			std::ofstream hdr("tmp.bin", std::ofstream::binary);


			hdr.write(headerBuff.get(), headerSize);
			hdr.close();

			std::ifstream pTmp("tmp.bin", std::ifstream::binary);

			if (!pTmp)
			{
				std::cout << "ERROR: Failed to open tmp.bin!" << std::endl;
				return 1;
			}


			 
			for (int i = 0; i < files * 3; i++)
			{

				while (temp != 0xD)
				{
					pTmp.read((char*)&temp, sizeof(char));
					tempString[strlen] = temp;
					strlen++;
				}
				std::string entry(tempString, strlen - 1);
				pakEntires.push_back(entry);
				strlen = 0;
				temp = 0;
				sprintf(tempString, "");
			}

			pTmp.close();
			std::experimental::filesystem::remove("tmp.bin");


			if (!o_param.empty())
			{
				if (!std::experimental::filesystem::exists(o_param))
					std::experimental::filesystem::create_directory(o_param);

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
			}

			// extract


			pFile.clear();
			for (int i = 0; i < pakEntires.size(); i += 3)
			{
				int offset = 0, size = 0, buffer = 0;

				sscanf(pakEntires[i + 1].c_str(), "%d", &offset);
				sscanf(pakEntires[i + 2].c_str(), "%d", &size);

				pFile.seekg(offset,pFile.beg);

				std::cout << "Processing: " << pakEntires[i] << std::endl;

				std::ofstream oFile(pakEntires[i], std::ofstream::binary);


				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size);
				pFile.read(dataBuff.get(), size);

				for (int i = 0; i < size; i+= 4)
					*(int*)(&dataBuff[i]) = *(int*)(&dataBuff[i]) ^ key;

				oFile.write(dataBuff.get(), size);
				oFile.close();

			}

			std::cout << "Finished." << std::endl;
		}
	}
	if (mode == MODE_FAST_DEC)
	{
		// todo
	}
}
