#pragma once
#include "CrewSpawn.h"
#include "FTLGame.h"
#include "StatBoost.h"
#include "Constants.h"
#include "CustomCrewCommon.h"
#include "EnumClassHash.h"
#include <array>

struct CrewDefinition;
struct ActivatedPowerDefinition;
struct ActivatedPowerRequirements;

enum class CrewStat : unsigned int;
struct StatBoost;
struct StatBoostDefinition;

enum PowerReadyState : unsigned int
{
    POWER_NOT_READY_COOLDOWN,
    POWER_READY,
    POWER_NOT_READY_ACTIVATED,
    POWER_NOT_READY_ENEMY_SHIP,
    POWER_NOT_READY_PLAYER_SHIP,
    POWER_NOT_READY_ENEMY_IN_ROOM,
    POWER_NOT_READY_FRIENDLY_IN_ROOM,
    POWER_NOT_READY_WHITELIST,
    POWER_NOT_READY_ENEMY_WHITELIST,
    POWER_NOT_READY_FRIENDLY_WHITELIST,
    POWER_NOT_READY_ENEMY_BLACKLIST,
    POWER_NOT_READY_FRIENDLY_BLACKLIST,
    POWER_NOT_READY_SYSTEM_IN_ROOM,
    POWER_NOT_READY_HAS_CLONEBAY,
    POWER_NOT_READY_AI_DISABLED,
    POWER_NOT_READY_OUT_OF_COMBAT,
    POWER_NOT_READY_IN_COMBAT,
    POWER_NOT_READY_SYSTEM,
    POWER_NOT_READY_SYSTEM_FUNCTIONAL,
    POWER_NOT_READY_MIN_HEALTH,
    POWER_NOT_READY_MAX_HEALTH,
    POWER_NOT_READY_SYSTEM_DAMAGED,
    POWER_NOT_READY_CHARGES,
    POWER_NOT_READY_EXTRACONDITION_OR,
    POWER_NOT_READY_EXTRACONDITION_TRUE = 16384,
    POWER_NOT_READY_EXTRACONDITION_FALSE = 32768
};

extern const std::array<std::string, numStats> powerReadyStateExtraTextTrue;

extern const std::array<std::string, numStats> powerReadyStateExtraTextFalse;

struct CrewAnimation_Extend
{
public:
    CrewAnimation *orig;
    Animation* effectAnim = nullptr;
    Animation* tempEffectAnim = nullptr;
    Animation* effectFinishAnim = nullptr;
    std::unordered_map<uint64_t, Animation*> boostAnim = std::unordered_map<uint64_t, Animation*>();
    GL_Texture* tempEffectStrip = nullptr;
    bool tempEffectBaseVisible = true;

    std::string crewAnimationType = "human";
    bool isMantisAnimation = false;
    bool isIonDrone = false;
    bool canPunch = true;

    Pointf effectPos;
    Pointf effectWorldPos;
    bool powerDone = true;

    bool temporaryPowerActive;
    bool isAbilityDrone = false;

    void OnInit(const std::string& name, Pointf position, bool enemy);
    void PreparePower(ActivatedPowerDefinition* def);

    ~CrewAnimation_Extend();
};

struct CrewMember_Extend
{
public:
    CrewMember *orig;
    int selfId = -1;
    bool canPhaseThroughDoors = false;
    bool isHealing = false;
    TimerHelper* passiveHealTimer = nullptr;
    TimerHelper* deathTimer = nullptr;
    int lastRoom = -1;
    int lastShipId = -1;
    bool exploded = false;
    bool triggerExplosion = false;

    bool hasSpecialPower = false;
    bool hasTemporaryPower = false;

    std::pair<float, float> powerCooldown = std::pair<float, float>();
    std::pair<float, float> temporaryPowerDuration = std::pair<float, float>();
    std::pair<int, int> powerCharges = std::pair<int, int>();

    int powerRoom;
    int powerShip;

    bool powerActivated = false;
    bool temporaryPowerActive = false;

    void ActivatePower();
    void PreparePower();
    void ActivateTemporaryPower();
    void TemporaryPowerFinished();
    Damage* GetPowerDamage();
    PowerReadyState PowerReady();
    PowerReadyState PowerReq(const ActivatedPowerRequirements *req);

    unsigned int powerChange;
    unsigned int powerDefIdx = 0;
    ActivatedPowerDefinition* GetPowerDef() const;
    ActivatedPowerDefinition* CalculatePowerDef();

    ExplosionDefinition deathEffectChange;
    bool hasDeathExplosion;

    bool isIonDrone = false;
    bool isAbilityDrone = false;

    bool noSlot = false;
    bool noClone = false;

    float prevStun = 0.f; // for use in stun resistance checking

    std::vector<StatBoost> outgoingStatBoosts = std::vector<StatBoost>();
    std::vector<StatBoost> outgoingAbilityStatBoosts = std::vector<StatBoost>();
    std::vector<StatBoost> tempOutgoingStatBoosts = std::vector<StatBoost>();
//    std::vector<StatBoost> outgoingTimedStatBoosts = std::vector<StatBoost>();
//    std::vector<StatBoost> outgoingTimedAbilityStatBoosts = std::vector<StatBoost>();
    std::unordered_map<CrewStat, std::vector<StatBoost>, EnumClassHash> timedStatBoosts = std::unordered_map<CrewStat, std::vector<StatBoost>, EnumClassHash>();
//    std::vector<StatBoost> personalStatBoosts;

    float extraMedbay = 0.f;

    std::string originalColorRace = ""; // for color layers
    std::string originalRace = "";
    std::string transformRace = "";

    void Initialize(CrewBlueprint& bp, int shipId, bool enemy, CrewAnimation *animation, bool isTransform = false);
    bool TransformRace(const std::string& newRace);
    static void TransformColors(CrewBlueprint& bp, CrewBlueprint *newBlueprint);

    CrewMember_Extend()
    {
        selfId = Globals::GetNextSpaceId();
    }

    ~CrewMember_Extend()
    {
        delete passiveHealTimer;
        delete deathTimer;
    }

    std::pair<float,int> statCache[numCachedStats] = {};

    bool BoostCheck(StatBoost& statBoost);
    bool CheckExtraCondition(CrewExtraCondition condition);
    int CalculateMaxHealth(const CrewDefinition* def);
    float CalculateStat(CrewStat stat, const CrewDefinition* def, bool* boolValue=nullptr);

    std::string GetRace();
    CrewDefinition *GetDefinition();
};

CrewMember_Extend* Get_CrewMember_Extend(const CrewMember* c);
CrewAnimation_Extend* Get_CrewAnimation_Extend(CrewAnimation *c);

#define CM_EX Get_CrewMember_Extend
#define CMA_EX Get_CrewAnimation_Extend
