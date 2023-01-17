#ifndef INCLUDE_LOGGING_HPP_
#define INCLUDE_LOGGING_HPP_
#include <string>
std::wstring const get_log_path(void);
void set_log_path(std::wstring path);
void ensure_logging(void);
#endif  /* INCLUDE_LOGGING_HPP_ */