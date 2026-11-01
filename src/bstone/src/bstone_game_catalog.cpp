#include "bstone_game_catalog.h"
// Tables moved unchanged from id_ca.cpp; original copyright/license in header.
bool Assets::are_official_levels(
	const std::string& levels_hash)
{
	for (const auto& resource : get_all_resources())
	{
		if (resource.hash_string == levels_hash)
		{
			return true;
		}
	}

	return false;
}

const AssetsResources& Assets::get_aog_sw_v1_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "0c3de403b524107809fa9308b730d60e8e41ba93"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "3d4e8b62aa4683671027513ad9720f76f176ba5b"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "55b9eb9ed555b0f249c2cefd54ecc3f511bfcd55"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "293464e7143ff7e6faf5f5b20799e76a394d65bf"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "b54d48d35d095e27a3c9130cfa59ed9c4f05abe7"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "d1fa19131281d30787125f946881652be876f57a"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "cc44c362e9e2c9f7b9fb2e4ba31331dde42a1e96"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "37412ca9139562fc31330d72470822de4d9ceb4a"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v2_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "0c3de403b524107809fa9308b730d60e8e41ba93"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "3d4e8b62aa4683671027513ad9720f76f176ba5b"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "058fe0b59c7aa020bf4e7509103892d4c7459aa6"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "e14b6172b6ab7568d3736f087f07a0df5eac5dad"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "6c0e273df7fd3940c038fc20a5bdfb81cd50e7ef"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v2_1_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "ed1ab61ad2529e046f966d9c1627508f76693ef8"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "b59cbe3793b4612b06254ece48bf8e961ab6f528"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "0635a4a2b823eef2904ed1d590d98362fb16621d"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v3_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "537676ddcafeee415c22bb9d00097b25bc7e13c5"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "9f9f4c40c9637af472ce5e1a360e51364c3a418a"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "e7fb8f241b8fa94739c3bd09ea05c1afc7bbfc95"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v1_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "6f19a144d8985e90f096fc1c67ade58e9051235c"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "4d00c5f5c843f99a266bd938648192a6eff17b5a"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "d4f81ace1701a7338d43ce07723c2adaafdc837c"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "02dc27d4810e3ffa26540b310eac6091f5e16de0"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "639ec9e7a81ad83fc5b5c557cf4fc5fa28b9676b"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "0a700732ccbc72f95318a6226a7e1ad78ac713bb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v2_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "e83b690836c9edf9ef60f6189b8384fb2319b735"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "67a679e3b107db8685ba5ff1643a38f9291b00bf"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "48b18caa86151610957b64b207cf2a2977ef7d57"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v2_1_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "e4ae3ef9a3ac158a832092b7b5487227337c6f13"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "6868e6cc4f8cb9160a218c5bce680a11f64c675a"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "1a3864cd12de4b8fd6be023b73d2d79e91b9018d"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v3_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "60da35e506d57753f83cca5f232a76dd1cf074ba"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "001037fafdff124befb5437c563d0b9c613b3c00"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "ff165ca5924d738853587a7e4ab1cd239e10e359"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_ps_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_vsi, "2ed9a587926b2b455f8176788f595d03f6359171"},
		AssetsResource{AssetsResourceType::audiot, audiot_vsi, "ecac8a800c5d021c8b68d170066c8859df9bd79a"},
		AssetsResource{AssetsResourceType::eanim, eanim_vsi, "b668330e56ceb069c1b972cefddd8a3d618a14af"},
		AssetsResource{AssetsResourceType::ianim, ianim_vsi, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_vsi, "c3fb0a9b81e0240d12c952fe5b57b78c1108aa48"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_vsi, "acd03031d526eeaee64072518adf6814f96e2a37"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_vsi, "061692d166f68d7e0b81568725439078cc728f33"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_vsi, "e619a3e6245f79888da5436df138d63204d2caba"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_vsi, "ad5fed9de5d1d82145df3ff2286e4bc62cb80e2a"},
		AssetsResource{AssetsResourceType::vswap, vswap_vsi, "4113ce83c42e69dc5cb20de79f41971a100f876e"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_all_resources()
{
	static const auto all_resources = make_all_resources();

	static const auto all_assets_resources = AssetsResources{
		all_resources.data(),
		static_cast<std::intptr_t>(all_resources.size())
	};

	return all_assets_resources;
}

Assets::AllResources Assets::make_all_resources()
{
	auto all_resources = AllResources{};
	all_resources.reserve(40);

	all_resources.insert(all_resources.end(), get_aog_sw_v1_0_resources().begin(), get_aog_sw_v1_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_sw_v2_0_resources().begin(), get_aog_sw_v2_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_sw_v2_1_resources().begin(), get_aog_sw_v2_1_resources().end());
	all_resources.insert(all_resources.end(), get_aog_sw_v3_0_resources().begin(), get_aog_sw_v3_0_resources().end());

	all_resources.insert(all_resources.end(), get_aog_full_v1_0_resources().begin(), get_aog_full_v1_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_full_v2_0_resources().begin(), get_aog_full_v2_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_full_v2_1_resources().begin(), get_aog_full_v2_1_resources().end());
	all_resources.insert(all_resources.end(), get_aog_full_v3_0_resources().begin(), get_aog_full_v3_0_resources().end());

	all_resources.insert(all_resources.end(), get_ps_resources().begin(), get_ps_resources().end());

	return all_resources;
}


const AssetsResources& resources_for_version(AssetsVersion version)
{
    switch(version)
    {
    case AssetsVersion::aog_sw_v1_0: return Assets::get_aog_sw_v1_0_resources();
    case AssetsVersion::aog_sw_v2_0: return Assets::get_aog_sw_v2_0_resources();
    case AssetsVersion::aog_sw_v2_1: return Assets::get_aog_sw_v2_1_resources();
    case AssetsVersion::aog_sw_v3_0: return Assets::get_aog_sw_v3_0_resources();
    case AssetsVersion::aog_full_v1_0: return Assets::get_aog_full_v1_0_resources();
    case AssetsVersion::aog_full_v2_0: return Assets::get_aog_full_v2_0_resources();
    case AssetsVersion::aog_full_v2_1: return Assets::get_aog_full_v2_1_resources();
    case AssetsVersion::aog_full_v3_0: return Assets::get_aog_full_v3_0_resources();
    case AssetsVersion::ps: return Assets::get_ps_resources();
    default: { static const AssetsResources empty{}; return empty; }
    }
}
const char* assets_version_label(AssetsVersion version) noexcept
{
    switch(version)
    {
    case AssetsVersion::aog_sw_v1_0: return "1.0";
    case AssetsVersion::aog_sw_v2_0: return "2.0";
    case AssetsVersion::aog_sw_v2_1: return "2.1";
    case AssetsVersion::aog_sw_v3_0: return "3.0";
    case AssetsVersion::aog_full_v1_0: return "1.0";
    case AssetsVersion::aog_full_v2_0: return "2.0";
    case AssetsVersion::aog_full_v2_1: return "2.1";
    case AssetsVersion::aog_full_v3_0: return "3.0";
    case AssetsVersion::ps: return "PS 1.0/1.1 (shared signatures)";
    default: return "UNKNOWN";
    }
}
