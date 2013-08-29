#include "UserInput.hpp"

#include <iostream>

namespace UserInput
{
	int getInt(int low, int high, const std::string& prompt)
{
	int choice = -1;
	while(!(choice >= low && choice <= high))
	{
		std::cout << std::endl;
		std::cout << prompt << " ";
		std::cin >> choice;
		if(!std::cin) std::cout << "Sorry, I didn't understand that."
			<< "Please enter an integer from " << 
			std::to_string(static_cast<long long>(low)) <<
			" to " <<
			std::to_string(static_cast<long long>(high)) <<
			", and press enter.\n";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return choice;
}
}
