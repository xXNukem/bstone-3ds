#include "bstone_3ds_messages.h"

#include <3ds.h>
#include <string>
#include <vector>

namespace bstone {
namespace n3ds_hud {



MessageState& message_state()
{
    static MessageState state;
    return state;
}

void capture_message(const char* message, int type)
{
    auto& state = message_state();

    state.lines.clear();
    state.type = type;
    state.started_ms = osGetTime();

    if (!message)
    {
        return;
    }

    std::string plain;

    // Strip original BStone formatting instructions.
    for (const char* p = message;
         *p && plain.size() < 1024;)
    {
        if (*p == '^' && p[1] && p[2])
        {
            const char a = p[1];
            const char b = p[2];
            int length = 0;

            if ((a == 'F' && b == 'C') ||
                (a == 'A' && b == 'N') ||
                (a == 'B' && b == 'G'))
            {
                length = 5;
            }
            else if ((a == 'L' && b == 'M') ||
                     (a == 'S' && b == 'H') ||
                     (a == 'P' && (b == 'X' || b == 'Y')))
            {
                length = 6;
            }
            else if ((a == 'D' && b == 'M') ||
                     (a == 'E' && b == 'P'))
            {
                length = 3;
            }

            if (length != 0)
            {
                while (length-- > 0 && *p)
                {
                    ++p;
                }

                continue;
            }
        }

        const auto ch = static_cast<unsigned char>(*p++);

        if (ch == '\r' || ch == '\n')
        {
            if (!plain.empty() && plain.back() != '\n')
            {
                plain += '\n';
            }
        }
        else if (ch >= 32 && ch <= 126)
        {
            plain += static_cast<char>(ch);
        }
    }

    // Fit text within the narrow R.E.B.A. panel.
    constexpr std::size_t max_columns = 23;
    std::string line;
    std::size_t pos = 0;

    while (pos < plain.size())
    {
        if (plain[pos] == '\n')
        {
            if (!line.empty())
            {
                state.lines.push_back(line);
                line.clear();
            }

            ++pos;
            continue;
        }

        if (plain[pos] == ' ' || plain[pos] == '\t')
        {
            ++pos;
            continue;
        }

        const auto start = pos;

        while (pos < plain.size() &&
               plain[pos] != '\n' &&
               plain[pos] != ' ' &&
               plain[pos] != '\t')
        {
            ++pos;
        }

        auto word = plain.substr(start, pos - start);

        while (!word.empty())
        {
            const std::size_t space = line.empty() ? 0 : 1;

            if (line.size() + space >= max_columns)
            {
                state.lines.push_back(line);
                line.clear();
                continue;
            }

            const auto room = max_columns - line.size() - space;

            if (word.size() > room && !line.empty())
            {
                state.lines.push_back(line);
                line.clear();
                continue;
            }

            if (!line.empty())
            {
                line += ' ';
            }

            const auto count =
                word.size() < room ? word.size() : room;

            line.append(word, 0, count);
            word.erase(0, count);

            if (!word.empty())
            {
                state.lines.push_back(line);
                line.clear();
            }
        }
    }

    if (!line.empty())
    {
        state.lines.push_back(line);
    }
}

} // namespace n3ds_hud
} // namespace bstone
