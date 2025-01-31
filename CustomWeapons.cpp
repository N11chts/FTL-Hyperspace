#include "CustomWeapons.h"
#include "CustomOptions.h"
#include "CustomDamage.h"
#include "CrewMember_Extend.h"
#include "Projectile_Extend.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iomanip>

CustomWeaponManager *CustomWeaponManager::instance = new CustomWeaponManager();
CustomWeaponDefinition *CustomWeaponManager::currentWeapon = nullptr;

HOOK_METHOD(BlueprintManager, ProcessWeaponBlueprint, (rapidxml::xml_node<char>* node) -> WeaponBlueprint)
{
    LOG_HOOK("HOOK_METHOD -> BlueprintManager::ProcessWeaponBlueprint -> Begin (CustomWeapons.cpp)\n")
    WeaponBlueprint ret = super(node);

    auto weaponDef = CustomWeaponDefinition();
    weaponDef.name = node->first_attribute("name")->value();

    weaponDef.customDamage = new CustomDamageDefinition();
    bool hasCustomDamage = false;

    for (auto child = node->first_node(); child; child = child->next_sibling())
    {
        std::string name = child->name();
        std::string val = child->value();

        if (name == "freeMissileChance")
        {
            weaponDef.freeMissileChance = boost::lexical_cast<int>(val);
        }
        if (name == "descriptionOverride")
        {
            weaponDef.descriptionOverride.data = val;
            weaponDef.descriptionOverride.isLiteral = true;
            if (child->first_attribute("id"))
            {
                weaponDef.descriptionOverride.data = child->first_attribute("id")->value();
                weaponDef.descriptionOverride.isLiteral = false;
            }
        }
        if (name == "advancedDescriptionOverride")
        {
            weaponDef.advancedDescriptionOverride.data = val;
            weaponDef.advancedDescriptionOverride.isLiteral = true;
            if (child->first_attribute("id"))
            {
                weaponDef.advancedDescriptionOverride.data = child->first_attribute("id")->value();
                weaponDef.advancedDescriptionOverride.isLiteral = false;
            }
        }
        if (name == "tooltipOverride")
        {
            weaponDef.tooltipOverride.data = val;
            weaponDef.tooltipOverride.isLiteral = true;
            if (child->first_attribute("id"))
            {
                weaponDef.tooltipOverride.data = child->first_attribute("id")->value();
                weaponDef.tooltipOverride.isLiteral = false;
            }
        }
        if (name == "advancedTooltipOverride")
        {
            weaponDef.advancedTooltipOverride.data = val;
            weaponDef.advancedTooltipOverride.isLiteral = true;
            if (child->first_attribute("id"))
            {
                weaponDef.advancedTooltipOverride.data = child->first_attribute("id")->value();
                weaponDef.advancedTooltipOverride.isLiteral = false;
            }
        }
        if (name == "hideEventTooltip")
        {
            weaponDef.hideEventTooltip = EventsParser::ParseBoolean(val);
        }
        if (name == "accuracyMod")
        {
            hasCustomDamage = true;
            weaponDef.customDamage->accuracyMod = boost::lexical_cast<int>(val);
        }
        if (name == "droneAccuracyMod")
        {
            hasCustomDamage = true;
            weaponDef.customDamage->droneAccuracyMod = boost::lexical_cast<int>(val);
        }
        if (name == "noSysDamage")
        {
            hasCustomDamage = true;
            weaponDef.customDamage->noSysDamage = EventsParser::ParseBoolean(val);
        }
        if (name == "noPersDamage")
        {
            hasCustomDamage = true;
            weaponDef.customDamage->noPersDamage = EventsParser::ParseBoolean(val);
        }
        if (name == "ionBeamFix" && ret.type == 2) // Ion beam fix only valid for beams
        {
            hasCustomDamage = true;
            weaponDef.customDamage->ionBeamFix = EventsParser::ParseBoolean(val);
        }
        if (name == "simultaneousFire")
        {
            weaponDef.simultaneousFire = EventsParser::ParseBoolean(val);
        }
        if (name == "fireTime")
        {
            weaponDef.fireTime = boost::lexical_cast<float>(val);
        }
        if (name == "angularRadius") // affects flak drones
        {
            weaponDef.angularRadius = boost::lexical_cast<float>(val);
        }
        if (name == "statBoostChance")
        {
            hasCustomDamage = true;
            weaponDef.customDamage->statBoostChance = boost::lexical_cast<int>(val);
        }
        if (name == "statBoosts")
        {
            hasCustomDamage = true;
            for (auto statBoostNode = child->first_node(); statBoostNode; statBoostNode = statBoostNode->next_sibling())
            {
                if (strcmp(statBoostNode->name(), "statBoost") == 0)
                {
                    weaponDef.customDamage->statBoosts.push_back(StatBoostManager::GetInstance()->ParseStatBoostNode(statBoostNode, StatBoostDefinition::BoostSource::AUGMENT));
                }
            }
        }
        if (name == "crewSpawnChance")
        {
            hasCustomDamage = true;
            weaponDef.customDamage->crewSpawnChance = boost::lexical_cast<int>(val);
        }
        if (name == "spawnCrew")
        {
            hasCustomDamage = true;
            CrewSpawn *newSpawn = CrewSpawn::ParseCrewSpawn(child, false);
            if (!newSpawn->race.empty())
            {
                weaponDef.customDamage->crewSpawns.push_back(newSpawn);
            }
        }

        if (name == "iconScale")
        {
            weaponDef.iconScale = boost::lexical_cast<float>(val);
        }

        if (name == "projectiles")
        {
            for (auto projectileNode = child->first_node(); projectileNode; projectileNode = projectileNode->next_sibling())
            {
                if (strcmp(projectileNode->name(), "projectile") == 0)
                {
                    if (projectileNode->first_attribute("blueprint"))
                    {
                        weaponDef.miniProjectileOverride[projectileNode->value()] = projectileNode->first_attribute("blueprint")->value();
                    }
                }
            }
        }
    }

    // Default chance if tag not specified (100% if tags specified, 0% otherwise)
    if (weaponDef.customDamage->statBoostChance == -1)
    {
        weaponDef.customDamage->statBoostChance = weaponDef.customDamage->statBoosts.empty() ? 0 : 10;
    }
    if (weaponDef.customDamage->crewSpawnChance == -1)
    {
        weaponDef.customDamage->crewSpawnChance = weaponDef.customDamage->crewSpawns.empty() ? 0 : 10;
    }

    if (hasCustomDamage)
    {
        weaponDef.customDamage->GiveId();
    }
    else
    {
        delete weaponDef.customDamage;
        weaponDef.customDamage = &CustomDamageDefinition::defaultDef;
    }

    CustomWeaponManager::instance->AddWeaponDefinition(weaponDef);

    return ret;
}

HOOK_METHOD(ShipManager, OnRender, (bool showInterior, bool doorControlMode) -> void)
{
    LOG_HOOK("HOOK_METHOD -> ShipManager::OnRender -> Begin (CustomWeapons.cpp)\n")
    if (bJumping)
    {
        auto time = jumpAnimation.current_time;
        if (time >= 1.0)
        {
            CSurface::GL_SetColorTint(GL_Color(255.f, 255.f, 255.f, 1.f - ((time - 1.f) * 2.f)));
            RenderWeapons();
            CSurface::GL_RemoveColorTint();
        }
        else
        {
            RenderWeapons();
        }
    }
    super(showInterior, doorControlMode);

}

static bool artillery = false;

HOOK_METHOD(ProjectileFactory, NumTargetsRequired, () -> int)
{
    LOG_HOOK("HOOK_METHOD -> ProjectileFactory::NumTargetsRequired -> Begin (CustomWeapons.cpp)\n")
    int ret = super();

    if (artillery)
    {
        int rooms = ShipGraph::GetShipInfo(!iShipId)->rooms.size();
        if (ret > rooms)
        {
            return rooms;
        }
    }

    return ret;
}

HOOK_METHOD(ArtillerySystem, OnLoop, () -> void)
{
    LOG_HOOK("HOOK_METHOD -> ArtillerySystem::OnLoop -> Begin (CustomWeapons.cpp)\n")
    artillery = true;
    super();
    artillery = false;
}

HOOK_METHOD(ProjectileFactory, SpendMissiles, () -> int)
{
    LOG_HOOK("HOOK_METHOD -> ProjectileFactory::SpendMissiles -> Begin (CustomWeapons.cpp)\n")
    if (iSpendMissile > 0)
    {
        int randomNum = random32() % 100;

        if (randomNum < CustomWeaponManager::instance->GetWeaponDefinition(blueprint->name)->freeMissileChance)
        {
            iSpendMissile = 0;
            if (iShipId == 0) G_->GetEventSystem()->AddEvent(12);
        }
    }

    return super();
}

// Weapon Types:
// 0: LASER
// 1: MISSILES
// 2: BEAM
// 3: BOMB
// 4: BURST

static Button* smallAutoFireButton;

HOOK_METHOD(WeaponControl, constructor, () -> void)
{
    LOG_HOOK("HOOK_METHOD -> WeaponControl::constructor -> Begin (CustomWeapons.cpp)\n")
    super();

    smallAutoFireButton = new Button();
    smallAutoFireButton->OnInit("button_small_autofireOn", Point(0, 0));
    smallAutoFireButton->hitbox.w = 28;
    smallAutoFireButton->hitbox.h = 28;
}

HOOK_METHOD(WeaponControl, LinkShip, (ShipManager *ship) -> void)
{
    LOG_HOOK("HOOK_METHOD -> WeaponControl::LinkShip -> Begin (CustomWeapons.cpp)\n")
    super(ship);

    if(autoFiring){
        smallAutoFireButton->SetImageBase("button_small_autofireOn");
        smallAutoFireButton->bRenderOff = true;
    }
    else {
        smallAutoFireButton->SetImageBase("button_small_autofireOff");
        smallAutoFireButton->bRenderOff = true;
    }
}

HOOK_METHOD(WeaponControl, OnRender, (bool unk) -> void)
{
    LOG_HOOK("HOOK_METHOD -> WeaponControl::OnRender -> Begin (CustomWeapons.cpp)\n")
    super(unk);

    if (this->shipManager->myBlueprint.weaponSlots <= 2 && this->shipManager->myBlueprint.weaponSlots > 0 && this->shipManager->HasSystem(3))
    {
        smallAutoFireButton->hitbox.x = this->location.x + 184;
        smallAutoFireButton->hitbox.y = this->location.y + 61;

        CSurface::GL_PushMatrix();
        CSurface::GL_Translate(this->location.x + 184, this->location.y + 61, 0.0);
        smallAutoFireButton->OnRender();
        CSurface::GL_PopMatrix();
    }
}

HOOK_METHOD(WeaponControl, MouseMove, (int x, int y) -> void)
{
    LOG_HOOK("HOOK_METHOD -> WeaponControl::MouseMove -> Begin (CustomWeapons.cpp)\n")
    super(x, y);

    if (this->shipManager->myBlueprint.weaponSlots <= 2 && this->shipManager->myBlueprint.weaponSlots > 0 && this->shipManager->HasSystem(3) && !Dragging())
    {
        smallAutoFireButton->MouseMove(x, y, false);
        if(smallAutoFireButton->bHover){
            smallAutoFireButton->bRenderOff = false;
            smallAutoFireButton->bRenderSelected = true;

            std::string replaceWith;
            std::string tooltip = G_->GetTextLibrary()->GetText("tooltip_autofire");

            replaceWith = Settings::GetHotkeyName("autofire");
            boost::algorithm::replace_all(tooltip, "\\1", replaceWith);

            replaceWith = Settings::GetHotkeyName("force_autofire");
            boost::algorithm::replace_all(tooltip, "\\2", replaceWith);

            G_->GetMouseControl()->bForceTooltip = true;
            G_->GetMouseControl()->SetTooltip(tooltip);
        }
        else {
            smallAutoFireButton->bRenderOff = true;
            smallAutoFireButton->bRenderSelected = false;

        }

    }
}

HOOK_METHOD(WeaponControl, LButton, (int x, int y, bool holdingShift) -> bool)
{
    LOG_HOOK("HOOK_METHOD -> WeaponControl::LButton -> Begin (CustomWeapons.cpp)\n")
    bool ret = super(x, y, holdingShift);

    if (this->shipManager->myBlueprint.weaponSlots <= 2 && this->shipManager->myBlueprint.weaponSlots > 0 && this->shipManager->HasSystem(3))
    {
        if (smallAutoFireButton->bActive && smallAutoFireButton->bHover)
        {
            SetAutofiring(!autoFiring, false);
            if (autoFiring) {
                smallAutoFireButton->SetImageBase("button_small_autofireOn");
                smallAutoFireButton->bRenderSelected = false;
                smallAutoFireButton->bRenderOff = true;
            }
            else {
                smallAutoFireButton->SetImageBase("button_small_autofireOff");
                smallAutoFireButton->bRenderSelected = false;
                smallAutoFireButton->bRenderOff = true;
            }
            return autoFiring;
        }
    }

    return ret;
}

HOOK_METHOD(WeaponControl, KeyDown, (SDLKey key) -> bool)
{
    LOG_HOOK("HOOK_METHOD -> WeaponControl::KeyDown -> Begin (CustomWeapons.cpp)\n")
    bool ret = super(key);

    if (this->shipManager->myBlueprint.weaponSlots <= 2 && this->shipManager->myBlueprint.weaponSlots > 0 && this->shipManager->HasSystem(3))
    {
        if (key == Settings::GetHotkey("autofire"))
        {
            if (smallAutoFireButton->bActive)
            {
                SetAutofiring(!autoFiring, false);
                if (autoFiring)
                {
                    smallAutoFireButton->SetImageBase("button_small_autofireOn");
                    smallAutoFireButton->bRenderSelected = false;
                    smallAutoFireButton->bRenderOff = true;
                }
                else
                {
                    smallAutoFireButton->SetImageBase("button_small_autofireOff");
                    smallAutoFireButton->bRenderSelected = false;
                    smallAutoFireButton->bRenderOff = true;
                }
                return autoFiring;
            }
        }
    }

    return ret;
}

static ProjectileFactory *simultaneousFireWeapon = nullptr;
HOOK_METHOD(ProjectileFactory, GetProjectile, () -> Projectile*)
{
    LOG_HOOK("HOOK_METHOD -> ProjectileFactory::GetProjectile -> Begin (CustomWeapons.cpp)\n")
    bool fireShot = weaponVisual.bFireShot;
    Projectile* ret = super();

    if (!queuedProjectiles.empty())
    {
        if (fireShot && CustomWeaponManager::instance->GetWeaponDefinition(blueprint->name)->simultaneousFire)
        {
            simultaneousFireWeapon = this;
        }

        if (blueprint->type == 4 && !blueprint->miniProjectiles.empty())
        {
            if (!fireShot && ret == nullptr)
            {
                if (queuedProjectiles.size() % blueprint->miniProjectiles.size() != 0)
                {
                    ret = queuedProjectiles.back();
                    queuedProjectiles.pop_back();
                    return ret;
                }
                else if (simultaneousFireWeapon == this)
                {
                    ret = queuedProjectiles.back();
                    queuedProjectiles.pop_back();
                }
            }

            if (ret != nullptr)
            {
                // Offset fix for charge flak
                int i = (queuedProjectiles.size() / blueprint->miniProjectiles.size()) * blueprint->miniProjectiles.size();
                if (i < queuedProjectiles.size())
                {
                    for (int j=i+1; j<queuedProjectiles.size(); ++j)
                    {
                        queuedProjectiles[j]->position = queuedProjectiles[i]->position;
                        queuedProjectiles[j]->last_position = queuedProjectiles[i]->last_position;
                    }
                    ret->position = queuedProjectiles[i]->position;
                    ret->last_position = queuedProjectiles[i]->last_position;
                }
            }
        }
        else
        {
            if (simultaneousFireWeapon == this && !fireShot && ret == nullptr)
            {
                ret = queuedProjectiles.back();
                queuedProjectiles.pop_back();
            }
        }
    }
    else
    {
        if (simultaneousFireWeapon == this) simultaneousFireWeapon = nullptr;
    }

    return ret;
}

HOOK_METHOD(WeaponAnimation, StartFire, () -> bool)
{
    LOG_HOOK("HOOK_METHOD -> WeaponAnimation::StartFire -> Begin (CustomWeapons.cpp)\n")
    bool ret = super();

    if (ret && iChargeLevels > 1 && CustomWeaponManager::currentWeapon && CustomWeaponManager::currentWeapon->simultaneousFire)
    {
        int chargeLength = (anim.info.numFrames - iChargedFrame) / iChargeLevels;
        anim.SetCurrentFrame(iChargedFrame + chargeLength * boostLevel);
    }
    return ret;
}

HOOK_METHOD(ProjectileFactory, constructor, (const WeaponBlueprint* bp, int shipId) -> void)
{
    LOG_HOOK("HOOK_METHOD -> ProjectileFactory::constructor -> Begin (CustomWeapons.cpp)\n")
    super(bp, shipId);

    if (bp->type != 2)
    {
        auto def = CustomWeaponManager::instance->GetWeaponDefinition(blueprint->name);
        if (def->fireTime)
        {
            weaponVisual.SetFireTime(def->fireTime);
        }
    }
}

// Fix for weapon animations with many frames.
HOOK_METHOD(WeaponAnimation, Update, () -> void)
{
    LOG_HOOK("HOOK_METHOD -> WeaponAnimation::Update -> Begin (CustomWeapons.cpp)\n")
    if (bFiring)
    {
        auto cFPS = G_->GetCFPS();
        float speedFactor = cFPS->SpeedFactor;
        float speed = speedFactor * 0.0625f * anim.info.numFrames/anim.tracker.time; // frame length * anim FPS
        if (speed > 1.f)
        {
            speed = std::ceil(speed);
            int iSpeed = speed;
            cFPS->SpeedFactor = speedFactor / speed;
            for (int i=1; i<iSpeed; ++i)
            {
                super();
            }
            cFPS->SpeedFactor = speedFactor;
        }
        else
        {
            super();
        }
    }
    else
    {
        super();
    }
}

// Icon Scale
HOOK_METHOD(WeaponBlueprint, RenderIcon, (float scale) -> void)
{
    LOG_HOOK("HOOK_METHOD -> WeaponBlueprint::RenderIcon -> Begin (CustomWeapons.cpp)\n")
    scale *= CustomWeaponManager::instance->GetWeaponDefinition(name)->iconScale;
    if (scale != 1.f)
    {
        CSurface::GL_PushMatrix();
        CSurface::GL_Scale(scale,scale,0.f);
        super(scale);
        CSurface::GL_PopMatrix();
    }
    else
    {
        super(scale);
    }
}

// Flak stuff
HOOK_METHOD_PRIORITY(ProjectileFactory, GetProjectile, -1000, () -> Projectile*)
{
    LOG_HOOK("HOOK_METHOD_PRIORITY -> ProjectileFactory::GetProjectile -> Begin (CustomWeapons.cpp)\n")

    Projectile* ret = super();

    if (ret && blueprint->type == 4 && !blueprint->miniProjectiles.empty())
    {
        CustomWeaponManager::ProcessMiniProjectile(ret, blueprint, boostLevel);
    }

    return ret;
}

void CustomWeaponManager::ProcessMiniProjectile(Projectile *proj, const WeaponBlueprint *bp, int boostLevel)
{
    if (proj->death_animation.fScale == 0.25f) // fake projectile
    {
        PR_EX(proj)->customDamage.Clear();
    }
    else
    {
        auto &miniProjectileOverride = instance->GetWeaponDefinition(bp->name)->miniProjectileOverride;
        if (!miniProjectileOverride.empty())
        {
            auto it = miniProjectileOverride.find(proj->flight_animation.animName);
            if (it != miniProjectileOverride.end())
            {
                WeaponBlueprint *newBp = G_->GetBlueprints()->GetWeaponBlueprint(it->second);
                proj->Initialize(*newBp);
                proj->flight_animation = G_->GetAnimationControl()->GetAnimation(it->first);

                // apply new blueprint chain effect
                if (newBp->boostPower.type == 2 && boostLevel > 0)
                {
                    int boostAmount = (boostLevel-1)*newBp->boostPower.amount;
                    if (proj->damage.iDamage > 0)
                    {
                        proj->damage.iDamage += boostAmount;
                    }
                    else if (proj->damage.iIonDamage > 0)
                    {
                        proj->damage.iIonDamage += boostAmount;
                    }
                    else if (proj->damage.iPersDamage > 0)
                    {
                        proj->damage.iPersDamage += boostAmount;
                    }
                    else
                    {
                        proj->damage.iSystemDamage += boostAmount;
                    }
                }
            }
        }
    }
}
