// SPDX-License-Identifier: GPL-2.0-or-later
#include "bstone_3ds_installation.h"
#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <sys/stat.h>
namespace bstone { namespace n3ds_install {
namespace {
std::vector<AssetsVersion> versions(Edition e)
{
    if (e == Edition::planet_strike) { return {AssetsVersion::ps}; }
    if (e == Edition::aog_full) { return {AssetsVersion::aog_full_v3_0,AssetsVersion::aog_full_v2_1,AssetsVersion::aog_full_v2_0,AssetsVersion::aog_full_v1_0}; }
    return {AssetsVersion::aog_sw_v3_0,AssetsVersion::aog_sw_v2_1,AssetsVersion::aog_sw_v2_0,AssetsVersion::aog_sw_v1_0};
}
bool exists(const std::string& path, std::int64_t* size = nullptr)
{
    struct stat st{};
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) { return false; }
    if (size) { *size = st.st_size; }
    return true;
}
std::string path_for(const std::string& dir, const char* name, std::int64_t& size)
{
    auto path = dir + "/" + name;
    if (exists(path, &size)) { return path; }
    std::string lower = name;
    for (auto& ch : lower) { ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); }
    path = dir + "/" + lower;
    return exists(path, &size) ? path : std::string{};
}
}
const char* edition_name(Edition e) noexcept
{ return e == Edition::aog_shareware ? "SHAREWARE" : "FULL"; }
const char* state_name(State s) noexcept
{
    switch (s) {
    case State::absent:return "NOT INSTALLED";
    case State::incomplete:return "INCOMPLETE";
    case State::present:return "PRESENT - NOT VERIFIED";
    case State::verifying:return "VERIFYING SHA-1";
    case State::ready:return "READY - SHA-1 VERIFIED";
    default:return "INVALID / UNKNOWN DATA";
    }
}
Scanner::Scanner(const std::string& root) : root_{root} { rescan(); }
AssetsVersion identify_version(Edition edition, const std::vector<File>& files)
{
    for (auto version : versions(edition))
    {
        const auto& resources = resources_for_version(version);
        if (files.size() != static_cast<std::size_t>(resources.get_size())) { continue; }
        bool match = true;
        for (std::size_t i = 0; i < files.size(); ++i)
        { match &= files[i].hash == resources[static_cast<std::intptr_t>(i)].hash_string; }
        if (match) { return version; }
    }
    return AssetsVersion::none;
}
Scanner::~Scanner() { close(); }
void Scanner::close() { if (file_) { std::fclose(file_); file_ = nullptr; } active_ = -1; sha_ = Sha1{}; }
void Scanner::rescan()
{
    close(); entries.clear();
    const std::array<Edition,5> editions{{Edition::aog_full,Edition::aog_shareware,Edition::aog_full,Edition::aog_shareware,Edition::planet_strike}};
    for (int i = 0; i < 5; ++i)
    {
        Installation e{}; e.edition = editions[i]; e.legacy = i == 2 || i == 3;
        e.directory = e.legacy ? root_ : root_ + (i == 4 ? "/games/ps" : "/games/aog");
        for (const auto& resource : resources_for_version(versions(e.edition).front()))
        {
            File f{}; f.name = resource.file_name; f.path = path_for(e.directory, resource.file_name, f.size);
            f.present = !f.path.empty(); e.found += f.present; e.files.push_back(std::move(f));
        }
        const bool all = e.found == int(e.files.size());
        const bool nonempty = std::all_of(e.files.begin(), e.files.end(), [](const File& f){return f.size > 0;});
        e.state = !e.found ? State::absent : all && nonempty ? State::present : State::incomplete;
        entries.push_back(std::move(e));
    }
}
void Scanner::step(int index)
{
    auto& e = entries.at(index);
    if (e.state != State::present && e.state != State::verifying) { return; }
    if (active_ != index) { close(); active_ = index; }
    e.state = State::verifying;
    if (e.checked == int(e.files.size()))
    {
        e.version = identify_version(e.edition, e.files);
        e.state = e.version == AssetsVersion::none ? State::invalid : State::ready;
        close(); return;
    }
    auto& f = e.files[e.checked];
    if (!file_) { file_ = std::fopen(f.path.c_str(), "rb"); sha_ = Sha1{}; }
    if (!file_) { e.state = State::invalid; close(); return; }
    const auto count = std::fread(buffer_.data(), 1, buffer_.size(), file_);
    if (std::ferror(file_)) { e.state = State::invalid; close(); return; }
    if (count) { sha_.process(buffer_.data(), count); }
    if (std::feof(file_))
    {
        sha_.finish();
        constexpr char hex[] = "0123456789abcdef";
        for (auto value : sha_.get_digest()) { f.hash += hex[value >> 4]; f.hash += hex[value & 15]; }
        ++e.checked;
        std::fclose(file_); file_ = nullptr;
    }
}
int Scanner::preferred(bool ps) const
{
    if (ps) { return 4; }
    // Complete Full before complete SW; new layout before legacy within edition.
    for (int i : {0,2,1,3}) { if (entries[i].state == State::present || entries[i].state == State::ready) { return i; } }
    int best = 0;
    for (int i : {0,2,1,3}) { if (entries[i].found > entries[best].found) { best = i; } }
    return best;
}
Selection& selection() { static Selection selected; return selected; }
Selection make_selection(const Installation& e, const std::string& root)
{
    Selection s{};
    if (e.state != State::ready || e.version == AssetsVersion::none ||
        identify_version(e.edition, e.files) != e.version) { return s; }
    // Recheck presence/length at launch, not just the original presence scan.
    for (const auto& f : e.files) { std::int64_t size{}; if (!exists(f.path,&size) || size != f.size) { return s; } }
    s.active = true; s.edition = e.edition; s.version = e.version; s.data_directory = e.directory;
    const auto key = e.edition == Edition::planet_strike ? "ps" : e.edition == Edition::aog_full ? "aog_full" : "aog_sw";
    s.profile_directory = root + "/profiles/" + key + "/";
    if (e.edition != Edition::planet_strike) { s.legacy_directory = root + "/"; }
    return s;
}
std::string profile_read_path(const Selection& s, const std::string& name)
{
    const auto modern = s.profile_directory + name;
    if (exists(modern) || s.legacy_directory.empty()) { return modern; }
    const auto legacy = s.legacy_directory + name;
    return exists(legacy) ? legacy : modern;
}
bool create_directories(const std::string& path)
{
    for (std::size_t i = 1; i <= path.size(); ++i)
    {
        if (i != path.size() && path[i] != '/') { continue; }
        const auto part = path.substr(0,i);
        if (!part.empty() && part.back() == ':') { continue; }
        if (mkdir(part.c_str(),0777) != 0)
        {
            if (errno != EEXIST) { return false; }
            struct stat st{};
            if (stat(part.c_str(),&st) != 0 || !S_ISDIR(st.st_mode)) { return false; }
        }
    }
    return true;
}
} }
