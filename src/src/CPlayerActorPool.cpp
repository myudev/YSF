#include "main.h"
#define MAX_ACTOR_DISTANCE 300.0f // TODO: FINISH ME ALSO PLOX

void CPActor::AddToWorld(unsigned short playerid)
{
	if (m_bStreamed) return;

	RakNet::BitStream bsActor;
	bsActor.Write(m_usActorID);
	bsActor.Write(m_iSkinID);
	bsActor.Write(m_vecPos.fX);
	bsActor.Write(m_vecPos.fY);
	bsActor.Write(m_vecPos.fZ);
	bsActor.Write(m_fRotation);
	bsActor.Write(m_fHealth);
	bsActor.Write(m_bInv);
	pRakServer->RPC(&RPC_WorldActorAdd, &bsActor, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, pRakServer->GetPlayerIDFromIndex(playerid), false, false);
	m_bStreamed = true;
}

void CPActor::RemoveFromWorld(unsigned short playerid)
{
	if (!m_bStreamed) return;

	RakNet::BitStream bsActor;
	bsActor.Write(m_usActorID);
	pRakServer->RPC(&RPC_WorldActorRemove, &bsActor, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, pRakServer->GetPlayerIDFromIndex(playerid), false, false);

	m_bStreamed = false;
}

unsigned short CPlayerActorPool::AddForPlayer(unsigned short playerid, int skin, CVector pos, float rot)
{
	// Skip unconnected players
	if (!IsPlayerConnectedEx(playerid)) return 0xFFFF;

	// Find free slot for (our) pool
	WORD slot = 0;
	while (slot != MAX_ACTORS)
	{
		if (!pPlayerData[playerid]->PlayerActors[slot]) break;
		slot++;
	}
	if (slot == MAX_ACTORS) return 0xFFFF;

	// Find actual free actorid... NOTE: We still can't be sure if the server creates another actor
	// at this id, it'll only work if the server isn't adding actors while adding player actors - myu.
	WORD internalslot = slot;
	while (slot != MAX_ACTORS)
	{
		if (!pNetGame->pActorPool->bValidActor[internalslot]) break;
		internalslot++;
	}
	if (internalslot == MAX_ACTORS) return 0xFFFF;

	// now create
	pPlayerData[playerid]->PlayerActors[slot] = new CPActor(internalslot, skin, pos, rot);

	return slot;
}

void CPlayerActorPool::RemoveForPlayer(unsigned short playerid, WORD id)
{
	if (id >= 0 && id <= MAX_ACTORS)
		SAFE_DELETE(pPlayerData[playerid]->PlayerActors[id]);
}

void CPlayerActorPool::Process()
{
	float distance;
	for (WORD playerid = 0; playerid != MAX_PLAYERS; playerid++)
	{
		if (!IsPlayerConnectedEx(playerid)) continue;
		CVector *vecPos = &pNetGame->pPlayerPool->pPlayer[playerid]->vecPosition;

		for (ActorMap::iterator p = pPlayerData[playerid]->PlayerActors.begin(); p != pPlayerData[playerid]->PlayerActors.end(); p++)
		{
			distance = GetDistance3D(vecPos, &p->second->GetPosition());

			if (distance < MAX_ACTOR_DISTANCE && !p->second->IsStreamed())
			{
				if (pNetGame->pPlayerPool->dwVirtualWorld[playerid] == static_cast<DWORD>(p->second->GetVirtualWorld()) || p->second->GetVirtualWorld() == -1)
				{
					//logprintf("streamin: %f - %d, world: %d (actorid: %d)", distance, playerid, p->second->GetVirtualWorld(), p->first);
					p->second->AddToWorld(playerid);
				}
			}
			else if (
				(distance >= MAX_ACTOR_DISTANCE || (pNetGame->pPlayerPool->dwVirtualWorld[playerid] != static_cast<DWORD>(p->second->GetVirtualWorld()) && p->second->GetVirtualWorld() != -1))
				&& p->second->IsStreamed())
			{
				//logprintf("streamout: %f - %d (actorid: %d)", distance, playerid, p->first);
				p->second->RemoveFromWorld(playerid);
			}
		}
	}
}
