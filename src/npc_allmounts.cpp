/*
 * NPC - All Mounts (Premium Mount Shop)
 *
 * Tienda de monturas configurable desde MySQL (`premium_mount_shop`).
 * Compatible con AzerothCore 3.3.5 (WotLK).
 *
 * v2026.06.13 - Reestructuración completa: tienda por categorías con paginación.
 */

#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// Configuración global del módulo (cargada desde npc_allmounts.conf)
// =============================================================================

bool AllMountsAnnounceModule;
bool AllMountsEnableAI;
uint32 AllMountsCostItemId;
std::string AllMountsCostItemName;
uint32 AllMountsNonPremiumMultiplier;
uint32 AllMountsPageSize;

// =============================================================================
// Sistema premium: cuentas con active=1 en la tabla `premium` (Character DB)
// =============================================================================

namespace
{
bool IsPremiumAccount(Player const* player)
{
    if (!player || !player->GetSession())
        return false;

    QueryResult result = CharacterDatabase.Query(
        "SELECT `AccountId` FROM `premium` WHERE `active`=1 AND `AccountId`={}",
        player->GetSession()->GetAccountId());

    return result != nullptr;
}

std::string const& CostItemDisplayName()
{
    static std::string const fallback = "objetos";
    return AllMountsCostItemName.empty() ? fallback : AllMountsCostItemName;
}

uint32 CalculateMountCost(uint32 premiumCost, Player const* player)
{
    if (IsPremiumAccount(player))
        return premiumCost;

    return premiumCost * AllMountsNonPremiumMultiplier;
}
} // namespace

// =============================================================================
// Datos de la tienda cargados desde `premium_mount_shop` (World DB)
// =============================================================================

struct MountShopEntry
{
    uint32 id;
    std::string category;
    uint32 spellId;
    std::string name;
    uint32 premiumCost;
    bool enabled;
};

class MountShopData
{
public:
    static MountShopData* instance()
    {
        static MountShopData instance;
        return &instance;
    }

    // Crea la tabla si no existe (instalación automática al arrancar el worldserver)
    void EnsureTableExists()
    {
        WorldDatabase.DirectExecute(
            "CREATE TABLE IF NOT EXISTS `premium_mount_shop` ("
            "    `id` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,"
            "    `category` VARCHAR(50) NOT NULL DEFAULT 'General',"
            "    `spellid` INT UNSIGNED NOT NULL,"
            "    `name` VARCHAR(100) NOT NULL,"
            "    `premium_cost` INT UNSIGNED NOT NULL DEFAULT 1,"
            "    `enabled` TINYINT(1) NOT NULL DEFAULT 1,"
            "    UNIQUE KEY (`spellid`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");
    }

    // Recarga todas las monturas habilitadas y reconstruye la lista de categorías
    void Load()
    {
        _entries.clear();
        _categories.clear();
        _entryById.clear();

        QueryResult result = WorldDatabase.Query(
            "SELECT `id`, `category`, `spellid`, `name`, `premium_cost`, `enabled` "
            "FROM `premium_mount_shop` "
            "WHERE `enabled` = 1 "
            "ORDER BY `category`, `name`");

        if (!result)
            return;

        do
        {
            Field* fields = result->Fetch();

            MountShopEntry entry;
            entry.id = fields[0].Get<uint32>();
            entry.category = fields[1].Get<std::string>();
            entry.spellId = fields[2].Get<uint32>();
            entry.name = fields[3].Get<std::string>();
            entry.premiumCost = fields[4].Get<uint32>();
            entry.enabled = fields[5].Get<bool>();

            _entries.push_back(entry);
            _entryById[entry.id] = entry;
        } while (result->NextRow());

        RebuildCategories();
    }

    std::vector<std::string> const& GetCategories() const
    {
        return _categories;
    }

    MountShopEntry const* GetEntry(uint32 id) const
    {
        auto itr = _entryById.find(id);
        if (itr == _entryById.end())
            return nullptr;

        return &itr->second;
    }

    // Monturas de una categoría que el jugador aún no conoce
    std::vector<MountShopEntry const*> GetAvailableMounts(Player const* player, std::string const& category) const
    {
        std::vector<MountShopEntry const*> mounts;

        for (MountShopEntry const& entry : _entries)
        {
            if (entry.category != category)
                continue;

            if (player && player->HasSpell(entry.spellId))
                continue;

            mounts.push_back(&entry);
        }

        return mounts;
    }

private:
    void RebuildCategories()
    {
        std::vector<std::string> seen;

        for (MountShopEntry const& entry : _entries)
        {
            if (std::find(seen.begin(), seen.end(), entry.category) == seen.end())
                seen.push_back(entry.category);
        }

        _categories = std::move(seen);
    }

    std::vector<MountShopEntry> _entries;
    std::vector<std::string> _categories;
    std::unordered_map<uint32, MountShopEntry> _entryById;
};

#define sMountShop MountShopData::instance()

// =============================================================================
// Codificación de gossip: senders y acciones para navegación de la tienda
// =============================================================================

namespace MountShopGossip
{
enum Sender : uint32
{
    Main     = GOSSIP_SENDER_MAIN,
    Category = GOSSIP_SENDER_MAIN + 1,
    BuyMount = GOSSIP_SENDER_MAIN + 2,
    PrevPage = GOSSIP_SENDER_MAIN + 3,
    NextPage = GOSSIP_SENDER_MAIN + 4,
    Back     = GOSSIP_SENDER_MAIN + 5,
};

inline uint32 EncodePageAction(uint32 categoryIndex, uint32 page)
{
    return (categoryIndex << 16) | (page & 0xFFFF);
}

inline void DecodePageAction(uint32 action, uint32& categoryIndex, uint32& page)
{
    categoryIndex = action >> 16;
    page = action & 0xFFFF;
}

std::string FormatMountLabel(MountShopEntry const& entry, Player const* player)
{
    uint32 cost = CalculateMountCost(entry.premiumCost, player);
    std::ostringstream label;
    label << entry.name << " (" << cost << " " << CostItemDisplayName() << ")";
    return label.str();
}
} // namespace MountShopGossip

// =============================================================================
// WorldScript: carga de configuración, creación de tabla y datos de la tienda
// =============================================================================

class AllMountsConfig : public WorldScript
{
public:
    AllMountsConfig() : WorldScript("AllMountsConfig_conf", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP
    }) {}

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload)
        {
            AllMountsAnnounceModule = sConfigMgr->GetOption<bool>("AllMountsNPC.Announce", true);
            AllMountsEnableAI = sConfigMgr->GetOption<bool>("AllMountsNPC.EnableAI", true);
            AllMountsCostItemId = sConfigMgr->GetOption<uint32>("AllMountsNPC.CostItemId", 29736);
            AllMountsCostItemName = sConfigMgr->GetOption<std::string>("AllMountsNPC.CostItemName", "Runa Arcana");
            AllMountsNonPremiumMultiplier = sConfigMgr->GetOption<uint32>("AllMountsNPC.NonPremiumMultiplier", 5);
            AllMountsPageSize = sConfigMgr->GetOption<uint32>("AllMountsNPC.PageSize", 15);

            if (AllMountsPageSize == 0)
                AllMountsPageSize = 15;
        }
    }

    void OnStartup() override
    {
        sMountShop->EnsureTableExists();
        sMountShop->Load();
    }
};

// =============================================================================
// PlayerScript: anuncio opcional al iniciar sesión
// =============================================================================

class AllMountsAnnounce : public PlayerScript
{
public:
    AllMountsAnnounce() : PlayerScript("AllMountsAnnounce", {
        PLAYERHOOK_ON_LOGIN
    }) {}

    void OnPlayerLogin(Player* player) override
    {
        if (AllMountsAnnounceModule)
            ChatHandler(player->GetSession()).SendSysMessage("Este servidor utiliza el módulo |cff4CFF00AllMountsNPC|r.");
    }
};

// =============================================================================
// Lógica de gossip compartida: categorías, listado paginado y compra
// =============================================================================

namespace MountShopGossipLogic
{
void ShowCategories(Player* player, Creature* creature)
{
    player->PlayerTalkClass->ClearMenus();

    std::vector<std::string> const& categories = sMountShop->GetCategories();

    if (categories.empty())
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "No hay monturas disponibles en este momento.", MountShopGossip::Main, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 601014, creature->GetGUID());
        return;
    }

    for (uint32 i = 0; i < categories.size(); ++i)
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, categories[i], MountShopGossip::Category, i);

    SendGossipMenuFor(player, 601014, creature->GetGUID());
}

void ShowMountPage(Player* player, Creature* creature, uint32 categoryIndex, uint32 page)
{
    player->PlayerTalkClass->ClearMenus();

    std::vector<std::string> const& categories = sMountShop->GetCategories();
    if (categoryIndex >= categories.size())
    {
        ShowCategories(player, creature);
        return;
    }

    std::string const& category = categories[categoryIndex];
    std::vector<MountShopEntry const*> mounts = sMountShop->GetAvailableMounts(player, category);

    if (mounts.empty())
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Volver", MountShopGossip::Back, MountShopGossip::EncodePageAction(categoryIndex, page));
        SendGossipMenuFor(player, 601014, creature->GetGUID());
        return;
    }

    uint32 const pageSize = AllMountsPageSize;
    uint32 const totalPages = (mounts.size() + pageSize - 1) / pageSize;

    if (page >= totalPages)
        page = totalPages - 1;

    uint32 const start = page * pageSize;
    uint32 const end = std::min(start + pageSize, static_cast<uint32>(mounts.size()));

    for (uint32 i = start; i < end; ++i)
    {
        MountShopEntry const* entry = mounts[i];
        AddGossipItemFor(
            player,
            GOSSIP_ICON_MONEY_BAG,
            MountShopGossip::FormatMountLabel(*entry, player),
            MountShopGossip::BuyMount,
            entry->id);
    }

    uint32 const encodedPage = MountShopGossip::EncodePageAction(categoryIndex, page);

    if (page > 0)
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Página anterior", MountShopGossip::PrevPage, encodedPage);

    if (page + 1 < totalPages)
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Página siguiente >>", MountShopGossip::NextPage, encodedPage);

    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Volver", MountShopGossip::Back, encodedPage);

    SendGossipMenuFor(player, 601014, creature->GetGUID());
}

bool TryPurchaseMount(Player* player, Creature* creature, uint32 shopId)
{
    MountShopEntry const* entry = sMountShop->GetEntry(shopId);
    if (!entry)
    {
        ChatHandler(player->GetSession()).SendSysMessage("Esa montura ya no está disponible.");
        CloseGossipMenuFor(player);
        return true;
    }

    if (player->HasSpell(entry->spellId))
    {
        ChatHandler(player->GetSession()).SendSysMessage("Ya conoces esta montura.");
        CloseGossipMenuFor(player);
        return true;
    }

    uint32 const cost = CalculateMountCost(entry->premiumCost, player);

    if (AllMountsCostItemId > 0 && cost > 0)
    {
        if (!player->HasItemCount(AllMountsCostItemId, cost, false))
        {
            std::ostringstream needMsg;
            needMsg << "Necesitas " << cost << "x " << CostItemDisplayName()
                    << " para comprar " << entry->name << ".";
            ChatHandler(player->GetSession()).SendSysMessage(needMsg.str());
            CloseGossipMenuFor(player);
            return true;
        }

        player->DestroyItemCount(AllMountsCostItemId, cost, true, false);
    }

    player->learnSpell(entry->spellId);

    std::ostringstream successMsg;
    successMsg << "Has aprendido " << entry->name << ".";
    ChatHandler(player->GetSession()).SendSysMessage(successMsg.str());

    CloseGossipMenuFor(player);
    return true;
}
} // namespace MountShopGossipLogic

// =============================================================================
// CreatureScript: NPC vendedor de monturas
// =============================================================================

class All_Mounts_NPC : public CreatureScript
{
public:
    All_Mounts_NPC() : CreatureScript("All_Mounts_NPC") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        MountShopGossipLogic::ShowCategories(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (sender)
        {
            case MountShopGossip::Main:
                CloseGossipMenuFor(player);
                return true;

            case MountShopGossip::Category:
                MountShopGossipLogic::ShowMountPage(player, creature, action, 0);
                return true;

            case MountShopGossip::BuyMount:
                return MountShopGossipLogic::TryPurchaseMount(player, creature, action);

            case MountShopGossip::PrevPage:
            case MountShopGossip::NextPage:
            {
                uint32 categoryIndex = 0;
                uint32 page = 0;
                MountShopGossip::DecodePageAction(action, categoryIndex, page);

                if (sender == MountShopGossip::PrevPage && page > 0)
                    --page;
                else if (sender == MountShopGossip::NextPage)
                    ++page;

                MountShopGossipLogic::ShowMountPage(player, creature, categoryIndex, page);
                return true;
            }

            case MountShopGossip::Back:
            {
                uint32 categoryIndex = 0;
                uint32 page = 0;
                MountShopGossip::DecodePageAction(action, categoryIndex, page);
                (void)page;
                (void)categoryIndex;
                MountShopGossipLogic::ShowCategories(player, creature);
                return true;
            }

            default:
                return false;
        }
    }

    // =============================================================================
    // IA pasiva: emotes ambientales del vendedor
    // =============================================================================

    struct NPC_PassiveAI : public ScriptedAI
    {
        NPC_PassiveAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 Choice;
        uint32 MessageTimer;

        void Reset() override
        {
            MessageTimer = urand(60000, 180000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!AllMountsEnableAI)
                return;

            if (MessageTimer > diff)
            {
                MessageTimer -= diff;
                return;
            }

            Choice = urand(1, 3);

            switch (Choice)
            {
                case 1:
                    me->Say("Explora mis establos: las mejores monturas de Azeroth te esperan.", LANG_UNIVERSAL);
                    break;
                case 2:
                    me->Say("Los jinetes premium disfrutan de mejores precios en cada montura.", LANG_UNIVERSAL);
                    break;
                default:
                    me->Say("Elige una categoría y encuentra tu montura perfecta.", LANG_UNIVERSAL);
                    break;
            }

            me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            MessageTimer = urand(60000, 180000);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new NPC_PassiveAI(creature);
    }
};

// =============================================================================
// Registro de scripts del módulo
// =============================================================================

void AddAllMountsNPCScripts()
{
    new AllMountsConfig();
    new AllMountsAnnounce();
    new All_Mounts_NPC();
}
