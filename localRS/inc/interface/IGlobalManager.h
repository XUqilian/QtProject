#pragma once

class IGlobalManager
{
public:
    virtual void InitManagers() = 0;
    virtual ~IGlobalManager() = default;
};

