#include <iostream>
#include <stdio.h>
#include <string>

#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

int WIDTH = 1920;
int HEIGHT = 1080;

int CARDS_IN_ROW = 12;

const float CARD_SCALE = 0.3f;

const std::string server_ip("34.72.173.188");
//const std::string server_ip("35.223.150.31");
RakNet::AddressOrGUID ServerAddress;

const int window_flags = ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoScrollbar;

char name[256] = "";

std::vector<std::string> connected_players;
int player_count;

enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_REQUEST_NAME,
	ID_SEND_NAME,
	ID_GAME_REQUEST_CARD,
	ID_GAME_SEND_CARD,
	ID_LOBBY_NUM_PLAYERS,
	ID_LOBBY_PLAYERS,
	ID_START_GAME,
	ID_DEAL_CARD
};

enum Suit
{
	Hearts = 0,
	Clubs = 1,
	Diamonds = 2,
	Spades = 3
};

enum Rank
{
	Leader = 0,
	Joker = 1,
	Two = 2,
	Three = 3,
	Four = 4,
	Five = 5,
	Six = 6,
	Seven = 7,
	Eight = 8,
	Nine = 9,
	Ten = 10,
	Jack = 11,
	Queen = 12,
	King = 13,
	Ace = 14
};

struct Card
{
	sf::Texture texture;
	sf::Sprite sprite;
	Suit suit;
	Rank rank;
	int value;
	bool selected;
};

Card* cards[4][15];

enum class State
{
	EnterName = 0,
	Connecting = 1,
	Lobby = 2,
	Playing = 3,
	Exit = 4
};

void InitCard(Card* card, std::string filepath, Suit suit, Rank rank)
{
	card->texture.loadFromFile(std::string("resources/cards/" + filepath).c_str());
	card->sprite.setTexture(card->texture);
	card->sprite.setScale(CARD_SCALE, CARD_SCALE);
	card->suit = suit;
	card->rank = rank;
	card->selected = false;
}

void DeleteCards()
{
	for (int suit = 0; suit < 3; ++suit)
	{
		for (int rank = 0; rank < 15; ++rank)
		{
			delete cards[suit][rank];
		}
	}
}

std::vector<Card*> hand;

State state = State::EnterName;

void LoadCardSprites()
{
	for (int suit = 0; suit < 4; ++suit)
	{
		for (int rank = 0; rank < 15; ++rank)
		{
			cards[suit][rank] = new Card();
		}
	}

	Suit hearts = Suit::Hearts;
	Suit clubs = Suit::Clubs;
	Suit diamonds = Suit::Diamonds;
	Suit spades = Suit::Spades;

	InitCard(cards[(int)hearts][(int)Rank::Two], "2_of_hearts.png", hearts, Rank::Two);
	InitCard(cards[(int)hearts][(int)Rank::Three], "3_of_hearts.png", hearts, Rank::Three);
	InitCard(cards[(int)hearts][(int)Rank::Four], "4_of_hearts.png", hearts, Rank::Four);
	InitCard(cards[(int)hearts][(int)Rank::Five], "5_of_hearts.png", hearts, Rank::Five);
	InitCard(cards[(int)hearts][(int)Rank::Six], "6_of_hearts.png", hearts, Rank::Six);
	InitCard(cards[(int)hearts][(int)Rank::Seven], "7_of_hearts.png",hearts, Rank::Seven);
	InitCard(cards[(int)hearts][(int)Rank::Eight], "8_of_hearts.png", hearts, Rank::Eight);
	InitCard(cards[(int)hearts][(int)Rank::Nine], "9_of_hearts.png", hearts, Rank::Nine);
	InitCard(cards[(int)hearts][(int)Rank::Ten], "10_of_hearts.png", hearts, Rank::Ten);
	InitCard(cards[(int)hearts][(int)Rank::Jack], "jack_of_hearts.png", hearts, Rank::Jack);
	InitCard(cards[(int)hearts][(int)Rank::Queen], "queen_of_hearts.png", hearts, Rank::Queen);
	InitCard(cards[(int)hearts][(int)Rank::King], "king_of_hearts.png", hearts, Rank::King);
	InitCard(cards[(int)hearts][(int)Rank::Ace], "ace_of_hearts.png", hearts, Rank::Ace);

	InitCard(cards[(int)clubs][(int)Rank::Leader], "leader.png", clubs, Rank::Leader);
	InitCard(cards[(int)clubs][(int)Rank::Joker], "black_joker.png", clubs, Rank::Joker);
	InitCard(cards[(int)clubs][(int)Rank::Two], "2_of_clubs.png", clubs, Rank::Two);
	InitCard(cards[(int)clubs][(int)Rank::Three], "3_of_clubs.png", clubs, Rank::Three);
	InitCard(cards[(int)clubs][(int)Rank::Four], "4_of_clubs.png", clubs, Rank::Four);
	InitCard(cards[(int)clubs][(int)Rank::Five], "5_of_clubs.png", clubs, Rank::Five);
	InitCard(cards[(int)clubs][(int)Rank::Six], "6_of_clubs.png", clubs, Rank::Six);
	InitCard(cards[(int)clubs][(int)Rank::Seven], "7_of_clubs.png",clubs, Rank::Seven);
	InitCard(cards[(int)clubs][(int)Rank::Eight], "8_of_clubs.png", clubs, Rank::Eight);
	InitCard(cards[(int)clubs][(int)Rank::Nine], "9_of_clubs.png", clubs, Rank::Nine);
	InitCard(cards[(int)clubs][(int)Rank::Ten], "10_of_clubs.png", clubs, Rank::Ten);
	InitCard(cards[(int)clubs][(int)Rank::Jack], "jack_of_clubs.png", clubs, Rank::Jack);
	InitCard(cards[(int)clubs][(int)Rank::Queen], "queen_of_clubs.png", clubs, Rank::Queen);
	InitCard(cards[(int)clubs][(int)Rank::King], "king_of_clubs.png", clubs, Rank::King);
	InitCard(cards[(int)clubs][(int)Rank::Ace], "ace_of_clubs.png", clubs, Rank::Ace);

	InitCard(cards[(int)diamonds][(int)Rank::Two], "2_of_diamonds.png", diamonds, Rank::Two);
	InitCard(cards[(int)diamonds][(int)Rank::Three], "3_of_diamonds.png", diamonds, Rank::Three);
	InitCard(cards[(int)diamonds][(int)Rank::Four], "4_of_diamonds.png", diamonds, Rank::Four);
	InitCard(cards[(int)diamonds][(int)Rank::Five], "5_of_diamonds.png", diamonds, Rank::Five);
	InitCard(cards[(int)diamonds][(int)Rank::Six], "6_of_diamonds.png", diamonds, Rank::Six);
	InitCard(cards[(int)diamonds][(int)Rank::Seven], "7_of_diamonds.png",diamonds, Rank::Seven);
	InitCard(cards[(int)diamonds][(int)Rank::Eight], "8_of_diamonds.png", diamonds, Rank::Eight);
	InitCard(cards[(int)diamonds][(int)Rank::Nine], "9_of_diamonds.png", diamonds, Rank::Nine);
	InitCard(cards[(int)diamonds][(int)Rank::Ten], "10_of_diamonds.png", diamonds, Rank::Ten);
	InitCard(cards[(int)diamonds][(int)Rank::Jack], "jack_of_diamonds.png", diamonds, Rank::Jack);
	InitCard(cards[(int)diamonds][(int)Rank::Queen], "queen_of_diamonds.png", diamonds, Rank::Queen);
	InitCard(cards[(int)diamonds][(int)Rank::King], "king_of_diamonds.png", diamonds, Rank::King);
	InitCard(cards[(int)diamonds][(int)Rank::Ace], "ace_of_diamonds.png", diamonds, Rank::Ace);

	InitCard(cards[(int)spades][(int)Rank::Two], "2_of_spades.png", spades, Rank::Two);
	InitCard(cards[(int)spades][(int)Rank::Three], "3_of_spades.png", spades, Rank::Three);
	InitCard(cards[(int)spades][(int)Rank::Four], "4_of_spades.png", spades, Rank::Four);
	InitCard(cards[(int)spades][(int)Rank::Five], "5_of_spades.png", spades, Rank::Five);
	InitCard(cards[(int)spades][(int)Rank::Six], "6_of_spades.png", spades, Rank::Six);
	InitCard(cards[(int)spades][(int)Rank::Seven], "7_of_spades.png",spades, Rank::Seven);
	InitCard(cards[(int)spades][(int)Rank::Eight], "8_of_spades.png", spades, Rank::Eight);
	InitCard(cards[(int)spades][(int)Rank::Nine], "9_of_spades.png", spades, Rank::Nine);
	InitCard(cards[(int)spades][(int)Rank::Ten], "10_of_spades.png", spades, Rank::Ten);
	InitCard(cards[(int)spades][(int)Rank::Jack], "jack_of_spades.png", spades, Rank::Jack);
	InitCard(cards[(int)spades][(int)Rank::Queen], "queen_of_spades.png", spades, Rank::Queen);
	InitCard(cards[(int)spades][(int)Rank::King], "king_of_spades.png", spades, Rank::King);
	InitCard(cards[(int)spades][(int)Rank::Ace], "ace_of_spades.png", spades, Rank::Ace);
}

void PositionCards()
{
	for (int suit = 0; suit < 4; ++suit)
	{
		for (int rank = 0; rank < 15; ++rank)
		{
			cards[suit][rank]->sprite.setPosition(125.0f * rank, 250.0f * suit);
		}
	}
}

void UpdateHandPosition()
{
	const int num_cards = hand.size();
	int num_rows = num_cards / CARDS_IN_ROW;
	if (num_rows * CARDS_IN_ROW < num_cards)
		++num_rows;

	int current_card = 0;
	int cards_left = num_cards;
	//for (int current_row = num_rows - 1; current_row >= 0; --current_row)
	for (int current_row = 0; current_row < num_rows; ++current_row)
	{
		int num_cards_in_row = cards_left > CARDS_IN_ROW ? CARDS_IN_ROW : cards_left;
		cards_left -= num_cards_in_row;
		for (int i = 0; i < num_cards_in_row; ++i)
		{
			hand[current_card++]->sprite.setPosition(i * 160.0f, HEIGHT - 225.0f - (current_row * 225.0f));
		}
	}
}

void AddCardToHand(Suit suit, Rank rank)
{
	hand.push_back(cards[(int)suit][(int)rank]);
}

void HandlePackets(RakNet::RakPeerInterface* peer)
{
	Card card;

	RakNet::Packet* packet;

	for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
	{
		switch (packet->data[0])
		{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			{
				printf("Another client has disconnected\n");
			} break;

			case ID_REMOTE_CONNECTION_LOST:
			{
				printf("Another client has lost the connection\n");
			} break;

			case ID_REMOTE_NEW_INCOMING_CONNECTION:
			{
				printf("A connection is incoming\n");
			} break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted\n");
				ServerAddress = packet->systemAddress;
			} break;

			case ID_NEW_INCOMING_CONNECTION:
			{
				printf("Another client has connected\n");
			} break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				printf("The server is full\n");
			} break;

			case ID_DISCONNECTION_NOTIFICATION:
			{
				printf("We have been disconnected\n");
				state = State::Exit;
			} break;

			case ID_CONNECTION_LOST:
			{
				printf("Connection lost\n");
				state = State::Exit;
			} break;

			case ID_CONNECTION_ATTEMPT_FAILED:
			{
				printf("Failed to connect to server\n");
				state = State::Exit;
				//return -1;
			} break;

			case ID_GAME_MESSAGE_1:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			} break;

			case ID_REQUEST_NAME:
			{
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_SEND_NAME);
				RakNet::RakString rs = name;
				bsOut.Write(rs);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			} break;

			case ID_GAME_REQUEST_CARD:
			{
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_GAME_SEND_CARD);
				bsOut.Write("Ten of Spades");
				bsOut.Write((int)Ten);
				bsOut.Write((int)Spades);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			} break;

			case ID_GAME_SEND_CARD:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(card.suit);
				bsIn.Read(card.rank);
				printf("Suit: %d\n", (int)card.suit);
				printf("Rank: %d\n\n", (int)card.rank);
			} break;

			case ID_LOBBY_NUM_PLAYERS:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(player_count);
			} break;

			case ID_LOBBY_PLAYERS:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				connected_players.clear();
				for (int i = 0; i < player_count; ++i)
				{
					RakNet::RakString rs;
					bsIn.Read(rs);
					std::string temp = rs.C_String();
					connected_players.push_back(temp);
				}
			} break;

			case ID_START_GAME:
			{
				state = State::Playing;
			} break;

			case ID_DEAL_CARD:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				Card card;
				bsIn.Read(card.suit);
				bsIn.Read(card.rank);
				AddCardToHand(card.suit, card.rank);
			} break;

			default:
			{
				printf("Message with identifier %i has arrived\n", packet->data[0]);
			} break;
		}
	}
}

void StartGame(RakNet::RakPeerInterface* peer)
{
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_START_GAME);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, ServerAddress, false);
}

int main(int argc, char** argv)
{
	std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
	for (std::size_t i = 0; i < modes.size(); ++i)
	{
	    sf::VideoMode mode = modes[i];
	    std::cout << "Mode #" << i << ": "
	              << mode.width << "x" << mode.height << " - "
	              << mode.bitsPerPixel << " bpp" << std::endl;
	}

	sf::RenderWindow window(modes[0], "Hearts", sf::Style::Fullscreen);

	WIDTH = modes[0].width;
	HEIGHT = modes[0].height;

	state = State::Playing;

	if (WIDTH == 3300)
		CARDS_IN_ROW = 20;
	else
		CARDS_IN_ROW = 12;

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();

	LoadCardSprites();
	PositionCards();

	AddCardToHand(Suit::Spades, Rank::Queen);
	AddCardToHand(Suit::Hearts, Rank::Ace);
	AddCardToHand(Suit::Clubs, Rank::Seven);
	AddCardToHand(Suit::Spades, Rank::Five);
	AddCardToHand(Suit::Clubs, Rank::Leader);
	AddCardToHand(Suit::Diamonds, Rank::Jack);

	Card card;
	card.suit = Suit::Spades;
	card.rank = Rank::Jack;

	sf::Font font;
	font.loadFromFile("resources/fonts/LiberationMono-Regular.ttf");

	sf::Text your_hand_text;
	your_hand_text.setFont(font);
	your_hand_text.setString("Your hand:");
	your_hand_text.setCharacterSize(54);
	your_hand_text.setPosition(100, HEIGHT - 600);
	your_hand_text.setFillColor(sf::Color::White);

	sf::RectangleShape hand_divider;
	hand_divider.setSize(sf::Vector2f(WIDTH, 10.0f));
	hand_divider.setPosition(0, HEIGHT - 500.0f);
	hand_divider.setFillColor(sf::Color::Black);

	sf::Texture background_tex;
	if (modes[0].width == 3300)
		background_tex.loadFromFile("resources/textures/card_table_background_3300_2100.jpg");
	else if (modes[0].width == 2560)
		background_tex.loadFromFile("resources/textures/card_table_background_2560_1440.jpg");
	else
		background_tex.loadFromFile("resources/textures/card_table_background_1920_1080.jpg");

	sf::Sprite background;
	background.setTexture(background_tex);
	background.setPosition(0.0f, 0.0f);

/*
	for (int suit = 0; suit < 2; ++suit)
	{
		for (int rank = 2; rank < 15; ++rank)
		{
			AddCardToHand((Suit)suit, (Rank)rank);
		}
	}
*/

	UpdateHandPosition();

	ImGui::SFML::Init(window);

	sf::Clock clock;
	while (window.isOpen())
	{
		if (state >= State::Connecting)
		{
			HandlePackets(peer);
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_DISCONNECTION_NOTIFICATION);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, ServerAddress, false);
					window.close();
				}
				else if (event.key.code == sf::Keyboard::P)
				{
					hand.pop_back();
				}
				else if (event.key.code == sf::Keyboard::L)
				{
					for (int i = hand.size() - 1; i >= 0; --i)
					{
						if (hand[i]->selected)
						{
							hand.erase(hand.begin() + i);
						}
					}
				}
			}

			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{
				sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
				for (auto card : hand)
				{
					if (card->sprite.getGlobalBounds().contains((float)mouse_pos.x, (float)mouse_pos.y))
					{
						card->selected = !card->selected;
						if (card->selected)
							card->sprite.setColor(sf::Color(200, 200, 200, 255));
						else
							card->sprite.setColor(sf::Color(255, 255, 255, 255));
						break;
					}
				}
			}
		}

		UpdateHandPosition();

		if (state == State::Connecting)
		{
			static bool init = true;
			if (init)
			{
				init = false;
				RakNet::SocketDescriptor sd;
				peer->Startup(1, &sd, 1);

				printf("Starting the client.\n");
				peer->Connect(server_ip.c_str(), SERVER_PORT, 0, 0);
				state = State::Lobby;
			}
		}

		ImGui::SFML::Update(window, clock.restart());

		if (state == State::EnterName)
		{
			ImGui::SetNextWindowSize(ImVec2(400, 200));
			ImGui::Begin("Window", NULL, ImGuiWindowFlags_NoTitleBar |
							ImGuiWindowFlags_NoMove |
							ImGuiWindowFlags_NoResize |
							ImGuiWindowFlags_NoBackground |
							ImGuiWindowFlags_NoScrollbar);


			ImGui::InputTextWithHint("", "Enter Name", name, IM_ARRAYSIZE(name));

			if (ImGui::Button("Connect"))
			{
				state = State::Connecting;
			}
			ImGui::End();
		}

		if (state == State::Lobby)
		{
			ImGui::SetNextWindowSize(ImVec2(400, 1000));
			ImGui::Begin("Window", NULL, window_flags);
			ImGui::Text("Users in Lobby");
			for (int i = 0; i < connected_players.size(); ++i)
			{
				ImGui::Text("%s", connected_players[i].c_str());
			}
			if (ImGui::Button("Start"))
			{
				StartGame(peer);
			}
			ImGui::End();
		}

		window.clear(sf::Color(120, 120, 120, 255));
		window.draw(background);
		ImGui::SFML::Render(window);

		if (state == State::Playing)
		{
			for (auto card : hand)
				window.draw(card->sprite);
			window.draw(your_hand_text);

			window.draw(hand_divider);
		}

		window.display();

		if (state == State::Exit)
		{
			break;
		}
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);

	ImGui::SFML::Shutdown();

	DeleteCards();

	return 0;
}
