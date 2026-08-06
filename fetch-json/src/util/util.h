//
//  util.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef util_h
#define util_h

#include <cassert>
#include <iostream>
#include <map>
#include <random>
#include <sstream>

// Non-Member Functions

// Return string_views copied to strings
std::vector<std::string>           copy(const std::vector<std::string_view> value);

// Return the decimal conversion of a hex string
int                                decimal(const std::string hex);

// Return the digits comprising a floating-point number; the decimal point is represented by INT_MAX
std::vector<int>                   digits(const double number);

/**
 * Return string escaped by double quotations
 */
std::string                        escape(const std::string string);

/**
 * Return true if value can be parsed into an integer, otherwise return false
 */
bool                               is_int(const std::string value);

/**
 * Return true if value can be parsed into a floating-point number, otherwise return false
 */
bool                               is_number(const std::string value);

// Return true if b is a power of n
bool                               is_pow(const size_t b, const size_t n);

// Return true if value includes at least one double quotation
bool                               is_string(const std::string value);

// Return values joined by delimiter
std::string                        join(std::vector<std::string> values, std::string delimeter);

/**
 * Merge double quotation-escaped tokens
 */
void                               merge(std::vector<std::string>& values, const std::string delimiter = "");

std::map<std::string, std::string> options(int argc, const char* argv[]);

/**
 * Return value parsed into an integer
 */
int                                parse_int(const std::string value);

/**
 * Return value parsed into a floating-point number
 */
double                             parse_number(const std::string value);

/**
 * Return the next power of n for b
 * I.e. pow(15, 2) = 16
 */
int                                pow(const int b, const int n = 2);

std::string                        replace(const std::string text, const std::string pattern, const std::string new_pattern);

std::string                        replace_all(const std::string text, const std::string pattern, const std::string new_pattern);

// Return string split by delimiter; reference only
std::vector<std::string_view>      split(const std::string_view string, const std::string delimeter);

// Return string split by delimiter; copy
std::vector<std::string>           split(const std::string string, const std::string delimeter);

// Return string split by delimiter (legacy); copy
void                               split(std::vector<std::string>& target, const std::string source, const std::string delimeter);

// Return true if text starts with pattern; otherwise, return false
bool                               starts_with(const std::string text, const std::string pattern);

// Return string split by whitespace; reference only
std::vector<std::string_view>      tokens(const std::string_view string);

// Return string split by whitespace; copy
std::vector<std::string>           tokens(const std::string string);

// Return string split by whitespace (legacy); reference only
void                               tokens(std::vector<std::string>& target, const std::string source);

// Return lowercase string
std::string                        tolowerstr(std::string string);

// Return uppercase string
std::string                        toupperstr(std::string string);

/**
 * Return string trimmed of leading and trailing whitespace
 */
std::string                        trim(const std::string string);

// Return string trimmed of trailing whitespace
std::string                        trim_end(const std::string string);

// Return string trimmed of leading whitespace
std::string                        trim_start(const std::string string);

// Return floating-point number formatted to min precision
std::string                        truncate(const double number, const int min_prec = 0);

/**
 * Unescape double quotation-escaped string
 */
std::string                        unescape(const std::string string);

#endif /* util_h */
