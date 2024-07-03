#pragma once
class Connection
{
public:
	virtual void RunIOContext() = 0;
	virtual void StopIOContext() = 0;
};

