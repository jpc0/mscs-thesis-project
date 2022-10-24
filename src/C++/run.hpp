/*
  Header file for the runner module.
*/

#ifndef _RUN_HPP
#define _RUN_HPP

#include <set>
#include <variant>
#include <vector>
#include <functional>
#include <string_view>
#include <sys/time.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "input.hpp"

#if defined(__INTEL_LLVM_COMPILER)
#define LANG "cpp-intel"
#elif defined(__llvm__)
#define LANG "cpp-llvm"
#elif defined(__GNUC__)
#define LANG "cpp-gcc"
#endif

double get_time();

template<class T>
class Runner {
public:
	virtual T initializer(std::string_view) = 0;
	virtual int algorithm(T const &, std::string_view const &) = 0;
	virtual int run(std::string name, int argc, char* argv[]);
};

template<class T>
class MultiRunner {
public:
	virtual T initializer(std::vector<std::string_view>) = 0;
	virtual std::vector<int> algorithm(T const &, std::string_view const &) = 0;
	virtual int run(std::string name, int argc, char* argv[]);
};

template<class T>
int Runner<T>::run(std::string name, int argc, char* argv[]){
	if (argc < 3 || argc > 4) {
		std::ostringstream error;
		error << "Usage: " << argv[0] << " <sequences> <patterns> [ <answers> ]";
		throw std::runtime_error{error.str()};
	}

	// Read the three data files. Any of these that encounter an error will
	// throw an exception. The filenames are in the order: sequences patterns
	// answers.
	std::vector<std::string> sequences_data = read_sequences(argv[1]);
	int sequences_count = sequences_data.size();
	std::vector<std::string> patterns_data = read_patterns(argv[2]);
	int patterns_count = patterns_data.size();
	std::vector<std::vector<int>> answers_data;
	if (argc == 4) {
		answers_data = read_answers(argv[3]);
		int answers_count = answers_data.size();
		if (answers_count != patterns_count)
			throw std::runtime_error{
				"Count mismatch between patterns file and answers file"};
	}

	// Run it. For each sequence, try each pattern against it. The code function
	// pointer will return the number of matches found, which will be compared to
	// the table of answers for that pattern. Report any mismatches.
	double start_time = get_time();
	int return_code = 0; // Used for noting if some number of matches fail
	for (int pattern = 0; pattern < patterns_count; pattern++) {
		std::string_view pattern_str = patterns_data[pattern];
		// Pre-process the pattern before applying it to all sequences.
		T pat_data = initializer(pattern_str);

		for (int sequence = 0; sequence < sequences_count; sequence++) {
			std::string& sequence_str = sequences_data[sequence];

			int matches = algorithm(pat_data, sequence_str);

			if (answers_data.size() && matches != answers_data[pattern][sequence]) {
				std::cerr << "Pattern " << pattern + 1 << " mismatch against sequence "
					<< sequence + 1 << " (" << matches
					<< " != " << answers_data[pattern][sequence] << ")\n";
				return_code++;
			}
		}
	}
	// Note the end time.
	double end_time = get_time();

	std::cout << "language: " << LANG << "\n"
		<< "algorithm: " << name << "\n"
		<< "runtime: " << std::setprecision(8) << end_time - start_time
		<< "\n";

	return return_code;
};

template<class T>
int MultiRunner<T>::run(std::string name, int argc, char* argv[]){
	if (argc < 3 || argc > 4) {
		std::ostringstream error;
		error << "Usage: " << argv[0] << " <sequences> <patterns> [ <answers> ]";
		throw std::runtime_error{error.str()};
	}

	// Read the three data files. Any of these that encounter an error will
	// throw an exception. The filenames are in the order: sequences patterns
	// answers.
	std::vector<std::string> sequences_data = read_sequences(argv[1]);
	int sequences_count = sequences_data.size();
	std::vector<std::string> patterns_data = read_patterns(argv[2]);
	int patterns_count = patterns_data.size();
	std::vector<std::vector<int>> answers_data;
	if (argc == 4) {
		answers_data = read_answers(argv[3]);
		int answers_count = answers_data.size();
		if (answers_count != patterns_count)
			throw std::runtime_error{
				"Count mismatch between patterns file and answers file"};
	}
	std::vector<std::string_view> sv_patterns_data{patterns_data.begin(), patterns_data.end()};

	// Run it. For each sequence, try each pattern against it. The code function
	// pointer will return the number of matches found, which will be compared to
	// the table of answers for that pattern. Report any mismatches.
	double start_time = get_time();
	int return_code = 0; // Used for noting if some number of matches fail

	// Pre-process the patterns before applying to all sequences.
	T pat_data = initializer(sv_patterns_data);

	for (int sequence = 0; sequence < sequences_count; sequence++) {
		std::string_view sequence_str = sequences_data[sequence];

		std::vector<int> matches = algorithm(pat_data, sequence_str);

		if (answers_data.size()) {
			for (int pattern = 0; pattern < patterns_count; pattern++) {
				if (matches[pattern] != answers_data[pattern][sequence]) {
					std::cerr << "Pattern " << pattern + 1
						<< " mismatch against sequence " << sequence + 1 << " ("
						<< matches[pattern]
						<< " != " << answers_data[pattern][sequence] << ")\n";
					return_code++;
				}
			}
		}
	}
	// Note the end time.
	double end_time = get_time();

	std::cout << "language: " << LANG << "\n"
		<< "algorithm: " << name << "\n"
		<< "runtime: " << std::setprecision(8) << end_time - start_time
		<< "\n";

	return return_code;
}

#endif // !_RUN_HPP
