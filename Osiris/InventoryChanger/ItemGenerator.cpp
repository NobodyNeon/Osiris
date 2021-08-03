#include <algorithm>
#include <cmath>
#include <random>
#include <span>
#include <vector>

#include "../Helpers.h"
#include "Inventory.h"
#include "ItemGenerator.h"

#include "../SDK/ItemSchema.h"

static float generateWear() noexcept
{
    float wear;
    if (const auto condition = Helpers::random(1, 10000); condition <= 1471)
        wear = Helpers::random(0.0f, 0.07f);
    else if (condition <= 3939)
        wear = Helpers::random(0.07f, 0.15f);
    else if (condition <= 8257)
        wear = Helpers::random(0.15f, 0.38f);
    else if (condition <= 9049)
        wear = Helpers::random(0.38f, 0.45f);
    else
        wear = Helpers::random(0.45f, 1.0f);
    return wear;
}

using StaticData::TournamentMap;

[[nodiscard]] static std::array<StickerConfig, 5> generateSouvenirStickers(std::uint32_t tournamentID, TournamentMap map, TournamentStage stage, TournamentTeam team1, TournamentTeam team2, ProPlayer player) noexcept;

std::pair<std::size_t, std::size_t> ItemGenerator::generateItemFromContainer(const InventoryItem& caseItem) noexcept
{
    assert(caseItem.isCase());

    const auto& caseData = StaticData::cases()[caseItem.get().dataIndex];
    assert(caseData.hasLoot());

    const auto unlockedItemIdx = StaticData::caseLoot()[Helpers::random(static_cast<int>(caseData.lootBeginIdx), static_cast<int>(caseData.lootEndIdx - 1))];
    std::size_t dynamicDataIdx = Inventory::INVALID_DYNAMIC_DATA_IDX;

    if (const auto& item = StaticData::gameItems()[unlockedItemIdx]; caseData.willProduceStatTrak && item.isMusic()) {
        DynamicMusicData dynamicData;
        dynamicData.statTrak = 0;
        dynamicDataIdx = Inventory::emplaceDynamicData(std::move(dynamicData));
    } else if (item.isSkin()) {
        DynamicSkinData dynamicData;
        const auto& staticData = StaticData::paintKits()[item.dataIndex];
        dynamicData.wear = std::lerp(staticData.wearRemapMin, staticData.wearRemapMax, generateWear());
        dynamicData.seed = Helpers::random(1, 1000);

        if (caseData.isSouvenirPackage()) {
            dynamicData.tournamentID = caseData.souvenirPackageTournamentID;
            const auto& souvenir = Inventory::dynamicSouvenirPackageData(caseItem.getDynamicDataIndex());
            dynamicData.tournamentStage = souvenir.tournamentStage;
            dynamicData.tournamentTeam1 = souvenir.tournamentTeam1;
            dynamicData.tournamentTeam2 = souvenir.tournamentTeam2;
            dynamicData.proPlayer = souvenir.proPlayer;
            dynamicData.stickers = generateSouvenirStickers(caseData.souvenirPackageTournamentID, caseData.tournamentMap, dynamicData.tournamentStage, dynamicData.tournamentTeam1, dynamicData.tournamentTeam2, dynamicData.proPlayer);
        } else if (Helpers::random(0, 9) == 0) {
            dynamicData.statTrak = 0;
        }

        dynamicDataIdx = Inventory::emplaceDynamicData(std::move(dynamicData));
    }

    return std::make_pair(unlockedItemIdx, dynamicDataIdx);
}

struct Match {
    TournamentMap map;
    TournamentStage stage;
    TournamentTeam team1;
    TournamentTeam team2;
    std::array<ProPlayer, 10> mvpPlayers;

    [[nodiscard]] bool hasMVPs() const noexcept { return std::ranges::find(mvpPlayers, ProPlayer{}) != mvpPlayers.begin(); }
    [[nodiscard]] ProPlayer getRandomMVP() const noexcept
    {
        if (!hasMVPs())
            return ProPlayer{};
        return mvpPlayers[Helpers::random(0, std::distance(mvpPlayers.begin(), std::ranges::find(mvpPlayers, ProPlayer{})) - 1)];
    }
};

struct Tournament {
    std::uint32_t tournamentID;
    std::span<const Match> matches;
};

constexpr auto dreamHack2013Matches = std::to_array<Match>({
    // Group A
    { TournamentMap::Mirage, GroupStage, NatusVincere, Fnatic, {} },
    { TournamentMap::Dust2, GroupStage, ClanMystik, LGBEsports, {} },
    { TournamentMap::Mirage, GroupStage, LGBEsports, NatusVincere, {} },
    { TournamentMap::Inferno, GroupStage, ClanMystik, Fnatic, {} },
    { TournamentMap::Inferno, GroupStage, ClanMystik,LGBEsports, {} },

    // Group B
    { TournamentMap::Inferno, GroupStage, NinjasInPyjamas, RecursiveEsports, {} },
    { TournamentMap::Dust2, GroupStage, UniversalSoldiers, IBUYPOWER, {} },
    { TournamentMap::Inferno, GroupStage, RecursiveEsports, IBUYPOWER, {} },
    { TournamentMap::Inferno, GroupStage, NinjasInPyjamas, UniversalSoldiers, {} },
    { TournamentMap::Inferno, GroupStage, RecursiveEsports, UniversalSoldiers, {} },

    // Group C
    { TournamentMap::Inferno, GroupStage, VeryGames, Xapso, {} },
    { TournamentMap::Inferno, GroupStage, NFaculty, ComplexityGaming, {} },
    { TournamentMap::Train, GroupStage, NFaculty, Xapso, {} },
    { TournamentMap::Inferno, GroupStage, ComplexityGaming, VeryGames, {} },
    { TournamentMap::Dust2, GroupStage, NFaculty, VeryGames, {} },

    // Group D
    { TournamentMap::Inferno, GroupStage, AstanaDragons, _ReasonGaming, {} },
    { TournamentMap::Inferno, GroupStage, CopenhagenWolves, SKGaming, {} },
    { TournamentMap::Inferno, GroupStage, SKGaming, _ReasonGaming, {} },
    { TournamentMap::Nuke, GroupStage, AstanaDragons, CopenhagenWolves, {} },
    { TournamentMap::Nuke, GroupStage, AstanaDragons, _ReasonGaming, {} },

    // Quarterfinals
    { TournamentMap::Dust2, Quarterfinal, LGBEsports, NinjasInPyjamas, {} },
    { TournamentMap::Train, Quarterfinal, LGBEsports, NinjasInPyjamas, {} },
    { TournamentMap::Mirage, Quarterfinal, LGBEsports, NinjasInPyjamas, {} },

    { TournamentMap::Dust2, Quarterfinal, CopenhagenWolves, VeryGames, {} },
    { TournamentMap::Inferno, Quarterfinal, CopenhagenWolves, VeryGames, {} },
    { TournamentMap::Mirage, Quarterfinal, CopenhagenWolves, VeryGames, {} },

    { TournamentMap::Inferno, Quarterfinal, Fnatic, RecursiveEsports, {} },
    { TournamentMap::Dust2, Quarterfinal, Fnatic, RecursiveEsports, {} },
    { TournamentMap::Train, Quarterfinal, Fnatic, RecursiveEsports, {} },

    { TournamentMap::Nuke, Quarterfinal, AstanaDragons, ComplexityGaming, {} },
    { TournamentMap::Dust2, Quarterfinal, AstanaDragons, ComplexityGaming, {} },
    { TournamentMap::Inferno, Quarterfinal, AstanaDragons, ComplexityGaming, {} },

    // Semifinals
    { TournamentMap::Dust2, Semifinal, NinjasInPyjamas, VeryGames, {} },
    { TournamentMap::Inferno, Semifinal, NinjasInPyjamas, VeryGames, {} },
    { TournamentMap::Nuke, Semifinal, NinjasInPyjamas, VeryGames, {} },

    { TournamentMap::Train, Semifinal, Fnatic, ComplexityGaming, {} },
    { TournamentMap::Mirage, Semifinal, Fnatic, ComplexityGaming, {} },

    // Grand Final
    { TournamentMap::Dust2, GrandFinal, NinjasInPyjamas, Fnatic, {} },
    { TournamentMap::Inferno, GrandFinal, NinjasInPyjamas, Fnatic, {} },
    { TournamentMap::Train, GrandFinal, NinjasInPyjamas, Fnatic, {} },
});

constexpr auto emsOneKatowice2014Matches = std::to_array<Match>({
    // Group A
    { TournamentMap::Dust2, GroupStage, Titan, Mousesports, {} },
    { TournamentMap::Mirage, GroupStage, VirtusPro, HellRaisers, {} },
    { TournamentMap::Mirage, GroupStage, Titan, VirtusPro, {} },
    { TournamentMap::Dust2, GroupStage, Mousesports, HellRaisers, {} },
    { TournamentMap::Inferno, GroupStage, Titan, HellRaisers, {} },

    // Group B
    { TournamentMap::Dust2, GroupStage, NinjasInPyjamas, _3DMax, {} },
    { TournamentMap::Inferno, GroupStage, TeamLDLC, VoxEminor, {} },
    { TournamentMap::Inferno, GroupStage, NinjasInPyjamas, TeamLDLC, {} },
    { TournamentMap::Inferno, GroupStage, _3DMax, VoxEminor, {} },
    { TournamentMap::Inferno, GroupStage, TeamLDLC, _3DMax, {} },

    // Group C
    { TournamentMap::Inferno, GroupStage, Fnatic, ReasonGaming, {} },
    { TournamentMap::Nuke, GroupStage, TeamDignitas, IBUYPOWER, {} },
    { TournamentMap::Inferno, GroupStage, ReasonGaming, TeamDignitas, {} },
    { TournamentMap::Inferno, GroupStage, Fnatic, IBUYPOWER, {} },
    { TournamentMap::Inferno, GroupStage, Fnatic, ReasonGaming, {} },

    // Group D
    { TournamentMap::Inferno, GroupStage, ComplexityGaming, ClanMystik, {} },
    { TournamentMap::Inferno, GroupStage, LGBEsports, NatusVincere, {} },
    { TournamentMap::Dust2, GroupStage, LGBEsports, ComplexityGaming, {} },
    { TournamentMap::Dust2, GroupStage, NatusVincere, ClanMystik, {} },
    { TournamentMap::Nuke, GroupStage, ComplexityGaming, ClanMystik, {} },

    // Quarterfinals
    { TournamentMap::Dust2, Quarterfinal, ComplexityGaming, NinjasInPyjamas, {} },
    { TournamentMap::Nuke, Quarterfinal, ComplexityGaming, NinjasInPyjamas, {} },
    { TournamentMap::Train, Quarterfinal, ComplexityGaming, NinjasInPyjamas, {} },

    { TournamentMap::Dust2, Quarterfinal, TeamDignitas, HellRaisers, {} },
    { TournamentMap::Mirage, Quarterfinal, TeamDignitas, HellRaisers, {} },

    { TournamentMap::Mirage, Quarterfinal, VirtusPro, TeamLDLC, {} },
    { TournamentMap::Inferno, Quarterfinal, VirtusPro, TeamLDLC, {} },

    { TournamentMap::Inferno, Quarterfinal, Fnatic, LGBEsports, {} },
    { TournamentMap::Mirage, Quarterfinal, Fnatic, LGBEsports, {} },
    { TournamentMap::Train, Quarterfinal, Fnatic, LGBEsports, {} },

    // Semifinals
    { TournamentMap::Inferno, Semifinal, TeamDignitas, NinjasInPyjamas, {} },
    { TournamentMap::Nuke, Semifinal, TeamDignitas, NinjasInPyjamas, {} },

    { TournamentMap::Inferno, Semifinal, LGBEsports, VirtusPro, {} },
    { TournamentMap::Mirage, Semifinal, LGBEsports, VirtusPro, {} },
    { TournamentMap::Train, Semifinal, LGBEsports, VirtusPro, {} },

    // Grand Final
    { TournamentMap::Mirage, GrandFinal, NinjasInPyjamas, VirtusPro, {} },
    { TournamentMap::Inferno, GrandFinal, NinjasInPyjamas, VirtusPro, {} },
});

// Starting with ESL One Cologne 2014 souvenir packages are bound to a certain map. Matches of those tournaments must be sorted by map.

constexpr auto eslOneCologne2014Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, CopenhagenWolves, LondonConspiracy, {} },
    { TournamentMap::Cache, GroupStage, VirtusPro, IBUYPOWER, {} },
    { TournamentMap::Cache, GrandFinal, Fnatic, NinjasInPyjamas, {} },
    { TournamentMap::Cobblestone, GroupStage, NinjasInPyjamas, EpsilonEsports, {} },
    { TournamentMap::Cobblestone, GroupStage, IBUYPOWER, Fnatic, {} },
    { TournamentMap::Cobblestone, GroupStage, IBUYPOWER, DATTeam, {} },
    { TournamentMap::Cobblestone, Quarterfinal, Cloud9, NinjasInPyjamas, {} },
    { TournamentMap::Cobblestone, Semifinal, TeamLDLC, NinjasInPyjamas, {} },
    { TournamentMap::Cobblestone, GrandFinal, Fnatic, NinjasInPyjamas, {} },
    { TournamentMap::Dust2, GroupStage, TeamWolf, NinjasInPyjamas, {} },
    { TournamentMap::Dust2, GroupStage, NatusVincere, CopenhagenWolves, {} },
    { TournamentMap::Dust2, GroupStage, TeamDignitas, VoxEminor, {} },
    { TournamentMap::Dust2, GroupStage, Cloud9, Titan, {} },
    { TournamentMap::Dust2, Quarterfinal, NatusVincere, Fnatic, {} },
    { TournamentMap::Dust2, Quarterfinal, EpsilonEsports, TeamDignitas, {} },
    { TournamentMap::Dust2, Quarterfinal, VirtusPro, TeamLDLC, {} },
    { TournamentMap::Dust2, Quarterfinal, Cloud9, NinjasInPyjamas, {} },
    { TournamentMap::Dust2, Semifinal,  TeamDignitas, Fnatic, {} },
    { TournamentMap::Inferno, GroupStage, EpsilonEsports, HellRaisers, {} },
    { TournamentMap::Inferno, GroupStage, NatusVincere, CopenhagenWolves, {} },
    { TournamentMap::Inferno, GroupStage, TeamLDLC, NatusVincere, {} },
    { TournamentMap::Inferno, Quarterfinal, NatusVincere, Fnatic, {} },
    { TournamentMap::Inferno, Quarterfinal, EpsilonEsports, TeamDignitas, {} },
    { TournamentMap::Inferno, Semifinal, TeamLDLC, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, GrandFinal, Fnatic, NinjasInPyjamas, {} },
    { TournamentMap::Mirage, GroupStage, HellRaisers, TeamWolf, {} },
    { TournamentMap::Mirage, GroupStage, TeamDignitas, Cloud9, {} },
    { TournamentMap::Mirage, Quarterfinal, VirtusPro, TeamLDLC, {} },
    { TournamentMap::Nuke, GroupStage, TeamLDLC, LondonConspiracy, {} },
    { TournamentMap::Nuke, GroupStage, Titan, VoxEminor, {} },
    { TournamentMap::Nuke, GroupStage, Titan, TeamDignitas, {} },
    { TournamentMap::Nuke, Semifinal, TeamLDLC, NinjasInPyjamas, {} },
    { TournamentMap::Nuke, Quarterfinal, Cloud9, NinjasInPyjamas, {} },
    { TournamentMap::Nuke, Quarterfinal, NatusVincere, Fnatic, {} },
    { TournamentMap::Overpass, GroupStage, NinjasInPyjamas, HellRaisers, {} },
    { TournamentMap::Overpass, GroupStage, VirtusPro, DATTeam, {} },
    { TournamentMap::Overpass, GroupStage, VirtusPro, Fnatic, {} },
    { TournamentMap::Overpass, Semifinal, TeamDignitas, Fnatic, {} },
});
static_assert(std::ranges::is_sorted(eslOneCologne2014Matches, {}, &Match::map));

constexpr auto dreamHack2014Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, BravadoGaming, Cloud9, {} },
    { TournamentMap::Cache, GroupStage, NinjasInPyjamas, ESCGaming, {} },
    { TournamentMap::Cache, Quarterfinal, VirtusPro, PENTASports, {} },
    { TournamentMap::Cache, Quarterfinal, TeamLDLC, Fnatic, {} },
    { TournamentMap::Cache, Semifinal, NinjasInPyjamas, VirtusPro, {} },
    { TournamentMap::Cobblestone, GroupStage, TeamLDLC, ESCGaming, {} },
    { TournamentMap::Cobblestone, Quarterfinal, NatusVincere, TeamDignitas, {} },
    { TournamentMap::Dust2, GroupStage, Fnatic, Cloud9, {} },
    { TournamentMap::Dust2, GroupStage, PENTASports, CopenhagenWolves, {} },
    { TournamentMap::Dust2, Quarterfinal, HellRaisers, NinjasInPyjamas, {} },
    { TournamentMap::Dust2, Quarterfinal, TeamLDLC, Fnatic, {} },
    { TournamentMap::Dust2, Semifinal, NatusVincere, TeamLDLC, {} },
    { TournamentMap::Dust2, GrandFinal, TeamLDLC, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, GroupStage, Cloud9, HellRaisers, {} },
    { TournamentMap::Inferno, GroupStage, IBUYPOWER, PENTASports, {} },
    { TournamentMap::Inferno, GroupStage, VirtusPro, MyXMG, {} },
    { TournamentMap::Inferno, Quarterfinal, HellRaisers, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, Semifinal, NinjasInPyjamas, VirtusPro, {} },
    { TournamentMap::Inferno, Semifinal, NatusVincere, TeamLDLC, {} },
    { TournamentMap::Inferno, GrandFinal, TeamLDLC, NinjasInPyjamas, {} },
    { TournamentMap::Mirage, GroupStage, Fnatic, BravadoGaming, {} },
    { TournamentMap::Mirage, GroupStage, Fnatic, HellRaisers, {} },
    { TournamentMap::Mirage, GroupStage, ESCGaming, PlanetkeyDynamics, {} },
    { TournamentMap::Mirage, GroupStage, NatusVincere, Flipsid3Tactics, {} },
    { TournamentMap::Mirage, GroupStage, MyXMG, Flipsid3Tactics, {} },
    { TournamentMap::Mirage, Quarterfinal, VirtusPro, PENTASports, {} },
    { TournamentMap::Mirage, Quarterfinal, NatusVincere, TeamDignitas, {} },
    { TournamentMap::Nuke, GroupStage, TeamDignitas, PENTASports, {} },
    { TournamentMap::Nuke, GroupStage, IBUYPOWER, CopenhagenWolves, {} },
    { TournamentMap::Nuke, GroupStage, TeamDignitas, IBUYPOWER, {} },
    { TournamentMap::Nuke, GroupStage, VirtusPro, NatusVincere, {} },
    { TournamentMap::Nuke, Semifinal, NinjasInPyjamas, VirtusPro, {} },
    { TournamentMap::Overpass, GroupStage, NinjasInPyjamas, PlanetkeyDynamics, {} },
    { TournamentMap::Overpass, GroupStage, NinjasInPyjamas, TeamLDLC, {} },
    { TournamentMap::Overpass, GroupStage, Flipsid3Tactics, NatusVincere, {} },
    { TournamentMap::Overpass, Quarterfinal, TeamLDLC, Fnatic, {} },
    { TournamentMap::Overpass, GrandFinal, TeamLDLC, NinjasInPyjamas, {} },
});
static_assert(std::ranges::is_sorted(dreamHack2014Matches, {}, &Match::map));

constexpr auto eslOneKatowice2015Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, TeamEnVyUs, LGBEsports, {} },
    { TournamentMap::Cache, GroupStage, PENTASports, Titan, {} },
    { TournamentMap::Cache, GroupStage, VoxEminor, Flipsid3Tactics, {} },
    { TournamentMap::Cache, Quarterfinal, Fnatic, PENTASports, {} },
    { TournamentMap::Cache, Quarterfinal, TeamEnVyUs, NatusVincere, {} },
    { TournamentMap::Cache, Semifinal, TeamEnVyUs, NinjasInPyjamas, {} },
    { TournamentMap::Cache, GrandFinal, Fnatic, NinjasInPyjamas, {} },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, Fnatic, {} },
    { TournamentMap::Cobblestone, GroupStage, TeamEnVyUs, Titan, {} },
    { TournamentMap::Cobblestone, Quarterfinal, TeamEnVyUs, NatusVincere, {} },
    { TournamentMap::Cobblestone, Semifinal, VirtusPro, Fnatic, {} },
    { TournamentMap::Dust2, GroupStage, PENTASports, LGBEsports, {} },
    { TournamentMap::Dust2, GroupStage, PENTASports, LGBEsports, {} },
    { TournamentMap::Dust2, GroupStage, CounterLogicGaming, KeydStars, {} },
    { TournamentMap::Dust2, Quarterfinal, TeamEnVyUs, NatusVincere, {} },
    { TournamentMap::Dust2, Quarterfinal, TeamSoloMidKinguin, NinjasInPyjamas, {} },
    { TournamentMap::Dust2, Semifinal, TeamEnVyUs, NinjasInPyjamas, {} },
    { TournamentMap::Dust2, GrandFinal, Fnatic, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, GroupStage, Fnatic, VoxEminor, {} },
    { TournamentMap::Inferno, GroupStage, VoxEminor, NatusVincere, {} },
    { TournamentMap::Inferno, GroupStage, KeydStars, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, GroupStage, HellRaisers, KeydStars, {} },
    { TournamentMap::Inferno, GroupStage, VirtusPro, Cloud9G2A, {} },
    { TournamentMap::Inferno, Quarterfinal, Fnatic, PENTASports, {} },
    { TournamentMap::Inferno, Quarterfinal, TeamSoloMidKinguin, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, GrandFinal, Fnatic, NinjasInPyjamas, {} },
    { TournamentMap::Mirage, GroupStage, NatusVincere, Flipsid3Tactics, {} },
    { TournamentMap::Mirage, GroupStage, CounterLogicGaming, NinjasInPyjamas, {} },
    { TournamentMap::Mirage, Quarterfinal, KeydStars, VirtusPro, {} },
    { TournamentMap::Mirage, Semifinal, VirtusPro, Fnatic, {} },
    { TournamentMap::Nuke, GroupStage, CounterLogicGaming, HellRaisers, {} },
    { TournamentMap::Nuke, GroupStage, Cloud9G2A, TeamSoloMidKinguin, {} },
    { TournamentMap::Nuke, GroupStage, TeamSoloMidKinguin, _3DMax, {} },
    { TournamentMap::Nuke, Quarterfinal, KeydStars, VirtusPro, {} },
    { TournamentMap::Nuke, Quarterfinal, TeamSoloMidKinguin, NinjasInPyjamas, {} },
    { TournamentMap::Overpass, GroupStage, VirtusPro, _3DMax, {} },
    { TournamentMap::Overpass, GroupStage, TeamSoloMidKinguin, Cloud9G2A, {} },
    { TournamentMap::Overpass, Quarterfinal, KeydStars, VirtusPro, {} },
});
static_assert(std::ranges::is_sorted(eslOneKatowice2015Matches, {}, &Match::map));

constexpr auto eslOneCologne2015Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, NinjasInPyjamas, TeamSoloMid, { f0rest, GeT_RiGhT, Xyp9x, karrigan, device, dupreeh } },
    { TournamentMap::Cache, Semifinal, TeamSoloMid, TeamEnVyUs, { device, cajunb, Xyp9x, dupreeh, karrigan, NBK, kioShiMa, Happy, kennyS, apEX } },
    { TournamentMap::Cobblestone, GroupStage, TeamEnVyUs, LuminosityGaming, { kennyS, Happy, apEX, NBK, kioShiMa, FalleN, coldzera, fer, steel, boltz } },
    { TournamentMap::Cobblestone, GroupStage, Mousesports, Flipsid3Tactics, { nex, denis, chrisJ, gobb, Spiidi, markeloff, bondik, B1ad3, DavCost, WorldEdit } },
    { TournamentMap::Cobblestone, GroupStage, LuminosityGaming, Flipsid3Tactics, { FalleN, coldzera, boltz, fer, steel, B1ad3, markeloff, DavCost, bondik, WorldEdit } },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, CounterLogicGaming, { flamie, Zeus, Edward, seized, GuardiaN, jdm64, tarik, FNS, reltuC, hazed } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, TeamImmunity, { pashaBiceps, TaZ, byali, Snax, NEO, USTILO, emagine, James } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, Cloud9G2A, { TaZ, byali, Snax, NEO, pashaBiceps, shroud, Skadoodle, n0thing, freakazoid } },
    { TournamentMap::Cobblestone, Semifinal, Fnatic, VirtusPro, { flusha, pronax, JW,  KRIMZ, olofmeister, pashaBiceps, TaZ, byali, NEO } },
    { TournamentMap::Cobblestone, GrandFinal, Fnatic, TeamEnVyUs, { KRIMZ, flusha, JW, olofmeister, pronax, kioShiMa, Happy, apEX } },
    { TournamentMap::Dust2, GroupStage, NinjasInPyjamas, CounterLogicGaming, { f0rest, allu, friberg, Xizt, GeT_RiGhT, tarik, reltuC, FNS, jdm64, hazed } },
    { TournamentMap::Dust2, GroupStage, TeamImmunity, TeamKinguin, { Rickeh, USTILO, emagine, James, SnypeR, ScreaM, dennis, rain, fox, Maikelele } },
    { TournamentMap::Dust2, GroupStage, TeamKinguin, Cloud9G2A, { ScreaM, rain, Maikelele, dennis, fox, Skadoodle, freakazoid, shroud, seangares, n0thing } },
    { TournamentMap::Dust2, Quarterfinal, TeamSoloMid, TeamKinguin, { karrigan, dupreeh, Xyp9x, device, cajunb, dennis, Maikelele, ScreaM, rain } },
    { TournamentMap::Dust2, Semifinal, TeamSoloMid, TeamEnVyUs, { device, cajunb, Xyp9x, dupreeh, karrigan, NBK, kioShiMa, Happy, kennyS } },
    { TournamentMap::Dust2, GrandFinal, Fnatic, TeamEnVyUs, { KRIMZ, flusha, JW, olofmeister, pronax, kioShiMa, kennyS, Happy, NBK, apEX } },
    { TournamentMap::Inferno, GroupStage, NinjasInPyjamas, Renegades, { allu, f0rest, GeT_RiGhT, Xizt, jks, SPUNJ } },
    { TournamentMap::Inferno, GroupStage, TeamEnVyUs, Flipsid3Tactics, { apEX, kennyS, kioShiMa, Happy, NBK, bondik, DavCost, WorldEdit }},
    { TournamentMap::Inferno, GroupStage, Fnatic, NatusVincere, { olofmeister, flusha, JW, KRIMZ, pronax, flamie, seized } },
    { TournamentMap::Inferno, Quarterfinal, NatusVincere, TeamEnVyUs, { Zeus, seized, flamie, GuardiaN, Edward, apEX, kennyS, NBK, Happy, kioShiMa } },
    { TournamentMap::Inferno, Quarterfinal, NinjasInPyjamas, VirtusPro, { allu, GeT_RiGhT, Xizt, friberg, Snax, NEO, byali, pashaBiceps } },
    { TournamentMap::Inferno, Semifinal, TeamSoloMid, TeamEnVyUs, { device, cajunb, dupreeh, karrigan, NBK, kioShiMa, kennyS, apEX } },
    { TournamentMap::Inferno, Semifinal, Fnatic, VirtusPro, { flusha, pronax, JW,  KRIMZ, olofmeister, pashaBiceps, TaZ, byali, NEO, Snax } },
    { TournamentMap::Mirage, GroupStage, Renegades, Titan, { SPUNJ, AZR, yam, jks, Maniac, shox, RpK, SmithZz } },
    { TournamentMap::Mirage, GroupStage, TeamEBettle, Fnatic, { Hyper, GruBy, olofmeister, pronax, flusha, KRIMZ, JW } },
    { TournamentMap::Mirage, GroupStage, CounterLogicGaming, TeamEBettle, { tarik, hazed, FNS, jdm64, reltuC, peet, GruBy, rallen, Hyper } },
    { TournamentMap::Mirage, Quarterfinal, NatusVincere, TeamEnVyUs, { Zeus, seized, flamie, GuardiaN, Edward, apEX, kennyS, NBK, Happy, kioShiMa } },
    { TournamentMap::Mirage, Quarterfinal, Fnatic, LuminosityGaming, { flusha, olofmeister, JW, pronax, KRIMZ, coldzera, fer, boltz, FalleN } },
    { TournamentMap::Mirage, Semifinal, Fnatic, VirtusPro, { flusha, pronax, JW,  KRIMZ, pashaBiceps, TaZ, byali, NEO, Snax } },
    { TournamentMap::Overpass, GroupStage, LuminosityGaming, TeamKinguin, { boltz, fer, coldzera, FalleN, fox, ScreaM, Maikelele, rain } },
    { TournamentMap::Overpass, GroupStage, NatusVincere, Titan, { flamie, seized, Edward, GuardiaN, Zeus, shox, Maniac, SmithZz, Ex6TenZ, RpK } },
    { TournamentMap::Overpass, GroupStage, Mousesports, Cloud9G2A, { denis, gobb, chrisJ, nex, Spiidi, seangares, Skadoodle, shroud, n0thing, freakazoid } },
    { TournamentMap::Overpass, Quarterfinal, TeamSoloMid, TeamKinguin, { dupreeh, device, karrigan, Xyp9x, cajunb, ScreaM, Maikelele, fox, rain } },
    { TournamentMap::Train, GroupStage, TeamSoloMid, Renegades, { device, dupreeh, karrigan, Xyp9x, cajunb, AZR, SPUNJ, jks, Havoc, yam } },
    { TournamentMap::Train, Quarterfinal, NinjasInPyjamas, VirtusPro, { allu, f0rest, GeT_RiGhT, Xizt, friberg, TaZ, Snax, NEO, byali, pashaBiceps } },
    { TournamentMap::Train, Quarterfinal, Fnatic, LuminosityGaming, { flusha, olofmeister, JW, pronax, KRIMZ, coldzera, fer, boltz, steel } },
});
static_assert(std::ranges::is_sorted(eslOneCologne2015Matches, {}, &Match::map));

constexpr auto dreamHackClujNapoka2015Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, Flipsid3Tactics, TeamSoloMid, { WorldEdit, B1ad3, DavCost, dupreeh, Xyp9x, karrigan, device, cajunb } },
    { TournamentMap::Cache, GroupStage, G2Esports, Mousesports, { Maikelele, jkaem, fox, dennis, rain, NiKo, chrisJ, denis } },
    { TournamentMap::Cache, GroupStage, Titan, NinjasInPyjamas, { shox, RpK, ScreaM, Ex6TenZ, SmithZz, f0rest, allu, Xizt, friberg, GeT_RiGhT } },
    { TournamentMap::Cache, Quarterfinal, TeamEnVyUs, Fnatic, { NBK, apEX, Happy, kioShiMa, kennyS, KRIMZ, pronax } },
    { TournamentMap::Cache, Quarterfinal, VirtusPro, G2Esports, { Snax, pashaBiceps, byali, TaZ, NEO, jkaem, rain, dennis, Maikelele, fox } },
    { TournamentMap::Cache, Semifinal, TeamEnVyUs, G2Esports, { kennyS, Happy, apEX, kioShiMa, jkaem, dennis, rain, fox } },
    { TournamentMap::Cobblestone, GroupStage, Fnatic, VexedGaming, { flusha, KRIMZ, pronax, JW, olofmeister, Furlan, GruBy, Hyper, peet, rallen } },
    { TournamentMap::Cobblestone, GroupStage, VexedGaming, Cloud9, { Furlan, GruBy, Hyper, peet, rallen, seangares, Skadoodle, shroud, freakazoid } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, TeamLiquid, { Snax, TaZ, byali, pashaBiceps, NEO, EliGE, FugLy, Hiko, adreN } },
    { TournamentMap::Cobblestone, GroupStage, Titan, NinjasInPyjamas, { RpK, SmithZz, shox, ScreaM, Ex6TenZ, Xizt, f0rest, allu, GeT_RiGhT, friberg } },
    { TournamentMap::Cobblestone, GroupStage, TeamEnVyUs, TeamDignitas, { NBK, kioShiMa, Happy, apEX, kennyS, aizy, Kjaerbye } },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, CounterLogicGaming, { GuardiaN, flamie, Zeus, Edward, seized, jdm64, reltuC, tarik, FNS, hazed } },
    { TournamentMap::Cobblestone, GroupStage, TeamDignitas, CounterLogicGaming, { aizy, Pimp, Kjaerbye, tenzki, MSL, FNS, reltuC, tarik, jdm64 } },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, CounterLogicGaming, { seized, GuardiaN, Edward, flamie, Zeus, hazed, jdm64, tarik, FNS, reltuC } },
    { TournamentMap::Cobblestone, Quarterfinal, TeamEnVyUs, Fnatic, { NBK, kioShiMa, kennyS, apEX, Happy, olofmeister, KRIMZ, JW } },
    { TournamentMap::Cobblestone, GrandFinal, TeamEnVyUs, NatusVincere, { kennyS, apEX, kioShiMa, NBK, seized, GuardiaN, flamie, Edward } },
    { TournamentMap::Dust2, GroupStage, LuminosityGaming, Cloud9, { coldzera, steel, fer, boltz, FalleN, seangares, n0thing, Skadoodle, shroud, freakazoid } },
    { TournamentMap::Dust2, GroupStage, TeamSoloMid, G2Esports, { cajunb, dupreeh, Xyp9x, karrigan, device, dennis, Maikelele, fox, rain, jkaem } },
    { TournamentMap::Dust2, GroupStage, Titan, NinjasInPyjamas, { shox, f0rest, allu, Xizt, friberg, GeT_RiGhT } },
    { TournamentMap::Dust2, GroupStage, Cloud9, Fnatic, { freakazoid, Skadoodle, n0thing, flusha, KRIMZ, pronax, JW, olofmeister } },
    { TournamentMap::Dust2, Quarterfinal, NinjasInPyjamas, TeamSoloMid, { GeT_RiGhT, allu, Xizt, friberg, Xyp9x, cajunb, dupreeh, device, karrigan } },
    { TournamentMap::Dust2, Quarterfinal, NatusVincere, LuminosityGaming, { GuardiaN, flamie, seized, Zeus, Edward, fer, steel, boltz, coldzera } },
    { TournamentMap::Dust2, Semifinal, TeamEnVyUs, G2Esports, { kennyS, Happy, apEX, NBK, kioShiMa, jkaem, dennis, rain, fox, Maikelele } },
    { TournamentMap::Dust2, Semifinal, NinjasInPyjamas, NatusVincere, { allu, GeT_RiGhT, f0rest, Xizt, flamie, GuardiaN, Zeus, Edward, seized } },
    { TournamentMap::Inferno, GroupStage, LuminosityGaming, Fnatic, { coldzera, steel, fer, boltz, FalleN, flusha, KRIMZ, pronax, JW } },
    { TournamentMap::Inferno, GroupStage, Mousesports, G2Esports, { gobb, nex, chrisJ, denis, NiKo, fox, dennis, jkaem, Maikelele, rain } },
    { TournamentMap::Inferno, GroupStage, G2Esports, Mousesports, { Maikelele, jkaem, fox, dennis, rain, NiKo, chrisJ, denis, gobb, nex } },
    { TournamentMap::Inferno, GroupStage, NatusVincere, CounterLogicGaming, { seized, GuardiaN, Edward, flamie, Zeus, hazed, jdm64, tarik, FNS } },
    { TournamentMap::Inferno, Semifinal, TeamEnVyUs, G2Esports, { kennyS, Happy, apEX, NBK, kioShiMa, jkaem, dennis, rain, fox, Maikelele } },
    { TournamentMap::Mirage, GroupStage, G2Esports, Mousesports, { Maikelele, jkaem, fox, dennis, rain, NiKo, chrisJ, denis, gobb } },
    { TournamentMap::Mirage, GroupStage, TeamLiquid, NinjasInPyjamas, { adreN, nitr0, FugLy, Hiko, f0rest, Xizt, GeT_RiGhT, friberg, allu } },
    { TournamentMap::Mirage, GroupStage, NatusVincere, TeamEnVyUs, { seized, Zeus, flamie, GuardiaN, Edward, kennyS, apEX, NBK, kioShiMa } },
    { TournamentMap::Mirage, Quarterfinal, TeamEnVyUs, Fnatic, { apEX, Happy, kennyS, NBK, flusha, olofmeister, KRIMZ, JW, pronax } },
    { TournamentMap::Overpass, GroupStage, Cloud9, Fnatic, { seangares, n0thing, Skadoodle, shroud, freakazoid, flusha, KRIMZ, pronax, JW, olofmeister } },
    { TournamentMap::Overpass, GroupStage, Flipsid3Tactics, Mousesports, { WorldEdit, markeloff, bondik, B1ad3, NiKo, nex, gobb, chrisJ, denis } },
    { TournamentMap::Overpass, Quarterfinal, NatusVincere, LuminosityGaming, { GuardiaN, flamie, seized, Zeus, Edward, fer, steel, boltz, coldzera, FalleN } },
    { TournamentMap::Train, GroupStage, VirtusPro, Titan, { Snax, byali, pashaBiceps, TaZ, NEO, Ex6TenZ, SmithZz, RpK, shox } },
    { TournamentMap::Train, GroupStage, NatusVincere, CounterLogicGaming, { seized, GuardiaN, Edward, flamie, Zeus, hazed, jdm64, tarik, FNS } },
    { TournamentMap::Train, Quarterfinal, VirtusPro, G2Esports, { Snax, pashaBiceps, byali, TaZ, NEO, jkaem, rain, dennis, Maikelele, fox } },
    { TournamentMap::Train, Quarterfinal, NinjasInPyjamas, TeamSoloMid, { friberg, Xizt, f0rest, allu, GeT_RiGhT, karrigan, Xyp9x, dupreeh, cajunb } },
    { TournamentMap::Train, Semifinal, NinjasInPyjamas, NatusVincere, { allu, GeT_RiGhT, flamie, GuardiaN, Zeus, Edward, seized } },
    { TournamentMap::Train, GrandFinal, TeamEnVyUs, NatusVincere, { kennyS, Happy, apEX, kioShiMa, NBK, seized, GuardiaN, flamie, Zeus, Edward } },
});
static_assert(std::ranges::is_sorted(dreamHackClujNapoka2015Matches, {}, &Match::map));

constexpr auto mlgColumbus2016Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, Flipsid3Tactics, NinjasInPyjamas, { bondik, Shara, B1ad3, WorldEdit, markeloff, f0rest, GeT_RiGhT, friberg, Xizt, THREAT } },
    { TournamentMap::Cache, GroupStage, Mousesports, NinjasInPyjamas, { chrisJ, NiKo, Spiidi, denis, nex, f0rest, GeT_RiGhT, friberg, Xizt, THREAT } },
    { TournamentMap::Cache, GroupStage, FaZeClan, TeamLiquid, { rain, aizy, Maikelele, fox, jkaem, nitr0, adreN, EliGE, Hiko, s1mple } },
    { TournamentMap::Cache, GroupStage, GambitEsports, TeamEnVyUs, { wayLander, AdreN, mou, Dosia, apEX, Happy, NBK, DEVIL } },
    { TournamentMap::Cache, GroupStage, CounterLogicGaming, GambitEsports, { hazed, FugLy, jdm64, tarik, mou, AdreN, hooch, Dosia, wayLander } },
    { TournamentMap::Cache, Quarterfinal, Fnatic, Astralis, { flusha, olofmeister, dupreeh, device, karrigan, Xyp9x, cajunb } },
    { TournamentMap::Cache, Quarterfinal, TeamLiquid, CounterLogicGaming, { nitr0, s1mple, adreN, EliGE, Hiko, tarik, jdm64, hazed, reltuC, FugLy } },
    { TournamentMap::Cache, Quarterfinal, LuminosityGaming, VirtusPro, { coldzera, FalleN, TACO, fer, fnx, byali, Snax, TaZ, pashaBiceps, NEO } },
    { TournamentMap::Cache, Semifinal, TeamLiquid, LuminosityGaming, { s1mple, Hiko, nitr0, adreN, FalleN, coldzera, fer, fnx, TACO } },
    { TournamentMap::Cobblestone, GroupStage, Flipsid3Tactics, Mousesports, { bondik, Shara, B1ad3, WorldEdit, markeloff, chrisJ, NiKo, Spiidi, denis, nex } },
    { TournamentMap::Cobblestone, GroupStage, Mousesports, NinjasInPyjamas, { NiKo, Spiidi, denis, f0rest, GeT_RiGhT, friberg, Xizt, THREAT } },
    { TournamentMap::Cobblestone, GroupStage, Fnatic, FaZeClan, { KRIMZ, JW, dennis, flusha, olofmeister, rain, aizy, jkaem } },
    { TournamentMap::Cobblestone, GroupStage, CounterLogicGaming, TeamEnVyUs, { reltuC, tarik, jdm64, hazed, FugLy, DEVIL, Happy, apEX, NBK } },
    { TournamentMap::Cobblestone, GroupStage, CounterLogicGaming, GambitEsports, { hazed, FugLy, jdm64, tarik, reltuC, AdreN, hooch, Dosia, wayLander } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, NatusVincere, { TaZ, byali, Snax, NEO, GuardiaN, Edward, seized, flamie, Zeus } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, G2Esports, { byali, Snax, NEO, TaZ, pashaBiceps, SmithZz, Ex6TenZ, ScreaM, RpK } },
    { TournamentMap::Cobblestone, Quarterfinal, LuminosityGaming, VirtusPro, { coldzera, FalleN, TACO, fer, fnx, byali, Snax, pashaBiceps, NEO } },
    { TournamentMap::Dust2, GroupStage, Fnatic, TeamLiquid, { KRIMZ, JW, dennis, flusha, olofmeister, Hiko, s1mple, adreN, EliGE, nitr0 } },
    { TournamentMap::Dust2, GroupStage, G2Esports, Cloud9, { SmithZz, Ex6TenZ, shox, ScreaM, RpK, Skadoodle, n0thing, shroud } },
    { TournamentMap::Dust2, Semifinal, NatusVincere, Astralis, { Edward, flamie, Zeus, seized, cajunb, dupreeh, Xyp9x } },
    { TournamentMap::Inferno, GroupStage, Splyce, FaZeClan, { arya, Professor_Chaos, jasonR, rain, aizy, jkaem, fox, Maikelele } },
    { TournamentMap::Inferno, GroupStage, VirtusPro, G2Esports, { byali, Snax, NEO, TaZ, pashaBiceps, SmithZz, Ex6TenZ, shox, ScreaM, RpK } },
    { TournamentMap::Inferno, Quarterfinal, NatusVincere, NinjasInPyjamas, { Edward, flamie, seized, GuardiaN, Zeus, friberg, f0rest, Xizt, GeT_RiGhT, THREAT } },
    { TournamentMap::Inferno, Semifinal, NatusVincere, Astralis, { Edward, flamie, Zeus, GuardiaN, seized, device, cajunb, dupreeh, Xyp9x, karrigan } },
    { TournamentMap::Mirage, GroupStage, LuminosityGaming, Mousesports, { coldzera, fnx, fer, FalleN, TACO, chrisJ, NiKo, Spiidi, denis, nex } },
    { TournamentMap::Mirage, GroupStage, LuminosityGaming, NinjasInPyjamas, { coldzera, fnx, fer, FalleN, TACO, f0rest, friberg, Xizt, THREAT } },
    { TournamentMap::Mirage, GroupStage, Fnatic, FaZeClan, { KRIMZ, JW, dennis, flusha, rain, aizy, jkaem, fox, Maikelele } },
    { TournamentMap::Mirage, GroupStage, CounterLogicGaming, GambitEsports, { hazed, FugLy, jdm64, tarik, reltuC, AdreN, hooch, Dosia, wayLander, mou } },
    { TournamentMap::Mirage, Quarterfinal, NatusVincere, NinjasInPyjamas, { Edward, flamie, seized, GuardiaN, Zeus, friberg, f0rest, Xizt, GeT_RiGhT } },
    { TournamentMap::Mirage, Quarterfinal, TeamLiquid, CounterLogicGaming, { nitr0, s1mple, adreN, EliGE, Hiko, tarik, hazed, reltuC } },
    { TournamentMap::Mirage, Semifinal, TeamLiquid, LuminosityGaming, { EliGE, s1mple, Hiko, nitr0, adreN, FalleN, coldzera, fer, fnx, TACO } },
    { TournamentMap::Mirage, GrandFinal, NatusVincere, LuminosityGaming, { Zeus, Edward, flamie, seized, GuardiaN, coldzera, FalleN, fer, fnx, TACO } },
    { TournamentMap::Nuke, AllStar, AllStarTeamAmerica, AllStarTeamEurope, { s1mple, shroud, tarik, Hiko, Skadoodle, GeT_RiGhT, NiKo, kennyS, rain, Maikelele } },
    { TournamentMap::Overpass, GroupStage, Mousesports, NinjasInPyjamas, { chrisJ, NiKo, Spiidi, denis, nex, f0rest, GeT_RiGhT, friberg, Xizt, THREAT } },
    { TournamentMap::Overpass, GroupStage, Astralis, CounterLogicGaming, { Xyp9x, device, dupreeh, karrigan, cajunb, hazed, jdm64, tarik, reltuC } },
    { TournamentMap::Overpass, Quarterfinal, Fnatic, Astralis, { JW, flusha, dennis, olofmeister, dupreeh, device, karrigan, Xyp9x, cajunb } },
    { TournamentMap::Overpass, Quarterfinal, LuminosityGaming, VirtusPro, { coldzera, FalleN, TACO, fer, fnx, byali, Snax, TaZ, pashaBiceps, NEO } },
    { TournamentMap::Overpass, GrandFinal, NatusVincere, LuminosityGaming, { GuardiaN, coldzera, FalleN, fer, fnx, TACO } },
    { TournamentMap::Train, GroupStage, Splyce, Fnatic, { DAVEY, Professor_Chaos, jasonR, dennis, KRIMZ, flusha, olofmeister, JW } },
    { TournamentMap::Train, GroupStage, GambitEsports, Astralis, { mou, AdreN, device, cajunb, Xyp9x, karrigan, dupreeh } },
    { TournamentMap::Train, GroupStage, Cloud9, NatusVincere, { Skadoodle, freakazoid, n0thing, flamie, seized, Edward, Zeus, GuardiaN } },
    { TournamentMap::Train, GroupStage, G2Esports, VirtusPro, { RpK, byali, Snax, NEO, TaZ, pashaBiceps } },
    { TournamentMap::Train, GroupStage, VirtusPro, G2Esports, { byali, Snax, NEO, TaZ, pashaBiceps, SmithZz, Ex6TenZ, shox, ScreaM, RpK } },
});
static_assert(std::ranges::is_sorted(mlgColumbus2016Matches, {}, &Match::map));

constexpr auto eslOneCologne2016Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, TeamDignitas, Astralis, { k0nfig, RUBINO, MSL, tenzki, cajunb, karrigan, Xyp9x, device, gla1ve } },
    { TournamentMap::Cache, GroupStage, Flipsid3Tactics, NinjasInPyjamas, { wayLander, markeloff, WorldEdit, Shara, B1ad3, pyth, friberg } },
    { TournamentMap::Cache, GroupStage, FaZeClan, Fnatic, { kioShiMa, rain, aizy, jkaem, KRIMZ, dennis, flusha, olofmeister, JW } },
    { TournamentMap::Cache, Quarterfinal, GambitEsports, Fnatic, { spaze, AdreN, KRIMZ, flusha, olofmeister, dennis, JW } },
    { TournamentMap::Cache, Semifinal, TeamLiquid, Fnatic, { s1mple, EliGE, nitr0, jdm64, Hiko, KRIMZ, JW, olofmeister, dennis, flusha } },
    { TournamentMap::Cobblestone, GroupStage, TeamDignitas, CounterLogicGaming, { RUBINO, tenzki, MSL, k0nfig, cajunb, tarik } },
    { TournamentMap::Cobblestone, GroupStage, TeamDignitas, Astralis, { k0nfig, RUBINO, MSL, tenzki, cajunb, karrigan, Xyp9x, device, gla1ve } },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, NinjasInPyjamas, { seized, flamie, GuardiaN, Edward, Zeus, GeT_RiGhT, friberg, pyth, Xizt } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, TeamLiquid, { Snax, TaZ, NEO, byali, pashaBiceps, s1mple, nitr0, jdm64, EliGE } },
    { TournamentMap::Cobblestone, GroupStage, TeamLiquid, Mousesports, { Hiko, EliGE, s1mple, jdm64, nitr0, denis, chrisJ, Spiidi, NiKo, nex } },
    { TournamentMap::Cobblestone, GroupStage, SKGaming, G2Esports, { coldzera, FalleN, TACO, fer, fnx, RpK, SmithZz, ScreaM, shox, bodyy } },
    { TournamentMap::Cobblestone, GroupStage, SKGaming, FaZeClan, { coldzera, TACO, FalleN, fer, fnx, aizy, rain, jkaem, kioShiMa } },
    { TournamentMap::Cobblestone, Quarterfinal, TeamLiquid, NatusVincere, { Hiko, s1mple, jdm64, EliGE, nitr0, Edward, GuardiaN, seized } },
    { TournamentMap::Cobblestone, Semifinal, VirtusPro, SKGaming, { TaZ, NEO, Snax, pashaBiceps, byali, coldzera, FalleN, fer, TACO, fnx } },
    { TournamentMap::Cobblestone, Semifinal, TeamLiquid, Fnatic, { s1mple, EliGE, nitr0, jdm64, Hiko, KRIMZ, olofmeister, dennis, flusha } },
    { TournamentMap::Cobblestone, GrandFinal, TeamLiquid, SKGaming, { jdm64, Hiko, EliGE, coldzera, FalleN, TACO, fer, fnx } },
    { TournamentMap::Dust2, GroupStage, CounterLogicGaming, GambitEsports, { hazed, pita, koosta, AdreN, spaze, mou, hooch, Dosia } },
    { TournamentMap::Dust2, GroupStage, GambitEsports, Astralis, { mou, AdreN, Dosia, spaze, device, gla1ve, karrigan } },
    { TournamentMap::Dust2, GroupStage, OpTicGaming, NinjasInPyjamas, { RUSH, daps, NAF, pyth, f0rest, GeT_RiGhT, Xizt } },
    { TournamentMap::Dust2, GroupStage, FaZeClan, Fnatic, { aizy, kioShiMa, jkaem, rain, fox, KRIMZ, dennis, flusha, olofmeister, JW } },
    { TournamentMap::Mirage, GroupStage, TeamDignitas, Astralis, { k0nfig, RUBINO, MSL, tenzki, cajunb, karrigan, Xyp9x, device, gla1ve } },
    { TournamentMap::Mirage, GroupStage, Flipsid3Tactics, NinjasInPyjamas, { wayLander, markeloff, WorldEdit, Shara, GeT_RiGhT, pyth, Xizt, friberg, f0rest } },
    { TournamentMap::Mirage, GroupStage, TeamLiquid, Mousesports, { Hiko, EliGE, s1mple, jdm64, nitr0, denis, chrisJ, Spiidi, nex } },
    { TournamentMap::Mirage, GroupStage, FaZeClan, Fnatic, { kioShiMa, fox, aizy, jkaem, KRIMZ, dennis, flusha, olofmeister, JW } },
    { TournamentMap::Mirage, Quarterfinal, Flipsid3Tactics, SKGaming, { WorldEdit, B1ad3, markeloff, fnx, coldzera, fer, FalleN, TACO } },
    { TournamentMap::Mirage, Semifinal, VirtusPro, SKGaming, { TaZ, NEO, Snax, pashaBiceps, byali, coldzera, FalleN, fer, TACO, fnx } },
    { TournamentMap::Nuke, Quarterfinal, Flipsid3Tactics, SKGaming, { WorldEdit, B1ad3, wayLander, markeloff, Shara, fnx, coldzera, fer, FalleN, TACO } },
    { TournamentMap::Nuke, Quarterfinal, TeamLiquid, NatusVincere, { Hiko, s1mple, jdm64, EliGE, nitr0, Edward, GuardiaN, Zeus, seized, flamie } },
    { TournamentMap::Nuke, Semifinal, VirtusPro, SKGaming, { TaZ, NEO, Snax, pashaBiceps, coldzera, FalleN, fer, TACO, fnx } },
    { TournamentMap::Overpass, GroupStage, TeamDignitas, Astralis, { k0nfig, MSL, cajunb, tenzki, RUBINO, device, gla1ve, dupreeh, Xyp9x, karrigan } },
    { TournamentMap::Overpass, GroupStage, Flipsid3Tactics, NinjasInPyjamas, { wayLander, markeloff, WorldEdit, Shara, B1ad3, GeT_RiGhT, pyth, Xizt, friberg, f0rest } },
    { TournamentMap::Overpass, Quarterfinal, Astralis, VirtusPro, { device, Xyp9x, gla1ve, karrigan, pashaBiceps, TaZ, byali, Snax } },
    { TournamentMap::Train, GroupStage, NatusVincere, Flipsid3Tactics, { GuardiaN, seized, Zeus, flamie, Edward, B1ad3, wayLander, WorldEdit, Shara } },
    { TournamentMap::Train, GroupStage, Flipsid3Tactics, OpTicGaming, { WorldEdit, B1ad3, wayLander, markeloff, Shara, mixwell, NAF, RUSH, stanislaw } },
    { TournamentMap::Train, GroupStage, TeamEnVyUs, TeamLiquid, { kennyS, apEX, Happy, NBK, Hiko, EliGE, s1mple, jdm64, nitr0 } },
    { TournamentMap::Train, GroupStage, VirtusPro, Mousesports, { Snax, TaZ, NEO, byali, pashaBiceps, chrisJ, NiKo, Spiidi, denis } },
    { TournamentMap::Train, GroupStage, Mousesports, TeamEnVyUs, { denis, chrisJ, Spiidi, NiKo, nex, NBK, kennyS, apEX, DEVIL, Happy } },
    { TournamentMap::Train, GroupStage, G2Esports, Fnatic, { ScreaM, RpK, SmithZz, shox, bodyy, KRIMZ, dennis, flusha, olofmeister, JW } },
    { TournamentMap::Train, Quarterfinal, Astralis, VirtusPro, { device, gla1ve, karrigan, pashaBiceps, TaZ, byali, Snax, NEO } },
    { TournamentMap::Train, Quarterfinal, TeamLiquid, NatusVincere, { s1mple, jdm64, EliGE, nitr0, Edward, GuardiaN, Zeus, seized, flamie } },
    { TournamentMap::Train, Quarterfinal, GambitEsports, Fnatic, { spaze, mou, hooch, AdreN, KRIMZ, flusha, olofmeister, dennis, JW } },
    { TournamentMap::Train, GrandFinal, TeamLiquid, SKGaming, { jdm64, Hiko, nitr0, s1mple, coldzera, TACO, fnx, FalleN, fer } },
});
static_assert(std::ranges::is_sorted(eslOneCologne2016Matches, {}, &Match::map));

constexpr auto eleagueAtlanta2017Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, G2Esports, Fnatic, { RpK, bodyy, shox, ScreaM, SmithZz, KRIMZ, olofmeister, dennis, discodoplan } },
    { TournamentMap::Cache, GroupStage, TeamLiquid, TeamEnVyUs, { EliGE, Pimp, Hiko, jdm64, nitr0, NBK, apEX, kennyS, SIXER, Happy } },
    { TournamentMap::Cache, GroupStage, Mousesports, HellRaisers, { chrisJ, denis, loWel, NiKo, Spiidi, DeadFox, ANGE1, bondik, STYKO, Zero } },
    { TournamentMap::Cache, GroupStage, GODSENT, TeamEnVyUs, { Lekr0, flusha, pronax, NBK, apEX, kennyS, SIXER, Happy } },
    { TournamentMap::Cache, GroupStage, OpTicGaming, GODSENT, { RUSH, NAF, mixwell, tarik, Lekr0, flusha, pronax, JW, znajder } },
    { TournamentMap::Cache, Quarterfinal, GambitEsports, Fnatic, { AdreN, Hobbit, Zeus, mou, twist, dennis, discodoplan, olofmeister, KRIMZ } },
    { TournamentMap::Cache, Quarterfinal, North, VirtusPro, { Magisk, MSL, k0nfig, cajunb, RUBINO, Snax, NEO, pashaBiceps, TaZ, byali } },
    { TournamentMap::Cache, Semifinal, Astralis, Fnatic, { dupreeh, device, Xyp9x, Kjaerbye, gla1ve, KRIMZ, twist, discodoplan, dennis, olofmeister } },
    { TournamentMap::Cobblestone, GroupStage, North, GambitEsports, { k0nfig, cajunb, MSL, Magisk, RUBINO, Zeus, AdreN, Dosia, mou, Hobbit } },
    { TournamentMap::Cobblestone, GroupStage, VirtusPro, OpTicGaming, { Snax, TaZ, byali, pashaBiceps, NEO, RUSH, NAF, stanislaw, mixwell } },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, Mousesports, { Edward, s1mple, seized, flamie, GuardiaN, chrisJ, denis, loWel } },
    { TournamentMap::Cobblestone, GroupStage, NatusVincere, TeamEnVyUs, { Edward, s1mple, seized, flamie, GuardiaN, NBK, apEX, kennyS, SIXER } },
    { TournamentMap::Cobblestone, GroupStage, Fnatic, North, { twist, KRIMZ, olofmeister, dennis, discodoplan, k0nfig, cajunb, MSL, Magisk, RUBINO } },
    { TournamentMap::Cobblestone, GroupStage, Fnatic, TeamEnVyUs, { twist, olofmeister, KRIMZ, discodoplan, dennis, apEX, NBK, kennyS, SIXER, Happy } },
    { TournamentMap::Cobblestone, Quarterfinal, North, VirtusPro, { Magisk, MSL, k0nfig, cajunb, RUBINO, Snax, NEO, pashaBiceps, TaZ, byali } },
    { TournamentMap::Cobblestone, Semifinal, VirtusPro, SKGaming, { NEO, pashaBiceps, TaZ, byali, fox, FalleN, coldzera, TACO } },
    { TournamentMap::Dust2, GroupStage, NatusVincere, SKGaming, { Edward, s1mple, seized, flamie, GuardiaN, coldzera, fer } },
    { TournamentMap::Dust2, GroupStage, Mousesports, Fnatic, { chrisJ, denis, loWel, NiKo, Spiidi, twist, KRIMZ, olofmeister, dennis, discodoplan } },
    { TournamentMap::Dust2, GroupStage, Astralis, SKGaming, { device, gla1ve, Xyp9x, Kjaerbye, dupreeh, coldzera, fox, fer, FalleN, TACO } },
    { TournamentMap::Dust2, Quarterfinal, NatusVincere, Astralis, { flamie, GuardiaN, s1mple, seized, Edward, device, gla1ve, Kjaerbye, dupreeh, Xyp9x } },
    { TournamentMap::Dust2, Quarterfinal, GambitEsports, Fnatic, { mou, Hobbit, twist, dennis, discodoplan, olofmeister, KRIMZ } },
    { TournamentMap::Mirage, GroupStage, HellRaisers, SKGaming, { DeadFox, ANGE1, bondik, STYKO, coldzera, fer, TACO, fox, FalleN } },
    { TournamentMap::Mirage, GroupStage, SKGaming, FaZeClan, { coldzera, fer, TACO, fox, FalleN, rain, allu, kioShiMa, karrigan, aizy } },
    { TournamentMap::Mirage, GroupStage, HellRaisers, North, { DeadFox, ANGE1, bondik, STYKO, Zero, k0nfig, cajunb, MSL, Magisk, RUBINO } },
    { TournamentMap::Mirage, GroupStage, Astralis, TeamLiquid, { device, gla1ve, Kjaerbye, dupreeh, Hiko, nitr0 } },
    { TournamentMap::Mirage, Quarterfinal, NatusVincere, Astralis, { flamie, GuardiaN, s1mple, seized, Edward, device, gla1ve, Kjaerbye, Xyp9x } },
    { TournamentMap::Mirage, Quarterfinal, SKGaming, FaZeClan, { fer, fox, FalleN, coldzera, TACO, kioShiMa, rain, karrigan, aizy, allu } },
    { TournamentMap::Nuke, GroupStage, FaZeClan, Flipsid3Tactics, { rain, allu, kioShiMa, karrigan, aizy, electronic, B1ad3, WorldEdit } },
    { TournamentMap::Nuke, GroupStage, G2Esports, VirtusPro, { RpK, bodyy, shox, ScreaM, SmithZz, Snax, TaZ, byali, NEO } },
    { TournamentMap::Nuke, GroupStage, FaZeClan, TeamLiquid, { rain, allu, kioShiMa, karrigan, aizy, EliGE, Pimp, Hiko, jdm64, nitr0 } },
    { TournamentMap::Nuke, GroupStage, TeamLiquid, Mousesports, { EliGE, Pimp, Hiko, jdm64, nitr0, loWel, NiKo, chrisJ } },
    { TournamentMap::Nuke, GroupStage, TeamEnVyUs, FaZeClan, { kennyS, Happy, SIXER, rain, allu, kioShiMa, karrigan, aizy } },
    { TournamentMap::Nuke, Semifinal, Astralis, Fnatic, { device, Xyp9x, Kjaerbye, gla1ve, KRIMZ, discodoplan, dennis } },
    { TournamentMap::Nuke, GrandFinal, Astralis, VirtusPro, { dupreeh, device, Xyp9x, gla1ve, NEO, Snax, pashaBiceps, TaZ, byali } },
    { TournamentMap::Overpass, GroupStage, GambitEsports, GODSENT, { Zeus, AdreN, Dosia, mou, Hobbit, JW, Lekr0, znajder, flusha, pronax } },
    { TournamentMap::Overpass, GroupStage, TeamLiquid, Flipsid3Tactics, { EliGE, Pimp, Hiko, jdm64, electronic, B1ad3, WorldEdit, markeloff, wayLander } },
    { TournamentMap::Overpass, GroupStage, FaZeClan, GambitEsports, { rain, allu, kioShiMa, karrigan, aizy, Zeus, AdreN, Dosia, mou, Hobbit } },
    { TournamentMap::Overpass, GroupStage, G2Esports, North, { RpK, bodyy, shox, ScreaM, SmithZz, k0nfig, cajunb, MSL, Magisk, RUBINO } },
    { TournamentMap::Overpass, GroupStage, GODSENT, North, { Lekr0, flusha, pronax, JW, znajder, k0nfig, cajunb, MSL, Magisk, RUBINO } },
    { TournamentMap::Overpass, Quarterfinal, NatusVincere, Astralis, { flamie, s1mple, seized, Edward, device, Kjaerbye, dupreeh, Xyp9x } },
    { TournamentMap::Overpass, Quarterfinal, GambitEsports, Fnatic, { AdreN, Hobbit, Zeus, mou, Dosia, KRIMZ, olofmeister } },
    { TournamentMap::Overpass, Quarterfinal, North, VirtusPro, { Magisk, MSL, Snax, NEO, pashaBiceps, TaZ, byali } },
    { TournamentMap::Overpass, Quarterfinal, SKGaming, FaZeClan, { fer, fox, FalleN, coldzera, TACO, kioShiMa, allu, karrigan } },
    { TournamentMap::Overpass, GrandFinal, Astralis, VirtusPro, { Kjaerbye, dupreeh, device, Xyp9x, gla1ve, NEO, Snax, pashaBiceps, TaZ, byali } },
    { TournamentMap::Train, GroupStage, GODSENT, Astralis, { JW, Lekr0, znajder, flusha, pronax, device, gla1ve, Kjaerbye, Xyp9x, dupreeh } },
    { TournamentMap::Train, GroupStage, Astralis, OpTicGaming, { device, gla1ve, Kjaerbye, Xyp9x, dupreeh, RUSH, NAF, stanislaw, mixwell, tarik } },
    { TournamentMap::Train, GroupStage, VirtusPro, GambitEsports, { Snax, TaZ, byali, pashaBiceps, NEO, Zeus, AdreN, Dosia, mou, Hobbit } },
    { TournamentMap::Train, GroupStage, G2Esports, Astralis, { RpK, bodyy, shox, ScreaM, device, gla1ve, Kjaerbye, Xyp9x, dupreeh } },
    { TournamentMap::Train, GroupStage, OpTicGaming, Flipsid3Tactics, { RUSH, NAF, stanislaw, mixwell, tarik, electronic, WorldEdit, markeloff, wayLander } },
    { TournamentMap::Train, Quarterfinal, SKGaming, FaZeClan, { fer, fox, FalleN, coldzera, TACO, allu, kioShiMa } },
    { TournamentMap::Train, Semifinal, VirtusPro, SKGaming, { NEO, Snax, pashaBiceps, TaZ, byali, fox, fer, FalleN, coldzera, TACO } },
    { TournamentMap::Train, GrandFinal, Astralis, VirtusPro, { Kjaerbye, dupreeh, Xyp9x, gla1ve, NEO, Snax, pashaBiceps, TaZ, byali } },
});
static_assert(std::ranges::is_sorted(eleagueAtlanta2017Matches, {}, &Match::map));

constexpr auto pglKrakow2017Matches = std::to_array<Match>({
    { TournamentMap::Cache, GroupStage, G2Esports, GambitEsports, { NBK, kennyS, apEX, bodyy, AdreN, Zeus, mou, Hobbit, Dosia } },
    { TournamentMap::Cache, GroupStage, VirtusPro, Fnatic, { TaZ, Snax, pashaBiceps, byali, NEO, JW, olofmeister, KRIMZ, dennis } },
    { TournamentMap::Cache, Quarterfinal, Astralis, SKGaming, { device, gla1ve, Kjaerbye, Xyp9x, dupreeh, fer, coldzera, felps, FalleN, TACO } },
    { TournamentMap::Cobblestone, GroupStage, North, Mousesports, { k0nfig, Magisk, cajunb, aizy, MSL, oskar, chrisJ, ropz, denis, loWel } },
    { TournamentMap::Cobblestone, GroupStage, G2Esports, Cloud9, { NBK, kennyS, shox, apEX, bodyy, n0thing, Skadoodle, Stewie2K, autimatic, shroud } },
    { TournamentMap::Cobblestone, Quarterfinal, Immortals, BIG, { steel, kNgV, LUCAS1, boltz, HEN1, LEGIJA, tabseN, nex, gobb, keev } },
    { TournamentMap::Cobblestone, Quarterfinal, North, VirtusPro, { Magisk, cajunb, k0nfig, MSL, TaZ, NEO, pashaBiceps, byali, Snax } },
    { TournamentMap::Cobblestone, GrandFinal, GambitEsports, Immortals, { Zeus, mou, Dosia, steel, kNgV, LUCAS1, boltz, HEN1 } },
    { TournamentMap::Inferno, GroupStage, Mousesports, GambitEsports, { oskar, chrisJ, ropz, denis, loWel, AdreN, Zeus, mou, Hobbit, Dosia } },
    { TournamentMap::Inferno, GroupStage, SKGaming, PENTASports, { coldzera, FalleN, fer, TACO, felps, kRYSTAL, suNny, zehN, innocent, HS } },
    { TournamentMap::Inferno, GroupStage, FaZeClan, BIG, { allu, rain, NiKo, kioShiMa, nex, tabseN, keev, gobb, LEGIJA } },
    { TournamentMap::Inferno, GroupStage, Cloud9, BIG, { Skadoodle, Stewie2K, autimatic, nex, tabseN, keev, gobb, LEGIJA } },
    { TournamentMap::Inferno, GroupStage, Astralis, SKGaming, { gla1ve, Kjaerbye, device, coldzera, FalleN, fer, TACO } },
    { TournamentMap::Inferno, GroupStage, SKGaming, BIG, { coldzera, FalleN, fer, TACO, felps, nex, tabseN, keev, gobb, LEGIJA } },
    { TournamentMap::Inferno, GroupStage, G2Esports, Astralis, { shox, apEX, kennyS, gla1ve, dupreeh, Kjaerbye, Xyp9x, device } },
    { TournamentMap::Inferno, Quarterfinal, Fnatic, GambitEsports, { JW, olofmeister, flusha, dennis, Zeus, mou, Hobbit, Dosia, AdreN } },
    { TournamentMap::Inferno, Quarterfinal, Immortals, BIG, { steel, kNgV, LUCAS1, boltz, HEN1, LEGIJA, tabseN, gobb, keev } },
    { TournamentMap::Inferno, Semifinal, GambitEsports, Astralis, { mou, Hobbit, Dosia, AdreN, device, gla1ve, Kjaerbye, Xyp9x, dupreeh } },
    { TournamentMap::Inferno, Semifinal, VirtusPro, Immortals, { NEO, pashaBiceps, byali, Snax, steel, kNgV, LUCAS1, boltz, HEN1 } },
    { TournamentMap::Inferno, GrandFinal, GambitEsports, Immortals, { Zeus, mou, Hobbit, Dosia, AdreN, steel, kNgV, LUCAS1, boltz, HEN1 } },
    { TournamentMap::Mirage, GroupStage, Fnatic, Flipsid3Tactics, { JW, olofmeister, flusha, KRIMZ, dennis, electronic, B1ad3, wayLander, markeloff } },
    { TournamentMap::Mirage, GroupStage, Cloud9, North, { n0thing, Skadoodle, Stewie2K, autimatic, shroud, k0nfig, Magisk, cajunb, aizy } },
    { TournamentMap::Mirage, GroupStage, North, PENTASports, { k0nfig, Magisk, cajunb, aizy, MSL, kRYSTAL, suNny, zehN, HS } },
    { TournamentMap::Mirage, GroupStage, VegaSquadron, PENTASports, { mir, hutji, jR, keshandr, chopper, kRYSTAL, suNny, zehN, innocent, HS } },
    { TournamentMap::Mirage, GroupStage, FaZeClan, Flipsid3Tactics, { allu, rain, NiKo, kioShiMa, karrigan, electronic, B1ad3, wayLander, markeloff, WorldEdit } },
    { TournamentMap::Mirage, GroupStage, VirtusPro, North, { TaZ, Snax, pashaBiceps, byali, NEO, k0nfig, Magisk, cajunb, aizy, MSL } },
    { TournamentMap::Mirage, GroupStage, NatusVincere, Fnatic, { s1mple, seized, flamie, GuardiaN, olofmeister, flusha, KRIMZ, dennis } },
    { TournamentMap::Mirage, Semifinal, VirtusPro, Immortals, { TaZ, pashaBiceps, byali, Snax, steel, kNgV, LUCAS1, boltz, HEN1 } },
    { TournamentMap::Nuke, GroupStage, VegaSquadron, VirtusPro, { mir, hutji, TaZ, Snax, pashaBiceps, byali, NEO } },
    { TournamentMap::Nuke, GroupStage, Fnatic, Astralis, { JW, flusha, KRIMZ, dennis, gla1ve, dupreeh, Kjaerbye, Xyp9x, device } },
    { TournamentMap::Nuke, Quarterfinal, North, VirtusPro, { Magisk, cajunb, k0nfig, MSL, aizy, NEO, pashaBiceps, byali, Snax } },
    { TournamentMap::Overpass, GroupStage, NatusVincere, G2Esports, { s1mple, Edward, flamie, GuardiaN, NBK, kennyS, shox, apEX, bodyy } },
    { TournamentMap::Overpass, GroupStage, Astralis, Immortals, { gla1ve, dupreeh, Kjaerbye, Xyp9x, device, LUCAS1, boltz, kNgV, steel, HEN1 } },
    { TournamentMap::Overpass, GroupStage, NatusVincere, Immortals, { s1mple, Edward, flamie, GuardiaN, seized, LUCAS1, boltz, kNgV, steel, HEN1 } },
    { TournamentMap::Overpass, GroupStage, SKGaming, Immortals, { coldzera, FalleN, fer, TACO, felps, LUCAS1, boltz, kNgV, HEN1 } },
    { TournamentMap::Overpass, GroupStage, G2Esports, Fnatic, { NBK, kennyS, apEX, bodyy, JW, olofmeister, flusha, KRIMZ, dennis } },
    { TournamentMap::Overpass, Quarterfinal, Astralis, SKGaming, { device, gla1ve, Kjaerbye, Xyp9x, dupreeh, coldzera, fer, felps } },
    { TournamentMap::Overpass, Semifinal, GambitEsports, Astralis, { Zeus, mou, Hobbit, AdreN, device, gla1ve, Kjaerbye, Xyp9x, dupreeh } },
    { TournamentMap::Train, GroupStage, FaZeClan, Mousesports, { allu, rain, NiKo, karrigan, oskar, chrisJ, ropz, denis, loWel } },
    { TournamentMap::Train, GroupStage, NatusVincere, Flipsid3Tactics, { s1mple, seized, flamie, GuardiaN, electronic, B1ad3, markeloff, WorldEdit } },
    { TournamentMap::Train, GroupStage, VegaSquadron, Immortals, { mir, hutji, jR, keshandr, LUCAS1, boltz, kNgV, steel, HEN1 } },
    { TournamentMap::Train, GroupStage, VirtusPro, GambitEsports, { TaZ, Snax, pashaBiceps, byali, NEO,  AdreN, Zeus, mou, Hobbit, Dosia } },
    { TournamentMap::Train, GroupStage, Mousesports, Cloud9, { oskar, chrisJ, ropz, denis, loWel, n0thing, Skadoodle, Stewie2K, autimatic, shroud } },
    { TournamentMap::Train, GroupStage, PENTASports, Flipsid3Tactics, { kRYSTAL, suNny, zehN, innocent, HS, electronic, B1ad3, wayLander, markeloff, WorldEdit } },
    { TournamentMap::Train, GroupStage, Cloud9, VirtusPro, { n0thing, Skadoodle, autimatic, shroud, TaZ, Snax, pashaBiceps, byali, NEO } },
    { TournamentMap::Train, GroupStage, Immortals, Flipsid3Tactics, { LUCAS1, boltz, kNgV, steel, HEN1, B1ad3, wayLander, markeloff, WorldEdit } },
    { TournamentMap::Train, Quarterfinal, Fnatic, GambitEsports, { JW, KRIMZ, olofmeister, flusha, dennis, Zeus, mou, Hobbit, Dosia, AdreN } },
    { TournamentMap::Train, Quarterfinal, Immortals, BIG, { steel, kNgV, LUCAS1, boltz, HEN1, LEGIJA, tabseN, nex, gobb, keev } },
    { TournamentMap::Train, Semifinal, GambitEsports, Astralis, { Zeus, mou, Hobbit, Dosia, AdreN, device, gla1ve, Kjaerbye, Xyp9x, dupreeh } },
    { TournamentMap::Train, GrandFinal, GambitEsports, Immortals, { Zeus, mou, Hobbit, Dosia, AdreN, steel, kNgV, LUCAS1, boltz, HEN1 } },
});
static_assert(std::ranges::is_sorted(pglKrakow2017Matches, {}, &Match::map));

constexpr auto eleagueBoston2018Matches = std::to_array<Match>({
    { TournamentMap::Cache, ChallengersStage, Cloud9, TeamEnVyUs, { Skadoodle, tarik, autimatic, Stewie2K, RUSH, ScreaM, RpK, Happy, SIXER } },
    { TournamentMap::Cache, ChallengersStage, TeamEnVyUs, Renegades, { xms, RpK, Happy, SIXER, AZR, Nifty, NAF, USTILO, jks } },
    { TournamentMap::Cache, ChallengersStage, MisfitsGaming, Avangar, { devoduvek, AmaNEk, ShahZaM, SicK, seangares, qikert, buster, Jame, dimasick, KrizzeN } },
    { TournamentMap::Cache, GroupStage, VirtusPro, QuantumBellatorFire, { pashaBiceps, Snax, TaZ, Boombl4, waterfaLLZ, jmqa, Kvik, balblna } },
    { TournamentMap::Cache, GroupStage, Fnatic, FaZeClan, { JW, Lekr0, KRIMZ, Golden, rain, olofmeister, GuardiaN, karrigan, NiKo } },
    { TournamentMap::Cache, GroupStage, Astralis, Mousesports, { dupreeh, device, chrisJ, oskar, ropz, STYKO, suNny } },
    { TournamentMap::Cache, GroupStage, G2Esports, Cloud9, { NBK, shox, apEX, kennyS, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
    { TournamentMap::Cache, GroupStage, QuantumBellatorFire, G2Esports, { Boombl4, jmqa, Kvik, NBK, shox, bodyy, apEX, kennyS } },
    { TournamentMap::Cache, GroupStage, FaZeClan, SKGaming, { olofmeister, GuardiaN, karrigan, NiKo, FalleN, felps, fer, TACO, coldzera } },
    { TournamentMap::Cache, Quarterfinal, FaZeClan, Mousesports, { rain, olofmeister, GuardiaN, karrigan, NiKo, oskar, ropz, STYKO, suNny } },
    { TournamentMap::Cobblestone, ChallengersStage, SproutEsports, SpaceSoldiers, { innocent, denis, zehN, Spiidi, kRYSTAL, XANTARES, paz, ngiN, Calyx } },
    { TournamentMap::Cobblestone, ChallengersStage, G2Esports, MisfitsGaming, { NBK, shox, bodyy, apEX, kennyS, devoduvek, ShahZaM, SicK, seangares } },
    { TournamentMap::Cobblestone, ChallengersStage, TeamLiquid, Flipsid3Tactics, { nitr0, Twistzz, EliGE, jdm64, wayLander, seized, markeloff, B1ad3 } },
    { TournamentMap::Cobblestone, ChallengersStage, TeamLiquid, Renegades, { nitr0, Twistzz, EliGE, jdm64, AZR, Nifty, NAF, USTILO, jks } },
    { TournamentMap::Cobblestone, ChallengersStage, SproutEsports, Renegades, { innocent, denis, zehN, Spiidi, kRYSTAL, AZR, Nifty, NAF, USTILO, jks } },
    { TournamentMap::Cobblestone, GroupStage, Cloud9, SpaceSoldiers, { Skadoodle, tarik, autimatic, Stewie2K, RUSH, XANTARES, paz, ngiN, Calyx, MAJ3R } },
    { TournamentMap::Cobblestone, GroupStage, BIG, North, { tabseN, LEGIJA, nex, keev, k0nfig, v4lde, aizy, cajunb, MSL } },
    { TournamentMap::Cobblestone, GroupStage, BIG, SpaceSoldiers, { tabseN, LEGIJA, nex, keev, gobb, XANTARES, paz, ngiN, Calyx, MAJ3R } },
    { TournamentMap::Cobblestone, Semifinal, Cloud9, SKGaming, { tarik, autimatic, Stewie2K, RUSH, FalleN, felps, fer, TACO, coldzera } },
    { TournamentMap::Inferno, ChallengersStage, QuantumBellatorFire, NatusVincere, { Boombl4, waterfaLLZ, jmqa, Kvik, s1mple, electronic, flamie, Zeus, Edward } },
    { TournamentMap::Inferno, ChallengersStage, G2Esports, FlashGaming, { NBK, shox, bodyy, apEX, Kaze, Attacker, Karsa, LoveYY, Summer } },
    { TournamentMap::Inferno, ChallengersStage, Cloud9, SproutEsports, { Skadoodle, tarik, autimatic, Stewie2K, RUSH, innocent, zehN, Spiidi, kRYSTAL } },
    { TournamentMap::Inferno, ChallengersStage, VegaSquadron, FaZeClan, { mir, jR, keshandr, hutji, chopper, rain, olofmeister, GuardiaN, karrigan, NiKo } },
    { TournamentMap::Inferno, ChallengersStage, FlashGaming, QuantumBellatorFire, { Kaze, Attacker, Karsa, LoveYY, Summer, Boombl4, waterfaLLZ, jmqa, Kvik, balblna } },
    { TournamentMap::Inferno, ChallengersStage, FlashGaming, TeamEnVyUs, { Kaze, Attacker, Karsa, LoveYY, Summer, xms, RpK, Happy, SIXER, ScreaM } },
    { TournamentMap::Inferno, ChallengersStage, QuantumBellatorFire, TeamEnVyUs, { Boombl4, waterfaLLZ, jmqa, Kvik, balblna, xms, Happy, SIXER, ScreaM } },
    { TournamentMap::Inferno, ChallengersStage, NatusVincere, TeamLiquid, { s1mple, electronic, flamie, Zeus, Edward, nitr0, Twistzz, EliGE, jdm64 } },
    { TournamentMap::Inferno, GroupStage, TeamLiquid, BIG, { nitr0, Twistzz, EliGE, jdm64, tabseN, LEGIJA, nex } },
    { TournamentMap::Inferno, GroupStage, QuantumBellatorFire, GambitEsports, { Boombl4, waterfaLLZ, jmqa, Kvik, balblna, AdreN, Dosia, Hobbit, fitch } },
    { TournamentMap::Inferno, GroupStage, G2Esports, TeamLiquid, { NBK, shox, bodyy, apEX, kennyS, nitr0, Twistzz } },
    { TournamentMap::Inferno, GroupStage, VirtusPro, Fnatic, { byali, Snax, TaZ, NEO, JW, Lekr0, KRIMZ, Golden, flusha } },
    { TournamentMap::Inferno, GroupStage, NatusVincere, BIG, { s1mple, electronic, flamie, Zeus, Edward, gobb } },
    { TournamentMap::Inferno, GroupStage, NatusVincere, Fnatic, { s1mple, electronic, flamie, Zeus, Edward, JW, KRIMZ, flusha } },
    { TournamentMap::Inferno, GroupStage, TeamLiquid, VegaSquadron, { nitr0, Twistzz, EliGE, jdm64, mir, jR, keshandr, hutji, chopper } },
    { TournamentMap::Inferno, Quarterfinal, NatusVincere, QuantumBellatorFire, { electronic, flamie, Zeus, Edward, Boombl4, waterfaLLZ, Kvik, balblna } },
    { TournamentMap::Inferno, Quarterfinal, SKGaming, Fnatic, { FalleN, felps, fer, TACO, coldzera, JW, KRIMZ, flusha, Lekr0, Golden } },
    { TournamentMap::Inferno, Semifinal, NatusVincere, FaZeClan, { s1mple, electronic, flamie, Zeus, rain, olofmeister, GuardiaN, karrigan, NiKo } },
    { TournamentMap::Inferno, Semifinal, Cloud9, SKGaming, { Skadoodle, tarik, autimatic, Stewie2K, RUSH, FalleN, felps, fer, TACO, coldzera } },
    { TournamentMap::Inferno, GrandFinal, FaZeClan, Cloud9, { rain, olofmeister, GuardiaN, karrigan, NiKo, Skadoodle, tarik, autimatic, Stewie2K } },
    { TournamentMap::Mirage, ChallengersStage, VegaSquadron, Renegades, { mir, jR, keshandr, hutji, chopper, AZR, Nifty, NAF, USTILO, jks } },
    { TournamentMap::Mirage, ChallengersStage, Avangar, Mousesports, { qikert, buster, Jame, dimasick, KrizzeN, chrisJ, oskar, ropz, STYKO, suNny } },
    { TournamentMap::Mirage, ChallengersStage, Mousesports, NatusVincere, { chrisJ, oskar, ropz, STYKO, suNny, electronic, Edward } },
    { TournamentMap::Mirage, ChallengersStage, Avangar, SpaceSoldiers, { buster, Jame, KrizzeN, XANTARES, paz, ngiN, Calyx, MAJ3R } },
    { TournamentMap::Mirage, ChallengersStage, FaZeClan, QuantumBellatorFire, { rain, GuardiaN, karrigan, NiKo, Boombl4, waterfaLLZ, Kvik } },
    { TournamentMap::Mirage, ChallengersStage, NatusVincere, SproutEsports, { s1mple, electronic, flamie, Zeus, Edward, Spiidi } },
    { TournamentMap::Mirage, ChallengersStage, TeamLiquid, VegaSquadron, { nitr0, Twistzz, EliGE, jdm64, mir, keshandr, hutji, chopper } },
    { TournamentMap::Mirage, ChallengersStage, Mousesports, SpaceSoldiers, { chrisJ, oskar, ropz, STYKO, suNny, XANTARES, paz, ngiN, Calyx, MAJ3R } },
    { TournamentMap::Mirage, ChallengersStage, Renegades, Mousesports, { AZR, Nifty, USTILO, chrisJ, oskar, ropz, STYKO, suNny } },
    { TournamentMap::Mirage, ChallengersStage, Avangar, Renegades, { qikert, buster, Jame, dimasick, KrizzeN, AZR, NAF, jks } },
    { TournamentMap::Mirage, ChallengersStage, TeamLiquid, Avangar, { nitr0, Twistzz, EliGE, jdm64, qikert, buster, Jame, dimasick, KrizzeN } },
    { TournamentMap::Mirage, GroupStage, SpaceSoldiers, SKGaming, { XANTARES, paz, Calyx, MAJ3R, FalleN, felps, fer, TACO, coldzera } },
    { TournamentMap::Mirage, GroupStage, Mousesports, SKGaming, { chrisJ, oskar, ropz, STYKO, suNny, FalleN, felps, fer, TACO, coldzera } },
    { TournamentMap::Mirage, GroupStage, Mousesports, VegaSquadron, { chrisJ, oskar, ropz, STYKO, suNny, mir, hutji, chopper } },
    { TournamentMap::Mirage, GroupStage, Fnatic, Astralis, { JW, Lekr0, KRIMZ, Golden, Xyp9x, gla1ve, Kjaerbye } },
    { TournamentMap::Mirage, GroupStage, Cloud9, VirtusPro, { Skadoodle, tarik, autimatic, Stewie2K, RUSH, Snax, TaZ, pashaBiceps } },
    { TournamentMap::Mirage, GroupStage, GambitEsports, Fnatic, { Hobbit, AdreN, JW, KRIMZ, flusha, Lekr0 } },
    { TournamentMap::Mirage, GroupStage, Cloud9, VegaSquadron, { Skadoodle, tarik, autimatic, Stewie2K, RUSH, jR, hutji } },
    { TournamentMap::Mirage, GroupStage, Mousesports, SpaceSoldiers, { chrisJ, oskar, ropz, STYKO, XANTARES, paz, ngiN, Calyx, MAJ3R } },
    { TournamentMap::Mirage, Quarterfinal, NatusVincere, QuantumBellatorFire, { s1mple, electronic, flamie, Edward, Boombl4, Kvik } },
    { TournamentMap::Mirage, Quarterfinal, G2Esports, Cloud9, { NBK, shox, bodyy, apEX, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
    { TournamentMap::Mirage, Quarterfinal, SKGaming, Fnatic, { FalleN, felps, fer, TACO, coldzera, JW, KRIMZ, flusha, Lekr0, Golden } },
    { TournamentMap::Mirage, Semifinal, NatusVincere, FaZeClan, { s1mple, electronic, Zeus, rain, olofmeister, GuardiaN, karrigan, NiKo } },
    { TournamentMap::Mirage, Semifinal, SKGaming, Cloud9, { coldzera, TACO, fer, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
    { TournamentMap::Mirage, GrandFinal, FaZeClan, Cloud9, { rain, olofmeister, GuardiaN, karrigan, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
    { TournamentMap::Nuke, GroupStage, NatusVincere, GambitEsports, { s1mple, flamie, Edward, AdreN, Dosia, Hobbit, mou, fitch } },
    { TournamentMap::Nuke, Quarterfinal, FaZeClan, Mousesports, { rain, olofmeister, GuardiaN, NiKo, chrisJ, oskar, ropz, STYKO, suNny } },
    { TournamentMap::Overpass, ChallengersStage, TeamLiquid, FaZeClan, { nitr0, Twistzz, EliGE, jdm64, rain, olofmeister, GuardiaN, karrigan, NiKo } },
    { TournamentMap::Overpass, ChallengersStage, Flipsid3Tactics, MisfitsGaming, { wayLander, WorldEdit, markeloff, B1ad3, devoduvek, AmaNEk, ShahZaM, SicK, seangares } },
    { TournamentMap::Overpass, ChallengersStage, G2Esports, VegaSquadron, { NBK, shox, bodyy, apEX, kennyS, mir, jR, keshandr, hutji, chopper } },
    { TournamentMap::Overpass, GroupStage, VegaSquadron, North, { mir, jR, keshandr, hutji, chopper, k0nfig, v4lde, aizy, cajunb } },
    { TournamentMap::Overpass, GroupStage, TeamLiquid, NatusVincere, { nitr0, Twistzz, EliGE, jdm64, s1mple, electronic, flamie, Zeus, Edward } },
    { TournamentMap::Overpass, GroupStage, SKGaming, GambitEsports, { FalleN, felps, fer, TACO, coldzera, AdreN, Dosia, Hobbit, mou, fitch } },
    { TournamentMap::Overpass, Quarterfinal, G2Esports, Cloud9, { NBK, bodyy, kennyS, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
    { TournamentMap::Overpass, Quarterfinal, SKGaming, Fnatic, { FalleN, felps, TACO, coldzera, JW, KRIMZ, flusha, Lekr0, Golden } },
    { TournamentMap::Overpass, GrandFinal, FaZeClan, Cloud9, { rain, olofmeister, GuardiaN, karrigan, NiKo, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
    { TournamentMap::Train, ChallengersStage, Cloud9, Mousesports, { Skadoodle, tarik, autimatic, RUSH, chrisJ, oskar } },
    { TournamentMap::Train, ChallengersStage, SpaceSoldiers, MisfitsGaming, { XANTARES, paz, ngiN, Calyx, MAJ3R, devoduvek, AmaNEk, ShahZaM, SicK, seangares } },
    { TournamentMap::Train, ChallengersStage, Avangar, Flipsid3Tactics, { qikert, buster, Jame, KrizzeN, seized, WorldEdit, markeloff, B1ad3 } },
    { TournamentMap::Train, ChallengersStage, NatusVincere, FaZeClan, { flamie, Zeus, Edward, rain, olofmeister, GuardiaN, karrigan, NiKo } },
    { TournamentMap::Train, ChallengersStage, QuantumBellatorFire, Avangar, { Boombl4, waterfaLLZ, jmqa, Kvik, balblna, buster, KrizzeN } },
    { TournamentMap::Train, GroupStage, FaZeClan, VegaSquadron, { rain, olofmeister, GuardiaN, karrigan, NiKo, mir, jR, hutji } },
    { TournamentMap::Train, GroupStage, Astralis, North, { dupreeh, device, Kjaerbye, gla1ve, Xyp9x, k0nfig, v4lde, aizy, cajunb, MSL } },
    { TournamentMap::Train, GroupStage, GambitEsports, SpaceSoldiers, { AdreN, Dosia, Hobbit, mou, fitch, XANTARES, paz, Calyx, MAJ3R } },
    { TournamentMap::Train, GroupStage, Mousesports, QuantumBellatorFire, { chrisJ, oskar, ropz, STYKO, Boombl4, waterfaLLZ, jmqa, Kvik, balblna } },
    { TournamentMap::Train, GroupStage, Astralis, Cloud9, { device, Xyp9x, dupreeh, Skadoodle, tarik, autimatic, Stewie2K, RUSH } },
});
static_assert(std::ranges::is_sorted(eleagueBoston2018Matches, {}, &Match::map));

constexpr auto faceitLondon2018Matches = std::to_array<Match>({
    // Challengers Stage

    // Round 1
    { TournamentMap::Inferno, ChallengersStage, Rogue, SpaceSoldiers, {} },
    { TournamentMap::Mirage, ChallengersStage, VirtusPro, NinjasInPyjamas, {} },
    { TournamentMap::Inferno, ChallengersStage, Tyloo, GambitEsports, {} },
    { TournamentMap::Overpass, ChallengersStage, BIG, Renegades, {} },
    { TournamentMap::Mirage, ChallengersStage, VegaSquadron, TeamSpirit, {} },
    { TournamentMap::Overpass, ChallengersStage, North, HellRaisers, {} },
    { TournamentMap::Mirage, ChallengersStage, TeamLiquid, OpTicGaming, {} },
    { TournamentMap::Inferno, ChallengersStage, Astralis, ComplexityGaming, {} },

});

constexpr auto tournaments = std::to_array<Tournament>({
    { 1, dreamHack2013Matches },
    { 3, emsOneKatowice2014Matches },
    { 4, eslOneCologne2014Matches },
    { 5, dreamHack2014Matches },
    { 6, eslOneKatowice2015Matches },
    { 7, eslOneCologne2015Matches },
    { 8, dreamHackClujNapoka2015Matches },
    { 9, mlgColumbus2016Matches },
    { 10, eslOneCologne2016Matches },
    { 11, eleagueAtlanta2017Matches },
    { 12, pglKrakow2017Matches },
    { 13, eleagueBoston2018Matches },
});

static_assert(std::ranges::is_sorted(tournaments, {}, &Tournament::tournamentID));

[[nodiscard]] static std::span<const Match> getTournamentMatches(std::uint32_t tournamentID) noexcept
{
    if (const auto it = std::ranges::lower_bound(tournaments, tournamentID, {}, &Tournament::tournamentID); it != tournaments.end() && it->tournamentID == tournamentID)
        return it->matches;
    assert(false && "Missing tournament match data!");
    return {};
}

static auto operator<=>(TournamentMap a, TournamentMap b) noexcept
{
    return static_cast<std::underlying_type_t<TournamentMap>>(a) <=> static_cast<std::underlying_type_t<TournamentMap>>(b);
}

[[nodiscard]] static std::span<const Match> filterMatchesToMap(std::span<const Match> matches, TournamentMap map) noexcept
{
    if (map == TournamentMap::None)
        return matches;

    assert(std::ranges::is_sorted(matches, {}, &Match::map));

    if (const auto [begin, end] = std::ranges::equal_range(matches, map, {}, &Match::map); begin != end)
        return { begin, end };

    assert(false && "Couldn't find a match played on a map of a souvenir package!");
    return {};
}

[[nodiscard]] static auto generateSouvenirPackageData(const StaticData::Case& caseData) noexcept
{
    assert(caseData.isSouvenirPackage());
    DynamicSouvenirPackageData dynamicData;

    if (const auto matches = getTournamentMatches(caseData.souvenirPackageTournamentID); !matches.empty()) {
        if (const auto matchesOnMap = filterMatchesToMap(matches, caseData.tournamentMap); !matchesOnMap.empty()) {
            const auto& randomMatch = matchesOnMap[Helpers::random(0, matchesOnMap.size() - 1)];
            dynamicData.tournamentStage = randomMatch.stage;
            dynamicData.tournamentTeam1 = randomMatch.team1;
            dynamicData.tournamentTeam2 = randomMatch.team2;
            dynamicData.proPlayer = randomMatch.getRandomMVP();
        }
    }

    return dynamicData;
}

std::size_t ItemGenerator::createDefaultDynamicData(std::size_t gameItemIndex) noexcept
{
    std::size_t index = Inventory::INVALID_DYNAMIC_DATA_IDX;

    if (const auto& item = StaticData::gameItems()[gameItemIndex]; item.isSkin()) {
        const auto& staticData = StaticData::paintKits()[item.dataIndex];
        DynamicSkinData dynamicData;
        dynamicData.wear = std::lerp(staticData.wearRemapMin, staticData.wearRemapMax, Helpers::random(0.0f, 0.07f));
        dynamicData.seed = Helpers::random(1, 1000);
        index = Inventory::emplaceDynamicData(std::move(dynamicData));
    } else if (item.isGlove()) {
        const auto& staticData = StaticData::paintKits()[item.dataIndex];
        DynamicGloveData dynamicData;
        dynamicData.wear = std::lerp(staticData.wearRemapMin, staticData.wearRemapMax, Helpers::random(0.0f, 0.07f));
        dynamicData.seed = Helpers::random(1, 1000);
        index = Inventory::emplaceDynamicData(std::move(dynamicData));
    } else if (item.isAgent()) {
        index = Inventory::emplaceDynamicData(DynamicAgentData{});
    } else if (item.isMusic()) {
        index = Inventory::emplaceDynamicData(DynamicMusicData{});
    } else if (item.isCase()) {
        if (const auto& staticData = StaticData::cases()[item.dataIndex]; staticData.isSouvenirPackage())
            index = Inventory::emplaceDynamicData(generateSouvenirPackageData(staticData));
    }

    return index;
}

[[nodiscard]] static const Match* findTournamentMatch(std::uint32_t tournamentID, TournamentMap map, TournamentStage stage, TournamentTeam team1, TournamentTeam team2) noexcept
{
    const auto matches = getTournamentMatches(tournamentID);
    if (matches.empty())
        return nullptr;

    const auto matchesOnMap = filterMatchesToMap(matches, map);
    if (matchesOnMap.empty())
        return nullptr;

    const auto match = std::ranges::find_if(matchesOnMap, [stage, team1, team2](const auto& match) { return match.stage == stage && match.team1 == team1 && match.team2 == team2; });
    return (match != matchesOnMap.end() ? &*match : nullptr);
}

[[nodiscard]] static std::array<StickerConfig, 5> generateSouvenirStickers(std::uint32_t tournamentID, TournamentMap map, TournamentStage stage, TournamentTeam team1, TournamentTeam team2, ProPlayer player) noexcept
{
    std::array<StickerConfig, 5> stickers;

    stickers[0].stickerID = StaticData::findSouvenirTournamentSticker(tournamentID);

    if (tournamentID != 1) {
        stickers[1].stickerID = StaticData::getTournamentTeamGoldStickerID(tournamentID, team1);
        stickers[2].stickerID = StaticData::getTournamentTeamGoldStickerID(tournamentID, team2);
        if (const auto match = findTournamentMatch(tournamentID, map, stage, team1, team2); match && match->hasMVPs())
            stickers[3].stickerID = StaticData::getTournamentPlayerGoldStickerID(tournamentID, static_cast<int>(player));
    }

    std::mt19937 gen{ std::random_device{}() };
    std::shuffle(stickers.begin(), stickers.begin() + 4, gen);

    return stickers;
}
