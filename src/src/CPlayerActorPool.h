#ifndef YSF_ACTORPOOL_H
#define YSF_ACTORPOOL_H
#include "CVector.h"

class CPActor {
public:
	CPActor(unsigned short id, int skin, CVector pos, float rot) : m_usActorID(id), m_iSkinID(skin), m_vecPos(pos), m_fRotation(rot), m_bStreamed(false), m_iInterior(0), m_iVirtualWorld(-1), m_fHealth(100), m_bInv(1){}
	//CPActor(unsigned short id, int skin, CVector pos, float rot);
	void AddToWorld(unsigned short playerid);
	void RemoveFromWorld(unsigned short playerid);
	
	inline int GetVirtualWorld() { return m_iVirtualWorld; }
	inline bool IsStreamed() { return m_bStreamed; }
	inline CVector& GetPosition() { return m_vecPos; }
private:
	unsigned short m_usActorID;
	int m_iSkinID;
	int m_iVirtualWorld;
	int m_iInterior;
	CVector m_vecPos;
	float m_fRotation;
	float m_fHealth;
	BYTE  m_bInv;
	bool m_bStreamed;
};

#ifdef _WIN32 // xD.. C++11 doesn't like BitStream.h/.cpp on linux
#include <unordered_map>
typedef std::unordered_map<int, CPActor*> ActorMap;
#else
#include <map>
typedef std::map<int, CPActor*> ActorMap;
#endif

class CPlayerActorPool {
public:
	CPlayerActorPool() = default;
	~CPlayerActorPool() = default;

	unsigned short AddForPlayer(unsigned short playerid, int skin, CVector pos, float rot);
	void RemoveForPlayer(unsigned short playerid, WORD id);
	void Process();
private:
	DWORD m_dwStreamTick;
};

#endif