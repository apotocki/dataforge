#include <string>
#include <string_view>
#include <iostream>

#include "dataforge/quark_push_iterator.hpp"
#include "dataforge/quark_pull_iterator.hpp"
#include "dataforge/base_xx/base64.hpp"

int main(int argc, char** argv)
{
    using namespace dataforge;
    using namespace std::literals::string_view_literals;

    std::string_view test_str = "Hello, World!";
    
    // Encoding: Convert string to Base64 using push iterator
    std::string base64_result;
    auto push_it = quark_push_iterator(int8 | base64, std::back_inserter(base64_result));
    *push_it = test_str;
    push_it.finish(); // Flush any buffered data to complete the encoding
    std::cout << "Original String: \"" << test_str << "\"\n";
    std::cout << "Base64 Encoded:  " << base64_result << "\n\n";

    // Decoding: Convert Base64 back to string using pull iterator
    std::string restored_result;
    auto pull_it = quark_pull_iterator(base64 | int8, base64_result);
    for (auto span = *pull_it; !span.empty(); span = *++pull_it) {
        std::copy(span.begin(), span.end(), std::back_inserter(restored_result));
    }
    std::cout << "Base64 Input:    " << base64_result << "\n";
    std::cout << "Decoded String:  \"" << restored_result << "\"\n\n";

    // Demonstrate round-trip consistency
    if (test_str == restored_result) {
        std::cout << "? Round-trip encoding/decoding successful!\n";
    } else {
        std::cout << "? Round-trip failed - strings don't match\n";
    }

    // Example with longer text to show Base64 padding
    std::string_view longer_text = "The quick brown fox jumps over the lazy dog"sv;
    base64_result.clear();
    
    push_it = quark_push_iterator(int8 | base64, std::back_inserter(base64_result));
    *push_it = longer_text;
    push_it.finish();
    
    std::cout << "\nLonger example:\n";
    std::cout << "Original: \"" << longer_text << "\"\n";
    std::cout << "Base64:   " << base64_result << "\n";
    std::cout << "Length:   " << longer_text.length() << " bytes -> " 
              << base64_result.length() << " Base64 characters\n";

    return 0;
}
