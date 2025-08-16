#include <string>
#include <string_view>
#include <iostream>
#include <iomanip>
#include <vector>

#include "dataforge/quark_push_iterator.hpp"
#include "dataforge/checksum/crc.hpp"
#include "dataforge/base_xx/base16.hpp"

int main(int argc, char** argv)
{
    using namespace dataforge;
    using namespace std::literals::string_view_literals;

    std::cout << "=== CRC (Cyclic Redundancy Check) Examples ===\n\n";

    // Standard test data used in CRC specifications
    std::string_view test_data = "123456789"sv;
    std::cout << "Test data: \"" << test_data << "\"\n\n";

    // =================================================================
    // CRC8 Examples - 8-bit checksums for simple error detection
    // =================================================================
    std::cout << "--- CRC8 Variants ---\n";
    
    uint8_t crc8_result;
    std::vector<std::pair<crc8_type, std::string_view>> crc8_types = {
        {crc8_type::DEFAULT, "DEFAULT"},
        {crc8_type::MAXIM, "MAXIM (1-Wire)"},
        {crc8_type::CDMA2000, "CDMA2000"},
        {crc8_type::ITU, "ITU-T"},
        {crc8_type::ROHC, "ROHC"}
    };

    for (const auto& [type, name] : crc8_types) {
        auto push_it = quark_push_iterator(int8 | crc(type), &crc8_result);
        *push_it = test_data;
        push_it.finish();
        std::cout << std::left << std::setw(15) << name << ": 0x" 
                  << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                  << static_cast<unsigned>(crc8_result) << std::dec << "\n";
    }

    // =================================================================
    // CRC16 Examples - 16-bit checksums for moderate error detection
    // =================================================================
    std::cout << "\n--- CRC16 Variants ---\n";
    
    uint16_t crc16_result;
    std::vector<std::pair<crc16_type, std::string_view>> crc16_types = {
        {crc16_type::DEFAULT, "DEFAULT (ARC)"},
        {crc16_type::CCITT_FALSE, "CCITT-FALSE"},
        {crc16_type::XMODEM, "XMODEM"},
        {crc16_type::MODBUS, "MODBUS"},
        {crc16_type::USB, "USB"},
        {crc16_type::KERMIT, "KERMIT"}
    };

    for (const auto& [type, name] : crc16_types) {
        auto push_it = quark_push_iterator(int8 | crc(type), &crc16_result);
        *push_it = test_data;
        push_it.finish();
        std::cout << std::left << std::setw(15) << name << ": 0x" 
                  << std::hex << std::uppercase << std::setfill('0') << std::setw(4) 
                  << crc16_result << std::dec << "\n";
    }

    // =================================================================
    // CRC32 Examples - 32-bit checksums for strong error detection
    // =================================================================
    std::cout << "\n--- CRC32 Variants ---\n";
    
    uint32_t crc32_result;
    std::vector<std::pair<crc32_type, std::string_view>> crc32_types = {
        {crc32_type::DEFAULT, "DEFAULT (ZLIB)"},
        {crc32_type::BZIP2, "BZIP2"},
        {crc32_type::MPEG2, "MPEG2"},
        {crc32_type::POSIX, "POSIX"},
        {crc32_type::JAMCRC, "JAMCRC"}
    };

    for (const auto& [type, name] : crc32_types) {
        auto push_it = quark_push_iterator(int8 | crc(type), &crc32_result);
        *push_it = test_data;
        push_it.finish();
        std::cout << std::left << std::setw(15) << name << ": 0x" 
                  << std::hex << std::uppercase << std::setfill('0') << std::setw(8) 
                  << crc32_result << std::dec << "\n";
    }

    // =================================================================
    // CRC64 Examples - 64-bit checksums for very strong error detection
    // =================================================================
    std::cout << "\n--- CRC64 Variants ---\n";
    
    uint64_t crc64_result;
    std::vector<std::pair<crc64_type, std::string_view>> crc64_types = {
        {crc64_type::DEFAULT, "DEFAULT"},
        {crc64_type::WE, "WE"},
        {crc64_type::XZ, "XZ"}
    };

    for (const auto& [type, name] : crc64_types) {
        auto push_it = quark_push_iterator(int8 | crc(type), &crc64_result);
        *push_it = test_data;
        push_it.finish();
        std::cout << std::left << std::setw(15) << name << ": 0x" 
                  << std::hex << std::uppercase << std::setfill('0') << std::setw(16) 
                  << crc64_result << std::dec << "\n";
    }

    // =================================================================
    // Practical Application Examples
    // =================================================================
    std::cout << "\n=== Practical Applications ===\n\n";

    // File integrity checking example
    std::cout << "--- File Integrity Checking ---\n";
    std::string_view file_content = "This is a sample file content for integrity verification."sv;
    
    auto push_it = quark_push_iterator(int8 | crc(crc32_type::DEFAULT), &crc32_result);
    *push_it = file_content;
    push_it.finish();
    
    std::cout << "File content: \"" << file_content << "\"\n";
    std::cout << "CRC32 checksum: 0x" << std::hex << std::uppercase << crc32_result << std::dec << "\n\n";

    // Network packet validation example
    std::cout << "--- Network Packet Validation ---\n";
    std::string_view packet_data = "Hello, Network!"sv;
    
    auto net_push_it = quark_push_iterator(int8 | crc(crc16_type::CCITT_FALSE), &crc16_result);
    *net_push_it = packet_data;
    net_push_it.finish();
    
    std::cout << "Packet data: \"" << packet_data << "\"\n";
    std::cout << "CRC16-CCITT checksum: 0x" << std::hex << std::uppercase << crc16_result << std::dec << "\n\n";

    // =================================================================
    // Pipeline Integration Example
    // =================================================================
    std::cout << "--- Pipeline Integration ---\n";
    std::cout << "Computing CRC32 and converting to hex string:\n";
    
    std::string hex_result;
    // Complex pipeline: bytes ? CRC32 ? convert to big-endian int32 ? to bytes ? hex encoding
    auto pipeline_it = quark_push_iterator(
        int8 | crc(crc32_type::DEFAULT) / int32 | be/int8 | base16u, 
        std::back_inserter(hex_result)
    );
    
    *pipeline_it = test_data;
    pipeline_it.finish();
    
    std::cout << "Input: \"" << test_data << "\"\n";
    std::cout << "CRC32 as hex string: " << hex_result << " (big-endian format)\n\n";

    // =================================================================
    // Incremental Processing Example
    // =================================================================
    std::cout << "--- Incremental Processing ---\n";
    std::cout << "Processing data in chunks (demonstrates CRC streaming capability):\n";
    
    auto inc_push_it = quark_push_iterator(int8 | crc(crc32_type::DEFAULT), &crc32_result);
    
    // Process data in chunks
    std::string_view chunk1 = "123"sv;
    std::string_view chunk2 = "456"sv; 
    std::string_view chunk3 = "789"sv;
    
    *inc_push_it = chunk1;
    *inc_push_it = chunk2;
    *inc_push_it = chunk3;
    inc_push_it.finish();
    
    std::cout << "Chunk 1: \"" << chunk1 << "\"\n";
    std::cout << "Chunk 2: \"" << chunk2 << "\"\n";
    std::cout << "Chunk 3: \"" << chunk3 << "\"\n";
    std::cout << "Combined CRC32: 0x" << std::hex << std::uppercase << crc32_result << std::dec;
    std::cout << " (same as processing \"123456789\" at once)\n\n";

    std::cout << "=== Summary ===\n";
    std::cout << "• CRC8:  Fast, simple error detection for small data\n";
    std::cout << "• CRC16: Balanced performance/reliability for protocols\n";
    std::cout << "• CRC32: Strong error detection for files/networking\n";
    std::cout << "• CRC64: Maximum reliability for critical applications\n";
    std::cout << "\nDataForge pipeline features demonstrated:\n";
    std::cout << "• Multiple CRC algorithm support with consistent API\n";
    std::cout << "• Seamless integration with endianness conversion (be/le)\n";
    std::cout << "• Chaining with encoding operations (base16, base64, etc.)\n";
    std::cout << "• Streaming/incremental processing for large datasets\n";

    return 0;
}