// MQFindItemWnd.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup.

#include <mq/Plugin.h>

PreSetup("MQFindItemWnd");
PLUGIN_VERSION(0.1);
//#define DEBUGGING

struct Option {
	std::string Name;
	bool IsSelected = false;
	int ID;
	Option(std::string Name, int ID) :Name(Name), ID(ID) {}
};

/*
* Do not under any circumstances re-order the enums unless they change in EQ.
* These are in order so that the options line up with the results and they can
* be compared in a loop!
*
* If you want to change their order in the list in the window (visible order)
* then change the order in the std::map<OptionType, DropDownOption> MenuData
*/

enum LocationID {
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

enum SlotID {
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

enum RaceID {
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

enum ClassID {
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

enum ItemTypeID {
	ItemType_1H_Slashing,
	ItemType_2H_Slashing,
	ItemType_1H_Piercing,
	ItemType_1H_Blunt,
	ItemType_2H_Blunt,
	ItemType_Bow,
	//What is 6?
	ItemType_Throwing = 7,
	ItemType_Shield,
	//What is 9
	ItemType_Armor = 10,
	ItemType_Misc,
	ItemType_Lockpicks, /**/
	//What is 13?
	ItemType_Food = 14,
	ItemType_Drink,
	ItemType_Light,
	ItemType_Combinable,
	ItemType_Bandage,
	ItemType_Ammo, /*,*/
	ItemType_Spell,/*ItemType_Scroll,*/
	ItemType_Potion,
	//What is 22?
	ItemType_Wind_Instrument = 23,
	ItemType_Stringed_Instrument,
	ItemType_Brass_Instruments,
	ItemType_Percussion_Instrument,
	ItemType_Arrow,
	//What is 28?
	ItemType_Jewelry = 29,
	ItemType_Skull,
	ItemType_Book,
	ItemType_Note,
	ItemType_Key,
	ItemType_Coin,
	ItemType_2H_Piercing,
	ItemType_FishingPole,
	ItemType_FishingBait,
	ItemType_Alcohol,
	ItemType_KeyV2,
	ItemType_Compass,
	//What is 41?
	ItemType_Poison = 42,
	//What is 43?
	//What is 44?
	ItemType_Martial = 45,
	//What is 46?
	//What is 47?
	//What is 48?
	//What is 49?
	//What is 50?
	//What is 51?
	ItemType_Charm = 52,
	ItemType_ArmorDye,
	ItemType_Augmentation,//Did not function
	ItemType_AugSolvent,
	ItemType_AugDistiller,
	//What is 57?
	ItemType_Banner = 58,
	//What is 59?
	ItemType_RecipeBook = 60,


	ItemType_AltCurrency = 63,
	ItemType_PerfectedDistiller,


	ItemType_Mount = 68,
	ItemType_Illusion,
	ItemType_Familiar = 70,//Last "real" ItemType

	ItemType_All_Effects,//Will have to combine all the look ups for effects?
	ItemType_Collectible,//pItem->Collectible
	ItemType_Container,//pItem->IsContainer
	ItemType_Focus_Effect,//pItem->Focus stuff
	ItemType_Placeable,//pItem->Placable

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

enum AugSlotID {
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
#if (!IS_EMU_CLIENT)
	OptionType_Prestige,
#endif
	OptionType_AugSlots
};

std::map<OptionType, std::string> DropDownOptions{
	{OptionType_Location, "Location"},
	{OptionType_Slots, "Slots"},
	{OptionType_Stats, "Stats"},
	{OptionType_Race, "Races"},
	{OptionType_Class, "Classes"},
	{OptionType_ItemType, "Types"},
#if (!IS_EMU_CLIENT)
	{OptionType_Prestige, "Prestige"},
#endif
	{OptionType_AugSlots, "AugSlots"}
};

std::map<OptionType, DropDownOption> MenuData = {
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

	{OptionType_Slots, { {
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

	{OptionType_Race, { {
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
			/*Option("Alternate Ability", ItemType_Alternate_Ability),*/
			Option("All Effects", ItemType_All_Effects),
			Option("Armor", ItemType_Armor),
			Option("Arrow", ItemType_Arrow),
			Option("Augmentation", ItemType_Augmentation),
			Option("Bandage", ItemType_Bandage),
			Option("Book", ItemType_Book),
			Option("Bow", ItemType_Bow),
			Option("Brass Instruments", ItemType_Brass_Instruments),
			Option("Collectible", ItemType_Collectible),
			Option("Combinable", ItemType_Combinable),
			Option("Container", ItemType_Container),
			Option("Drink", ItemType_Drink),
			Option("Focus Effect", ItemType_Focus_Effect),
			Option("Food", ItemType_Food),
			Option("Jewelry", ItemType_Jewelry),
			Option("Key", ItemType_Key),
			Option("Light", ItemType_Light),
			Option("Martial", ItemType_Martial),
			Option("Misc", ItemType_Misc),
			Option("Note", ItemType_Note),
			Option("Percussion Instrument", ItemType_Percussion_Instrument),
			Option("Placeable", ItemType_Placeable),
			Option("Potion", ItemType_Potion),
			/*Option("Scroll", ItemType_Scroll),*/
			Option("Shield", ItemType_Shield),
			Option("Stringed Instrument", ItemType_Stringed_Instrument),
			Option("Throwing", ItemType_Throwing),
			Option("Wind Instrument", ItemType_Wind_Instrument)
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

constexpr MQColor light_blue = MQColor(84, 172, 210);
constexpr MQColor grey = MQColor(37, 37, 37);
constexpr MQColor black = MQColor(12, 12, 12);
constexpr MQColor blue = MQColor(0, 0, 255);
constexpr MQColor teal = MQColor(0, 255, 255);
constexpr MQColor green = MQColor(0, 255, 0);
constexpr MQColor magenta = MQColor(255, 0, 255);
constexpr MQColor orange = MQColor(255, 153, 0);
constexpr MQColor purple = MQColor(128, 0, 128);
constexpr MQColor red = MQColor(255, 0, 0);
constexpr MQColor white = MQColor(255, 255, 255);
constexpr MQColor yellow = MQColor(255, 255, 0);

//dark colors
constexpr MQColor black_dark = MQColor(12, 12, 12);
constexpr MQColor blue_dark = MQColor(0, 0, 153);
constexpr MQColor grey_dark = MQColor(28, 28, 28);
constexpr MQColor teal_dark = MQColor(0, 153, 153);
constexpr MQColor green_dark = MQColor(0, 153, 0);
constexpr MQColor magenta_dark = MQColor(255, 0, 255);
constexpr MQColor orange_dark = MQColor(153, 102, 0);
constexpr MQColor purple_dark = MQColor(153, 0, 153);
constexpr MQColor red_dark = MQColor(230, 0, 0);
constexpr MQColor white_dark = MQColor(255, 255, 255);
constexpr MQColor yellow_dark = MQColor(153, 153, 0);

//light colors

constexpr MQColor grey_light = { 166, 166, 166 }; // might want to make this MQColor(166, 166, 166); to match the others

static bool bOnlyShowDroppable = false;
static bool bOnlyShowNoDrop = false;


void PopulateListBoxes() {
	static const ImVec2 listBoxSize = ImVec2(150, 100);
	for (auto& [type, data] : MenuData) {

		//Label for this section
		ImGui::Text("%s", DropDownOptions[type].c_str());

		//Clear button for each listbox right aligned
		ImGui::SameLine(ImGui::GetWindowWidth() - 70);
		std::string clearBtnLabel = "Clear##" + std::to_string(type);
		if (ImGui::SmallButton(clearBtnLabel.c_str())) {
			for (auto& opt : data.OptionList) opt.IsSelected = false;
		}

		//UniqueId for each listbox.
		std::string internalID = "##List" + std::to_string(type);
		if (ImGui::BeginListBox(internalID.c_str(), listBoxSize)) {
			for (auto& opt : data.OptionList) {
				//Render each item as a Selectable
				//If it's clicked, it toggles its own bool directly
				if (ImGui::Selectable(opt.Name.c_str(), opt.IsSelected)) {
					opt.IsSelected = !opt.IsSelected;
				}
			}
			ImGui::EndListBox();
		}
		ImGui::Spacing();
	}
}

void GetRaces(ItemClient* pItem, std::vector<int>& vVector) {
	if (!pItem)
		return;

	int cmp = GetItemFromContents(pItem)->Races;
	for (int num = 0; num < NUM_RACES; num++) {
		if (cmp & (1 << num)) {
			int tmp = num + 1;
			vVector.emplace_back(tmp);
			//Can output all the races like this.
			//WriteChatf("%s", pEverQuest->GetRaceDesc(tmp));
		}
	}
}

/*
This will check if any of the options were selected in a category
We can use that to determine if we should check the filters at all.
*/
bool IsAnySelected(const std::vector<Option>& OptionData) {
	//None is all. Basically if nothing is selected, then any will do.
	for (auto& option : OptionData) {
		if (option.IsSelected) {
			return true;
		}
	}

	return false;
}

bool MatchesRaces(ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	auto& raceData = MenuData[OptionType_Race].OptionList;
	bool anyRaceSelected = IsAnySelected(raceData);

	if (anyRaceSelected) {
		int cmp = GetItemFromContents(pItem)->Races;
		bool foundMatch = false;
		std::vector<int> Races;
		GetRaces(pItem, Races);
		for (auto& option : raceData) {
			if (option.IsSelected) {
				//Check if the race is found by Option.ID
				auto it = std::find(Races.begin(), Races.end(), option.ID);
				if (it != Races.end()) {
					foundMatch = true;
					break;//If one of the races is found, we don't need to check any others.
				}
			}
		}

		return foundMatch;
	}

	return true;
}

void GetClasses(ItemClient* pItem, std::vector<int>& vVector) {
	if (!pItem)
		return;

	int cmp = GetItemFromContents(pItem)->Classes;
	bool foundMatch = false;
	for (int num = 0; num < TotalPlayerClasses; num++) {
		if (cmp & (1 << num)) {
			int tmp = num + 1;
			vVector.emplace_back(tmp);
			//Can output all the races like this.
			//WriteChatf("%s", pEverQuest->GetClassThreeLetterCode(tmp));
		}
	}
}

bool MatchesClasses(ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	auto& classData = MenuData[OptionType_Class].OptionList;
	bool anyClassSelected = IsAnySelected(classData);

	if (anyClassSelected) {
		bool foundMatch = false;
		std::vector<int> Classes;
		GetClasses(pItem, Classes);
		for (auto& option : classData) {
			if (option.IsSelected) {
				auto it = std::find(Classes.begin(), Classes.end(), option.ID);
				if (it != Classes.end()) {
					foundMatch = true;
					break;//Only need to match one filter.
				}
			}
		}

		return foundMatch;
	}

	return true;
}

void GetSlots(ItemClient* pItem, std::vector<int>& vVector) {
	if (!pItem)
		return;

	int cmp = GetItemFromContents(pItem)->EquipSlots;
	bool foundMatch = false;
	for (int i = 0; i < NUM_WORN_ITEMS; i++) {
		if (cmp & (1 << i)) {
			vVector.emplace_back(i);
			//Can output all the equippable slots like this.
			//WriteChatf("\a-tWorn Slot for: %s -> [%d]: %s", pItem->GetItemDefinition()->Name, i, szItemSlot[i]);
		}
	}
}

bool MatchesSlots(ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	auto& slotData = MenuData[OptionType_Slots].OptionList;
	bool anySlotSelected = IsAnySelected(slotData);

	if (anySlotSelected) {
		bool foundMatch = false;
		std::vector<int> Slots;
		GetSlots(pItem, Slots);
		for (auto& option : slotData) {
			if (option.IsSelected) {
				//WriteChatf("Option: %s selected", option.Name);
				auto it = std::find(Slots.begin(), Slots.end(), option.ID);
				if (it != Slots.end()) {
					foundMatch = true;
					//WriteChatf("\a-tWorn Slot for: %s -> [%d]: %s", pItem->GetItemDefinition()->Name, *it, szItemSlot[*it]);
					break;
				}
			}
		}

		return foundMatch;
	}

	return true;
}

bool MatchesItemType(ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
	}

	auto& itemTypeData = MenuData[OptionType_ItemType].OptionList;
	bool anySlotSelected = IsAnySelected(itemTypeData);

	if (anySlotSelected) {
		bool foundMatch = false;
		for (auto& option : itemTypeData) {
			if (option.IsSelected) {
				if (pItem->GetItemClass() == option.ID) {
					foundMatch = true;
					break;
				}
			}
		}

		WriteChatf("Item: \ap%s is ItemClass: %s", pItemDef->Name, szItemClasses[pItem->GetItemClass()]);
		return foundMatch;
	}

	return true;
}

bool DoesItemMatchFilters(ItemClient* pItem) {
	if (!pItem) {
		return false;
	}

	ItemDefinition* pItemDef = pItem->GetItemDefinition();
	if (!pItemDef) {
		return false;
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

	return true;
}

/**
 * Avoid Globals if at all possible, since they persist throughout your program.
 * But if you must have them, here is the place to put them.
 */
bool ShowMQFindItemWndWindow = true;
bool bItemsPopulated = false;
std::vector<std::string> vItemList;
void PopulateAllItems(PlayerClient* pChar, const char* szArgs);
/**
 * @fn InitializePlugin
 *
 * This is called once on plugin initialization and can be considered the startup
 * routine for the plugin.
 */
PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQFindItemWnd::Initializing version %f", MQ2Version);

	// Examples:
	// AddCommand("/mycommand", MyCommand);
	AddCommand("/searchitem", PopulateAllItems);
	// AddXMLFile("MQUI_MyXMLFile.xml");
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
	DebugSpewAlways("MQFindItemWnd::Shutting down");
	RemoveCommand("/searchitem");
	// Examples:
	// RemoveCommand("/mycommand");
	// RemoveXMLFile("MQUI_MyXMLFile.xml");
	// RemoveMQ2Data("mytlo");
}

/**
 * @fn OnCleanUI
 *
 * This is called once just before the shutdown of the UI system and each time the
 * game requests that the UI be cleaned.  Most commonly this happens when a
 * /loadskin command is issued, but it also occurs when reaching the character
 * select screen and when first entering the game.
 *
 * One purpose of this function is to allow you to destroy any custom windows that
 * you have created and cleanup any UI items that need to be removed.
 */
PLUGIN_API void OnCleanUI()
{
	// DebugSpewAlways("MQFindItemWnd::OnCleanUI()");
}

/**
 * @fn OnReloadUI
 *
 * This is called once just after the UI system is loaded. Most commonly this
 * happens when a /loadskin command is issued, but it also occurs when first
 * entering the game.
 *
 * One purpose of this function is to allow you to recreate any custom windows
 * that you have setup.
 */
PLUGIN_API void OnReloadUI()
{
	// DebugSpewAlways("MQFindItemWnd::OnReloadUI()");
}

/**
 * @fn OnDrawHUD
 *
 * This is called each time the Heads Up Display (HUD) is drawn.  The HUD is
 * responsible for the net status and packet loss bar.
 *
 * Note that this is not called at all if the HUD is not shown (default F11 to
 * toggle).
 *
 * Because the net status is updated frequently, it is recommended to have a
 * timer or counter at the start of this call to limit the amount of times the
 * code in this section is executed.
 */
PLUGIN_API void OnDrawHUD()
{
/*
	static std::chrono::steady_clock::time_point DrawHUDTimer = std::chrono::steady_clock::now();
	// Run only after timer is up
	if (std::chrono::steady_clock::now() > DrawHUDTimer)
	{
		// Wait half a second before running again
		DrawHUDTimer = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
		DebugSpewAlways("MQFindItemWnd::OnDrawHUD()");
	}
*/
}

/**
 * @fn SetGameState
 *
 * This is called when the GameState changes.  It is also called once after the
 * plugin is initialized.
 *
 * For a list of known GameState values, see the constants that begin with
 * GAMESTATE_.  The most commonly used of these is GAMESTATE_INGAME.
 *
 * When zoning, this is called once after @ref OnBeginZone @ref OnRemoveSpawn
 * and @ref OnRemoveGroundItem are all done and then called once again after
 * @ref OnEndZone and @ref OnAddSpawn are done but prior to @ref OnAddGroundItem
 * and @ref OnZoned
 *
 * @param GameState int - The value of GameState at the time of the call
 */
PLUGIN_API void SetGameState(int GameState)
{
	// DebugSpewAlways("MQFindItemWnd::SetGameState(%d)", GameState);
}


/**
 * @fn OnPulse
 *
 * This is called each time MQ2 goes through its heartbeat (pulse) function.
 *
 * Because this happens very frequently, it is recommended to have a timer or
 * counter at the start of this call to limit the amount of times the code in
 * this section is executed.
 */
PLUGIN_API void OnPulse()
{
/*
	static std::chrono::steady_clock::time_point PulseTimer = std::chrono::steady_clock::now();
	// Run only after timer is up
	if (std::chrono::steady_clock::now() > PulseTimer)
	{
		// Wait 5 seconds before running again
		PulseTimer = std::chrono::steady_clock::now() + std::chrono::seconds(5);
		DebugSpewAlways("MQFindItemWnd::OnPulse()");
	}
*/
}

/**
 * @fn OnWriteChatColor
 *
 * This is called each time WriteChatColor is called (whether by MQ2Main or by any
 * plugin).  This can be considered the "when outputting text from MQ" callback.
 *
 * This ignores filters on display, so if they are needed either implement them in
 * this section or see @ref OnIncomingChat where filters are already handled.
 *
 * If CEverQuest::dsp_chat is not called, and events are required, they'll need to
 * be implemented here as well.  Otherwise, see @ref OnIncomingChat where that is
 * already handled.
 *
 * For a list of Color values, see the constants for USERCOLOR_.  The default is
 * USERCOLOR_DEFAULT.
 *
 * @param Line const char* - The line that was passed to WriteChatColor
 * @param Color int - The type of chat text this is to be sent as
 * @param Filter int - (default 0)
 */
PLUGIN_API void OnWriteChatColor(const char* Line, int Color, int Filter)
{
	// DebugSpewAlways("MQFindItemWnd::OnWriteChatColor(%s, %d, %d)", Line, Color, Filter);
}

/**
 * @fn OnIncomingChat
 *
 * This is called each time a line of chat is shown.  It occurs after MQ filters
 * and chat events have been handled.  If you need to know when MQ2 has sent chat,
 * consider using @ref OnWriteChatColor instead.
 *
 * For a list of Color values, see the constants for USERCOLOR_. The default is
 * USERCOLOR_DEFAULT.
 *
 * @param Line const char* - The line of text that was shown
 * @param Color int - The type of chat text this was sent as
 *
 * @return bool - Whether to filter this chat from display
 */
PLUGIN_API bool OnIncomingChat(const char* Line, DWORD Color)
{
	// DebugSpewAlways("MQFindItemWnd::OnIncomingChat(%s, %d)", Line, Color);
	return false;
}

/**
 * @fn OnAddSpawn
 *
 * This is called each time a spawn is added to a zone (ie, something spawns). It is
 * also called for each existing spawn when a plugin first initializes.
 *
 * When zoning, this is called for all spawns in the zone after @ref OnEndZone is
 * called and before @ref OnZoned is called.
 *
 * @param pNewSpawn PSPAWNINFO - The spawn that was added
 */
PLUGIN_API void OnAddSpawn(PSPAWNINFO pNewSpawn)
{
	// DebugSpewAlways("MQFindItemWnd::OnAddSpawn(%s)", pNewSpawn->Name);
}

/**
 * @fn OnRemoveSpawn
 *
 * This is called each time a spawn is removed from a zone (ie, something despawns
 * or is killed).  It is NOT called when a plugin shuts down.
 *
 * When zoning, this is called for all spawns in the zone after @ref OnBeginZone is
 * called.
 *
 * @param pSpawn PSPAWNINFO - The spawn that was removed
 */
PLUGIN_API void OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	// DebugSpewAlways("MQFindItemWnd::OnRemoveSpawn(%s)", pSpawn->Name);
}

/**
 * @fn OnAddGroundItem
 *
 * This is called each time a ground item is added to a zone (ie, something spawns).
 * It is also called for each existing ground item when a plugin first initializes.
 *
 * When zoning, this is called for all ground items in the zone after @ref OnEndZone
 * is called and before @ref OnZoned is called.
 *
 * @param pNewGroundItem PGROUNDITEM - The ground item that was added
 */
PLUGIN_API void OnAddGroundItem(PGROUNDITEM pNewGroundItem)
{
	// DebugSpewAlways("MQFindItemWnd::OnAddGroundItem(%d)", pNewGroundItem->DropID);
}

/**
 * @fn OnRemoveGroundItem
 *
 * This is called each time a ground item is removed from a zone (ie, something
 * despawns or is picked up).  It is NOT called when a plugin shuts down.
 *
 * When zoning, this is called for all ground items in the zone after
 * @ref OnBeginZone is called.
 *
 * @param pGroundItem PGROUNDITEM - The ground item that was removed
 */
PLUGIN_API void OnRemoveGroundItem(PGROUNDITEM pGroundItem)
{
	// DebugSpewAlways("MQFindItemWnd::OnRemoveGroundItem(%d)", pGroundItem->DropID);
}

/**
 * @fn OnBeginZone
 *
 * This is called just after entering a zone line and as the loading screen appears.
 */
PLUGIN_API void OnBeginZone()
{
	// DebugSpewAlways("MQFindItemWnd::OnBeginZone()");
}

/**
 * @fn OnEndZone
 *
 * This is called just after the loading screen, but prior to the zone being fully
 * loaded.
 *
 * This should occur before @ref OnAddSpawn and @ref OnAddGroundItem are called. It
 * always occurs before @ref OnZoned is called.
 */
PLUGIN_API void OnEndZone()
{
	// DebugSpewAlways("MQFindItemWnd::OnEndZone()");
}

/**
 * @fn OnZoned
 *
 * This is called after entering a new zone and the zone is considered "loaded."
 *
 * It occurs after @ref OnEndZone @ref OnAddSpawn and @ref OnAddGroundItem have
 * been called.
 */
PLUGIN_API void OnZoned()
{
	// DebugSpewAlways("MQFindItemWnd::OnZoned()");
}

/**
 * @fn OnUpdateImGui
 *
 * This is called each time that the ImGui Overlay is rendered. Use this to render
 * and update plugin specific widgets.
 *
 * Because this happens extremely frequently, it is recommended to move any actual
 * work to a separate call and use this only for updating the display.
 */

PLUGIN_API void OnUpdateImGui() {
	if (GetGameState() != GAMESTATE_INGAME) {
		return;
	}

	if (!ShowMQFindItemWndWindow) {
		return;
	}

#pragma region Styles
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 50.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200.0f, 150.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 9);
	// 5 push style Var; if we add more, make sure we pop
	const int iPushPopVar = 5;

	ImGui::PushStyleColor(ImGuiCol_FrameBg, grey.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, black.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_Tab, black.ToImU32());
	ImGui::PushStyleColor(ImGuiCol_TabActive, grey.ToImU32());
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
	constexpr int iPushPopColor = 13;
#pragma endregion Styles
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;

	//Begin main window
	if (!ImGui::Begin("MQFindItemWnd", &ShowMQFindItemWndWindow, ImGuiWindowFlags_MenuBar)) {
		ImGui::PopStyleVar(iPushPopVar);
		ImGui::PopStyleColor(iPushPopColor);
		ImGui::End();
		return;
	}


	if (ImGui::BeginMenuBar()) {
		if (ImGui::Button("Search")) {
			EzCommand("/searchitem");
		}
		ImGui::EndMenuBar();
	}

	ImGui::BeginChild("##FindItemOptions", ImVec2(180, ImGui::GetContentRegionAvail().y), 0, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::Button("Reset Options")) {

		}


		if (ImGui::Checkbox("Only Droppable", &bOnlyShowDroppable)) {
			//Just an example to save the selection between sessions.
			//WriteINI("FindItemOption", "OnlyDroppable", bbOnlyShowDroppable, ThisINIFileName);
		}

		if (ImGui::Checkbox("Only NoDrop", &bOnlyShowNoDrop)) {

		}

		PopulateListBoxes();

		ImGui::EndChild();

	//Results on right
	ImGui::SameLine();
	ImGui::BeginChild("##FindItemResults", ImVec2(0, 0), ImGuiChildFlags_Borders);

		static int item_current_idx = 0;
		if (ImGui::BeginListBox("##Results", ImGui::GetContentRegionAvail())) {
			for (size_t i = 0; i < vItemList.size(); i++) {
				ImGui::PushID(i);
					const bool is_selected = (item_current_idx == i);

				if (ImGui::Selectable(vItemList.at(i).c_str(), is_selected)) {
					item_current_idx = static_cast<int>(i);
				}

				//Write the tooltip.
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(vItemList.at(i).c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}

				ImGui::PopID();
			}

			ImGui::EndListBox();
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

/**
 * @fn OnMacroStart
 *
 * This is called each time a macro starts (ex: /mac somemacro.mac), prior to
 * launching the macro.
 *
 * @param Name const char* - The name of the macro that was launched
 */
PLUGIN_API void OnMacroStart(const char* Name)
{
	// DebugSpewAlways("MQFindItemWnd::OnMacroStart(%s)", Name);
}

/**
 * @fn OnMacroStop
 *
 * This is called each time a macro stops (ex: /endmac), after the macro has ended.
 *
 * @param Name const char* - The name of the macro that was stopped.
 */
PLUGIN_API void OnMacroStop(const char* Name)
{
	// DebugSpewAlways("MQFindItemWnd::OnMacroStop(%s)", Name);
}

/**
 * @fn OnLoadPlugin
 *
 * This is called each time a plugin is loaded (ex: /plugin someplugin), after the
 * plugin has been loaded and any associated -AutoExec.cfg file has been launched.
 * This means it will be executed after the plugin's @ref InitializePlugin callback.
 *
 * This is also called when THIS plugin is loaded, but initialization tasks should
 * still be done in @ref InitializePlugin.
 *
 * @param Name const char* - The name of the plugin that was loaded
 */
PLUGIN_API void OnLoadPlugin(const char* Name)
{
	// DebugSpewAlways("MQFindItemWnd::OnLoadPlugin(%s)", Name);
}

/**
 * @fn OnUnloadPlugin
 *
 * This is called each time a plugin is unloaded (ex: /plugin someplugin unload),
 * just prior to the plugin unloading.  This means it will be executed prior to that
 * plugin's @ref ShutdownPlugin callback.
 *
 * This is also called when THIS plugin is unloaded, but shutdown tasks should still
 * be done in @ref ShutdownPlugin.
 *
 * @param Name const char* - The name of the plugin that is to be unloaded
 */
PLUGIN_API void OnUnloadPlugin(const char* Name)
{
	// DebugSpewAlways("MQFindItemWnd::OnUnloadPlugin(%s)", Name);
}

void OutPutItemDetails(ItemClient* pItem) {
	if (!pItem) {
		return;
	}

	ItemDefinition* pItemDef = GetItemFromContents(pItem);
	if (!pItemDef) {
		return;
	}

	if (pItem->GetItemClass() == ItemClass_None) {
		WriteChatf("\arItemClass_None found on \ap%s", pItemDef->Name);
	}

	if (DoesItemMatchFilters(pItem)) {
		vItemList.emplace_back(pItemDef->Name);
	}

	//If an item is a container
	if (pItem->IsContainer()) {//bool
		for (ItemClient* ContainerItem : pItem->Contents) {
			if (ContainerItem) {
				OutPutItemDetails(ContainerItem);//Recursive call - bad idea?
			}
		}
	}

	return;

	/*Everyting below this return in this function
	* is a search for information and will be purged eventually
	*/




	if (pItem->GetType()) {//uint8_t
		WriteChatf("Type: pItem->GetType() %u", pItem->GetType());
	}

	//Is Armor Type?? Perhaps we'll be able to search for Chain, Plate, Cloth etc?
	///*0x0f0*/ int                   ArmorType;
	if (pItem->ArmorType) {
		WriteChatf("ArmorType: %d", pItem->ArmorType);
	}

	///*0x00c*/ int                   AugFlag;
	if (pItem->AugFlag) {
		//This is an aug?
		WriteChatf("AugFlag: %d", pItem->AugFlag);
	}

	//Has value?
	///*0x028*/ int                   Price;
	if (pItem->Price) {
		WriteChatf("Price: %d", pItem->Price);
	}

	///*0x074*/ bool                  bRealEstateItemPlaceable;
	if (pItem->bRealEstateItemPlaceable) {
		WriteChatf("Realestate Placeable");
	}

	//Is NoDrop?
	///*0x080*/ int                   NoDropFlag;
	if (pItem->NoDropFlag) {
		//Sort out the int for this flag. Determine if we're on a free loot server etc?
		WriteChatf("NoDropFlag: %d", pItem->NoDropFlag);
	}

	//Is Ornamentation?
	///*0x084*/ int                   OrnamentationIcon;
	if (pItem->OrnamentationIcon) {
		WriteChatf("OrnamentationIcon: %d", pItem->OrnamentationIcon);
	}

	//StackCount - This is how many are in a stack.
	if (pItem->IsStackable() && pItem->StackCount) {
		WriteChatf("Stackable to: %d Currently contains: %d", pItemDef->StackSize, pItem->StackCount);
	}

	//This is if it can be stacked.
	/*if (pItem->IsStackable()) {

	}*/

	//Evolving Items
	///*0x10c*/ bool                  IsEvolvingItem;
	if (pItem->IsEvolvingItem) {
		//bool converted to const char array, int, int, double
		WriteChatf("Evolving Item Status: %s Level: %d/%d Exp: %2.2f", (pItem->EvolvingExpOn ? "On" : "Off"), pItem->EvolvingCurrentLevel, pItem->EvolvingMaxLevel, pItem->EvolvingExpPct);
	}

	for (uint8_t i = eqlib::ItemSpellType_Clicky; i < ItemSpellType_Max; i++) {
		eqlib::ItemSpellTypes currentType = static_cast<eqlib::ItemSpellTypes>(i);
		if (const ItemSpellData::SpellData* pSpellData = pItemDef->GetSpellData(currentType)) {
			EQ_Spell* pSpell = GetSpellByID(pSpellData->SpellID);
			if (!pSpell)
				continue;

			switch (currentType) {
				case ItemSpellType_Clicky:
					WriteChatf("Cliky Spell: %s CastTime: %2.2f Lvl Req: %hhu Charges: %d", pSpell->Name, pSpellData->CastTime * 0.001f, pSpellData->RequiredLevel, pItem->Charges);
					break;
				case ItemSpellType_Proc:
					WriteChatf("Proc: %s ProcRate: %d", pSpell->Name, pSpellData->ProcRate);
					break;
				case ItemSpellType_Worn:
					WriteChatf("Worn Spell: %s", pSpell->Name);
					break;
				case ItemSpellType_Focus:
					WriteChatf("Focus Spell: %s", pSpell->Name);
					break;
				case ItemSpellType_Scroll:
					break;
				case ItemSpellType_Focus2:
					WriteChatf("Secondary Spell: %s", pSpell->Name);
					break;
				default:
					WriteChatf("Unaccounted for ItemSpellType encountered i: %d", i);
			}

		}
	}

	if (pItemDef->AugData.Sockets) {

	}

	if (pItemDef->AugData.Sockets) {
		for (const eqlib::ItemAugmentationSocket& augslot : pItemDef->AugData.Sockets) {
			if (augslot.bVisible && augslot.Type != 0) {
				//augslot.Type
				//augslot.bInfusible
				WriteChatf("AugSlot: %d", augslot.Type);
			}
		}
	}
}

void PopulateAllItems(PlayerClient* pChar, const char* szArgs) {
	if (!pLocalPC) {
		return;
	}

	vItemList.clear();//Must clear this list or every time you hit find it just adds to it.

	auto& LocationData = MenuData[OptionType_Location].OptionList;
	bool anyLocationSelected = IsAnySelected(LocationData);

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Equipped].IsSelected) {
		for (int iWornSlot = InvSlot_FirstWornItem; iWornSlot <= InvSlot_LastWornItem; iWornSlot++) {
			if (ItemClient* pItem = pLocalPC->GetInventorySlot(iWornSlot)) {
				OutPutItemDetails(pItem);
			}
		}
	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Bags].IsSelected) {
		for (int iBagSlot = InvSlot_FirstBagSlot; iBagSlot <= InvSlot_LastBagSlot; iBagSlot++) {
			if (ItemClient* pItem = pLocalPC->GetInventorySlot(iBagSlot)) {
				OutPutItemDetails(pItem);
			}
		}
	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Bank].IsSelected) {
		for (int i = 0; i < NUM_BANK_SLOTS; i++) {
			if (ItemClient* pItem = pLocalPC->BankItems.GetItem(i)) {
				OutPutItemDetails(pItem);
			}
		}

	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_Shared_Bank].IsSelected) {
		for (int i = 0; i < NUM_SHAREDBANK_SLOTS; i++) {
			if (ItemClient* pItem = pLocalPC->SharedBankItems.GetItem(i)) {
				OutPutItemDetails(pItem);
			}
		}
	}

#if (!IS_EMU_CLIENT)
	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_TradeSkillDepot].IsSelected) {
		//Populate later?
	}

	if (!anyLocationSelected || MenuData[OptionType_Location].OptionList[Loc_DragonsHorde].IsSelected) {
		//Populate later?
	}
#endif
}