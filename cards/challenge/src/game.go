package main

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"golang.org/x/crypto/nacl/secretbox"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
)

type Hand []Card

type Game struct {
	Deck   *Deck
	Player Hand
	Dealer Hand
	Stood  bool
	Bet    int
}

type Session struct {
	Game    *Game
	Balance int
	Bet     int
}

// Each hand
type HandState int

// Current game
type GameState int

// Overall session
type SessionState int

const (
	HandUnder = HandState(iota)
	HandBust
	HandBlackjack
)

const (
	GamePlaying = GameState(iota)
	DealerWins
	PlayerWins
	PlayerWinsBlackjack
	Push
	Idle
)

const (
	SessionPlaying = SessionState(iota)
	SessionBroke
	SessionWon
)

const (
	AmountPlayerStarts = 1000
	AmountPlayerWins   = 100000
	MinBet             = 10
	MaxBet             = 500
)

var (
	secretKey  *[32]byte
	secretFlag *string
)

func (h *Hand) Cards() []Card {
	return []Card(*h)
}

func (h *Hand) String() string {
	elements := make([]string, 0, len(h.Cards()))
	for _, c := range h.Cards() {
		elements = append(elements, c.String())
	}
	return strings.Join(elements, ", ")
}

func (h *Hand) Value() int {
	aces := 0
	value := 0
	for _, c := range h.Cards() {
		if c.Rank == RankAce {
			aces++
		}
		value += c.Value()
	}
	for value <= 11 && aces > 0 {
		value += 10
		aces--
	}
	return value
}

func (h *Hand) State() HandState {
	if h.Value() == 21 && len(h.Cards()) == 2 {
		return HandBlackjack
	}
	if h.Value() > 21 {
		return HandBust
	}
	return HandUnder
}

func NewGame() *Game {
	g := &Game{
		Deck:   NewDeck(),
		Player: Hand(make([]Card, 0)),
		Dealer: Hand(make([]Card, 0)),
	}
	return g
}

func (g *Game) String() string {
	return fmt.Sprintf("Deck: %s\n\tPlayer: %s\n\tDealer: %s\n\tStood: %s",
		g.Deck.String(), g.Player.String(), g.Dealer.String(), strconv.FormatBool(g.Stood))
}

// Initial deal
func (g *Game) Deal() {
	g.Player = Hand(make([]Card, 0))
	g.Dealer = Hand(make([]Card, 0))
	g.Deck.Shuffle()
	for i := 0; i < 2; i++ {
		g.Player = Hand(append(g.Player.Cards(), g.Deck.Next()))
		g.Dealer = Hand(append(g.Dealer.Cards(), g.Deck.Next()))
	}
}

// Player hit
func (g *Game) Hit() {
	g.Player = Hand(append(g.Player.Cards(), g.Deck.Next()))
	if g.Player.Value() >= 21 {
		g.Stand()
	}
}

// Player stand
func (g *Game) Stand() {
	if g.Stood {
		return
	}
	g.Stood = true
	g.DealerPlay()
}

func (g *Game) DealerPlay() {
	if g.Player.State() == HandBust {
		return
	}
	soft17 := func() bool {
		hasAce := false
		for _, c := range g.Dealer.Cards() {
			if c.Rank == RankAce {
				hasAce = true
			}
		}
		return hasAce && g.Dealer.Value() == 17
	}
	for g.Dealer.Value() < 17 || soft17() {
		g.Dealer = Hand(append(g.Dealer.Cards(), g.Deck.Next()))
	}
}

func NewSession() *Session {
	return &Session{
		Game:    NewGame(),
		Balance: AmountPlayerStarts,
	}
}

// Serialization magic!

type secretState struct {
	Game    *Game
	Balance int
}

type secretStateWrapper secretState

func (ss *secretStateWrapper) MarshalText() ([]byte, error) {
	buf, err := json.Marshal((*secretState)(ss))
	if err != nil {
		return nil, err
	}
	var nonce [24]byte
	if _, err := rand.Read(nonce[:]); err != nil {
		return nil, err
	}
	ct := make([]byte, 0, len(buf)+secretbox.Overhead)
	ct = secretbox.Seal(ct, buf, &nonce, getSecretKey())
	ct = append(nonce[:], ct...)
	outbuf := make([]byte, hex.EncodedLen(len(ct)))
	hex.Encode(outbuf, ct)
	return outbuf, nil
}

func (ss *secretStateWrapper) UnmarshalText(inbuf []byte) error {
	buf := make([]byte, hex.DecodedLen(len(inbuf)))
	if _, err := hex.Decode(buf, inbuf); err != nil {
		return err
	}
	var nonce [24]byte
	copy(nonce[:], buf[:24])
	buf = buf[24:]
	out := make([]byte, 0, len(buf)-secretbox.Overhead)
	if out, ok := secretbox.Open(out, buf, &nonce, getSecretKey()); !ok {
		return fmt.Errorf("Invalid state!")
	} else {
		if err := json.Unmarshal(out, (*secretState)(ss)); err != nil {
			return err
		}
	}
	return nil
}

type jsonSession struct {
	SecretState  *secretStateWrapper
	PlayerHand   [][]string
	DealerHand   [][]string
	Balance      int
	GameState    string
	SessionState string
	Bet          int
	Flag         string `json:",omitempty"`
}

func (s *Session) MarshalJSON() ([]byte, error) {
	jss := &jsonSession{
		SecretState: &secretStateWrapper{
			Game:    s.Game,
			Balance: s.Balance,
		},
		Balance:      s.Balance,
		GameState:    s.Game.State().String(),
		SessionState: s.State().String(),
		PlayerHand:   make([][]string, 0, len(s.Game.Player.Cards())),
		DealerHand:   make([][]string, 0, len(s.Game.Dealer.Cards())),
		Bet:          s.Bet,
	}
	// Fill in hands
	for _, c := range s.Game.Player.Cards() {
		jss.PlayerHand = append(jss.PlayerHand, []string{c.Rank.String(), c.Suit.String()})
	}
	for i, c := range s.Game.Dealer.Cards() {
		if i == 0 && s.Game.State() == GamePlaying {
			// Blind first dealer card
			jss.DealerHand = append(jss.DealerHand, []string{"X", "X"})
		} else {
			jss.DealerHand = append(jss.DealerHand, []string{c.Rank.String(), c.Suit.String()})
		}
	}
	if s.Balance > AmountPlayerWins {
		jss.Flag = getFlag()
	}
	return json.Marshal(jss)
}

func (s *Session) UnmarshalJSON(buf []byte) error {
	jss := &jsonSession{}
	if err := json.Unmarshal(buf, jss); err != nil {
		return err
	}
	s.Game = jss.SecretState.Game
	s.Balance = jss.SecretState.Balance
	s.Bet = jss.Bet
	return nil
}

func (s *Session) Deal() error {
	if s.Bet > s.Balance {
		return fmt.Errorf("Invalid bet: not enough chips!")
	}
	if s.Bet > MaxBet || s.Bet < MinBet {
		return fmt.Errorf("Invalid bet!")
	}
	s.Game.Stood = false
	s.Game.Deal()
	s.Game.Bet = s.Bet
	s.Balance -= s.Bet
	// In case of BJ
	s.Score()
	return nil
}

func (s *Session) Double() error {
	if s.Game.State() != GamePlaying {
		return fmt.Errorf("Not playing!")
	}
	if s.Game.Bet > s.Balance {
		return fmt.Errorf("Invalid bet: not enough chips!")
	}
	if len(s.Game.Player.Cards()) != 2 {
		return fmt.Errorf("Can't double after first cards.")
	}
	s.Balance -= s.Game.Bet
	s.Game.Bet *= 2
	s.Game.Hit()
	s.Game.Stand()
	return nil
}

func (s *Session) Hit() error {
	if s.Game.State() != GamePlaying {
		return fmt.Errorf("Not playing!")
	}
	s.Game.Hit()
	return s.Score()
}

func (s *Session) Stand() error {
	if s.Game.State() != GamePlaying {
		return fmt.Errorf("Not playing!")
	}
	s.Game.Stand()
	return s.Score()
}

func (s *Session) Score() error {
	switch s.Game.State() {
	case Idle:
		return nil
	case GamePlaying:
		return nil
	case PlayerWinsBlackjack:
		// Original bet + 3/2
		s.Balance += (s.Game.Bet * 5 / 2)
		return nil
	case PlayerWins:
		// Original bet + win
		s.Balance += (s.Game.Bet * 2)
		return nil
	case DealerWins:
		return nil
	case Push:
		s.Balance += s.Game.Bet
		return nil
	default:
		return fmt.Errorf("Unknown scoring!")
	}
}

func (g *Game) State() GameState {
	switch {
	case len(g.Player.Cards()) == 0:
		return Idle
	case g.Player.State() == HandBust:
		return DealerWins
	case g.Player.State() == HandBlackjack && g.Dealer.State() == HandBlackjack:
		return Push
	case g.Player.State() == HandBlackjack:
		return PlayerWinsBlackjack
	case g.Dealer.State() == HandBlackjack:
		return DealerWins
	case g.Dealer.State() == HandBust:
		return PlayerWins
	case g.Stood && g.Player.Value() == g.Dealer.Value():
		return Push
	case g.Stood && g.Player.Value() > g.Dealer.Value():
		return PlayerWins
	case g.Stood:
		return DealerWins
	default:
		return GamePlaying
	}
}

func (s *Session) State() SessionState {
	switch {
	case s.Balance < 0:
		return SessionBroke
	case s.Balance > AmountPlayerWins:
		return SessionWon
	default:
		return SessionPlaying
	}
}

func (s GameState) String() string {
	switch s {
	case Idle:
		return "Idle"
	case DealerWins:
		return "DealerWins"
	case PlayerWins:
		return "PlayerWins"
	case Push:
		return "Push"
	case PlayerWinsBlackjack:
		return "Blackjack"
	case GamePlaying:
		return "Playing"
	default:
		return "UNKNOWN"
	}
}

func (s GameState) MarshalText() ([]byte, error) {
	return []byte(s.String()), nil
}

func (s SessionState) String() string {
	switch s {
	case SessionBroke:
		return "Broke"
	case SessionWon:
		return "Won"
	default:
		return "Playing"
	}
}

func (s SessionState) MarshalText() ([]byte, error) {
	return []byte(s.String()), nil
}

func mainTest() {
	s := NewSession()
	buf, _ := json.Marshal(s)
	fmt.Printf("%s\n", buf)
	fmt.Printf("%v\n", s)
	s2 := &Session{}
	if err := json.Unmarshal(buf, s2); err != nil {
		fmt.Printf("Unmarshal error: %s\n", err)
	}
	fmt.Printf("%v\n", s2)
}

// Get the secret key, either in memory or load from file
func getSecretKey() *[32]byte {
	if secretKey != nil {
		return secretKey
	}
	if fp, err := os.Open("secret.key"); err != nil {
		panic(err)
	} else {
		if buf, err := ioutil.ReadAll(fp); err != nil {
			panic(err)
		} else {
			if len(buf) < 32 {
				panic("Not enough entropy!")
			}
			secretKey = &[32]byte{}
			copy((*secretKey)[:], buf[:32])
		}
		fp.Close()
	}
	return secretKey
}

// Get the secret flag, either in memory or load from file
func getFlag() string {
	if secretFlag != nil {
		return *secretFlag
	}
	if fp, err := os.Open("flag.txt"); err != nil {
		panic(err)
	} else {
		if buf, err := ioutil.ReadAll(fp); err != nil {
			panic(err)
		} else {
			flag := strings.TrimSpace(string(buf))
			secretFlag = &flag
		}
	}
	return *secretFlag
}
