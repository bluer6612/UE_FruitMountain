#pragma once

#include "Modules/ModuleManager.h"

class FUEFruitMountain : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};