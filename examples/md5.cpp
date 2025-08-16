#include <string>
#include <string_view>
#include <iostream>

#include "dataforge/quark_push_iterator.hpp"
#include "dataforge/hashes/md5.hpp"
#include "dataforge/base_xx/base16.hpp"

int main(int argc, char** argv)
{
    using namespace dataforge;
    using namespace std::literals::string_view_literals;

    std::string_view test_str = "The quick brown fox jumps over the lazy dog."sv;
    std::string md5_hex_result;

    // Create a processing pipeline: input bytes -> MD5 hash -> uppercase hex encoding
    auto push_it = quark_push_iterator(int8 | md5 | base16u, std::back_inserter(md5_hex_result));
    
    // Method 1: Push the entire string at once
    *push_it = test_str;
    push_it.finish(); // Flush any buffered data to complete the hash calculation
    std::cout << "MD5 hash of \"" << test_str << "\": " << md5_hex_result << std::endl;

    // Method 2: Byte-by-byte processing using standard algorithms
    md5_hex_result.clear();
    push_it = std::copy(test_str.begin(), test_str.end(), std::move(push_it));
    push_it.finish();
    std::cout << "MD5 hash of \"" << test_str << "\": " << md5_hex_result << std::endl;

    // Method 3: Incremental processing with mixed data types
    md5_hex_result.clear();
    *push_it = "The quick brown fo"sv; // Use string_view to avoid including null terminator
    *push_it = 'x';                     // Push individual character
    *push_it = " jumps over the lazy dog"sv;
    *push_it = '.';                     // Complete with final character
    push_it.finish();
    std::cout << "MD5 hash of \"" << test_str << "\": " << md5_hex_result << std::endl;

    return 0;
}
