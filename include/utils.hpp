

void                        set_nonblocking(int sock);
std::string&                pop(std::string& str);
std::string					readKey(const std::string& line);
std::string&				capitalize(std::string& str);
std::string&				to_upper(std::string& str);
std::string&				to_lower(std::string& str);
std::string&				strip(std::string& str, char c);
std::string					to_string(size_t n);
std::string					readValue(const std::string& line);
std::vector<std::string>	split(const std::string& str, char c);
void	                    ft_skip_spacenl(const char *str, int *i);
int		                    ft_atoi(const char *str);
bool                        compare_langs(const std::pair<std::string, float> first, const std::pair<std::string, float> second);