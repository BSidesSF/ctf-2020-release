package main

import (
	"crypto/rand"
	"fmt"
	"math/big"
	"strings"
)

type Suit uint8
type Rank uint8

const (
	SuitSpades = Suit(iota)
	SuitClubs
	SuitDiamonds
	SuitHearts
	SuitsEnd
)

const (
	RankAce = Rank(iota + 1)
	Rank2
	Rank3
	Rank4
	Rank5
	Rank6
	Rank7
	Rank8
	Rank9
	Rank10
	RankJ
	RankQ
	RankK
	RanksEnd
)

var sessions uint32

type Card struct {
	Suit
	Rank
}

type Deck struct {
	Cards [52]Card
	Pos   uint8
}

func NewDeck() *Deck {
	d := &Deck{}
	// Allocate cards
	for i := 0; i < 52; i++ {
		d.Cards[i].Suit = Suit(i / 13)
		d.Cards[i].Rank = Rank(i%13 + 1)
	}
	return d
}

func (d *Deck) Shuffle() {
	for i := range d.Cards {
		max := big.NewInt(int64(len(d.Cards) - i))
		r, err := rand.Int(rand.Reader, max)
		if err != nil {
			panic(err)
		}
		p := i + int(r.Int64())
		d.Cards[i], d.Cards[p] = d.Cards[p], d.Cards[i]
	}
	d.Pos = 0
}

func (d *Deck) Next() Card {
	c := d.Cards[d.Pos]
	d.Pos++
	if int(d.Pos) >= len(d.Cards) {
		d.Shuffle()
	}
	return c
}

func (d *Deck) String() string {
	cn := make([]string, 0, len(d.Cards))
	for _, c := range d.Cards {
		cn = append(cn, c.String())
	}
	return strings.Join(cn, "\n")
}

func (d *Deck) Left() int {
	return len(d.Cards) - int(d.Pos)
}

func (c Card) String() string {
	return c.Rank.String() + " of " + c.Suit.String()
}

func (c Card) Value() int {
	switch c.Rank {
	case RankAce:
		return 1 // Can be 11, handled elsewhere
	case RankJ, RankQ, RankK:
		return 10
	default:
		return int(c.Rank)
	}
}

func (s Suit) String() string {
	switch s {
	case SuitSpades:
		return "Spades"
	case SuitClubs:
		return "Clubs"
	case SuitDiamonds:
		return "Diamonds"
	case SuitHearts:
		return "Hearts"
	default:
		return "Unknown"
	}
}

func (r Rank) String() string {
	switch r {
	case RankAce:
		return "Ace"
	case RankJ:
		return "Jack"
	case RankQ:
		return "Queen"
	case RankK:
		return "King"
	default:
		return fmt.Sprintf("%d", uint8(r))
	}
}
