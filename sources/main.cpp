
#include "../headers/WebServ.hpp"

int main(int argc, char **argv) 
{
	if (argc == 1 || argc == 2) 
    {
		try 
		{
			std::string config;
            if (argc == 1) 
            {
                config = "config/default.conf";
            } else 
            {
                config = argv[1];
            }
		}
		catch (std::exception &e)
        {
			std::cerr << e.what() << std::endl;
			return (ERROR);
		}
    }
    else 
	{
		return (ERROR);
	}
    return (SUCCESS);
}