#include "PrecompiledHeader.h"

#include "Counters.h"
#include "MTGCardInstance.h"

Counter::Counter(MTGCardInstance * _target, int _power, int _toughness)
{
    init(_target, "", _power, _toughness);
}

Counter::Counter(MTGCardInstance * _target, const char * _name, int _power, int _toughness)
{
    init(_target, _name, _power, _toughness);
}

int Counter::init(MTGCardInstance * _target, const char * _name, int _power, int _toughness)
{
    target = _target;
    name = _name;
    power = _power;
    toughness = _toughness;
    nb = 1;
    return 1;
}

bool Counter::sameAs(const char * _name, int _power, int _toughness)
{
    if (power == 0 && toughness == 0)
        return (name.compare(_name) == 0);
    return (power == _power && toughness == _toughness);
}

bool Counter::cancels(int _power, int _toughness)
{
    if (power == 0 && toughness == 0)
        return false;
    return (power == -_power && toughness == -_toughness);
}

int Counter::added()
{
    if (power != 0 || toughness != 0)
    {
        target->power += power;
        target->addToToughness(toughness);
    }
    return 1;
}

int Counter::removed()
{
    if (power != 0 || toughness != 0)
    {
        target->power -= power;
        target->addToToughness(-toughness);
    }
    return 1;
}

Counters::Counters(MTGCardInstance * _target) :
    target(_target)
{
    mCount = 0;
}
Counters::~Counters()
{
    for (int i = 0; i < mCount; i++)
    {
        SAFE_DELETE(counters[i]);
    }
}

int Counters::addCounter(const char * _name, int _power, int _toughness)
{
 /*   for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->cancels(_power, _toughness) && counters[i]->nb > 0)
        {
            counters[i]->removed();
            counters[i]->nb--;
            return mCount;
        }
    }*/
	//leaving this commented out incase it was placed here to avoid some bug(maybe a misunderstanding of how counters work in MTG)
	//bug fix, creatures that have abilities based on +1/+1 counters should still be allowed to use their counter
	//putting a -1/-1 counter is not the same thing as removing a 1/1 counter.
	//consider this effect, put a 1/1 counter on creature, then put a - 1/-1 counter on creature. it never saids
	//remove the counter 1/1, it saids put a NEW counter -1/-1.
	//the card should have both a 1/1 and a -1/-1 counter at this point.
	//if an effect states that it should take off the counters, then it should be done with the following code
	//counter(1/1,-1) this is removel of 1/1 counters. judging by the primitives its fully understood as i see this method being used often.

    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->sameAs(_name, _power, _toughness))
        {
            counters[i]->added();
            counters[i]->nb++;
            return mCount;
        }
    }
    Counter * counter = NEW Counter(target, _name, _power, _toughness);
    counters[mCount] = counter;
    counter->added();
    mCount++;
    return mCount;
}

int Counters::addCounter(int _power, int _toughness)
{
    return addCounter("", _power, _toughness);
}

int Counters::init()
{
    for (int i = mCount - 1; i >= 0; i--)
    {
        while (counters[i]->nb >= 1)
        {
            counters[i]->removed();
            counters[i]->nb--;
        }
    }
    return 1;
}

int Counters::removeCounter(const char * _name, int _power, int _toughness)
{
    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->sameAs(_name, _power, _toughness))
        {
            if (counters[i]->nb < 1)
                return 0;
            counters[i]->removed();
            counters[i]->nb--;
            return mCount;
        }
    }
    return 0;
}

int Counters::removeCounter(int _power, int _toughness)
{
    return removeCounter("", _power, _toughness);
}

Counter * Counters::hasCounter(const char * _name, int _power, int _toughness)
{
    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->sameAs(_name, _power, _toughness))
        {
            if (counters[i]->nb > 0)
                return counters[i];
        }
    }
    return NULL;
}

Counter * Counters::hasCounter(int _power, int _toughness)
{
    return hasCounter("", _power, _toughness);
}

Counter * Counters::getNext(Counter * previous)
{
    int found = 0;
    for (int i = 0; i < mCount; i++)
    {
        if (found && counters[i]->nb > 0)
            return counters[i];
        if (counters[i] == previous)
            found = 1;
    }
    return NULL;
}