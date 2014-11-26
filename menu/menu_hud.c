/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "menu_main.h"

void HUD_Draw(void)
{
	if(cvShowMenu.value <= 0)
		return;

	Menu_GetScreenSize();

	// Draw the crosshair...
	if(cvShowCrosshair.bValue)
	{
		Engine.Client_SetMenuCanvas(CANVAS_CROSSHAIR);
		Engine.DrawPic(va(MENU_HUD_PATH"crosshair%i",cvShowCrosshair.iValue-1),1.0f,-16,-16,32,32);
	}

	// Draw the rest...
	Engine.Client_SetMenuCanvas(CANVAS_DEFAULT);

#ifdef GAME_ADAMAS
	Engine.DrawFill(0,iMenuHeight-64,iMenuWidth,64,0,0,0,0.6f);

	if(cvShowHealth.bValue)
	{
		int	iClientArmor	= Engine.Client_GetStat(STAT_ARMOR),
			iClientHealth	= Engine.Client_GetStat(STAT_HEALTH);

		if(iClientHealth < 0)
			iClientHealth = 0;

		Engine.DrawPic(MENU_HUD_PATH"health",1.0f,30,iMenuHeight-64,64,64);

		if(iClientHealth >= 100)
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",iClientHealth/100),1.0f,80,iMenuHeight-64,64,64);

		if(iClientHealth >= 10)
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",(iClientHealth%100)/10),1.0f,112,iMenuHeight-64,64,64);

		Engine.DrawPic(va(MENU_HUD_PATH"num%i",iClientHealth%10),1.0f,144,iMenuHeight-64,64,64);
	}

	if(cvShowAmmo.bValue)
	{
		int iClientAmmo = Engine.Client_GetStat(STAT_AMMO);

		Engine.DrawPic(MENU_HUD_PATH"ammo",1.0f,200,iMenuHeight-64,64,64);

		if(iClientAmmo >= 100)
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",iClientAmmo/100),1.0f,250,iMenuHeight-64,64,64);

		if(iClientAmmo >= 10)
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",(iClientAmmo%100)/10),1.0f,282,iMenuHeight-64,64,64);

		Engine.DrawPic(va(MENU_HUD_PATH"num%i",iClientAmmo%10),1.0f,314,iMenuHeight-64,64,64);
	}

	// Display the players current score.
	Engine.DrawString(iMenuWidth-128,iMenuHeight-16,va("SCORE: %i",Engine.Client_GetStat(STAT_FRAGS)));
#if 0
	Engine.DrawString(iMenuWidth-128,iMenuHeight-32,va("LIVES: %i",Engine.Client_GetStat(STAT_LIVES)));
#endif
#elif GAME_ICTUS
	Engine.DrawPic(MENU_BASE_PATH"int040a",1.0f,iMenuWidth-252,iMenuHeight-92,252,92);
#elif GAME_OPENKATANA
	{
		int iHSize[2],iNSize[2],
			iHealth[3],iArmor[3],iAmmo[3];
		int armor	= Engine.Client_GetStat(STAT_ARMOR),
			health	= Engine.Client_GetStat(STAT_HEALTH),
			ammo	= Engine.Client_GetStat(STAT_AMMO);

		if(cvShowHealth.value == 1)
		{
			iHSize[WIDTH]	= 320;
			iHSize[HEIGHT]	= 156;

			iNSize[WIDTH] = 24;
			iNSize[HEIGHT] = 16;

			Engine.DrawPic(MENU_HUD_PATH"health",1.0f,0,iMenuHeight-iHSize[HEIGHT]+20,iHSize[WIDTH],iHSize[HEIGHT]);

			if(health > 0)
			{
				if (health >= 100)
				{
					iHealth[0] = health/100;
					Engine.DrawPic(va(MENU_HUD_PATH"num%i",iHealth[0]),1.0f,131,iMenuHeight-54+20,iNSize[WIDTH],iNSize[HEIGHT]);
				}

				if(health >= 10)
				{
					iHealth[1] = (health % 100)/10;
					Engine.DrawPic(va(MENU_HUD_PATH"num%i",iHealth[1]),1.0f,161,iMenuHeight-54+20,iNSize[WIDTH],iNSize[HEIGHT]);
				}

				iHealth[2] = health % 10;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iHealth[2]),1.0f,190,iMenuHeight-54+20,iNSize[WIDTH],iNSize[HEIGHT]);
			}
			else
				Engine.DrawPic(va(MENU_HUD_PATH"num0"),1.0f,190,iMenuHeight-54+20,iNSize[WIDTH],iNSize[HEIGHT]);

			if(armor > 0)
			{
				if (armor >= 100)
				{
					iArmor[0] = armor/100;
					Engine.DrawPic(va(MENU_HUD_PATH"num%i",iArmor[0]),1.0f,126,iMenuHeight-107+20,iNSize[WIDTH],iNSize[HEIGHT]);
				}

				if(armor >= 10)
				{
					iArmor[1] = (armor % 100)/10;
					Engine.DrawPic(va(MENU_HUD_PATH"num%i",iArmor[1]),1.0f,153,iMenuHeight-107+20,iNSize[WIDTH],iNSize[HEIGHT]);
				}

				iArmor[2] = armor % 10;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iArmor[2]),1.0f,181,iMenuHeight-107+20,iNSize[WIDTH],iNSize[HEIGHT]);
			}
			else
				Engine.DrawPic(va(MENU_HUD_PATH"num0"),1.0f,181,iMenuHeight-107+20,iNSize[WIDTH],iNSize[HEIGHT]);
		}
		else if(cvShowHealth.value == 2)
		{
			iNSize[WIDTH] = 24;
			iNSize[HEIGHT] = 16;

			Engine.DrawPic(va(MENU_HUD_PATH"cross"),1.0f,22,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);
			Engine.DrawPic(va(MENU_HUD_PATH"armor"),1.0f,22,iMenuHeight-70,iNSize[WIDTH],iNSize[HEIGHT]);

			if (health >= 100)
			{
				iHealth[0] = health/100;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iHealth[0]),1.0f,46,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);
			}

			if(health >= 10)
			{
				iHealth[1] = (health % 100)/10;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iHealth[1]),1.0f,70,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);
			}

			iHealth[2] = health % 10;
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",iHealth[2]),1.0f,94,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);

			if (armor >= 100)
			{
				iArmor[0] = armor/100;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iArmor[0]),1.0f,46,iMenuHeight-70,iNSize[WIDTH],iNSize[HEIGHT]);
			}

			if(armor >= 10)
			{
				iArmor[1] = (armor%100)/10;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iArmor[1]),1.0f,70,iMenuHeight-70,iNSize[WIDTH],iNSize[HEIGHT]);
			}

			iArmor[2] = armor%10;
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",iArmor[2]),1.0f,94,iMenuHeight-70,iNSize[WIDTH],iNSize[HEIGHT]);
		}

		if(cvShowAmmo.value == 1)
		{

			iHSize[WIDTH]	= 240;
			iHSize[HEIGHT]	= 114;

			iNSize[WIDTH]	= 24;
			iNSize[HEIGHT]	= 16;

			Engine.DrawPic(MENU_HUD_PATH"ammo",1.0f,iMenuWidth-iHSize[WIDTH],iMenuHeight-iHSize[HEIGHT]+20,iHSize[WIDTH],iHSize[HEIGHT]);

			if(ammo > 0)
			{
				if (ammo >= 100)
				{
					iAmmo[0] = ammo/100;
					Engine.DrawPic(va(MENU_HUD_PATH"num%i",iAmmo[0]),1.0f,iMenuWidth-134,iMenuHeight-69+20,iNSize[WIDTH],iNSize[HEIGHT]);
				}

				if(ammo >= 10)
				{
					iAmmo[1] = (ammo % 100)/10;
					Engine.DrawPic(va(MENU_HUD_PATH"num%i",iAmmo[1]),1.0f,iMenuWidth-96,iMenuHeight-69+20,iNSize[WIDTH],iNSize[HEIGHT]);
				}

				iAmmo[2] = ammo % 10;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iAmmo[2]),1.0f,iMenuWidth-57,iMenuHeight-69+20,iNSize[WIDTH],iNSize[HEIGHT]);
			}
			else
				Engine.DrawPic(va(MENU_HUD_PATH"num0"),1.0f,iMenuWidth-57,iMenuHeight-69+20,iNSize[WIDTH],iNSize[HEIGHT]);

		}
		else if(cvShowAmmo.value == 2)
		{

			iNSize[WIDTH]	= 24;
			iNSize[HEIGHT]	= 16;

			if(ammo >= 100)
			{
				iAmmo[0] = ammo/100;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iAmmo[0]),1.0f,iMenuWidth-94,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);
			}

			if(ammo >= 10)
			{
				iAmmo[1] = (ammo % 100)/10;
				Engine.DrawPic(va(MENU_HUD_PATH"num%i",iAmmo[1]),1.0f,iMenuWidth-70,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);
			}

			iAmmo[2] = ammo % 10;
			Engine.DrawPic(va(MENU_HUD_PATH"num%i",iAmmo[2]),1.0f,iMenuWidth-46,iMenuHeight-40,iNSize[WIDTH],iNSize[HEIGHT]);
		}
	}
#endif
}