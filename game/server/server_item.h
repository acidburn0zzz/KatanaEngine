#ifndef __SERVERITEM__
#define __SERVERITEM__

#include "shared_game.h"

typedef struct
{
	int		iNumber;						// Our items unique ID

	char	*cName,							// The name of the item
			*cModel,						// Model path
			*cSound;						// Sound that's emitted upon picking the item up

	void	(*Pickup)(edict_t *eEntity);	// Function that's called when the item is touched
} Item_t;

Item_t *Item_GetItem(int iItemID);
Item_t *Item_GetInventory(int iItemID,edict_t *eEntity);

void Item_Spawn(edict_t *eItem);
void Item_AddInventory(Item_t *iItem,edict_t *eEntity);
void Item_RemoveInventory(Item_t *iItem,edict_t *eEntity);

#endif
