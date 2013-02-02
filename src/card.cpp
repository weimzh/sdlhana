//
// Copyright (c) 2005, 2006 Wei Mingzhi <whistler@openoffice.org>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA
//

#include "main.h"

unsigned char CCard::m_ucCardFlags[6];

CCard::CCard():
m_iRenderEffect(0),
m_iValue(255)
{
}

CCard::CCard(unsigned char value):
m_iRenderEffect(0),
m_iValue(value)
{
}

CCard::~CCard()
{
}

int CCard::GetType() const
{
   unsigned char lights[] = {0, 8, 28, 40, 44, 255};
   unsigned char animals[] = {4, 12, 16, 20, 24, 29, 32, 36, 41, 255};
   unsigned char ribbons[] = {13, 17, 25, 42, 255};
   unsigned char redribbons[] = {1, 5, 9, 255};
   unsigned char blueribbons[] = {21, 33, 37, 255};

   unsigned char *p = lights;
   while (*p != 255) {
      if (m_iValue == *p) {
         return CARD_LIGHT;
      }
      p++;
   }

   p = animals;
   while (*p != 255) {
      if (m_iValue == *p) {
         return CARD_ANIMAL;
      }
      p++;
   }

   p = ribbons;
   while (*p != 255) {
      if (m_iValue == *p) {
         return CARD_RIBBON;
      }
      p++;
   }

   p = redribbons;
   while (*p != 255) {
      if (m_iValue == *p) {
         return CARD_RIBBON_RED;
      }
      p++;
   }

   p = blueribbons;
   while (*p != 255) {
      if (m_iValue == *p) {
         return CARD_RIBBON_BLUE;
      }
      p++;
   }

   return CARD_NONE;
}

void CCard::NewRound()
{
   memset(m_ucCardFlags, 0, sizeof(m_ucCardFlags));
}

CCard CCard::GetRandomCard()
{
   unsigned char a[48], i;
   char count = 0;

   for (i = 0; i < 48; i++) {
      if (!(m_ucCardFlags[i / 8] & (1 << (i & 7)))) {
         a[count++] = i;
      }
   }

   count--;
   if (count < 0) {
      TerminateOnError("CCard::GetRandomCard(): No cards left!");
   }

   i = RandomLong(0, count);
   m_ucCardFlags[a[i] / 8] |= (1 << (a[i] & 7));

   return CCard(a[i]);
}

void CCard::PutBackToPile(const CCard &c)
{
   assert(c.IsValid());
   unsigned char value = c.GetValue();
   m_ucCardFlags[value / 8] &= ~(1 << (value & 7));
}

