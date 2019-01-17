#pragma once
#include <stdio.h>

#include "AtomicExpression.h"


AtomicExpression::~AtomicExpression()
{
	printf("I got deleted");
}