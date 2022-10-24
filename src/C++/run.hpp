/*
  Header file for the runner module.
*/

#ifndef _RUN_HPP
#define _RUN_HPP

#include <set>
#include <string>
#include <variant>
#include <vector>
#include <functional>

using PatternData =  std::variant<std::string, std::vector<int>, unsigned long,
                     std::vector<unsigned long>>;
using algorithm = std::function<int(std::vector<PatternData>, std::string)>;
using initializer = std::function<std::vector<PatternData>(std::string)>;
extern int run(initializer init, algorithm algo, std::string name, int argc,
               char *argv[]);

using MultiPatternData =  std::variant<int, std::vector<int>, std::vector<std::vector<int>>,
                     std::vector<std::set<int>>>;

// typedef std::vector<int> (*mp_algorithm)(std::vector<MultiPatternData> const &,
//                                         std::string const &);
using mp_algorithm = std::function<std::vector<int>(std::vector<MultiPatternData> const &,
		std::string const &)>;
// typedef std::vector<MultiPatternData> (*mp_initializer)(
//     std::vector<std::string> const &);
using mp_initializer = std::function<std::vector<MultiPatternData>(std::vector<std::string> const &)>;
extern int run_multi(mp_initializer init, mp_algorithm algo, std::string name,
                     int argc, char *argv[]);

#endif // !_RUN_HPP
