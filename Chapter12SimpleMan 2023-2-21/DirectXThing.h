#pragma once
//Superclass for the 3 directX items, so I can update or render them in a loop
// like in Madhav's code with his items
// Will Briggs
// Spring 2022

class DirectXThing
{
public:
	virtual bool Update(float)=0;
};

