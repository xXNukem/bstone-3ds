#ifndef BSTONE_3DS_MESSAGES_INCLUDED
#define BSTONE_3DS_MESSAGES_INCLUDED

#if defined(__3DS__) || defined(_3DS)

#include <3ds.h>
#include <string>
#include <vector>

namespace bstone {
namespace n3ds_hud {

struct MessageState
{
    std::vector<std::string> lines;
    int type{};
    u64 started_ms{};
};

MessageState& message_state();

void capture_message(const char* message, int type);

} // namespace n3ds_hud
} // namespace bstone

#endif // 3DS
#endif // BSTONE_3DS_MESSAGES_INCLUDED
