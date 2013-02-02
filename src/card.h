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

#ifndef CARD_H
#define CARD_H

enum
{
   CARD_NONE = 0,
   CARD_ANIMAL,
   CARD_RIBBON,
   CARD_RIBBON_RED,
   CARD_RIBBON_BLUE,
   CARD_LIGHT,
};

enum
{
   EF_NONE      = 0,
   EF_DARK      = (1 << 0),
   EF_BOX       = (1 << 1),
};

class CCard
{
public:
   CCard();
   CCard(unsigned char value);
   ~CCard();

   inline int        GetValue() const    { return m_iValue; }
   inline int        GetMonth() const    { return m_iValue / 4 + 1; }

   inline bool operator==(const CCard &c) const { return GetMonth() == c.GetMonth(); }
   inline bool operator!=(const CCard &c) const { return GetMonth() != c.GetMonth(); }

   int               GetType()   const;
   inline int        GetOrder()  const   { int t = GetType(); return ((t == CARD_ANIMAL) ? 4 : ((t >= CARD_RIBBON && t <= CARD_RIBBON_BLUE) ? t - 1 : t)); }
   inline bool       IsRain()    const   { return (m_iValue == 40); }
   inline bool       IsSakeCup() const   { return (m_iValue == 32); }
   inline bool       IsBoar()    const   { return (m_iValue == 24); }
   inline bool       IsDeer()    const   { return (m_iValue == 36); }
   inline bool       IsButterfly() const { return (m_iValue == 20); }
   inline bool       IsMoon()    const   { return (m_iValue == 28); }
   inline bool       IsFlower()  const   { return (m_iValue == 8); }
   inline bool       IsBird()    const   { return (m_iValue == 4 || m_iValue == 12 || m_iValue == 29); }

   inline bool       IsValid()   const   { return (m_iValue < 48); }
   inline void       Destroy()           { m_iValue = 255; }

   static void       NewRound();
   static CCard      GetRandomCard();
   static void       PutBackToPile(const CCard &c);

   static unsigned char   m_ucCardFlags[6];

   inline CCard      Next()      const   { if (m_iValue >= 47) return CCard(0); return CCard(m_iValue + 1); }
   inline CCard      Prev()      const   { if (m_iValue <= 1) return CCard(47); return CCard(m_iValue - 1); }

   unsigned int      m_iRenderEffect;

private:
   unsigned char          m_iValue;
};

#endif

