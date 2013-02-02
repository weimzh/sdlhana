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

#ifndef PLAYER_H
#define PLAYER_H

typedef struct result_s
{
   int        score;

   char       five_lights;
   char       four_lights;
   char       rain_four_lights;
   char       three_lights;
   char       rain_three_lights; // for Korean mode

   char       red_ribbons;
   char       blue_ribbons;
   char       normal_ribbons; // for Korean mode
   char       ribbons;

   char       boar_deer_butterfly; // for Japanese mode
   char       animals;
   char       five_birds; // for Korean mode

   char       flower_meets_sakecup; // for Japanese mode
   char       moon_meets_sakecup; // for Japanese mode

   char       cards;

   char       dealer;
} result_t;

class CBasePlayer
{
public:
   CBasePlayer();
   virtual ~CBasePlayer();

   virtual bool         IsBot() = 0;

   virtual void         NewRound();
   virtual int          SelectCard() = 0;
   virtual bool         WantToContinue() = 0;
   virtual int          SelectCardOnDesk(int month, const CCard &c) = 0;

   void                 DeleteCard(int index);
   void                 AddCapturedCard(const CCard &c);
   void                 DeleteCapturedCard(int index);

   void                 DrawHand();
   void                 DrawCaptured();

   void                 CalcResult();
   void                 CalcAddResult();
   void                 DrawCurResult();
   void                 DrawAllResult();

   result_t             m_Result;

   inline CBasePlayer  *GetOpponent()        const  { return m_pOpponent; }
   inline void          SetOpponent(CBasePlayer *p) { m_pOpponent = p; }

   inline int           GetNumHandCard()     const  { return m_iNumHandCard; }
   inline CCard         GetHandCard(int i)   const  { assert(i >= 0 && i < m_iNumHandCard); return m_HandCards[i]; }
   inline int           GetNumCapturedCard() const  { return m_iNumCapturedCard; }
   inline CCard         GetCapturedCard(int i)const { assert(i >= 0 && i < m_iNumCapturedCard); return m_CapturedCard[i]; }
   bool                 HasCaptured(const CCard &c) const;

   inline bool          IsDealer()     const   { return (this == m_pDealer); }
   inline void          SetAsDealer()          { m_pDealer = this; }

   inline void          ShiftResult()          { m_PrevResult = m_Result; }
   inline int           GetPrevScore() const   { return m_PrevResult.score; }

   static CBasePlayer  *m_pDealer;
   static int           m_iMaxHandCards;

   static CBasePlayer  *GetDealer()            { return m_pDealer; }

   int                  m_iNumContinue;
   int                  m_iNumLeaveThree;

protected:
   int               m_iNumHandCard;  // number of cards in hand
   CCard             m_HandCards[8];  // cards in hand

   int               m_iNumCapturedCard;   // number of captured cards
   CCard             m_CapturedCard[48];   // captured cards

   result_t          m_PrevResult;         // result before declaring Koi-Koi

   CBasePlayer      *m_pOpponent;
};

class CPlayer : public CBasePlayer
{
public:
   CPlayer();
   virtual ~CPlayer();

   bool              IsBot()               { return false; }
   int               SelectCard();
   bool              WantToContinue();
   int               SelectCardOnDesk(int month, const CCard &c);
};

#endif

