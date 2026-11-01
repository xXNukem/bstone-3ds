// Original BStone resource identities and SHA-1 signatures.
// Copyright (c) 1992-2013 Apogee Entertainment, LLC
// Copyright (c) 2013-2024 Boris I. Bendovsky and Contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef BSTONE_GAME_CATALOG_INCLUDED
#define BSTONE_GAME_CATALOG_INCLUDED
#include <string>
#include <vector>
#include "bstone_span.h"
enum class AssetsResourceType
{
	none,
	audiohed,
	audiot,
	eanim,
	ganim,
	ianim,
	maphead,
	maptemp,
	sanim,
	vgadict,
	vgagraph,
	vgahead,
	vswap,
}; // AssetsResourceType

struct AssetsResource
{
	AssetsResourceType type{};
	const char* file_name{};
	const char* hash_string{};
}; // AssetsResource

using AssetsResources = bstone::Span<const AssetsResource>;

enum class AssetsVersion
{
	none,
	aog_sw_v1_0, // Aliens Of Gold (shareware, v1.0)
	aog_sw_v2_0, // Aliens Of Gold (shareware, v2.0)
	aog_sw_v2_1, // Aliens Of Gold (shareware, v2.1)
	aog_sw_v3_0, // Aliens Of Gold (shareware, v3.0)
	aog_full_v1_0, // Aliens Of Gold (full, v1.0)
	aog_full_v2_0, // Aliens Of Gold (full, v2.0)
	aog_full_v2_1, // Aliens Of Gold (full, v2.1)
	aog_full_v3_0, // Aliens Of Gold (full, v3.0)
	ps, // Planet Strike (full, v1.0/v1.1)
}; // AssetsVersion

class Assets
{
public:
	static constexpr auto max_size = 4000000;


	static bool are_official_levels(
		const std::string& levels_hash);


	static const AssetsResources& get_aog_sw_v1_0_resources();

	static const AssetsResources& get_aog_sw_v2_0_resources();

	static const AssetsResources& get_aog_sw_v2_1_resources();

	static const AssetsResources& get_aog_sw_v3_0_resources();


	static const AssetsResources& get_aog_full_v1_0_resources();

	static const AssetsResources& get_aog_full_v2_0_resources();

	static const AssetsResources& get_aog_full_v2_1_resources();

	static const AssetsResources& get_aog_full_v3_0_resources();


	static const AssetsResources& get_ps_resources();


	static const AssetsResources& get_all_resources();


private:
	// Aliens Of Gold (shareware)

	static constexpr const char* audiohed_bs1 = "AUDIOHED.BS1";
	static constexpr const char* audiot_bs1 = "AUDIOT.BS1";
	static constexpr const char* ianim_bs1 = "IANIM.BS1";
	static constexpr const char* maphead_bs1 = "MAPHEAD.BS1";
	static constexpr const char* maptemp_bs1 = "MAPTEMP.BS1";
	static constexpr const char* sanim_bs1 = "SANIM.BS1";
	static constexpr const char* vgadict_bs1 = "VGADICT.BS1";
	static constexpr const char* vgagraph_bs1 = "VGAGRAPH.BS1";
	static constexpr const char* vgahead_bs1 = "VGAHEAD.BS1";
	static constexpr const char* vswap_bs1 = "VSWAP.BS1";

	// Aliens Of Gold (registered)

	static constexpr const char* audiohed_bs6 = "AUDIOHED.BS6";
	static constexpr const char* audiot_bs6 = "AUDIOT.BS6";
	static constexpr const char* eanim_bs6 = "EANIM.BS6";
	static constexpr const char* ganim_bs6 = "GANIM.BS6";
	static constexpr const char* ianim_bs6 = "IANIM.BS6";
	static constexpr const char* maphead_bs6 = "MAPHEAD.BS6";
	static constexpr const char* maptemp_bs6 = "MAPTEMP.BS6";
	static constexpr const char* sanim_bs6 = "SANIM.BS6";
	static constexpr const char* vgadict_bs6 = "VGADICT.BS6";
	static constexpr const char* vgagraph_bs6 = "VGAGRAPH.BS6";
	static constexpr const char* vgahead_bs6 = "VGAHEAD.BS6";
	static constexpr const char* vswap_bs6 = "VSWAP.BS6";

	// Planet Strike

	static constexpr const char* audiohed_vsi = "AUDIOHED.VSI";
	static constexpr const char* audiot_vsi = "AUDIOT.VSI";
	static constexpr const char* eanim_vsi = "EANIM.VSI";
	static constexpr const char* ianim_vsi = "IANIM.VSI";
	static constexpr const char* maphead_vsi = "MAPHEAD.VSI";
	static constexpr const char* maptemp_vsi = "MAPTEMP.VSI";
	static constexpr const char* vgadict_vsi = "VGADICT.VSI";
	static constexpr const char* vgagraph_vsi = "VGAGRAPH.VSI";
	static constexpr const char* vgahead_vsi = "VGAHEAD.VSI";
	static constexpr const char* vswap_vsi = "VSWAP.VSI";


	using AllResources = std::vector<AssetsResource>;

	static AllResources make_all_resources();
}; // Assets


const AssetsResources& resources_for_version(AssetsVersion version);
const char* assets_version_label(AssetsVersion version) noexcept;
#endif
