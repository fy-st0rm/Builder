#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;

#include "json/json.h"

// g++ -c "-Ijson/include" builder.cpp "json/src/jsoncpp.cpp"
// g++ -o builder builder.o jsoncpp.o


class Builder
{
private:
	std::string _config_path;
	Json::Value config;

	// Tokens
	Json::Value include_dir, lib_dir, linkers, flags, src, exclude_src, type, build_name, run;

public:
	Builder(const std::string& config_path) : _config_path(config_path) {}
	~Builder() {}

public:
	void start()
	{
		read_config();

		compile_src();
		build_src();
		cleanup();
		run_exe();
	}

private:
	void read_config()
	{
		Json::Reader reader;

		// Loading the setting file
		std::ifstream file(_config_path);

		// Converting into string
		std::string line;
		std::string new_file;
		while(std::getline(file, line))
		{
			new_file += line;
		}

		file.close();

		// Parsing json file
		reader.parse(new_file, config, false);

		include_dir = config["include_dir"];
		lib_dir 	= config["lib_dir"];
		linkers		= config["linkers"];
		flags 		= config["flags"];
		src 		= config["src"];
		exclude_src = config["exclude_src"];
		type 		= config["type"];
		build_name  = config["build_name"];
		run 		= config["run"];
	}

	// Build functions
	void compile_src()
	{
		std::string compile_cmd;
		
		// Adding the required compiler type
		if (json_to_str(type) == "cpp")
			compile_cmd += "g++ ";
		else if (json_to_str(type) == "c")
			compile_cmd += "gcc ";

		// Adding required flags
		for (auto& i : flags)
		{
			compile_cmd += json_to_str(i);
			compile_cmd += " ";
		}
		compile_cmd += " ";

		// Adding the include directories
		for (auto& i : include_dir)
		{
			compile_cmd += "\"-I" + json_to_str(i) + "\" ";
		}
		compile_cmd += " ";

		// Adding src files
		compile_cmd += "-c ";

		for (auto& i : src)
		{
			std::vector<std::string> src_files = list_dir("." + json_to_str(type), json_to_str(i));
			for (auto& j : src_files)
			{
				compile_cmd += "\"" + j + "\" ";
			}			
		}

		// Compiling...
		std::cout << compile_cmd << std::endl;
		system(compile_cmd.c_str());
	}

	void build_src()
	{
		std::string build_cmd;

		if (json_to_str(type) == "cpp")
			build_cmd += "g++ ";
		else if (json_to_str(type) == "c")
			build_cmd += "gcc ";

		// Adding the executable file
		build_cmd += "-o " + json_to_str(build_name) + " ";

		// Collecting and adding .o files
		std::vector<std::string> obj_files = list_dir(".o", "./");
		for (auto& i : obj_files)
		{
			std::string file = split_string(i, '/').back();
			build_cmd += file;
			build_cmd += " ";
		}
		build_cmd += " ";

		// Adding lib directory
		for (auto& i : lib_dir)
		{
			build_cmd += "\"-L" + json_to_str(i) + "\" ";
		}
		build_cmd += " ";

		// Adding linkers
		for (auto& i : linkers)
		{
			build_cmd += json_to_str(i);
			build_cmd += " ";
		}

		std::cout << build_cmd << std::endl;
		system(build_cmd.c_str());
	}

	void cleanup()
	{
		system("rm *.o");
	}

	void run_exe()
	{	
		if (run)
			system(("./" + json_to_str(build_name)).c_str());
	}

private:
	// Utils things

	std::string replace_string(std::string str, char what, const std::string& with)
	{
		for (int i = 0; i < str.size(); i++) 
		{
			if (str[i] == what) 
			{
				str.replace(i, 1, with);		
			}
		}
		return str;
	}

	std::string json_to_str(const Json::Value& json_val)
	{
		Json::FastWriter writer;
		std::string val = writer.write(json_val);

		val = replace_string(val, '\"', "");
		val = replace_string(val, '\n', "");

		return val;
	}

	std::vector<std::string> list_dir(const std::string& file_type, const std::string& path)
	{
		std::vector<std::string> files;


		for (const auto & entry : fs::directory_iterator(path.c_str()))
	    {
	    	auto path = entry.path();

	    	std::vector<std::string> splitted_string = split_string(path.string(), '/');
	   		std::string req_file = splitted_string[splitted_string.size() - 1];
 
	   		if (req_file.find(file_type) != std::string::npos)
	   		{
	   			files.push_back(path.string());
	   		}
	    }
	    return files;
	}

	std::vector<std::string> split_string(const std::string& string, const char splitting_chr)
	{
		std::vector<std::string> splitted_string;
		
		int p = 0;	// Pos index of the char
		int i = 0;	// index to find the splitting char

		// Splitting
		for (i = 0; i < string.size(); i++) {
			if (string[i] == splitting_chr) {
				
				// Creating a word
				std::string word;
				while (p < (int)i) {
					word += string[p];
					p++;
				}
				splitted_string.push_back(word);
				p++;
			}
		}
		
		// Getting last word
		std::string word;
		while (p < i) {
			word += string[p];
			p++;
		}
		splitted_string.push_back(word);

		return splitted_string;
	}
};

int main()
{
	Builder builder("Config.json");
	builder.start();
	return 0;
}
