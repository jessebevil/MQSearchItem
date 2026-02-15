// MQSearchItem.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup.

// ReSharper disable CppClangTidyReadabilityEnumInitialValue
// ReSharper disable CppClangTidyBugproneBranchClone
#include <mq/Plugin.h>
#include <mq/imgui/Widgets.h>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <ranges>

// Saved Searches helpers forward declarations (global scope)
namespace fs = std::filesystem;
static std::string SanitizeFileName(const std::string& name);
static std::string GetSavedSearchesDir();
static std::string GetSearchFilePath(const std::string& name);
static void EnsureSavedSearchDir();
static bool SaveCurrentSearchToFile(const std::string& path);
static bool LoadSearchFromFile(const std::string& path);
static bool DeleteSearchFile(const std::string& path);
static std::vector<std::string> ListSavedSearches();

PreSetup("MQSearchItem");
PLUGIN_VERSION(0.1);
//#define DEBUGGING

static void OutPutItemDetails(ItemPtr pItem, int topSlotIndex, int bagSlotIndex, const char* hostName, bool isAug, int augSlotIndex);

struct Option {
	std::string Name;
	bool IsSelected = false;
	uint8_t ID;
	Option(std::string Name, const uint8_t ID) :Name(std::move(Name)), ID(ID) {}
};

/*
* Do not under any circumstances re-order the enums unless they change in EQ.
* These are in order so that the options line up with the results and they can
* be compared in a loop!
*
* If you want to change their order in the list in the window (visible order)
* then change the order in the std::map<OptionType, DropDownOption> MenuData
*/

enum LocationID : uint8_t {
	Loc_Bank,
	Loc_Shared_Bank,
	Loc_Equipped,
	Loc_Bags,
	Loc_Real_Estate,
	Loc_Item_Overflow,
	Loc_Parcel,
#if (!IS_EMU_CLIENT)
	Loc_TradeSkillDepot,
	Loc_DragonsHorde
#endif
};

enum SlotID : uint8_t {
	Slot_Charm,
	Slot_LeftEar,
	Slot_Head,
	Slot_Face,
	Slot_RightEar,
	Slot_Neck,
	Slot_Shoulders,
	Slot_Arms,
	Slot_Back,
	Slot_LeftWrist,
	Slot_RightWrist,
	Slot_Range,
	Slot_Hands,
	Slot_Primary,
	Slot_Secondary,
	Slot_LeftFinger,
	Slot_RightFinger,
	Slot_Chest,
	Slot_Legs,
	Slot_Feet,
	Slot_Waist,
	Slot_PowerSource,
	Slot_Ammo,
};

#ifdef DEBUGGING
static void TestFunc(ItemClient* pItem) {
	// ItemDefinition* pItemDef = pItem->GetItemDefinition();
	//
	// pItemDef->Cost;
	// //pItemDef->eGMRequirement;
	// pItemDef->bPoofOnDeath;

#if (IS_EXPANSION_LEVEL(EXPANSION_LEVEL_SOR))
	pItemDef->Collectible;
#endif

	// /*Is instrument?*/
	// pItemDef->InstrumentType;
	// pItemDef->InstrumentMod;
	//
	// /*Should we add a diety filter?*/
	// pItemDef->Deity;//Definitely going to be bitmasked
	//
	// /*To search for magic items*/
	// pItemDef->Magic;
	//
	// /*Delay maybe instead we should do ratio?*/
	// pItemDef->Delay;
	//
	// pItemDef->Prestige;//This is for prestiege or not. I spelled that wrong. I know.
	//
	//
	// pItemDef->ItemClass;//WTF is this?
	//
	// pItemDef->AugRestrictions;//We'll need to review this later - I believe it's an enum that's unmapped.
	//
	// //Unclear what to do with this information at the moment - Perhaps we want to find things with faction mods
	// //Need more details
	// pItemDef->FactionModType[0x4];
	// pItemDef->FactionModValue[0x4];
	// pItemDef->SpellData;//Should be used in ItemType to find items with a spell attached to it I think.
	// pItemDef->Favor;
	// pItemDef->GuildFavor;
	// pItemDef->StackSize;//Going to use this for types?
	// pItemDef->TrophyBenefitID;//Trophy - Type? Favor usage for tribute?

}
#endif

enum StatID : uint8_t {
	Stat_AC,
	Stat_Accuracy,
	Stat_AGI,
	Stat_Attack,
	Stat_Attunable,
	Stat_Avoidance,
	Stat_BackstabDamage,
	Stat_CHA,
	Stat_Clairvoyance,
	Stat_CombatEffects,
	Stat_Damage,
	Stat_DamageShield,
	Stat_DEX,
	Stat_DmgBonus,//Use DmgBonusValue
	Stat_DoTShielding,
	Stat_DSMitigation,
	Stat_Efficiency,
	Stat_ElementalDamage,
	Stat_Endurance,
	Stat_EnduranceRegen,
	Stat_Favor,
	Stat_GuildFavor,
	Stat_Haste,
	Stat_HealAmount,
	Stat_Heirloom,
	Stat_HeroicAgi,
	Stat_HeroicCha,
	Stat_HeroicCorruption,
	Stat_HeroicDex,
	Stat_HeroicInt,
	Stat_HeroicPoison,
	Stat_HeroicSta,
	Stat_HeroicStr,
	Stat_HeroicSvCold,
	Stat_HeroicSvDisease,
	Stat_HeroicSvFire,
	Stat_HeroicSvMagic,
	Stat_HeroicWis,
	Stat_HP,
	Stat_HPRegen,
	Stat_INT,
	Stat_Mana,
	Stat_ManaRegen,
	Stat_Purity,
	Stat_Range,
	Stat_Ratio,
	Stat_Shielding,
	Stat_SpellDamage,
	Stat_SpellShield,
	Stat_STA,
	Stat_STR,
	Stat_StrikeThrough,
	Stat_StunResist,
	Stat_Summoned,//Pet items?
	Stat_SvCold,
	Stat_SvCorruption,
	Stat_SvDisease,
	Stat_SvFire,
	Stat_SvMagic,
	Stat_SvPoison,
	Stat_TradeSkills,//?
	Stat_Weight,
	Stat_WIS,

};

enum RaceID : uint8_t {
	Race_Human,
	Race_Barbarian,
	Race_Erudite,
	Race_WoodElf,
	Race_HighElf,
	Race_DarkElf,
	Race_HalfElf,
	Race_Dwarf,
	Race_Troll,
	Race_Ogre,
	Race_Halfling,
	Race_Gnome,
	Race_Iksar,
	Race_VahShir,
	Race_Froglok,
	Race_Drakkin
};

#ifdef DEBUGGING
static const char* szPlayerRaces[] = {
	"None",
	"Human",
	"Barbarian",
	"Erudite",
	"WoodElf",
	"HighElf",
	"DarkElf",
	"HalfElf",
	"Dwarf",
	"Troll",
	"Ogre",
	"Halfling",
	"Gnome",
	"Iksar",
	"VahShir",
	"Froglok",
	"Drakkin"
};
#endif

enum ClassID : uint8_t {
	Class_Warrior,
	Class_Cleric,
	Class_Paladin,
	Class_Ranger,
	Class_ShadowKnight,
	Class_Druid,
	Class_Monk,
	Class_Bard,
	Class_Rogue,
	Class_Shaman,
	Class_Necromancer,
	Class_Wizard,
	Class_Magician,
	Class_Enchanter,
	Class_Beastlord,
	Class_Berserker
};

enum DeityID : uint8_t {
	Deity_Agnostic,
	Deity_Bertoxxulous,
	Deity_Brell,
	Deity_Cazic,
	Deity_Erollisi,
	Deity_Bristlebane,
	Deity_Innoruuk,
	Deity_Karana,
	Deity_Mithaniel,
	Deity_Prexus,
	Deity_Quellious,
	Deity_Rallos,
	Deity_Rodcet,
	Deity_Solusek,
	Deity_Tribunal,
	Deity_Tunare,
	Deity_Veeshan
};

enum ItemTypeID : uint8_t {
	ItemType_1H_Slashing,//
	ItemType_2H_Slashing,//
	ItemType_1H_Piercing,//
	ItemType_1H_Blunt,//
	ItemType_2H_Blunt,//
	ItemType_Bow,//
	ItemType_Crossbow,//Won't Add to filters - Seems pointless?
	ItemType_Throwing,//
	ItemType_Shield,//
	ItemType_Spell,//Test Me
	ItemType_Armor,//
	ItemType_Misc,//
	ItemType_Lockpicks,//Test Me
	ItemType_Fist,//Test Me
	ItemType_Food,//
	ItemType_Drink,//
	ItemType_Light,//Not searching for this - Kinda pointless (but it does work)
	ItemType_Combinable,//
	ItemType_Bandage,//
	ItemType_Ammo,//
	ItemType_Scroll,//Test Me
	ItemType_Potion,//
	ItemType_Skill,//Test Me
	ItemType_Wind_Instrument,//
	ItemType_Stringed_Instrument,//
	ItemType_Brass_Instruments,//
	ItemType_Percussion_Instrument,//
	ItemType_Arrow,//
	ItemType_Bolt,//Crossbow Ammo - Not searching for this.
	ItemType_Jewelry,//
	ItemType_Artifact,//Test Me
	ItemType_Book,//
	ItemType_Note,//Test Me
	ItemType_Key,//
	ItemType_Ticket,// - No idea what this is for? Perhaps Shadowhaven gamble ticket?
	ItemType_2H_Piercing,//
	ItemType_FishingPole,//Test Me
	ItemType_FishingBait,//Test Me
	ItemType_Alcohol,//
	ItemType_House_Key,//Test Me - No Idea what it matches
	ItemType_Compass,//Works, but pretty pointless.
	ItemType_Metal_Key,//Test Me - Again, no idea what it matches
	ItemType_Poison,//Test Me
	ItemType_MagicArrow,//Test Me
	ItemType_MagicBolt,//Not going to use this one.
	ItemType_Martial,//
	ItemType_NotReal_HasEffects,
	ItemType_NotReal_Haste_Item,
	ItemType_NotReal_Has_FT,
	ItemType_NotReal_HasFocus,
	ItemType_Singing_Instrument,//
	ItemType_All_Instrument,//Test Me - Singing Short Sword?
	ItemType_Charm,//
	ItemType_ArmorDye,//
	ItemType_Augmentation,//
	ItemType_AugDestroySolvent,//
	ItemType_AugRemoveDistiller,//
	ItemType_AltAbility,//Ancient Cloak of Flames? - Maybe doesn't apply to emu.
	ItemType_GuildBanner,
	ItemType_BannerModifyToken,
	ItemType_RecipeBook,
	ItemType_VoluntarySpellCastBook,
	ItemType_SpellCastBook,
	ItemType_PointCurrency,
	ItemType_PerfectedDistiller,
	ItemType_NotReal_Placable,
	ItemType_NotReal_Collectible,
	ItemType_NotReal_Container,
	ItemType_Mount,
	ItemType_Illusion,
	ItemType_Familiar,//Last "real" ItemType

//#if (!IS_EMU_CLIENT)
	ItemType_All_Effects,//Will have to combine all the look ups for effects?
	ItemType_Collectible,//pItem->Collectible
	ItemType_Container,//pItem->IsContainer
	ItemType_Focus_Effect,//pItem->Focus stuff
	ItemType_Placeable,//pItem->Placable

//#endif
//Should probably add these to type.
// Option("Attunable", Stat_Attuneable),//ItemType?
//Option("Heirloom", Stat_Heirloom),//Both relevant and not - Itemtype?
//Option("Summoned", Stat_Summoned),//ItemType?
//Option("Tradeskill", Stat_TradeSkills),//Probably doesn't belong here. This is an itemtype question.

	//Custom Options I'm Adding perhaps:
	//Clicky
	//Worn Effect

	ItemType_None = 255
};

#if (!IS_EMU_CLIENT)
enum PrestigeID {
	Prestige_Prestige_Only,
	Prestige_Non_Prestige_Only
};
#endif

enum AugSlotID : uint8_t {
	Aug_1,
	Aug_2,
	Aug_3,
	Aug_4,
	Aug_5,
	Aug_6,
	Aug_7,
	Aug_8,
	Aug_9,
	Aug_10,
	Aug_11,
	Aug_12,
	Aug_13,
	Aug_14,
	Aug_15,
	Aug_16,
	Aug_17,
	Aug_20,
	Aug_21
};

struct DropDownOption {
	std::vector<Option> OptionList;
};

enum OptionType : uint8_t {
	OptionType_Location,
	OptionType_Slots,
	OptionType_Stats,
	OptionType_Race,
	OptionType_Class,
	OptionType_ItemType,
	OptionType_Deity,
#if (!IS_EMU_CLIENT)
	OptionType_Prestige,
#endif
	OptionType_AugSlots
};

static std::map<OptionType, std::string> DropDownOptions{
	{OptionType_Location, "Location"},
	{OptionType_Slots, "Slots"},
	{OptionType_Stats, "Stats"},
	{OptionType_Race, "Races"},
	{OptionType_Class, "Classes"},
	{OptionType_ItemType, "Types"},
	{OptionType_Deity, "Deity"},
#if (!IS_EMU_CLIENT)
	{OptionType_Prestige, "Prestige"},
#endif
	{OptionType_AugSlots, "AugSlots"}
};

static std::map<OptionType, DropDownOption> MenuData = {
	{OptionType_Location, { {
			Option("Bank", Loc_Bank),
			Option("Shared Bank", Loc_Shared_Bank),
			Option("Equipped", Loc_Equipped),
			Option("Bags", Loc_Bags),
			Option("Real Estates", Loc_Real_Estate),
			Option("Item Overflow", Loc_Item_Overflow),
			Option("Parcel", Loc_Parcel),
#if (!IS_EMU_CLIENT)//Dragon Horde also?
			Option("TradeSkill Depot", Loc_TradeSkillDepot),
			Option("Dragons Horde", Loc_DragonsHorde)
#endif
	} } },

	{ OptionType_Slots, { {
			Option("Ammo", Slot_Ammo),
			Option("Arms", Slot_Arms),
			Option("Back", Slot_Back),
			Option("Charm", Slot_Charm),
			Option("Chest", Slot_Chest),
			Option("Ear", Slot_LeftEar),
			//Option("Ear2", Slot_RightEar),
			Option("Face", Slot_Face),
			Option("Feet", Slot_Feet),
			Option("Fingers", Slot_LeftFinger),
			//Option("Fingers2", Slot_RightFinger),
			Option("Hands", Slot_Hands),
			Option("Head", Slot_Head),
			Option("Legs", Slot_Legs),
			Option("Neck", Slot_Neck),
			Option("Power Source", Slot_PowerSource),
			Option("Primary", Slot_Primary),
			Option("Range", Slot_Range),
			Option("Secondary", Slot_Secondary),
			Option("Shoulders", Slot_Shoulders),
			Option("Waist", Slot_Waist),
			Option("Wrist", Slot_LeftWrist),
			//Option("Wrist2", Slot_RightWrist),
	} } },

	{ OptionType_Stats, { {
			Option("Armor Class", Stat_AC),
			Option("Accuracy", Stat_Accuracy),
			Option("Agility", Stat_AGI),
			Option("Attack", Stat_Attack),
			Option("Avoidance", Stat_Avoidance),
			Option("Backstab Damage", Stat_BackstabDamage),
			Option("Charisma", Stat_CHA),
			Option("Clairvoyance", Stat_Clairvoyance),
			Option("Combat Effects", Stat_CombatEffects),
			Option("Damage", Stat_Damage),
			Option("Damage Shield", Stat_DamageShield),
			Option("Dexterity", Stat_DEX),
			Option("Damage Bonus", Stat_DmgBonus),
			Option("DoT Shielding", Stat_DoTShielding),
			Option("DS Mitigation", Stat_DSMitigation),
			Option("Efficiency", Stat_Efficiency),
			Option("Elemental Dmg", Stat_ElementalDamage),
			Option("Endurance", Stat_Endurance),
			Option("Endurance Regen", Stat_EnduranceRegen),
			Option("Tribute Value", Stat_Favor),
			Option("Guild Tribute Value", Stat_GuildFavor),
			Option("Haste", Stat_Haste),
			Option("Heal Amount", Stat_HealAmount),
			Option("Heroic AGI", Stat_HeroicAgi),
			Option("Heroic CHA", Stat_HeroicCha),
			Option("Heroic Sv Corruption", Stat_HeroicCorruption),
			Option("Heroic Dex", Stat_HeroicDex),
			Option("Heroic Int", Stat_HeroicInt),
			Option("Heroic Sv Poison", Stat_HeroicPoison),
			Option("Heroic STA", Stat_HeroicSta),
			Option("Heroic STR", Stat_HeroicStr),
			Option("Heroic SvCold", Stat_HeroicSvCold),
			Option("Heroic SvDisease", Stat_HeroicSvDisease),
			Option("Heroic SvFire", Stat_HeroicSvFire),
			Option("Heroic SvMagic", Stat_HeroicSvMagic),
			Option("Heroic WIS", Stat_HeroicWis),
			Option("Hitpoints", Stat_HP),
			Option("HP Regen", Stat_HPRegen),
			Option("INT", Stat_INT),
			Option("Mana", Stat_Mana),
			Option("Mana Regen", Stat_ManaRegen),
			Option("Purity", Stat_Purity),
			Option("Range", Stat_Range),
			Option("Ratio", Stat_Ratio),
			Option("Shielding", Stat_Shielding),
			Option("Spell Damage", Stat_SpellDamage),
			Option("Spell Shielding", Stat_SpellShield),
			Option("Stamina", Stat_STA),
			Option("Strength", Stat_STR),
			Option("StrikeThrough", Stat_StrikeThrough),
			Option("Stun Resist", Stat_StunResist),
			Option("Sv Cold", Stat_SvCold),
			Option("Sv Corruption", Stat_SvCorruption),
			Option("Sv Disease", Stat_SvDisease),
			Option("Sv Fire", Stat_SvFire),
			Option("Sv Magic", Stat_SvMagic),
			Option("Sv Poison", Stat_SvPoison),
			Option("Weight", Stat_Weight),
			Option("Wisdom", Stat_WIS),
	} } },

	{ OptionType_Race, { {
			Option("Barbarian", Race_Barbarian),
			Option("Dark Elf", Race_DarkElf),
			Option("Drakkin", Race_Drakkin),
			Option("Dwarf", Race_Dwarf),
			Option("Erudite", Race_Erudite),
			Option("Froglok", Race_Froglok),
			Option("Gnome", Race_Gnome),
			Option("Half Elf", Race_HalfElf),
			Option("Halfling", Race_Halfling),
			Option("High Elf", Race_HighElf),
			Option("Human", Race_Human),
			Option("Iksar", Race_Iksar),
			Option("Ogre", Race_Ogre),
			Option("Troll", Race_Troll),
			Option("Vah Shir", Race_VahShir),
			Option("Wood Elf", Race_WoodElf)
	} } },

	{ OptionType_Class, { {
			Option("Bard", Class_Bard),
			Option("Beastlord", Class_Beastlord),
			Option("Berserker", Class_Berserker),
			Option("Cleric", Class_Cleric),
			Option("Druid", Class_Druid),
			Option("Enchanter", Class_Enchanter),
			Option("Magician", Class_Magician),
			Option("Monk", Class_Monk),
			Option("Necromancer", Class_Necromancer),
			Option("Paladin", Class_Paladin),
			Option("Ranger", Class_Ranger),
			Option("Rogue", Class_Rogue),
			Option("Shadow Knight", Class_ShadowKnight),
			Option("Shaman", Class_Shaman),
			Option("Warrior", Class_Warrior),
			Option("Wizard", Class_Wizard)
	} } },

	{ OptionType_ItemType, { {
			Option("1H Blunt", ItemType_1H_Blunt),
			Option("1H Piercing", ItemType_1H_Piercing),
			Option("1H Slashing", ItemType_1H_Slashing),
			Option("2H Blunt", ItemType_2H_Blunt),
			Option("2H Piercing", ItemType_2H_Piercing),
			Option("2H Slashing", ItemType_2H_Slashing),
			Option("Alcohol", ItemType_Alcohol),
			Option("Alternate Ability", ItemType_AltAbility),
			Option("Alternate Currency", ItemType_PointCurrency),
			//Option("All Effects", ItemType_All_Effects),//We'll need to review how to implement
			Option("Ammo", ItemType_Ammo),
			Option("Armor", ItemType_Armor),
			Option("Armor Dye", ItemType_ArmorDye),
			Option("Arrow", ItemType_Arrow),
			Option("Artifact", ItemType_Artifact),
			Option("Aug Distiller (Remove)", ItemType_AugRemoveDistiller),
			Option("Augmentation", ItemType_Augmentation),
			Option("Aug Solvent (Destroy)", ItemType_AugDestroySolvent),
			Option("Bandage", ItemType_Bandage),
			Option("Banner", ItemType_GuildBanner),
			Option("Book", ItemType_Book),
			Option("Bow", ItemType_Bow),
			Option("Brass Instruments", ItemType_Brass_Instruments),
			Option("Charm", ItemType_Charm),
			Option("Token/Ticket", ItemType_Ticket),
#if (!IS_EMU_CLIENT)//Exceeds 70 entries in szItemClasses Maybe live specific?
			Option("Collectible", ItemType_Collectible),
#endif
			Option("Combinable", ItemType_Combinable),
			Option("Container", ItemType_Container),
			Option("Drink", ItemType_Drink),
			Option("Fishing Pole", ItemType_FishingPole),
			Option("Fishing Bait", ItemType_FishingBait),
#if (!IS_EMU_CLIENT)//Exceeds 70 entries in szItemClasses. We'll need to check this manually.
			Option("Focus Effect", ItemType_Focus_Effect),
#endif
			Option("Food", ItemType_Food),
			Option("House Key", ItemType_House_Key),
			Option("Instrument - All", ItemType_All_Instrument),
			Option("Jewelry", ItemType_Jewelry),
			Option("Key", ItemType_Key),
			Option("Lockpicks", ItemType_Lockpicks),
			/*Option("Light", ItemType_Light),//Never saw the point in having this option.*/
			Option("Magic Arrow", ItemType_MagicArrow),
			Option("Martial", ItemType_Martial),
			Option("Metal Key", ItemType_Metal_Key),
			Option("Misc", ItemType_Misc),
			Option("Note", ItemType_Note),
			Option("Percussion Instrument", ItemType_Percussion_Instrument),

#if (!IS_EMU_CLIENT)
			Option("Placeable", ItemType_Placeable),
#endif
			Option("Poison", ItemType_Poison),
			Option("Potion", ItemType_Potion),
			Option("Scroll", ItemType_Scroll),
			Option("Shield", ItemType_Shield),
			Option("Spell", ItemType_Spell),
			Option("Skill", ItemType_Skill),
			Option("Stringed Instrument", ItemType_Stringed_Instrument),
			Option("Throwing", ItemType_Throwing),
			Option("Wind Instrument", ItemType_Wind_Instrument),
	} } },
	
	{ OptionType_Deity, { {
		Option("Agnostic", Deity_Agnostic),
		Option("Bertoxxulous", Deity_Bertoxxulous),
		Option("Brell Serilis", Deity_Brell),
		Option("Bristlebane", Deity_Bristlebane),
		Option("Cazic-Thule", Deity_Cazic),
		Option("Erollisi Marr", Deity_Erollisi),
		Option("Innoruuk", Deity_Innoruuk),
		Option("Karana", Deity_Karana),
		Option("Mithaniel Marr", Deity_Mithaniel),
		Option("Prexus", Deity_Prexus),
		Option("Quellious", Deity_Quellious),
		Option("Rallos Zek", Deity_Rallos),
		Option("Rodcet Nife", Deity_Rodcet),
		Option("Solusek Ro", Deity_Solusek),
		Option("The Tribunal", Deity_Tribunal),
		Option("Tunare", Deity_Tunare),
		Option("Veeshan", Deity_Veeshan),
	} } },

#if (!IS_EMU_CLIENT)
	{ OptionType_Prestige, { {
			Option("Prestige Only", Prestige_Prestige_Only),
			Option("Non-Prestige Only", Prestige_Non_Prestige_Only)
	} } },
#endif

	{ OptionType_AugSlots, { {
			Option("1 (General: Single Stat)", Aug_1),
			Option("2 (General: Multiple Stats)", Aug_2),
			Option("3 (General: Spell Effect)", Aug_3),
			Option("4 (Weapon: General)", Aug_4),
			Option("5 (Weapon: Elem Damage)", Aug_5),
			Option("6 (Weapon: Base Damage)", Aug_6),
			Option("7 (General: Group)", Aug_7),
			Option("8 (General: Raid)", Aug_8),
			Option("9 (General: Dragons)", Aug_9),
			Option("10 (Crafted: Common)", Aug_10),
			Option("11 (Crafted: Group)", Aug_11),
			Option("12 (Crafted: Raid)", Aug_12),
			Option("13 (Energeiac: Group)", Aug_13),
			Option("14 (Energeiac: Raid)", Aug_14),
			Option("15 (Emblem)", Aug_15),
			Option("16 (Crafted: Group)", Aug_16),
			Option("17 (Crafted: Raid)", Aug_17),
			Option("20 (Ornamentation)", Aug_20),
			Option("21 (Special Ornamentation)", Aug_21)
	} } }
};

#ifdef DEBUGGING
static const char* szPlayerClasses[] = {
	"None",
	"Warrior",//1
	"Cleric",//2
	"Paladin",//3
	"Ranger",//4
	"Shadow Knight",//5
	"Druid",//6
	"Monk",//7
	"Bard",//8
	"Rogue",//9
	"Shaman",//10
	"Necromancer",//11
	"Wizard",
	"Mage",//13
	"Enchanter",//14
	"Beastlord",//15
	"Berserker"//16
};
#endif

constexpr MQColor light_blue = { 84, 172, 210 };
constexpr MQColor grey = { 37, 37, 37 };
constexpr MQColor black = { 12, 12, 12 };
constexpr MQColor blue = { 0, 0, 255 };
constexpr MQColor teal = { 0, 255, 255 };
constexpr MQColor green = { 0, 255, 0 };
constexpr MQColor magenta = { 255, 0, 255 };
constexpr MQColor orange = { 255, 153, 0 };
constexpr MQColor purple = { 128, 0, 128 };
constexpr MQColor red = { 255, 0, 0 };
constexpr MQColor white = { 255, 255, 255 };
constexpr MQColor yellow = { 255, 255, 0 };

//dark colors
constexpr MQColor black_dark = { 12, 12, 12 };
constexpr MQColor blue_dark = { 0, 0, 153 };
constexpr MQColor grey_dark = { 28, 28, 28 };
constexpr MQColor teal_dark = { 0, 153, 153 };
constexpr MQColor green_dark = { 0, 153, 0 };
constexpr MQColor magenta_dark = { 255, 0, 255 };
constexpr MQColor orange_dark = { 153, 102, 0 };
constexpr MQColor purple_dark = { 153, 0, 153 };
constexpr MQColor red_dark = { 230, 0, 0 };
constexpr MQColor white_dark = { 255, 255, 255 };
constexpr MQColor yellow_dark = { 153, 153, 0 };

//light colors

constexpr MQColor grey_light = { 166, 166, 166 }; // might want to make this MQColor(166, 166, 166); to match the others

static bool bOnlyShowDroppable = false;
static bool bOnlyShowNoDrop = false;
static char szSearchText[256] = "";
static char ReqMin[12] = "0";
static char ReqMax[12] = "255";
static char RecMin[12] = "0";
static char RecMax[12] = "255";


static void PopulateListBoxes() {
	for (auto& [type, data] : MenuData) {

		//Label for this section
		ImGui::Text("%s", DropDownOptions[type].c_str());

		//Clear button for each listbox right aligned
		ImGui::SameLine(ImGui::GetWindowWidth() - 70);
		std::string clearBtnLabel = "Clear##" + std::to_string(type);
		if (ImGui::SmallButton(clearBtnLabel.c_str())) {
			for (auto& opt : data.OptionList) {
				opt.IsSelected = false;
			}
		}

		//Prepare the preview string
		std::string previewValue;
		for (const auto& opt : data.OptionList) {
			if (opt.IsSelected) {
				if (!previewValue.empty()) {
					previewValue += ", ";
				}
				
				previewValue += opt.Name;
			}
		}
		
		if (previewValue.empty()) {
			previewValue = "Select options...";
		}

		ImGui::SetNextItemWidth(ImGui::GetWindowWidth()-30);
		//We're starting a combo box here - needs an EndCombo later, even if it's not in an if statement.
		std::string internalID = "##List" + std::to_string(type);
		const bool isOpened = ImGui::BeginCombo(internalID.c_str(), previewValue.c_str());

		//tooltip for the closed combo box - will show all options that are currently selected.
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(previewValue.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		if (isOpened) {//We're only doing this bit if the combo box is open.
			for (auto& opt : data.OptionList) {
				//add option
				if (ImGui::Selectable(opt.Name.c_str(), opt.IsSelected, ImGuiSelectableFlags_NoAutoClosePopups)) {
					opt.IsSelected = !opt.IsSelected;
				}

				//this shows the individual entry - in case it's cut off when the dropdown is open.
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(opt.Name.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Spacing();
	}
}

/*
This will check if any of the options were selected in a category
We can use that to determine if we should check the filters at all.
*/
template <typename T>
static bool IsAnySelected(const T& container) {
	return std::ranges::any_of(container, & Option::IsSelected);
}

static bool ValueFoundInMask(const int mask, const int id) {
	//Safety check to prevent undefined behavior from shifting >= 32
	if (id < 0 || id >= 32) {
		return false;
	}

	return (mask & (1U << id)) != 0;
}

// static bool ValueFoundInMask(const uint64_t mask, const int id) {
// 	if (id < 0 || id >= 64) {
// 		return false;
// 	}
// 	
// 	return (mask & (1ULL << id)) != 0;
// }

template <typename T>
static bool MatchesMask(const ItemClient* pItem, const OptionType type, T ItemDefinition::*maskField) {
	if (!pItem) {
		return false;
	}

	const ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
	}
	
	const auto& optionData = MenuData[type].OptionList;

	//If nothing is selected all results are valid.
	if (!IsAnySelected(optionData)) {
		return true;
	}

	// Access the specific mask from the item definition using the member pointer
	// Syntax (pItemDef->*maskField) dynamically picks .Races, .Classes, etc.
	const auto itemMask = pItemDef->*maskField;

	for (const auto& option : optionData) {
		if (option.IsSelected) {
			if (ValueFoundInMask(itemMask, option.ID)) {
				WriteChatf("Found: %s in mask", option.Name.c_str());
				return true;
			}
		}
	}

	return false;
}

static bool MatchesRaces(const ItemClient* pItem) {
	return MatchesMask(pItem, OptionType_Race, &ItemDefinition::Races);
}

static bool MatchesClasses(const ItemClient* pItem) {
	return MatchesMask(pItem, OptionType_Class, &ItemDefinition::Classes);
}

static bool MatchesSlots(const ItemClient* pItem) {
	return MatchesMask(pItem, OptionType_Slots, &ItemDefinition::EquipSlots);
}

static bool MatchesAugSlots(const ItemClient* pItem) {
	return MatchesMask(pItem, OptionType_AugSlots, &ItemDefinition::AugType);
}

static bool MatchesDeities(const ItemClient* pItem) {
	return MatchesMask(pItem, OptionType_Deity, &ItemDefinition::Deity);
}

static bool MatchesLevelRequirements(const ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	const ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
	}


	if (const int val = GetIntFromString(ReqMin, 0)) {
		if (pItemDef->RequiredLevel < val) {
			return false;
		}
	}

	if (const int val = GetIntFromString(ReqMax, 0)) {
		if (pItemDef->RequiredLevel > val) {
			return false;
		}
	}

	if (const int val = GetIntFromString(RecMin, 0)) {
		if (pItemDef->RecommendedLevel < val) {
			return false;
		}
	}

	if (const int val = GetIntFromString(RecMax, 0)) {
		if (pItemDef->RecommendedLevel > val) {
			return false;
		}
	}

	return true;
}

static bool MatchesItemType(const ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
	}

	const auto& itemTypeData = MenuData[OptionType_ItemType].OptionList;
	const bool anySlotSelected = IsAnySelected(itemTypeData);

	if (anySlotSelected) {
		bool foundMatch = false;
#ifdef DEBUGGING
		//Only needed for output when debugging. Otherwise we'd just return true when we found a match.
		int currentoption = 0;
#endif
		const uint8_t ItemType = pItem->GetItemClass();
		for (auto& option : itemTypeData) {
			if (option.IsSelected) {
#ifdef DEBUGGING
				currentoption = option.ID;
#endif

				switch (option.ID) {
					//Special consideration ItemTypes - where the results don't produce anything or as expected.
					//We also can use this for custom entries - like "clickies"

					case ItemType_Container://This actually doesn't produce all containers without this logic.
						if (pItem->IsContainer()) {
							foundMatch = true;
						}

						break;

					default://By default we just want to check everything here.
						if (ItemType == option.ID) {
							foundMatch = true;
						}

						break;
				}
			}

			if (foundMatch) {//We found a match - leave the for loop
				break;
			}
		}

#ifdef DEBUGGING
		if (ItemType > 70) {//This outputs the itemtype of the item.
			WriteChatf("ItemType for \ap%s\ax was \ar%hhu", pItemDef->Name, ItemType);
		}

		//These output the ItemType from the szItemClasses array for both the ItemType and type for the option.ID
		if (currentoption <= 70) {
			if (!foundMatch) {
				WriteChatf("\arItem: \ap%s\ax is \ayItemClass: \at%s\ax was looking for \a-t%s", pItemDef->Name, szItemClasses[ItemType], szItemClasses[currentoption]);
			}
			else {

				WriteChatf("\agItem: \ap%s\ax is \ayItemClass: \at%s\ax was looking for \a-t%s", pItemDef->Name, szItemClasses[ItemType], szItemClasses[currentoption]);
			}
		}
#endif

		return foundMatch;
	}

	return true;
}

static bool MatchesStats(const ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	const ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
	}

	const auto& statsData = MenuData[OptionType_Stats].OptionList;
	const bool anySlotSelected = IsAnySelected(statsData);

	if (anySlotSelected) {
		int hasStatCount = 0;
		int optioncount = 0;

		for (const auto& option : statsData) {
			if (option.IsSelected) {
				optioncount++;
				switch (option.ID) {
					case Stat_AC:
						if (pItemDef->AC) {
							hasStatCount++;
						}

						break;

					case Stat_Accuracy:
						if (pItemDef->Accuracy) {
							hasStatCount++;
						}

						break;

					case Stat_AGI:
						if (pItemDef->AGI) {
							hasStatCount++;
						}

						break;

					case Stat_Attack:
						if (pItemDef->Attack) {
							hasStatCount++;
						}

						break;

					case Stat_Avoidance:
						if (pItemDef->Avoidance) {
							hasStatCount++;
						}

						break;

					case Stat_BackstabDamage:
						if (pItemDef->BackstabDamage) {
							hasStatCount++;
						}

						break;

					case Stat_CHA:
						if (pItemDef->CHA) {
							hasStatCount++;
						}

						break;

					case Stat_Clairvoyance:
						if (pItemDef->Clairvoyance) {
							hasStatCount++;
						}

						break;

					case Stat_CombatEffects:
						if (pItemDef->CombatEffects) {
							hasStatCount++;
						}

						break;

					case Stat_Damage:
						if (pItemDef->Damage) {
							hasStatCount++;
						}

						break;

					case Stat_DamageShield:
						if (pItemDef->DamShield) {
							hasStatCount++;
						}

						break;

					case Stat_DEX:
						if (pItemDef->DEX) {
							hasStatCount++;
						}

						break;

					case Stat_DmgBonus:
						if (pItemDef->DmgBonusValue) {
							hasStatCount++;
						}

						break;

					case Stat_DoTShielding:
						if (pItemDef->DoTShielding) {
							hasStatCount++;
						}

						break;

					case Stat_DSMitigation:
						if (pItemDef->DamageShieldMitigation) {
							hasStatCount++;
						}

						break;

					case Stat_Efficiency:
						if (pItemDef->Damage && pItemDef->Delay) {
							hasStatCount++;
						}

						break;

					case Stat_ElementalDamage:
						if (pItemDef->ElementalDamage) {
							hasStatCount++;
						}

						break;

					case Stat_Endurance:
						if (pItemDef->Endurance) {
							hasStatCount++;
						}

						break;

					case Stat_EnduranceRegen:
						if (pItemDef->EnduranceRegen) {
							hasStatCount++;
						}

						break;

					case Stat_Favor:
						if (pItemDef->Favor) {
							hasStatCount++;
						}

						break;

					case Stat_GuildFavor:
						if (pItemDef->GuildFavor) {
							hasStatCount++;
						}

						break;

					case Stat_Haste:
						if (pItemDef->Haste) {
							hasStatCount++;
						}

						break;

					case Stat_HealAmount:
						if (pItemDef->HealAmount) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicAgi:
						if (pItemDef->HeroicAGI) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicCha:
						if (pItemDef->HeroicCHA) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicCorruption:
						if (pItemDef->HeroicSvCorruption) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicDex:
						if (pItemDef->HeroicDEX) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicInt:
						if (pItemDef->HeroicINT) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicPoison:
						if (pItemDef->HeroicSvPoison) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicSta:
						if (pItemDef->HeroicSTA) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicStr:
						if (pItemDef->HeroicSTR) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicSvCold:
						if (pItemDef->HeroicSvCold) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicSvDisease:
						if (pItemDef->HeroicSvDisease) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicSvFire:
						if (pItemDef->HeroicSvFire) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicSvMagic:
						if (pItemDef->HeroicSvMagic) {
							hasStatCount++;
						}

						break;

					case Stat_HeroicWis:
						if (pItemDef->HeroicWIS) {
							hasStatCount++;
						}

						break;

					case Stat_HP:
						if (pItemDef->HP) {
							hasStatCount++;
						}

						break;


					case Stat_HPRegen:
						if (pItemDef->HPRegen) {
							hasStatCount++;
						}

						break;

					case Stat_INT:
						if (pItemDef->INT) {
							hasStatCount++;
						}

						break;

					case Stat_Mana:
						if (pItemDef->Mana) {
							hasStatCount++;
						}

						break;

					case Stat_ManaRegen:
						if (pItemDef->ManaRegen) {
							hasStatCount++;
						}

						break;

					case Stat_Purity:
						if (pItemDef->Purity) {
							hasStatCount++;
						}

						break;

					case Stat_Range:
						if (pItemDef->Range) {
							hasStatCount++;
						}

						break;

					case Stat_Ratio:
						if (pItemDef->Damage && pItemDef->Delay) {
							hasStatCount++;
						}

						break;

					case Stat_Shielding:
						if (pItemDef->Shielding) {
							hasStatCount++;
						}

						break;

					case Stat_SpellDamage:
						if (pItemDef->SpellDamage) {
							hasStatCount++;
						}

						break;

					case Stat_SpellShield:
						if (pItemDef->SpellShield) {
							hasStatCount++;
						}

						break;

					case Stat_STA:
						if (pItemDef->STA) {
							hasStatCount++;
						}

						break;

					case Stat_STR:
						if (pItemDef->STR) {
							hasStatCount++;
						}

						break;

					case Stat_StrikeThrough:
						if (pItemDef->StrikeThrough) {
							hasStatCount++;
						}

						break;

					case Stat_StunResist:
						if (pItemDef->StunResist) {
							hasStatCount++;
						}

						break;

					case Stat_SvCold:
						if (pItemDef->SvCold) {
							hasStatCount++;
						}

						break;

					case Stat_SvCorruption:
						if (pItemDef->SvCorruption) {
							hasStatCount++;
						}

						break;

					case Stat_SvDisease:
						if (pItemDef->SvDisease) {
							hasStatCount++;
						}

						break;

					case Stat_SvFire:
						if (pItemDef->SvFire) {
							hasStatCount++;
						}

						break;

					case Stat_SvMagic:
						if (pItemDef->SvMagic) {
							hasStatCount++;
						}

						break;

					case Stat_SvPoison:
						if (pItemDef->SvPoison) {
							hasStatCount++;
						}

						break;

					case Stat_Weight:
						if (pItemDef->Weight) {
							hasStatCount++;
						}

						break;

					case Stat_WIS:
						if (pItemDef->WIS) {
							hasStatCount++;
						}

						break;

					default:
						WriteChatf("Unaccounted for Option.ID (\ar%d\ax) in MatchesStats", option.ID);
						break;
				}
			}
		}
		return hasStatCount == optioncount;
	}

	return true;
}

static bool DoesItemMatchFilters(const ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
	}

	//Filter by name
	if (szSearchText[0] != '\0') {
		std::string itemName = pItemDef->Name;
		std::string searchText = szSearchText;
		std::ranges::transform(itemName, itemName.begin(), [](const unsigned char c) { return std::tolower(c); });
		std::ranges::transform(searchText, searchText.begin(), [](const unsigned char c) { return std::tolower(c); });

		if (itemName.find(searchText) == std::string::npos) {
			return false;
		}
	}

	//No Drop Filters
	if (bOnlyShowDroppable && (!pItemDef->IsDroppable || pItem->NoDropFlag)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\ax is \arDroppable: \ay%d \arNoDropFlag: \ay%d", pItemDef->Name, pItemDef->IsDroppable, pItem->NoDropFlag);
#endif
		return false;
	}
	else if (bOnlyShowNoDrop && pItemDef->IsDroppable && !pItem->NoDropFlag) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\ax is \arDroppable: \ay%d \arNoDropFlag: \ay%d", pItemDef->Name, pItemDef->IsDroppable, pItem->NoDropFlag);
#endif
		return false;
	}

	if (!MatchesLevelRequirements(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesLevelRequirements(pItem)", pItemDef->Name);
#endif
		return false;
	}

	if (!MatchesSlots(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesSlot(pItem)", pItemDef->Name);
#endif
		return false;
	}

	if (!MatchesRaces(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesRace(pItem)", pItemDef->Name);
#endif
		return false;
	}

	if (!MatchesClasses(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesClasses(pItem)", pItemDef->Name);
#endif
		return false;
	}

	if (!MatchesItemType(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesItemType(pItem)", pItemDef->Name);
#endif
		return false;
	}
	
	if (!MatchesDeities(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesDeities", pItemDef->Deity);
#endif
		return false;
	}

	if (!MatchesAugSlots(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesAugType(pItem)", pItemDef->Name);
#endif
		return false;
	}

	if (!MatchesStats(pItem)) {
#ifdef DEBUGGING
		WriteChatf("\arExcluding: \ap%s\axin MatchesStats(pItem)", pItemDef->Name);
#endif
		return false;
	}

#ifdef DEBUGGING
	WriteChatf("\ap%s \agpassed all filters!", pItemDef->Name);
#endif
	return true;
}

/**
 * Avoid Globals if at all possible, since they persist throughout your program.
 * But if you must have them, here is the place to put them.
 */
static bool ShowMQSearchItemWindow = true;
struct LocationDetail {
	LocationID loc;
	// For top-level slot: inventory slot index for bags, bank/shared bank slot index, or worn slot index
	int topSlotIndex = -1; // 0-based index as scanned
	// If inside a container, this is the slot index within that container (0-based)
	int bagSlotIndex = -1;
	// Augment context
	bool isAug = false;
	int augSlotIndex = -1; // 0-based
	std::string hostName; // name of the item containing this (for augs)
};
struct QueryResult {
	ItemPtr item = nullptr;
	LocationDetail location;
};
static std::vector<QueryResult> vResults;
// During population, this indicates which location bucket we're scanning
static LocationID g_CurrentScanLocation = Loc_Bags;

static void PopulateAllItems(PlayerClient* pChar, const char* szArgs);

// Format a detailed, single-line location string per requirements
static std::string FormatLocation(const LocationDetail& d)
{
	auto baseLocToStr = [](const LocationID id) -> const char* {
		switch (id)
		{
			case Loc_Bank: return "Bank";
			case Loc_Shared_Bank: return "Shared Bank";
			case Loc_Equipped: return "Equipped";
			case Loc_Bags: return "Bags";
			case Loc_Real_Estate: return "Real Estate";
			case Loc_Item_Overflow: return "Item Overflow";
			case Loc_Parcel: return "Parcel";
#if (!IS_EMU_CLIENT)
			case Loc_TradeSkillDepot: return "Tradeskill Depot";
			case Loc_DragonsHorde: return "Dragon's Hoard";
#endif
			default: return "Unknown";
		}
	};

	std::string out;
	if (d.loc == Loc_Bags)
	{
		out = ""; // We'll build it starting with Bag below
	}
	else if (d.loc == Loc_Equipped)
	{
		auto getWornSlotName = [](const int slot) -> const char* {
			switch (slot) {
				case InvSlot_Charm: return "Charm";
				case InvSlot_LeftEar: return "Left Ear";
				case InvSlot_Head: return "Head";
				case InvSlot_Face: return "Face";
				case InvSlot_RightEar: return "Right Ear";
				case InvSlot_Neck: return "Neck";
				case InvSlot_Shoulders: return "Shoulders";
				case InvSlot_Arms: return "Arms";
				case InvSlot_Back: return "Back";
				case InvSlot_LeftWrist: return "Left Wrist";
				case InvSlot_RightWrist: return "Right Wrist";
				case InvSlot_Range: return "Range";
				case InvSlot_Hands: return "Hands";
				case InvSlot_Primary: return "Primary";
				case InvSlot_Secondary: return "Secondary";
				case InvSlot_LeftFingers: return "Left Fingers";
				case InvSlot_RightFingers: return "Right Fingers";
				case InvSlot_Chest: return "Chest";
				case InvSlot_Legs: return "Legs";
				case InvSlot_Feet: return "Feet";
				case InvSlot_Waist: return "Waist";
				case InvSlot_PowerSource: return "Power Source";
				case InvSlot_Ammo: return "Ammo";
				default: return "Equipped";
			}
		};
		out = getWornSlotName(d.topSlotIndex);
	}
	else
	{
		out = baseLocToStr(d.loc);
	}

	// Compute human-friendly numbers (1-based) where applicable
	auto oneBased = [](const int idx) { return idx >= 0 ? idx + 1 : -1; };

	// Append top level slot/bag information
	if (d.loc == Loc_Bank || d.loc == Loc_Shared_Bank)
	{
		if (d.bagSlotIndex >= 0 && d.topSlotIndex >= 0)
		{
			// Bank item inside a bag in a bank slot: "Bank - Bag Y Slot Z"
			out += " - Bag ";
			out += std::to_string(oneBased(d.topSlotIndex));
			out += " Slot ";
			out += std::to_string(oneBased(d.bagSlotIndex));
		}
		else if (d.topSlotIndex >= 0)
		{
			// Loose item directly in bank slot: "Bank - Slot X"
			out += " - Slot ";
			out += std::to_string(oneBased(d.topSlotIndex));
		}
	}
	else if (d.loc == Loc_Bags)
	{
		// Inventory bag slots: show Bag # and Slot # if inside a bag
		if (d.bagSlotIndex >= 0)
		{
			out += "Bag ";
			// Convert absolute inventory slot index to bag number (1-based)
			// InvSlot_FirstBagSlot is the start of main inventory bag slots
			int bagNum = -1;
			if (d.topSlotIndex >= 0)
			{
				bagNum = oneBased(d.topSlotIndex - InvSlot_FirstBagSlot);
				if (bagNum < 1) {
					bagNum = oneBased(d.topSlotIndex);
				}
			}
			out += std::to_string(bagNum);
			out += " Slot ";
			out += std::to_string(oneBased(d.bagSlotIndex));
		}
	}
	else if (d.loc == Loc_Equipped)
	{
		if (d.bagSlotIndex >= 0)
		{
			out += " - Slot ";
			out += std::to_string(oneBased(d.bagSlotIndex));
		}
	}

	// Augment context appended at end
	if (d.isAug)
	{
		out += " - Aug in ";
		//out += d.hostName;
		if (d.augSlotIndex >= 0)
		{
			out += " Slot ";
			out += std::to_string(oneBased(d.augSlotIndex));
		}
	}

	return out;
}

// Build list of selected stat IDs and their labels for dynamic columns
static std::vector<std::pair<StatID, std::string>> GetSelectedStatColumns()
{
	std::vector<std::pair<StatID, std::string>> out;
	const auto& statsData = MenuData[OptionType_Stats].OptionList;
	for (const auto& opt : statsData)
	{
		if (opt.IsSelected) {
			out.emplace_back(static_cast<StatID>(opt.ID), opt.Name);
		}
	}
	return out;
}

static float GetStatValueFloat(const ItemDefinition* def, const StatID stat) {
	if (!def) {
		return 0.0f;
	}

	switch (stat) {
		case Stat_Ratio: return (def->Damage && def->Delay ? (static_cast<float>(def->Delay) / static_cast<float>(def->Damage)) : 0.0f);
		default: return 0.0f;

	}
}

// Get numeric value for a given stat from an item definition
static int GetStatValue(const ItemDefinition* def, const StatID stat)
{
	if (!def) {
		return 0;
	}

	switch (stat)
	{

		case Stat_AC: return def->AC;
		case Stat_Accuracy: return def->Accuracy;
		case Stat_AGI: return def->AGI;
		case Stat_Attack: return def->Attack;
		case Stat_Attunable: return def->Attuneable ? 1 : 0;
		case Stat_Avoidance: return def->Avoidance;
		case Stat_BackstabDamage: return def->BackstabDamage;
		case Stat_CHA: return def->CHA;
		case Stat_Clairvoyance: return def->Clairvoyance;
		case Stat_CombatEffects: return def->CombatEffects;
		case Stat_Damage: return def->Damage;
		case Stat_DamageShield: return def->DamShield;
		case Stat_DEX: return def->DEX;
		case Stat_DmgBonus: return def->DmgBonusValue;
		case Stat_DoTShielding: return def->DoTShielding;
		case Stat_DSMitigation: return def->DamageShieldMitigation;
		case Stat_Efficiency: return (def->Damage && def->Delay ? (static_cast<int>(static_cast<float>(def->Damage) / static_cast<float>(def->Delay) * 100.0f)) : 0);
		case Stat_ElementalDamage: return def->ElementalDamage;
		case Stat_Endurance: return def->Endurance;
		case Stat_EnduranceRegen: return def->EnduranceRegen;
		case Stat_Favor: return def->Favor;
		case Stat_GuildFavor: return def->GuildFavor;
		case Stat_Haste: return def->Haste;
		case Stat_HealAmount: return def->HealAmount;
		case Stat_Heirloom: return def->Heirloom ? 1 : 0;
		case Stat_HeroicAgi: return def->HeroicAGI;
		case Stat_HeroicCha: return def->HeroicCHA;
		case Stat_HeroicCorruption: return def->HeroicSvCorruption;
		case Stat_HeroicDex: return def->HeroicDEX;
		case Stat_HeroicInt: return def->HeroicINT;
		case Stat_HeroicPoison: return def->HeroicSvPoison;
		case Stat_HeroicSta: return def->HeroicSTA;
		case Stat_HeroicStr: return def->HeroicSTR;
		case Stat_HeroicSvCold: return def->HeroicSvCold;
		case Stat_HeroicSvDisease: return def->HeroicSvDisease;
		case Stat_HeroicSvFire: return def->HeroicSvFire;
		case Stat_HeroicSvMagic: return def->HeroicSvMagic;
		case Stat_HeroicWis: return def->HeroicWIS;
		case Stat_HP: return def->HP;
		case Stat_HPRegen: return def->HPRegen;
		case Stat_INT: return def->INT;
		case Stat_Mana: return def->Mana;
		case Stat_ManaRegen: return def->ManaRegen;
		case Stat_Purity: return def->Purity;
		case Stat_Range: return def->Range;
		case Stat_Shielding: return def->Shielding;
		case Stat_SpellDamage: return def->SpellDamage;
		case Stat_SpellShield: return def->SpellShield;
		case Stat_STA: return def->STA;
		case Stat_STR: return def->STR;
		case Stat_StrikeThrough: return def->StrikeThrough;
		case Stat_StunResist: return def->StunResist;
		case Stat_Summoned: return def->Summoned ? 1 : 0;
		case Stat_SvCold: return def->SvCold;
		case Stat_SvCorruption: return def->SvCorruption;
		case Stat_SvDisease: return def->SvDisease;
		case Stat_SvFire: return def->SvFire;
		case Stat_SvMagic: return def->SvMagic;
		case Stat_SvPoison: return def->SvPoison;
		case Stat_TradeSkills: return def->TradeSkills ? 1 : 0;
		case Stat_Weight: return def->Weight;
		case Stat_WIS: return def->WIS;
		default: return 0;
	}
}
/**
 * @fn InitializePlugin
 *
 * This is called once on plugin initialization and can be considered the startup
 * routine for the plugin.
 */
PLUGIN_API void InitializePlugin()
{
	AddCommand("/searchitem", PopulateAllItems);
	// AddMQ2Data("mytlo", MyTLOData);
}

/**
 * @fn ShutdownPlugin
 *
 * This is called once when the plugin has been asked to shutdown.  The plugin has
 * not actually shut down until this completes.
 */
PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQSearchItem::Shutting down");
	RemoveCommand("/searchitem");
	// RemoveMQ2Data("mytlo");
}

PLUGIN_API void OnUpdateImGui() {
	if (GetGameState() != GAMESTATE_INGAME) {
		return;
	}

	if (!ShowMQSearchItemWindow) {
		return;
	}

#pragma region Styles
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 50.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200.0f, 150.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 9);
	// 5 push style Var; if we add more, make sure we pop
	static constexpr int iPushPopVar = 5;

	ImGui::PushStyleColor(ImGuiCol_FrameBg, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, black.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_Tab, black.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_TabSelected, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_TabHovered, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_Button, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, grey_dark.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, grey.ToImU32());
	//Header is for Listbox (Ignore/Immune lists)
	ImGui::PushStyleColor(ImGuiCol_Header, black.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, black.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, black.ToImU32());
	// 13 push style Color; if we add more, make sure we pop
	static constexpr int iPushPopColor = 13;
#pragma endregion Styles
	static ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoFocusOnAppearing;

	//Begin main window
	if (!ImGui::Begin("MQSearchItem", &ShowMQSearchItemWindow, flags)) {
		ImGui::PopStyleVar(iPushPopVar);
		ImGui::PopStyleColor(iPushPopColor);
		ImGui::End();
		return;
	}

	//Maybe it's silly, but we need this to open the popup that's referenced inside the menu bar.
	bool openSavePopup = false;

	if (ImGui::BeginMenuBar()) {
		if (ImGui::Button("Search")) {
			PopulateAllItems(pLocalPlayer, "");
		}

		//Reset all the options to defaults.
		if (ImGui::Button("Reset Options")) {
			szSearchText[0] = '\0';
			bOnlyShowDroppable = false;
			bOnlyShowNoDrop = false;
			strcpy_s(ReqMin, "0");
			strcpy_s(ReqMax, "255");
			strcpy_s(RecMin, "0");
			strcpy_s(RecMax, "255");
			for (auto& [OptionList] : MenuData | std::views::values) {
				for (auto& opt : OptionList) {
					opt.IsSelected = false;
				}
			}
		}

		//Saved Searches menu
		if (ImGui::BeginMenu("Saved Searches")) {
			if (ImGui::MenuItem("Save New Search...")) {
				openSavePopup = true;
			}

			if (ImGui::BeginMenu("Load Search")) {
				for (const auto& name : ListSavedSearches()) {
					if (ImGui::MenuItem(name.c_str())) {
						LoadSearchFromFile(GetSearchFilePath(name));
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Delete Search")) {
				for (const auto& name : ListSavedSearches()) {
					if (ImGui::MenuItem(name.c_str())) {
						DeleteSearchFile(GetSearchFilePath(name));
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (openSavePopup) {//If we said to open the popup in the menu, then here is where we implement it.
		ImGui::OpenPopup("SaveSearchPopup");
	}

	// Save popup
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 0), ImVec2(600, FLT_MAX));
	if (ImGui::BeginPopupModal("SaveSearchPopup")) {
		static char saveName[128] = "";
		ImGui::Text("Search Name:");
		ImGui::InputText("##SaveSearchName", saveName, IM_ARRAYSIZE(saveName));

		if (ImGui::Button("Save")) {
			if (saveName[0] != '\0') {
				EnsureSavedSearchDir();
				SaveCurrentSearchToFile(GetSearchFilePath(saveName));
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	static constexpr ImGuiWindowFlags childflags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("##SearchItemOptions", ImVec2(180, ImGui::GetContentRegionAvail().y), 0, childflags);

		ImGui::Text("Search:");
		ImGui::SameLine(ImGui::GetWindowWidth() - 70);
		if (ImGui::SmallButton("Clear##Search")) {
			szSearchText[0] = '\0';
		}

		if (ImGui::InputText("##SearchBox", szSearchText, IM_ARRAYSIZE(szSearchText), ImGuiInputTextFlags_EnterReturnsTrue)) {
			PopulateAllItems(pLocalPlayer, "");
		}

		if (ImGui::Checkbox("Only Droppable", &bOnlyShowDroppable)) {
			if (bOnlyShowDroppable) {
				bOnlyShowNoDrop = false;
			}
		}

		if (ImGui::Checkbox("Only NoDrop", &bOnlyShowNoDrop)) {
			if (bOnlyShowNoDrop) {
				bOnlyShowDroppable = false;
			}
		}

		ImGui::Text("Req. Lvl");
		ImGui::SameLine(ImGui::GetWindowWidth() - 70);

		if (ImGui::SmallButton("Clear##ReqLevel")) {
			strcpy_s(ReqMin, "0");
			strcpy_s(ReqMax, "255");
		}

		ImGui::Text("Min");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(35);
		ImGui::InputText("###MinRequired", ReqMin, 12);
		ImGui::SameLine();
		ImGui::Text("Max");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		ImGui::InputText("###MaxRequired", ReqMax, 12);

		ImGui::Text("Rec. Lvl");
		ImGui::SameLine(ImGui::GetWindowWidth() - 70);

		if (ImGui::SmallButton("Clear##RecLevel")) {
			strcpy_s(RecMin, "0");
			strcpy_s(RecMax, "255");
		}

		ImGui::Text("Min");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(35);
		ImGui::InputText("###MinReccomend", RecMin, 12);
		ImGui::SameLine();
		ImGui::Text("Max");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		ImGui::InputText("###MaxReccommend", RecMax, 12);

		PopulateListBoxes();

	ImGui::EndChild();

	//Results on right
	ImGui::SameLine();
	ImGui::BeginChild("##SearchItemResults", ImVec2(0, 0), ImGuiChildFlags_Borders);

	static std::shared_ptr<CTextureAnimation> pTAItemIcon;

	if (!pTAItemIcon) {
		pTAItemIcon = std::make_shared<CTextureAnimation>();
		if (CTextureAnimation* temp = pSidlMgr->FindAnimation("A_DragItem")) {
			pTAItemIcon = std::make_unique<CTextureAnimation>(*temp);
		}
	}


	auto selectedStats = GetSelectedStatColumns();
	constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable;
	const int columnCount = static_cast<int>(3 + selectedStats.size());

	if (ImGui::BeginTable("##ResultsTable", columnCount, tableFlags, ImGui::GetContentRegionAvail())) {
		ImGui::TableSetupScrollFreeze(0, 1);

		//Icon: fixed size, no resize, no sort
		static constexpr ImGuiTableColumnFlags iconFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;
		ImGui::TableSetupColumn("Icon", iconFlags, 32.0f, 0);

		//Name - default sort
		static constexpr ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
		ImGui::TableSetupColumn("Name", columnFlags, 0.0f, 1);

		//Location
		ImGui::TableSetupColumn("Location", columnFlags, 0.0f, 2);

		//Dynamic Stat columns
		for (size_t i = 0; i < selectedStats.size(); ++i) {
			ImGui::TableSetupColumn(selectedStats[i].second.c_str(), columnFlags, 0.0f, static_cast<ImGuiID>(3 + i));
		}

		ImGui::TableHeadersRow();

		//Sort Stats/Location/Name
		if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
			if (sortSpecs->SpecsDirty) {
				if (vResults.size() > 1) {
					std::ranges::sort(vResults, [&](const QueryResult& a, const QueryResult& b) {
						for (int n = 0; n < sortSpecs->SpecsCount; n++) {
							const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[n];

							if (!a.item || !b.item) {
								continue;
							}

							switch (spec->ColumnUserID) {
								case 1: // Name
								{
									const int res = _stricmp(a.item->GetName(), b.item->GetName());
									if (res != 0) {
										return (spec->SortDirection == ImGuiSortDirection_Ascending) ? (res < 0) : (res > 0);
									}
								}
									break;
								case 2: // Location
								{
									const int res = _stricmp(FormatLocation(a.location).c_str(), FormatLocation(b.location).c_str());
									if (res != 0) {
										return (spec->SortDirection == ImGuiSortDirection_Ascending) ? (res < 0) : (res > 0);
									}
								}
									break;
								default: // Stats
									if (spec->ColumnUserID >= 3) {
										const size_t statIdx = static_cast<size_t>(spec->ColumnUserID - 3);
										if (statIdx < selectedStats.size()) {
											const StatID sid = selectedStats[statIdx].first;
											switch (sid) {
												case Stat_Ratio://This is a float, and must be handled differently.
												{
													const float valA = GetStatValueFloat(a.item->GetItemDefinition(), sid);
													const float valB = GetStatValueFloat(b.item->GetItemDefinition(), sid);
													const float result = (valA < valB) ? -1.0f : (valA > valB) ? 1.0f : 0.0f;
													if (result != 0.0f) {
														return (spec->SortDirection == ImGuiSortDirection_Ascending) ? (result < 0.0f) : (result > 0.0f);
													}
												}

												break;

												default:
												{
													const int valA = GetStatValue(a.item->GetItemDefinition(), sid);
													const int valB = GetStatValue(b.item->GetItemDefinition(), sid);
													const int res = (valA < valB) ? -1 : (valA > valB) ? 1 : 0;
													if (res != 0) {
														return (spec->SortDirection == ImGuiSortDirection_Ascending) ? (res < 0) : (res > 0);
													}
												}

												break;
											}
										}
									}
									break;
							}
						}
						return false;
						});
				}
				sortSpecs->SpecsDirty = false;
			}
		}

		for (const auto& vResult : vResults) {
			ItemPtr item = vResult.item;
			if (!item) {
				continue;
			}

			const ItemDefinition* pItemDef = item->GetItemDefinition();
			if (!pItemDef) {
				continue;
			}

			ImGui::TableNextRow();

			// Icon
			ImGui::TableSetColumnIndex(0);
			if (pTAItemIcon) {
				static constexpr int iEQItemOffset = 500;
				static constexpr int iEQItemAltOffset = 336;
				const int iIconID = item->GetIconID();
				pTAItemIcon->SetCurCell(iIconID ? iIconID - iEQItemOffset : iEQItemAltOffset);
				mq::imgui::DrawTextureAnimation(pTAItemIcon.get(), CXSize(25, 25), true);
			}

			// Name (link)
			ImGui::TableSetColumnIndex(1);
			if (imgui::ItemLinkText(item->GetName(), GetColorForChatColor(USERCOLOR_LINK))) {
				char ItemLinkText[512] = { 0 };
				FormatItemLink(ItemLinkText, 512, item.get());
				TextTagInfo info = ExtractLink(ItemLinkText);
				ExecuteTextLink(info);
			}

			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(item->GetName());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			// Location
			ImGui::TableSetColumnIndex(2);
			if (!vResult.location.isAug) {
				char buf[128] = { 0 };
				sprintf_s(buf, 128, "GrabItem##%s", FormatLocation(vResult.location).c_str());
				if (ImGui::SmallButton(buf)) {
					if (pLocalPC->GetInventorySlot(InvSlot_Cursor)) {
						DoCommand("/autoinv");
					}
					
					switch (vResult.location.loc) {
						case Loc_Bank:
						case Loc_Shared_Bank:
							if (pBankWnd && pBankWnd->IsVisible()) {
								if (vResult.location.bagSlotIndex != -1) {
									const int Slot = vResult.location.topSlotIndex + 1;
									const int Slot2 = vResult.location.bagSlotIndex + 1;
									WriteChatf("Grabbing item in bank bag %d slot %d", Slot, Slot2);
									DoCommandf("/itemnotify in bank%d %d leftmouseup", Slot, Slot2);
								}
								else {
									const int Slot = vResult.location.topSlotIndex + 1;
									WriteChatf("Grabbing item in bank bag %d slot 0", Slot);
									DoCommandf("/itemnotify in bank%d 0 leftmouseup", Slot);
								}
							}
							break;
							
						case Loc_Bags:
						case Loc_Equipped:
							if (vResult.location.bagSlotIndex != -1) {
								const int Slot = vResult.location.topSlotIndex - 22;
								const int Slot2 = vResult.location.bagSlotIndex + 1;
								DoCommandf("/itemnotify in pack%d %d leftmouseup", Slot, Slot2);
							}
							else {
								const int Slot = vResult.location.topSlotIndex;
								DoCommandf("/itemnotify %d leftmouseup", Slot);
							}
							break;
							
						//TODO: handle these.
						case Loc_Item_Overflow:
						case Loc_Real_Estate:
						case Loc_Parcel:
							break;
						default:
							//Shouldn't be here...
							break;
					}					
				}
				ImGui::SameLine();
			}
			std::string locStr = FormatLocation(vResult.location);
			ImGui::TextUnformatted(locStr.c_str());

			// Dynamic Stat columns
			int colIndex = 3;
			for (const auto& sid : selectedStats | std::views::keys) {
				ImGui::TableSetColumnIndex(colIndex++);
				//Everything is normally uint8_t, int, or char - For floats, exceptions are made.
				switch (sid) {
					case Stat_Ratio:
					{
						const float val = GetStatValueFloat(pItemDef, sid);
						if (val != 0.0f) {
							ImGui::Text("%2.3f", val);
						}
						else {
							ImGui::TextUnformatted("");
						}
					}

					break;
					case Stat_Efficiency:
					{
						const int val = GetStatValue(pItemDef, sid);
						if (val != 0) {
							ImGui::Text("%d", val);
						}
						else {
							ImGui::TextUnformatted("");
						}
					}

					break;

					default://This is for anything Int.
					{
						const int val = GetStatValue(pItemDef, sid);
						if (val != 0) {
							ImGui::Text("%d", val);
						}
						else {
							ImGui::TextUnformatted("");
						}
					}

					break;
				}
			}
		}

		ImGui::EndTable();
	}

	ImGui::EndChild();

	//Left panel with drop downs and text search.
	// Option to save a search (button)
	// Option to load a search (drop down)
	//Location
	// Slot
	//Stat
	//Race
	//Class
	//Type
	//Prestige? - !IS_EMU_CLIENT - !TestServer?


	//End main window
	ImGui::PopStyleVar(iPushPopVar);
	ImGui::PopStyleColor(iPushPopColor);
	ImGui::End();
}

static void GetContainedAugs(const ItemClient* pItem, const int topSlotIndex, const int bagSlotIndex) {
	if (!pItem) {
		return;
	}

	if (pItem->IsContainer()) {
		return;
	}

	const ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return;
	}

	for (int i = 0; i <= MAX_AUG_SOCKETS; i++) {
		if (pItemDef->AugData.Sockets[i].bVisible) {
			ItemPtr pAug = pItem->GetHeldItem(i);
			if (!pAug) {
				continue;
			}

			OutPutItemDetails(pAug, topSlotIndex, bagSlotIndex, pItem->GetName(), true, i);
		}
	}
}

void OutPutItemDetails(ItemPtr pItem, const int topSlotIndex, const int bagSlotIndex, const char* hostName, const bool isAug, const int augSlotIndex) {
	if (!pItem) {
		return;
	}

	ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return;
	}

#ifdef DEBUGGING
	if (pItem->GetItemClass() == ItemClass_None) {
		WriteChatf("\arItemClass_None found on \ap%s", pItemDef->Name);
	}
#endif

	if (DoesItemMatchFilters(pItem.get())) {
		QueryResult res;
		res.item = pItem;
		res.location.loc = g_CurrentScanLocation;
		res.location.topSlotIndex = topSlotIndex;
		res.location.bagSlotIndex = bagSlotIndex;
		res.location.isAug = isAug;
		res.location.augSlotIndex = augSlotIndex;
		if (hostName) {
			res.location.hostName = hostName;
		}
		else {
			res.location.hostName.clear();
		}
		vResults.emplace_back(std::move(res));
	}

	//If an item is a container
	if (pItem->IsContainer()) {//bool
		const auto& held = pItem->GetHeldItems();
		for (int si = 0; si < held.GetSize(); ++si) {
			if (const ItemPtr ContainerItem = pItem->GetHeldItem(si)) {
				OutPutItemDetails(ContainerItem, topSlotIndex, si, nullptr, false, -1);
			}
		}
	}

	//Gets augs currently socketed in items.
	GetContainedAugs(pItem.get(), topSlotIndex, bagSlotIndex);

	return;

	/*Everyting below this return in this function
	* is a search for information and will be purged eventually
	*/

	//Evolving Items
	///*0x10c*/ bool                  IsEvolvingItem;
	// if (pItem->IsEvolvingItem) {
	// 	//bool converted to const char array, int, int, double
	// 	WriteChatf("Evolving Item Status: %s Level: %d/%d Exp: %2.2f", (pItem->EvolvingExpOn ? "On" : "Off"), pItem->EvolvingCurrentLevel, pItem->EvolvingMaxLevel, pItem->EvolvingExpPct);
	// }
	//
	//
	// //StackCount - This is how many are in a stack.
	// if (pItem->IsStackable() && pItem->StackCount) {
	// 	WriteChatf("Stackable to: %d Currently contains: %d", pItemDef->StackSize, pItem->StackCount);
	// }
	//
	// for (uint8_t i = eqlib::ItemSpellType_Clicky; i < ItemSpellType_Max; i++) {
	// 	eqlib::ItemSpellTypes currentType = static_cast<eqlib::ItemSpellTypes>(i);
	// 	if (const ItemSpellData::SpellData* pSpellData = pItemDef->GetSpellData(currentType)) {
	// 		EQ_Spell* pSpell = GetSpellByID(pSpellData->SpellID);
	// 		if (!pSpell) {
	// 			continue;
	// 		}
	//
	// 		switch (currentType) {
	// 			case ItemSpellType_Clicky:
	// 				WriteChatf("Cliky Spell: %s CastTime: %2.2f Lvl Req: %hhu Charges: %d", pSpell->Name, pSpellData->CastTime * 0.001f, pSpellData->RequiredLevel, pItem->Charges);
	// 				break;
	// 			case ItemSpellType_Proc:
	// 				WriteChatf("Proc: %s ProcRate: %d", pSpell->Name, pSpellData->ProcRate);
	// 				break;
	// 			case ItemSpellType_Worn:
	// 				WriteChatf("Worn Spell: %s", pSpell->Name);
	// 				break;
	// 			case ItemSpellType_Focus:
	// 				WriteChatf("Focus Spell: %s", pSpell->Name);
	// 				break;
	// 			case ItemSpellType_Scroll:
	// 				break;
	// 			case ItemSpellType_Focus2:
	// 				WriteChatf("Secondary Spell: %s", pSpell->Name);
	// 				break;
	// 			default:
	// 				WriteChatf("Unaccounted for ItemSpellType encountered i: %d", i);
	// 		}
	//
	// 	}
	// }
}

static bool bHadSearchResults = false;
PLUGIN_API void OnBeginZone() {
	if (!vResults.empty()) {
		bHadSearchResults = true;
	}

	vResults.clear();//When we zone, the ItemClient* is no longer value. So we need to clear the results.
}

PLUGIN_API void OnZoned() {
	if (bHadSearchResults) {
		DoCommand("/searchitem");
		bHadSearchResults = false;
	}
}

static std::string SanitizeFileName(const std::string& name) {
	std::string out;
	out.reserve(name.size());
	for (const char c : name) {
		if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*') {
			out += '_';
		}
		else {
			out += c;
		}
	}

	//trim spaces
	while (!out.empty() && (out.back() == ' ' || out.back() == '.')) {
		out.pop_back();
	}

	if (out.empty()) {
		out = "Search";
	}

	return out;
}

static std::string GetSavedSearchesDir() {
	return (fs::path(gPathConfig) / "SavedSearches").string();
}

static std::string GetSearchFilePath(const std::string& name) {
	const std::string cleaned = SanitizeFileName(name);
	const fs::path p = fs::path(GetSavedSearchesDir()) / (cleaned + ".ini");
	return p.string();
}

static void EnsureSavedSearchDir() {
	std::error_code ec;
	fs::create_directories(GetSavedSearchesDir(), ec);
}

static bool SaveCurrentSearchToFile(const std::string& path) {
	WritePrivateProfileString("General", "Search", szSearchText, path);
	WritePrivateProfileBool("General", "OnlyDroppable", bOnlyShowDroppable, path);
	WritePrivateProfileBool("General", "OnlyNoDrop", bOnlyShowNoDrop, path);
	WritePrivateProfileString("General", "ReqMin", ReqMin, path);
	WritePrivateProfileString("General", "ReqMax", ReqMax, path);
	WritePrivateProfileString("General", "RecMin", RecMin, path);
	WritePrivateProfileString("General", "RecMax", RecMax, path);

	// Menu selections
	for (auto& [type, data] : MenuData) {
		std::ostringstream oss;
		bool first = true;
		for (const auto& opt : data.OptionList) {
			if (opt.IsSelected) {
				if (!first) {
					oss << ',';
				}
				first = false;
				oss << std::to_string(opt.ID);
			}
		}
		const std::string section = std::string("Type_") + std::to_string(static_cast<int>(type));
		WritePrivateProfileString(section, "SelectedIDs", oss.str(), path);
	}

	return true;
}

static bool LoadSearchFromFile(const std::string& path) {

	const std::string sSearch = GetPrivateProfileString("General", "Search", "", path);
	strcpy_s(szSearchText, sSearch.c_str());
	bOnlyShowDroppable = GetPrivateProfileBool("General", "OnlyDroppable", false, path);
	bOnlyShowNoDrop = GetPrivateProfileBool("General", "OnlyNoDrop", false, path);
	const std::string reqMin = GetPrivateProfileString("General", "ReqMin", ReqMin, path);
	const std::string reqMax = GetPrivateProfileString("General", "ReqMax", ReqMax, path);
	const std::string recMin = GetPrivateProfileString("General", "RecMin", RecMin, path);
	const std::string recMax = GetPrivateProfileString("General", "RecMax", RecMax, path);
	strcpy_s(ReqMin, reqMin.c_str());
	strcpy_s(ReqMax, reqMax.c_str());
	strcpy_s(RecMin, recMin.c_str());
	strcpy_s(RecMax, recMax.c_str());

	//parse CSV selections
	auto parseCsv = [](const std::string& csv) {
		std::vector<int> ids;
		std::stringstream ss(csv);
		std::string token;
		while (std::getline(ss, token, ',')) {
			if (!token.empty()) {
				ids.push_back(GetIntFromString(token, 0));
			}
		}

		return ids;
	};

	for (auto& [type, data] : MenuData) {
		const std::string section = std::string("Type_") + std::to_string(static_cast<int>(type));
		const std::string csv = GetPrivateProfileString(section, "SelectedIDs", "", path);
		const auto ids = parseCsv(csv);

		//reset all first
		for (auto& opt : data.OptionList) {
			opt.IsSelected = false;
		}

		for (const int id : ids) {
			for (auto& opt : data.OptionList) {
				if (opt.ID == id) { opt.IsSelected = true; break; }
			}
		}
	}

	//auto-run search after loading
	EzCommand("/searchitem");
	return true;
}

static bool DeleteSearchFile(const std::string& path) {
	std::error_code ec;
	return fs::remove(path, ec);
}

static std::vector<std::string> ListSavedSearches() {
	std::vector<std::string> names;
	std::error_code ec;
	const fs::path dir = GetSavedSearchesDir();
	if (!fs::exists(dir, ec)) {
		return names;
	}

	for (const auto& entry : fs::directory_iterator(dir, ec)) {
		if (entry.is_regular_file()) {
			const auto& p = entry.path();
			if (p.has_extension() && p.extension() == ".ini") {
				names.emplace_back(p.stem().string());
			}
		}
	}

	std::sort(names.begin(), names.end());
	return names;
}

void PopulateAllItems(PlayerClient* pChar, const char* szArgs) {
	if (!pLocalPC) {
		return;
	}

	if (szArgs && szArgs[0] != '\0') {
		strcpy_s(szSearchText, szArgs);
	}

	if (!ShowMQSearchItemWindow) {
		ShowMQSearchItemWindow = !ShowMQSearchItemWindow;
	}

	vResults.clear();//Must clear this list or every time you hit find it just adds to it.

	const auto& LocationData = MenuData[OptionType_Location].OptionList;
	const bool anyLocationSelected = IsAnySelected(LocationData);

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Equipped].IsSelected) {
		g_CurrentScanLocation = Loc_Equipped;
		for (int iWornSlot = InvSlot_FirstWornItem; iWornSlot <= InvSlot_LastWornItem; iWornSlot++) {
			if (const ItemPtr pItem = pLocalPC->GetInventorySlot(iWornSlot)) {
				OutPutItemDetails(pItem, iWornSlot, -1, nullptr, false, -1);
			}
		}
	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Bags].IsSelected) {
		g_CurrentScanLocation = Loc_Bags;
		for (int iBagSlot = InvSlot_FirstBagSlot; iBagSlot <= InvSlot_LastBagSlot; iBagSlot++) {
			if (const ItemPtr pItem = pLocalPC->GetInventorySlot(iBagSlot)) {
				OutPutItemDetails(pItem, iBagSlot, -1, nullptr, false, -1);
			}
		}
	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Bank].IsSelected) {
		g_CurrentScanLocation = Loc_Bank;
		for (int i = 0; i < NUM_BANK_SLOTS; i++) {
			if (const ItemPtr pItem = pLocalPC->BankItems.GetItem(i)) {
				OutPutItemDetails(pItem, i, -1, nullptr, false, -1);
			}
		}

	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Shared_Bank].IsSelected) {
		g_CurrentScanLocation = Loc_Shared_Bank;
		for (int i = 0; i < NUM_SHAREDBANK_SLOTS; i++) {
			if (const ItemPtr pItem = pLocalPC->SharedBankItems.GetItem(i)) {
				OutPutItemDetails(pItem, i, -1, nullptr, false, -1);
			}
		}
	}

#if (!IS_EMU_CLIENT)
	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_TradeSkillDepot].IsSelected) {
		g_CurrentScanLocation = Loc_TradeSkillDepot;
		// Populate later?
	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_DragonsHorde].IsSelected) {
		g_CurrentScanLocation = Loc_DragonsHorde;
		// Populate later?
	}
#endif
}
