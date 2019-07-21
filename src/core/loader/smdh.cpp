// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include <vector>
#include "common/common_types.h"
#include "core/loader/loader.h"
#include "core/loader/smdh.h"
#include "video_core/utils.h"

namespace Loader {

bool IsValidSMDH(const std::vector<u8>& smdh_data) {
    if (smdh_data.size() < sizeof(Loader::SMDH))
        return false;

    u32 magic;
    memcpy(&magic, smdh_data.data(), sizeof(u32));

    return Loader::MakeMagic('S', 'M', 'D', 'H') == magic;
}

std::vector<u16> SMDH::GetIcon(bool large) const {
    u32 size;
    const u8* icon_data;

    if (large) {
        size = 48;
        icon_data = large_icon.data();
    } else {
        size = 24;
        icon_data = small_icon.data();
    }

    std::vector<u16> icon(size * size);
    for (u32 x = 0; x < size; ++x) {
        for (u32 y = 0; y < size; ++y) {
            u32 coarse_y = y & ~7;
            const u8* pixel = icon_data + VideoCore::GetMortonOffset(x, y, 2) + coarse_y * size * 2;
            icon[x + size * y] = (pixel[1] << 8) + pixel[0];
        }
    }
    return icon;
}

std::array<u16, 0x40> SMDH::GetShortTitle(Loader::SMDH::TitleLanguage language) const {
    return titles[static_cast<int>(language)].short_title;
}

SMDH::GameRegion SMDH::GetRegion() const {
    if (region_lockout == 0x7fffffff) {
        return GameRegion::RegionFree;
    }

    constexpr u32 taiwan_and_china =
        (1 << static_cast<u32>(GameRegion::Taiwan)) | (1 << static_cast<u32>(GameRegion::China));
    if (region_lockout == taiwan_and_china) {
        return GameRegion::Taiwan;
    } // hack to fix TWN games that support CHN consoles

    constexpr u32 REGION_COUNT = 7;
    u32 region = 0;
    for (; region < REGION_COUNT; ++region) {
        if (region_lockout & (1 << region)) {
            return static_cast<GameRegion>(region);
        }
    }

    return GameRegion::Invalid;
}

} // namespace Loader
