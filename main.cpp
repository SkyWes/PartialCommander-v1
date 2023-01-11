#include <iostream>
#include <string>
#include <filesystem>
#include <regex>

void FolderPathInput(std::string& folder_path);

void MainMenu(int& t, std::string_view folder_path);
void SubMenu(int& t, int& sub1, int& sub2);
void GetInput(int& input, int range);

void ExecuteTask(std::filesystem::path p, int& task, int& sub1, int& sub2);
void Clean(std::string& filename, std::string& new_filename, std::string delim = "_", const int& date_reformat = 1);
void CustomClean(std::string& filename, std::string& new_filename, int& sub1, int& sub2);
void RemoveNums(std::string& filename, std::string& new_filename);
void DateReformat(std::string& date, const int& reformat, const int& format);

bool DateMatch(std::string& filename, std::string& buffer, std::string& datex, int& format);
bool DateRegexSearch(std::string& filename, std::string& buffer, std::string& datex, std::regex& reg);

std::string FillerWords(std::string_view s);

int main(){
    char con{'Y'};
    std::cout << "Welcome to Partial Commander: File Name Cleaner v1 by sky_pal \n"  << std::endl;

    std::string folder_path{""};
    FolderPathInput(folder_path);

    while(con == 'Y' || con == 'y'){
        
        std::filesystem::path path(folder_path);
        if(!std::filesystem::exists(path)){
            std::cout << "The specified path does not exist!" << std::endl;
            FolderPathInput(folder_path);
            continue;
        }
        
        int task{};
        int sub1{};
        int sub2{};

        MainMenu(task, folder_path);
        
        if(task == 4) {
            FolderPathInput(folder_path);
            continue;
        }

        if(task == 1 || task == 2){    // Version 2 will have a better menu system
            SubMenu(task, sub1, sub2);
        }

        ExecuteTask(path, task, sub1, sub2);

        std::cout << "Done!" << std::endl;
        std::cout << "Perform another action? (Y|N) : " << std::endl;
        std::cin >> con;
    }
    std::cout << "Goodbye!" << std::endl;
    return 0;
}

void FolderPathInput(std::string& folder_path){
    std::cout << "Please enter the path of the folder: " << std::endl;
    std::cin >> folder_path;
}

void MainMenu(int& t, std::string_view folder_path) {
    std::cout << "Enter number of the task you wish to perform (" << folder_path << ") : " << std::endl;
    std::cout << "  (1) clean file names \n"
              << "  (2) clean dates only \n"
              << "  (3) Remove random numbers from beginning of file name \n"
              << "  (4) Change folder path" << std::endl; 
                 // (5) search and replace coming for version 2
    GetInput(t, 4);
}

void SubMenu(int& t, int& sub1, int& sub2) {
    switch (t)
    {
    case 1:
        std::cout << "Naming convention: \n" 
                  << "  (1) Default: yyyy-mm-dd_File_Name_v1.ext \n"
                  << "  (2) Custom" << std::endl;
        
        GetInput(t, 2);
        
        if(t == 1) {break;}

        std::cout << "Choose case format: \n"
                  << "  (1) Underscore_Delimeter.ext \n"
                  << "  (2) Hyphen-Delimeter.ext \n"
                  << "  (3) CamelCase.ext" << std::endl;
        
        GetInput(sub1,3);
        // Intentionally leaving no break here
    case 2:
        std::cout << "Choose date format: \n"
                  << "  (1) yyyy-mm-dd \n"
                  << "  (2) mm-dd-yyyy \n"
                  << "  (3) dd-mm-yyyy" << std::endl;
        
        GetInput(sub2,3);
        break;
    }
}

void GetInput(int& input, int range) {
    // Checking the validity of the input
    while(true) {
        std::cin >> input;
        if (std::cin.fail() || input < std::numeric_limits<int>::min() || input > std::numeric_limits<int>::max() || input < 1 || input > range) {
            std::cout << "Invalid input. Please enter the number of the task you wish to perform:";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            break;
        }
    }
     
}

void ExecuteTask(std::filesystem::path p, int& task, int& sub1, int& sub2) {
    
   switch (task)
    {
    case 1: // Loop for default file cleaning process 
        for(const auto& entry : std::filesystem::directory_iterator(p)){
            std::string filename = entry.path().filename().string();
            std::string new_filename{""};
            Clean(filename, new_filename);
            std::filesystem::rename(entry.path(), p.string() + "/" + new_filename);
        }
        break;
        
    case 2: //Loop for custom cleaning process
        for(const auto& entry : std::filesystem::directory_iterator(p)){
            std::string filename = entry.path().filename().string();
            std::string new_filename = "";
            CustomClean(filename, new_filename, sub1, sub2);
            std::filesystem::rename(entry.path(), p.string() + "/" + new_filename);
        }
        
        break;
    case 3: // Loop for removing random numbers from beginning of file
        for(const auto& entry : std::filesystem::directory_iterator(p)){
            std::string filename = entry.path().filename().string();
            std::string new_filename = "";
            RemoveNums(filename, new_filename);
            if(!new_filename.empty()){std::filesystem::rename(entry.path(), p.string() + "/" + new_filename);}
        } 
        break;
    }
}

void Clean(std::string& filename, std::string& new_filename, std::string delim, const int& date_reformat) {
    
    std::string buffer{""};
    std::string datex{""};
    int d_format{};

    //Extract and format date
    if(!DateMatch(filename, buffer, datex, d_format)){
        buffer = filename;
    }else if(d_format != date_reformat) {
        DateReformat(datex, date_reformat, d_format);
    }

    //Only clean the date, keep the rest of the file formatting
    if(delim == "CUSTOM") {
        new_filename = datex + buffer;
        return;
    }

    std::string ext{""};
    std::regex file_ext("\\.\\w+$");
    std::smatch ext_smatch;

    //Extract file extension
    if(std::regex_search(buffer, ext_smatch, file_ext)) {
        ext = ext_smatch.str(); 
        buffer = ext_smatch.format("$`"); // copy everything but file extension to buffer
    }

    //Start building new_filename
    if(!datex.empty()) {
        new_filename = datex;
    } 

    // std::regex clean("[A-Za-z0-9]+|&+"); // default
    std::regex clean("[A-Za-z0-9][a-z]+|[0-9A-Z]+|[0-9A-Z&a-z]");  //camel case aware
    int counter{};

    //Iterate through with regex to pull out words and numbers, build new_filename
    for(std::sregex_iterator i = std::sregex_iterator(buffer.begin(), buffer.end(), clean);
                             i != std::sregex_iterator();
                             ++i )
        {
            
            std::smatch m = *i;
            std::string m_str = m.str();
            std::string filler = FillerWords(m_str); //Check for filler words like 'the' 
            
            if(counter==0){ //Even filler words are Capitalized at beginning of titles 'The Title'
                m_str[0] = std::toupper(m_str[0]);
                new_filename += m_str;
                ++counter;
            }else if(m_str == ext.substr(1)){  // Discard any file extension duplicates: filename.pdf.pdf
                continue;
            }else if(filler != "" && delim != ""){   // Filler words: The_book_Of_THE_ages.pdf to The_Book_of_the_Ages.pdf
                new_filename += delim + filler;          // Unless CamelCase: TheBookOfTheAges.pdf
            }else {
                m_str[0] = std::toupper(m_str[0]);
                new_filename += delim + m_str;
            }
        }
    
    new_filename += ext; //Add back on the filename extension

    //If the new_filename name is empty for some reason, it will just keep the old file name
    if(new_filename.empty())
    {
        std::cout << "ERROR: new file name empty. Keeping old file name : " << filename << std::endl;
        new_filename = filename;
    }
    
}

void CustomClean(std::string& filename, std::string& new_filename, int& sub1, int& sub2) {
    switch (sub1)
    {
    case 0: // Just change the date
        Clean(filename, new_filename, "CUSTOM", sub2);
        break;
    case 1: // Underscore delimeter
        Clean(filename, new_filename, "_", sub2);
        break;
    case 2: // Hyphen delimeter
        Clean(filename, new_filename, "-", sub2);
        break;
    case 3: // CamelCase
        Clean(filename, new_filename, "", sub2);
        break;
    }
}

bool DateMatch(std::string& filename, std::string& buffer, std::string& datex, int& format) {

    std::regex date_ymd("(19|20)\\d\\d[- _/.](0[1-9]|1[012])[- _/.](0[1-9]|[12][0-9]|3[01])");

    if (DateRegexSearch(filename, buffer, datex, date_ymd)) { 
        format = 1;
        return true; 
    }

    std::regex date_mdy("(0[1-9]|1[012])[- _/.](0[1-9]|[12][0-9]|3[01])[- _/.](19|20)\\d\\d");

    if (DateRegexSearch(filename, buffer, datex, date_mdy)) { 
        format = 2;
        return true; 
    }

    std::regex date_dmy("(0[1-9]|[12][0-9]|3[01])[- _/.](0[1-9]|1[012])[- _/.](19|20)\\d\\d");

    if (DateRegexSearch(filename, buffer, datex, date_dmy)) { 
        format = 3;
        return true;  
    }
    return false;
}

bool DateRegexSearch(std::string& filename, std::string& buffer, std::string& datex, std::regex& reg) {
    std::smatch date_match;
    std::string dash_replace("-");

    if (std::regex_search(filename, date_match, reg)) {
        datex = date_match.str() + "-";
        std::regex date_delim("[\\.,\\s_]");
        datex = std::regex_replace(datex, date_delim, dash_replace);
        buffer = date_match.format("$`""$'");
        return true;
    }
    return false;
}

void DateReformat(std::string& date, const int& reformat, const int& format) {
    std::string buffer{""};
    switch (format)
    {
    case 1:
        if(reformat == 2){
            //yyyy-mm-dd to mm-dd-yyyy
           buffer = date.substr(5) + date.substr(0,4);
        }
        if(reformat == 3){
            //yyyy-mm-dd to dd-mm-yyyy
            buffer = date.substr(8,2) + date.substr(4,4) + date.substr(0,4);
        }
        break;
    case 2:
        if(reformat == 1){
            //mm-dd-yyyy to yyyy-mm-dd
            buffer = date.substr(6,5) + date.substr(0,5);
        }
        if(reformat == 3){  
            //mm-dd-yyyy to dd-mm-yyyy
            buffer = date.substr(3,3) + date.substr(0,3) + date.substr(6,4);
        }
        break;
    case 3:
        if(reformat == 1){
            //dd-mm-yyyy to yyyy-mm-dd
            buffer = date.substr(6,4) + date.substr(2,4) + date.substr(0,2);
        }
        if(reformat == 2){
            //dd-mm-yyyy to mm-dd-yyyy
            buffer = date.substr(3,3) + date.substr(0,3) + date.substr(6,4);
        }
        break;
    }
    date = buffer + "-";
}

void RemoveNums(std::string& filename, std::string& new_filename) {
    std::smatch num_match;
    std::regex reg("^[0-9]+");
    if (std::regex_search(filename, num_match, reg)) {
        new_filename = num_match.format("$`""$'");
    }
}

std::string FillerWords(std::string_view s) {
    if (s == "THE" || s == "The" || s == "the")
    {
        return "the";
    }
    if (s == "BY" || s == "By" || s == "by"){
        return "by";
    }
    if (s == "is" || s == "IS" || s == "Is"){
        return "is";
    }
    if (s == "of" || s == "Of" || s == "OF"){
        return "of";
    }
    if (s == "A" || s == "a"){
        return "a";
    }
    if (s == "at" || s == "AT" || s == "At")
    {
        return "at";
    }
    if (s == "as" || s == "AS" || s == "As")
    {
        return "as";
    }
    if (s == "for" || s == "FOR" || s == "For")
    {
        return "for";
    }
    if (s == "in" || s == "IN" || s == "In")
    {
        return "in";
    }
    if (s == "off" || s == "OFF" || s == "Off")
    {
        return "off";
    }
    if (s == "on" || s == "ON" || s == "On")
    {
        return "on";
    }
    if (s == "per" || s == "PER" || s == "Per")
    {
        return "per";
    }
    if (s == "to" || s == "TO" || s == "To")
    {
        return "to";
    }
    if (s == "up" || s == "UP" || s == "Up")
    {
        return "up";
    }
    if (s == "via" || s == "VIA" || s == "Via")
    {
        return "via";
    }
    if (s == "and" || s == "AND" || s == "And" || s == "&" || s == "&&")
    {
        return "and";
    }
    if (s == "but" || s == "BUT" || s == "But")
    {
        return "but";
    }
    if (s == "if" || s == "IF" || s == "If")
    {
        return "if";
    }
    if (s == "nor" || s == "NOR" || s == "Nor")
    {
        return "nor";
    }
    if (s == "or" || s == "OR" || s == "Or")
    {
        return "or";
    }
    if (s == "so" || s == "SO" || s == "So")
    {
        return "so";
    }
    if (s == "yet" || s == "YET" || s == "Yet")
    {
        return "yet";
    }
    return "";
}