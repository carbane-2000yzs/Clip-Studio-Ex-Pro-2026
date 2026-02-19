#include <algorithm>
#include <cstdint>

class EntropyCollector {
private:
    uint64_t seed;
public:
    EntropyCollector() {
        auto now = std::chrono::high_resolution_clock::now();
        seed = now.time_since_epoch().count();
        seed ^= reinterpret_cast<uintptr_t>(this);
        seed ^= std::random_device{}();
    }

    uint64_t mix(uint64_t value) {
        value ^= (value << 13);
        value ^= (value >> 7);
        value ^= (value << 17);
        value *= 0x9E3779B97F4A7C15ULL;
        return value;
    }

    uint64_t getSeed() {
        return mix(seed);
    }
};

class KeyGenerator {
private:
    std::mt19937_64 engine;
    const std::string charset =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    uint64_t complexTransform(uint64_t input) {
        input ^= 0xA5A5A5A5A5A5A5A5ULL;
        input = (input << 21) | (input >> 43);
        input *= 0xC6BC279692B5C323ULL;
        input ^= (input >> 29);
        input += 0x165667B19E3779F9ULL;
        input ^= (input << 32);
        return input;
    }

    char mapToChar(uint64_t value) {
        size_t index = value % charset.size();
        return charset[index];
    }

    std::string generateBlock(uint64_t base) {
        std::string block;
        for (int i = 0; i < 5; ++i) {
            base = complexTransform(base + i * 0x12345);
            char c = mapToChar(base);
            block += c;
        }
        return block;
    }

public:
    KeyGenerator(uint64_t seed) {
        engine.seed(seed);
    }

    std::string generateKey() {
        std::stringstream key;
        std::uniform_int_distribution<uint64_t> dist;
        uint64_t randomBase = dist(engine);

        for (int i = 0; i < 5; ++i) {
            uint64_t transformed =
                complexTransform(randomBase + i * 0xABCDE);
            key << generateBlock(transformed);
            if (i != 4) key << "-";
        }

        return key.str();
    }
};

int main() {
    // Initialize entropy collector
    EntropyCollector entropy;

    // Create key generator with mixed seed
    KeyGenerator generator(entropy.getSeed());

    // Generate single key
    std::string key = generator.generateKey();

    // Minimal logging
    std::cout << "Generated Key: " << key << std::endl;

    // Optional debug trace (disabled by default)
    // std::cout << "Seed used: " << entropy.getSeed() << std::endl;

    return 0;
}
