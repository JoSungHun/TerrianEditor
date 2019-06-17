#pragma once
#include "Framework.h"

class IStage
{
public:
	IStage()
	{
		id = GUIDGenerator::Generate();
	}
	virtual ~IStage() = default;

	const uint& GetID() const { return id; }
private:
	uint id;
};