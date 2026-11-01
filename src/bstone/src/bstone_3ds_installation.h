// Native launcher discovery; uses the engine's authoritative resource catalog.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef BSTONE_3DS_INSTALLATION_INCLUDED
#define BSTONE_3DS_INSTALLATION_INCLUDED
#include "bstone_game_catalog.h"
#include "bstone_sha1.h"
#include <cstdio>
#include <string>
#include <vector>
namespace bstone { namespace n3ds_install {
enum class Edition { aog_full, aog_shareware, planet_strike };
enum class State { absent, incomplete, present, verifying, ready, invalid };
struct File
{
    std::string name, path, hash;
    std::int64_t size{};
    bool present{};
};
struct Installation
{
    Edition edition{};
    State state{};
    bool legacy{};
    std::string directory;
    std::vector<File> files;
    AssetsVersion version{AssetsVersion::none};
    int found{}, checked{};
};
struct Selection
{
    bool active{};
    AssetsVersion version{AssetsVersion::none};
    Edition edition{};
    std::string data_directory, profile_directory, legacy_directory;
};
const char* edition_name(Edition edition) noexcept;
const char* state_name(State state) noexcept;
AssetsVersion identify_version(Edition edition, const std::vector<File>& files);
class Scanner
{
public:
    explicit Scanner(const std::string& root);
    ~Scanner();
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    void rescan();
    void step(int index); // at most one 32 KiB block per UI frame
    int preferred(bool ps) const;
    std::vector<Installation> entries;
private:
    std::string root_;
    std::FILE* file_{};
    int active_{-1};
    Sha1 sha_{};
    std::vector<unsigned char> buffer_ = std::vector<unsigned char>(32768);
    void close();
};
Selection& selection();
Selection make_selection(const Installation& entry, const std::string& root);
std::string profile_read_path(const Selection& selected, const std::string& filename);
bool create_directories(const std::string& path);
} }
#endif
