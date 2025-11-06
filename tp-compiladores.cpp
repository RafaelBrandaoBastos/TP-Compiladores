// C++ Program to Read a file line by line using getline
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

void lexicalAnalyzer(string line) {
    for (char c : line) {
        cout << "[" << c << "]" << endl; // imprime cada caractere (inclusive espaços)
    }
}

int main()
{
    // Create an input file stream object named 'file' and
    ifstream file("code.txt");
    ofstream MyFile("result.asm");

    // String to store each line of the file.
    string line;

    if (file.is_open())
    {
        // Read each line from the file and store it in the 'line' variable.
        while (getline(file, line))
        {
			lexicalAnalyzer(line);
        }

        // Close the file stream once all lines have been read.
        file.close();
    }
    else
    {
        // Print an error message to the standard error stream if the file cannot be opened.
        cerr << "Unable to open file!" << endl;
    }

    return 0;
}