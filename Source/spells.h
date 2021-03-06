#ifndef OTSERV_SPELLS_H_
#define OTSERV_SPELLS_H_

#include "Actions.h"
#include "baseevents.h"
#include "definitions.h"
#include "game.h"
#include "luascript.h"
#include "player.h"
#include "talkaction.h"

class InstantSpell;
class ConjureSpell;
class RuneSpell;
class Spell;

typedef std::map<uint32_t, RuneSpell*> RunesMap;
typedef std::map<std::string, InstantSpell*> InstantsMap;

class Spells : public BaseEvents
{
public:
	Spells();
	~Spells() override;

	Spell* getSpellByName(const std::string& name);
	RuneSpell* getRuneSpell(uint32_t id);
	RuneSpell* getRuneSpellByName(const std::string& name);

	InstantSpell* getInstantSpell(const std::string words);
	InstantSpell* getInstantSpellByName(const std::string& name);

	uint32_t getInstantSpellCount(const Player* player);
	InstantSpell* getInstantSpellByIndex(const Player* player, uint32_t index);

	TalkActionResult_t playerSaySpell(Player* player, SpeakClasses type, const std::string& words);

	static int32_t spellExhaustionTime;
	static int32_t spellInFightTime;

	static Position getCasterPosition(Creature* creature, Direction dir);
	std::string getScriptBaseName() override;

protected:
	void clear() override;
	LuaScriptInterface& getScriptInterface() override;
	Event* getEvent(const std::string& nodeName) override;
	bool registerEvent(Event* event, xmlNodePtr p) override;

	RunesMap runes;
	InstantsMap instants;

	friend class CombatSpell;
	LuaScriptInterface m_scriptInterface;
};

typedef bool(InstantSpellFunction)(const InstantSpell* spell, Creature* creature, const std::string& param);
typedef bool(ConjureSpellFunction)(const ConjureSpell* spell, Creature* creature, const std::string& param);
typedef bool(RuneSpellFunction)(const RuneSpell* spell,
                                Creature* creature,
                                Item* item,
                                const Position& posFrom,
                                const Position& posTo);

class BaseSpell
{
public:
	BaseSpell(){};
	virtual ~BaseSpell(){};

	virtual bool castSpell(Creature* creature) = 0;
	virtual bool castSpell(Creature* creature, Creature* target) = 0;
};

class CombatSpell : public Event, public BaseSpell
{
public:
	CombatSpell(Combat* _combat, bool _needTarget, bool _needDirection);
	~CombatSpell() override;

	bool castSpell(Creature* creature) override;
	bool castSpell(Creature* creature, Creature* target) override;
	bool configureEvent(xmlNodePtr p) override
	{
		return true;
	}

	// scripting
	bool executeCastSpell(Creature* creature, const LuaVariant& var);

	bool loadScriptCombat();
	Combat* getCombat()
	{
		return combat;
	}

protected:
	std::string getScriptEventName() override
	{
		return "onCastSpell";
	}
	bool needDirection;
	bool needTarget;
	Combat* combat;
};

class Spell : public BaseSpell
{
public:
	Spell();
	~Spell() override{};

	bool configureSpell(xmlNodePtr xmlspell);
	const std::string& getName() const
	{
		return name;
	}

	void postCastSpell(Player* player, bool finishedSpell = true, bool payCost = true) const;
#ifdef __PROTOCOL_76__
	void postCastSpell(Player* player, uint32_t manaCost, uint32_t soulCost) const;
#else
	void postCastSpell(Player* player, uint32_t manaCost) const;
#endif // __PROTOCOL_76__

#ifdef __PROTOCOL_76__
	int32_t getSoulCost(const Player* player) const;
#endif // __PROTOCOL_76__
	int32_t getManaCost(const Player* player) const;
	uint32_t getLevel() const
	{
		return level;
	}
	uint32_t getMagicLevel() const
	{
		return magLevel;
	}
	int32_t getMana() const
	{
		return mana;
	}
	int32_t getManaPercent() const
	{
		return manaPercent;
	}
	const bool isPremium() const
	{
		return premium;
	}

	virtual bool isInstant() const = 0;
	bool isLearnable() const
	{
		return learnable;
	}
	bool isBlockingCreature() const
	{
		return blockingCreature;
	}

	static ReturnValue CreateIllusion(Creature* creature, const Outfit_t outfit, int32_t time);
	static ReturnValue CreateIllusion(Creature* creature, const std::string& name, int32_t time);
	static ReturnValue CreateIllusion(Creature* creature, uint32_t itemId, int32_t time);

protected:
	bool playerSpellCheck(Player* player) const;
	bool playerInstantSpellCheck(Player* player, const Position& toPos);
	bool playerRuneSpellCheck(Player* player, const Position& toPos);

	bool learnable;
	bool enabled;
	bool premium;
	uint32_t level;
	int32_t magLevel;

	int32_t mana;
	int32_t manaPercent;
	int32_t lvPercent;
#ifdef __PROTOCOL_76__
	int32_t soul;
#endif // __PROTOCOL_76__
	int32_t range;
	uint32_t customExhaust;
	bool exhaustion;
	bool needTarget;
	bool needWeapon;
	bool selfTarget;
	bool blockingSolid;
	bool blockingCreature;
	bool isAggressive;

	typedef std::map<int32_t, bool> VocSpellMap;
	VocSpellMap vocSpellMap;

private:
	std::string name;
};

class InstantSpell : public TalkAction, public Spell
{
public:
	InstantSpell(LuaScriptInterface* _interface);
	~InstantSpell() override;

	bool configureEvent(xmlNodePtr p) override;
	bool loadFunction(const std::string& functionName) override;

	virtual bool playerCastInstant(Player* player, const std::string& param);

	bool castSpell(Creature* creature) override;
	bool castSpell(Creature* creature, Creature* target) override;

	// scripting
	bool executeCastSpell(Creature* creature, const LuaVariant& var);

	bool isInstant() const override
	{
		return true;
	}
	bool getHasParam() const
	{
		return hasParam;
	}
	bool canCast(const Player* player) const;
	bool canThrowSpell(const Creature* creature, const Creature* target) const;

protected:
	std::string getScriptEventName() override;

	static InstantSpellFunction HouseGuestList;
	static InstantSpellFunction HouseSubOwnerList;
	static InstantSpellFunction HouseDoorList;
	static InstantSpellFunction HouseKick;
	static InstantSpellFunction SearchPlayer;
	static InstantSpellFunction SummonMonster;
	static InstantSpellFunction Levitate;
	static InstantSpellFunction Illusion;

	static House* getHouseFromPos(Creature* creature);

	bool internalCastSpell(Creature* creature, const LuaVariant& var);

	bool needDirection;
	bool hasParam;
	bool checkLineOfSight;
	bool casterTargetOrDirection;
	InstantSpellFunction* function;
};

class ConjureSpell : public InstantSpell
{
public:
	ConjureSpell(LuaScriptInterface* _interface);
	~ConjureSpell() override;

	bool configureEvent(xmlNodePtr p) override;
	bool loadFunction(const std::string& functionName) override;

	bool playerCastInstant(Player* player, const std::string& param) override;

	bool castSpell(Creature* creature) override
	{
		return false;
	}
	bool castSpell(Creature* creature, Creature* target) override
	{
		return false;
	}

	uint32_t getConjureId() const
	{
		return conjureId;
	}
	uint32_t getConjureCount() const
	{
		return conjureCount;
	}
	uint32_t getReagentId() const
	{
		return conjureReagentId;
	}

protected:
	std::string getScriptEventName() override;

	static ReturnValue internalConjureItem(Player* player, uint32_t conjureId, uint32_t conjureCount);
	static ReturnValue internalConjureItem(Player* player,
	                                       uint32_t conjureId,
	                                       uint32_t conjureCount,
	                                       uint32_t reagentId,
	                                       slots_t slot,
	                                       bool test = false);

	static ConjureSpellFunction ConjureItem;
	static ConjureSpellFunction ConjureFood;

	bool internalCastSpell(Creature* creature, const LuaVariant& var);
	Position getCasterPosition(Creature* creature);

	ConjureSpellFunction* function;

	uint32_t conjureId;
	uint32_t conjureCount;
	uint32_t conjureReagentId;
};

class RuneSpell : public Action, public Spell
{
public:
	RuneSpell(LuaScriptInterface* _interface);
	~RuneSpell() override;

	bool configureEvent(xmlNodePtr p) override;
	bool loadFunction(const std::string& functionName) override;

	ReturnValue canExecuteAction(const Player* player, const Position& toPos) override;
	bool hasOwnErrorHandler() override
	{
		return true;
	}

	bool executeUse(Player* player,
	                Item* item,
	                const PositionEx& posFrom,
	                const PositionEx& posTo,
	                bool extendedUse,
	                uint32_t creatureId) override;

	bool castSpell(Creature* creature) override;
	bool castSpell(Creature* creature, Creature* target) override;

	// scripting
	bool executeCastSpell(Creature* creature, const LuaVariant& var);

	bool isInstant() const override
	{
		return false;
	}
	uint32_t getRuneItemId()
	{
		return runeId;
	}

protected:
	std::string getScriptEventName() override;

	static RuneSpellFunction Illusion;
	static RuneSpellFunction Convince;

	bool internalCastSpell(Creature* creature, const LuaVariant& var);

	bool hasCharges;
	uint32_t runeId;

	RuneSpellFunction* function;
};








#endif

