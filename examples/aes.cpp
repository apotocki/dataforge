#include <string>
#include <string_view>
#include <iostream>

#include "dataforge/quark_push_iterator.hpp"
#include "dataforge/ciphers/aes.hpp"
#include "dataforge/base_xx/base16.hpp"

int main(int argc, char** argv)
{
    using namespace dataforge;
    using namespace std::literals::string_view_literals;

    // Test data and AES-128 key (16 bytes)
    std::string_view test_str = "The quick brown fox jumps over the lazy dog."sv;
    std::span<const unsigned char> key128 = "123456789abcdef0"_bs;  // 16 bytes = 128 bits
    
    std::cout << "=== AES-128 Encryption/Decryption Example ===\n\n";
    std::cout << "Original text: \"" << test_str << "\"\n";
    std::cout << "Key (hex):     " << std::string(reinterpret_cast<const char*>(key128.data()), key128.size()) << "\n\n";

    // Encryption: plaintext -> AES encrypt -> hex encoding
    std::string encrypted_hex_result;
    auto encrypt_it = quark_push_iterator(
        int8 | aes(128, key128, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16u, 
        std::back_inserter(encrypted_hex_result)
    );
    *encrypt_it = test_str;
    encrypt_it.finish(); // Complete the encryption and flush any buffered data
    
    std::cout << "Encrypted (hex): " << encrypted_hex_result << "\n\n";

    // Decryption: hex decode -> AES decrypt -> plaintext
    std::string decrypted_result;
    auto decrypt_it = quark_push_iterator(
        base16u | int8 / aes(128, key128, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, 
        std::back_inserter(decrypted_result)
    );
    *decrypt_it = encrypted_hex_result;
    decrypt_it.finish(); // Complete the decryption and flush any buffered data
    
    std::cout << "Decrypted text:  \"" << decrypted_result << "\"\n";

    // Verify round-trip consistency
    if (test_str == decrypted_result) {
        std::cout << "? Round-trip encryption/decryption successful!\n\n";
    } else {
        std::cout << "? Round-trip failed - strings don't match\n\n";
    }

    // Demonstrate AES-256 with CBC mode and IV
    std::cout << "=== AES-256 CBC Mode with IV ===\n";
    std::span<const unsigned char> key256 = "123456789abcdef0123456789abcdef0"_bs;  // 32 bytes = 256 bits
    std::span<const unsigned char> iv = "abcdef0123456789"_bs;  // 16 bytes initialization vector
    
    std::cout << "Key size: 256-bit, Mode: CBC, IV: " 
              << std::string(reinterpret_cast<const char*>(iv.data()), iv.size()) << "\n";

    encrypted_hex_result.clear();
    encrypt_it = quark_push_iterator(
        int8 | aes(128, key256, cipher_mode_type::CBC, iv, padding_type::pkcs) / int8 | base16u, 
        std::back_inserter(encrypted_hex_result)
    );
    *encrypt_it = test_str;
    encrypt_it.finish();
    
    std::cout << "CBC Encrypted:   " << encrypted_hex_result << "\n";

    decrypted_result.clear();
    decrypt_it = quark_push_iterator(
        base16u | int8 / aes(128, key256, cipher_mode_type::CBC, iv, padding_type::pkcs) | int8, 
        std::back_inserter(decrypted_result)
    );
    *decrypt_it = encrypted_hex_result;
    decrypt_it.finish();
    
    std::cout << "CBC Decrypted:   \"" << decrypted_result << "\"\n";
    
    if (test_str == decrypted_result) {
        std::cout << "? AES-256 CBC round-trip successful!\n";
    } else {
        std::cout << "? AES-256 CBC round-trip failed\n";
    }

    return 0;
}
