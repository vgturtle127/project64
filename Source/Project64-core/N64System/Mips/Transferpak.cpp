// Project64 - A Nintendo 64 emulator
// http://www.pj64-emu.com/
// Copyright(C) 2001-2021 Project64
// Copyright(C) 2015 Bobby Smiles

// GNU/GPLv2 licensed: https://gnu.org/licenses/gpl-2.0.html
#include "stdafx.h"
#include "GBCart.h"
#include "Transferpak.h"

static transferpak tpak;

uint16_t gb_cart_address(unsigned int bank, uint16_t address)
{
    return (address & 0x3FFF) | ((bank & 0x3) * 0x4000);
}

void Transferpak::Init()
{
    //Quick check to ensure we dont have a ROM already
    if (tpak.gb_cart.rom == NULL)
    {
        memset(&tpak, 0, sizeof(tpak));
        tpak.access_mode = (!GBCart::init_gb_cart(&tpak.gb_cart, g_Settings->LoadStringVal(Game_Transferpak_ROM).c_str())) ? CART_NOT_INSERTED : CART_ACCESS_MODE_0;
        tpak.access_mode_changed = 0x44;
    }
}

void Transferpak::Release()
{
    if (tpak.gb_cart.rom != NULL)
    {
        GBCart::release_gb_cart(&tpak.gb_cart);
    }
}

void Transferpak::ReadFrom(uint16_t address, uint8_t * data)
{
	if ((address >= 0x8000) && (address <= 0x8FFF))
	{
        //Ensure we actually have a ROM loaded in first.
        if (tpak.gb_cart.rom == NULL)
        {
            Init();
        }

		//Get whether the GB cart is enabled or disabled
		uint8_t value = (tpak.enabled) ? 0x84 : 0x00;

		memset(data, value, 0x20);
	}
	else if ((address >= 0xB000) && (address <= 0xBFFF))
	{
		// Get the GB Cart access mode
		if (tpak.enabled)
		{
			memset(data, tpak.access_mode, 0x20);
			if (tpak.access_mode != CART_NOT_INSERTED)
			{
				data[0] |= tpak.access_mode_changed;
			}
   
            tpak.access_mode_changed = 0;
		}
	}
	else if (address >= 0xC000)
	{
		// Read the GB Cart
		if (tpak.enabled)
		{
            GBCart::read_gb_cart(&tpak.gb_cart, gb_cart_address(tpak.bank, address), data);
		}
	}

}

void Transferpak::WriteTo(uint16_t address, uint8_t * data)
{

    if ((address >= 0x8000) && (address <= 0x8FFF))
    {
        //Ensure we actually have a ROM loaded in first.
        if (tpak.gb_cart.rom == NULL)
        {
            Init();
        }

        //Set whether the gb cart is enabled or disabled.
        switch (*data)
        {
        case 0xFE:
            tpak.enabled = false;
            break;
        case 0x84:
            tpak.enabled = true;
            break;
        default:
            //Do nothing
            break;
        }
    }
     else if ((address >= 0xA000) && (address <= 0xAFFF))
    {
        //Set the bank for the GB Cart
        if (tpak.enabled)
        {
            tpak.bank = *data;
        }
    }
    else if ((address >= 0xB000) && (address <= 0xBFFF))
    {
        // Get the GB Cart access mode
        if (tpak.enabled)
        {
            tpak.access_mode_changed = 0x04;

            tpak.access_mode = ((*data & 1) == 0) ? CART_ACCESS_MODE_0 : CART_ACCESS_MODE_1;

            if ((*data & 0xFE) != 0)
            {
                //Unkown tpak write
            }
        }
    }
    else if (address >= 0xC000)
    {
        // Write to the GB Cart
        if (tpak.enabled)
        {
            GBCart::write_gb_cart(&tpak.gb_cart, gb_cart_address(tpak.bank, address), data);
            GBCart::save_gb_cart(&tpak.gb_cart);
        }
    }
}